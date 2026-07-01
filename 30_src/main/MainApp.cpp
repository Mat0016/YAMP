#include "MainApp.hpp"
#include "../../config/HardwareConfig.hpp"
#include "../../config/AppConfig.hpp"
#include "../../config/SystemConfig.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "MainApp";

// =============================================================================
// MainApp.cpp — Boucle principale VentecMonitoring
// =============================================================================

MainApp::MainApp()
    // --- LOW : Mémoire ---
    : m_psram()
    , m_flash()
    // --- MID : Communication ---
    , m_i2c()
    , m_ble()
    // --- LOW : Capteurs HAL ---
    , m_ina237(m_i2c,
               HardwareConfig::INA237_ADDR,
               HardwareConfig::INA237_SHUNT_OHM,
               HardwareConfig::INA237_MAX_A)
    , m_tmp126()
    // --- LOW : IHM HAL ---
    , m_led   (static_cast<gpio_num_t>(HardwareConfig::LED_GREEN_PIN),
               static_cast<gpio_num_t>(HardwareConfig::LED_RED_PIN))
    , m_buzzer(HardwareConfig::BUZZER_PIN)
    , m_button(static_cast<gpio_num_t>(HardwareConfig::BUTTON_PIN))
    // --- APP : Services ---
    , m_monitoring(m_ina237, m_tmp126)
    , m_alarm()
    , m_logging(m_psram)
    , m_config(m_flash)
{}

// =============================================================================
// INIT — Séquence d'initialisation complète
// =============================================================================

Status MainApp::init() {
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, " %s v%s — INIT", SystemConfig::APP_NAME, SystemConfig::APP_VERSION);
    ESP_LOGI(TAG, "========================================");

    bool anyError = false;

    // -------------------------------------------------------------------------
    // [1] PSRAM — doit être le premier (buffer logs)
    // -------------------------------------------------------------------------
    ESP_LOGI(TAG, "[1/9] Init PSRAM...");
    if (m_psram.init() != Status::OK) {
        ESP_LOGE(TAG, "  PSRAM FAILED");
        anyError = true;
    }

    // -------------------------------------------------------------------------
    // [2] Flash NVS — configuration persistante
    // -------------------------------------------------------------------------
    ESP_LOGI(TAG, "[2/9] Init Flash NVS...");
    if (m_flash.init() != Status::OK) {
        ESP_LOGW(TAG, "  Flash FAILED — config par défaut");
    }

    // -------------------------------------------------------------------------
    // [3] Config service — charge les seuils depuis Flash
    // -------------------------------------------------------------------------
    ESP_LOGI(TAG, "[3/9] Init ConfigService...");
    (void)m_config.init();

    // -------------------------------------------------------------------------
    // [4] IHM — LED et Buzzer (initialisés avant les capteurs pour feedback)
    // -------------------------------------------------------------------------
    ESP_LOGI(TAG, "[4/9] Init IHM (LED, Buzzer, Button)...");
    if ((m_led.init()    != Status::OK) ||
        (m_buzzer.init() != Status::OK) ||
        (m_button.init() != Status::OK)) {
        ESP_LOGE(TAG, "  IHM init FAILED");
        anyError = true;
    }

    // -------------------------------------------------------------------------
    // [5] Bus I2C
    // -------------------------------------------------------------------------
    ESP_LOGI(TAG, "[5/9] Init I2C bus...");
    if (m_i2c.init() != Status::OK) {
        ESP_LOGE(TAG, "  I2C FAILED — capteurs inaccessibles");
        anyError = true;
    }

    // -------------------------------------------------------------------------
    // [6] INA237 — Capteur courant/tension
    // =========================================================================
    ESP_LOGI(TAG, "[6/9] Init INA237 (courant/tension)...");
    if (m_ina237.init() != Status::OK) {
        ESP_LOGE(TAG, "  INA237 FAILED");
        anyError = true;
    }

    // -------------------------------------------------------------------------
    // [7] TMP126 — Capteur température numérique SPI
    // =========================================================================
    ESP_LOGI(TAG, "[7/9] Init TMP126 (température)...");
    if (m_tmp126.init() != Status::OK) {
        ESP_LOGE(TAG, "  TMP126 FAILED");
        anyError = true;
    }

    // -------------------------------------------------------------------------
    // [8] Services APP
    // -------------------------------------------------------------------------
    ESP_LOGI(TAG, "[8/9] Init services APP...");
    if ((m_monitoring.init() != Status::OK) ||
        (m_logging.init()    != Status::OK)) {
        ESP_LOGE(TAG, "  Services APP FAILED");
        anyError = true;
    }

    // -------------------------------------------------------------------------
    // [9] BLE
    // -------------------------------------------------------------------------
    ESP_LOGI(TAG, "[9/9] Init BLE...");
    if (m_ble.init() != Status::OK) {
        ESP_LOGW(TAG, "  BLE FAILED — fonctionnement sans Bluetooth");
    }

    // -------------------------------------------------------------------------
    // Résultat global — notifie l'AlarmService
    // -------------------------------------------------------------------------
    m_alarm.setInitSuccess(!anyError);

    if (!anyError) {
        ESP_LOGI(TAG, "========================================");
        ESP_LOGI(TAG, " INIT COMPLETE — Démarrage surveillance");
        ESP_LOGI(TAG, "========================================");
        m_logging.logEvent(SystemState::NORMAL, SensorData{}, "INIT OK");
    } else {
        ESP_LOGE(TAG, "========================================");
        ESP_LOGE(TAG, " INIT ERREUR(S) — État CRITICAL");
        ESP_LOGE(TAG, "========================================");
        m_logging.logEvent(SystemState::CRITICAL, SensorData{}, "INIT ERROR");
    }

    m_initialized = true;
    return anyError ? Status::ERROR : Status::OK;
}

