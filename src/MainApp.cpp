#include "MainApp.hpp"
#include <esp_log.h>

static const char* TAG = "MainApp";

// ============================================================
//  INIT
//  Appelé une seule fois depuis setup().
//  Initialise les bus, les drivers, la PSRAM et les services.
// ============================================================
Status MainApp::init() {
    ESP_LOGI(TAG, "=== YAMP — demarrage ===");

    // --- Bus I2C (Wire.h) — utilisé par INA237 ---------------
    // Wire.begin() appelé ici, les drivers utilisent Wire directement
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    Wire.setClock(I2C_FREQ_HZ);
    ESP_LOGI(TAG, "Bus I2C demarre (SDA=%d SCL=%d @ %u Hz)",
             PIN_I2C_SDA, PIN_I2C_SCL, I2C_FREQ_HZ);

    // --- IHM -------------------------------------------------
    m_led.init();
    m_buzzer.init();
    m_button.init();

    // --- PSRAM -----------------------------------------------
    if (!PSRAMManager::isAvailable()) {
        ESP_LOGW(TAG, "PSRAM non detectee — log desactive");
    } else {
        PSRAMManager::logInfo();
        if (!m_ringBuffer.init()) {
            ESP_LOGW(TAG, "Ring buffer non initialise");
        }
    }

    // --- Config (flash NVS + seuils) -------------------------
    if (m_config.init() != Status::OK) {
        ESP_LOGW(TAG, "Echec lecture flash — seuils par defaut");
    }

    // --- Capteurs (INA237 via Wire, TMP126 via SPI) ----------
    Status stMonitor = m_monitoring.init();
    if (stMonitor != Status::OK) {
        ESP_LOGE(TAG, "Echec init capteurs → etat CRITICAL");
        m_state = AgvState::CRITICAL;
        updateHmi();
        return Status::ERROR;
    }

    // --- BLE -------------------------------------------------
    m_ble.init();

    // --- Etat initial ----------------------------------------
    m_state = AgvState::NORMAL;
    m_buzzer.setPattern(BuzzerPattern::SUCCESS_BEEP);
    updateHmi();

    ESP_LOGI(TAG, "Init OK → etat NORMAL");
    return Status::OK;
}

// ============================================================
//  PROCESS — appelé dans loop()
//  Cadencé à APP_LOOP_PERIOD_MS sans delay().
// ============================================================
void MainApp::process() {
    uint32_t now = millis();
    if (now - m_lastLoopMs < APP_LOOP_PERIOD_MS) {
        m_buzzer.update(); // toujours mis à jour (pattern non bloquant)
        return;
    }
    m_lastLoopMs = now;

    bool bootPressed = m_button.isBootPressed();

    // ---- Machine à états ------------------------------------
    switch (m_state) {

        case AgvState::INIT:
            // Ne devrait pas arriver ici (init() gère la transition)
            if (init() != Status::OK) {
                m_state = AgvState::CRITICAL;
            }
            break;

        case AgvState::NORMAL:
        {
            m_meas = m_monitoring.acquire();
            AgvState next = m_alarm.evaluate(m_state, m_meas);
            if (next != m_state) {
                m_eventLog.log(next, m_meas, stateToStr(next));
                m_state = next;
            }
            if (bootPressed) { m_state = AgvState::INIT; }
            break;
        }

        case AgvState::WARNING:
        {
            m_meas = m_monitoring.acquire();
            AgvState next = m_alarm.evaluate(m_state, m_meas);
            if (next != m_state) {
                m_eventLog.log(next, m_meas, stateToStr(next));
                m_state = next;
            }
            if (bootPressed) { m_state = AgvState::INIT; }
            break;
        }

        case AgvState::CRITICAL:
            triggerSafetyShutdown();
            m_eventLog.log(AgvState::SAFE_SHUTDOWN, m_meas, "Safety shutdown");
            m_state = AgvState::SAFE_SHUTDOWN;
            break;

        case AgvState::SAFE_SHUTDOWN:
            // Figé jusqu'au reset manuel
            if (bootPressed) {
                ESP_LOGI(TAG, "Reset manuel → retour INIT");
                m_buzzer.stop();
                m_state = AgvState::INIT;
                (void)init();
            }
            break;

        default:
            // MISRA : défense contre corruption mémoire de l'état
            ESP_LOGE(TAG, "Etat FSM inconnu (%d) → CRITICAL",
                     static_cast<int>(m_state));
            m_state = AgvState::CRITICAL;
            break;
    }

    // --- Mise à jour IHM + BLE après transition d'état -------
    updateHmi();
    m_ble.notify(m_meas.temperature_degC, m_meas.current_A,
                 m_meas.voltage_V, m_state);

    m_buzzer.update();
}

// ============================================================
//  UPDATE HMI — LED + Buzzer selon l'état courant
// ============================================================
void MainApp::updateHmi() {
    switch (m_state) {
        case AgvState::INIT:
            m_led.setOff();
            break;

        case AgvState::NORMAL:
            m_led.setGreen();
            // buzzer OFF (SUCCESS_BEEP se termine seul via update())
            break;

        case AgvState::WARNING:
            m_led.setOrange();           // rouge + verte = orange
            m_buzzer.setPattern(BuzzerPattern::WARNING_SLOW);
            break;

        case AgvState::CRITICAL:
            m_led.setRed();
            m_buzzer.setPattern(BuzzerPattern::CRITICAL_START);
            break;

        case AgvState::SAFE_SHUTDOWN:
            m_led.setRed();
            // buzzer continue (CRITICAL_BEEP géré automatiquement)
            break;

        default:
            m_led.setRed();
            m_buzzer.setPattern(BuzzerPattern::CRITICAL_START);
            break;
    }
}

// ============================================================
//  SAFETY SHUTDOWN
//  Point d'extension : couper le relais de puissance ici.
// ============================================================
void MainApp::triggerSafetyShutdown() {
    ESP_LOGE(TAG, "SAFETY SHUTDOWN déclenché !");
    // Exemple : digitalWrite(PIN_RELAY_POWER, LOW);
    // (ajouter la broche dans HardwareConfig.hpp si nécessaire)
}

const char* MainApp::stateToStr(AgvState s) const {
    switch (s) {
        case AgvState::INIT:          return "INIT";
        case AgvState::NORMAL:        return "NORMAL";
        case AgvState::WARNING:       return "WARNING";
        case AgvState::CRITICAL:      return "CRITICAL";
        case AgvState::SAFE_SHUTDOWN: return "SAFE_SHUTDOWN";
        default:                      return "UNKNOWN";
    }
}