// =============================================================================
// RUN — Boucle principale (100 ms par itération)
// =============================================================================

void MainApp::run() {
    if (!m_initialized) {
        ESP_LOGE(TAG, "run() appelé sans init() — abandon");
        return;
    }

    while (true) {
        m_tickMs = xTaskGetTickCount() * portTICK_PERIOD_MS;

        // ----------------------------------------------------------------
        // 1. Lecture bouton (anti-rebond)
        // ----------------------------------------------------------------
        m_button.update(m_tickMs);

        if (m_button.isResetPressed()) {
            ESP_LOGI(TAG, "Bouton Reset détecté");
            m_alarm.applyReset();
            m_buzzer.reset();
        }

        // ----------------------------------------------------------------
        // 2. Mesure capteurs
        // ----------------------------------------------------------------
        if (m_monitoring.measure() != Status::OK) {
            ESP_LOGW(TAG, "Lecture capteurs FAILED");
        }
        const SensorData& data = m_monitoring.getData();

        // ----------------------------------------------------------------
        // 3. Évaluation alarmes (FSM)
        // ----------------------------------------------------------------
        const SystemState newState = m_alarm.evaluate(data);

        // Journalisation sur changement d'état
        if (m_alarm.stateChanged()) {
            m_logging.logEvent(newState, data);
        }

        // ----------------------------------------------------------------
        // 4. Mise à jour IHM (LED + Buzzer)
        // ----------------------------------------------------------------
        updateIHM(newState);

        // ----------------------------------------------------------------
        // 5. Mise à jour BLE
        // ----------------------------------------------------------------
        updateBLE(data, newState);

        m_prevState = newState;

        // ----------------------------------------------------------------
        // 6. Délai de la boucle
        // ----------------------------------------------------------------
        vTaskDelay(pdMS_TO_TICKS(AppConfig::MONITORING_PERIOD_MS));
    }
}

// =============================================================================
// Privées
// =============================================================================

void MainApp::updateIHM(SystemState state) {
    switch (state) {
        case SystemState::INIT:
            m_led.setMode(LedMode::OFF);
            m_buzzer.setMode(BuzzerMode::SILENT);
            break;

        case SystemState::NORMAL:
            m_led.setMode(LedMode::GREEN_STEADY);
            m_buzzer.setMode(BuzzerMode::SILENT);
            break;

        case SystemState::WARNING:
            m_led.setMode(LedMode::RED_BLINK);
            m_buzzer.setMode(BuzzerMode::WARNING_SLOW);
            break;

        case SystemState::CRITICAL:
        case SystemState::SAFE_SHUTDOWN:
            // LED rouge fixe + buzzer critique (2sec continu puis intermittent)
            // Le mode CRITICAL_ACTIVE est maintenu jusqu'au reset
            m_led.setMode(LedMode::RED_STEADY);
            if (m_buzzer.getMode() != BuzzerMode::CRITICAL_ACTIVE) {
                m_buzzer.setMode(BuzzerMode::CRITICAL_ACTIVE);
            }
            break;

        default:
            // Sécurité défensive
            m_led.setMode(LedMode::RED_STEADY);
            m_buzzer.setMode(BuzzerMode::CRITICAL_ACTIVE);
            break;
    }

    // Tick des machines d'états IHM
    m_led.update(m_tickMs);
    m_buzzer.update(m_tickMs);
}

void MainApp::updateBLE(const SensorData& data, SystemState state) {
    if (!m_ble.isInitialized()) { return; }

    BlePayload payload = {};
    payload.current_A  = data.current_A;
    payload.voltage_V  = data.voltage_V;
    payload.temp_C     = m_monitoring.getMaxTemp_C();
    payload.power_W    = data.power_W;
    payload.state      = static_cast<uint8_t>(state);
    payload.timestamp  = m_tickMs;

    m_ble.updateData(payload);
}
