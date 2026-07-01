#include "MainApp.hpp"

#include <esp_log.h>

namespace
{
    constexpr char kLogTag[] = "MainApp";
}

// ============================================================
//  INIT
//  Appele une seule fois depuis setup().
//  Initialise les bus, les drivers, la PSRAM et les services.
// ============================================================
Status MainApp::init()
{
    ESP_LOGI(kLogTag, "=== YAMP - demarrage ===");

    // Bus I2C utilise par INA237.
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    Wire.setClock(I2C_FREQ_HZ);
    ESP_LOGI(kLogTag,
             "Bus I2C demarre (SDA=%d SCL=%d @ %u Hz)",
             PIN_I2C_SDA,
             PIN_I2C_SCL,
             I2C_FREQ_HZ);

    status_led_.init();
    buzzer_.init();
    boot_button_.init();

    if (!PSRAMManager::isAvailable())
    {
        ESP_LOGW(kLogTag, "PSRAM non detectee - log desactive");
    }
    else
    {
        PSRAMManager::logInfo();

        if (!ring_buffer_.init())
        {
            ESP_LOGW(kLogTag, "Ring buffer non initialise");
        }
    }

    if (config_service_.init() != Status::OK)
    {
        ESP_LOGW(kLogTag, "Echec lecture flash - seuils par defaut");
    }

    const Status monitoring_status = monitoring_service_.init();
    if (monitoring_status != Status::OK)
    {
        ESP_LOGE(kLogTag, "Echec init capteurs - etat CRITICAL");
        current_state_ = AgvState::CRITICAL;
        updateHmi();
        return Status::ERROR;
    }

    ble_manager_.init();

    current_state_ = AgvState::NORMAL;
    buzzer_.setPattern(BuzzerPattern::SUCCESS_BEEP);
    updateHmi();

    ESP_LOGI(kLogTag, "Init OK - etat NORMAL");
    return Status::OK;
}

// ============================================================
//  PROCESS
//  Appele par loop(). Cadencement sans delay().
// ============================================================
void MainApp::process()
{
    const uint32_t current_time_ms = millis();

    if ((current_time_ms - last_loop_time_ms_) < APP_LOOP_PERIOD_MS)
    {
        buzzer_.update();
        return;
    }

    last_loop_time_ms_ = current_time_ms;

    const bool is_boot_button_pressed = boot_button_.isBootPressed();

    switch (current_state_)
    {
        case AgvState::INIT:
            if (init() != Status::OK)
            {
                current_state_ = AgvState::CRITICAL;
            }
            break;

        case AgvState::NORMAL:
        case AgvState::WARNING:
            processNominalState(is_boot_button_pressed);
            break;

        case AgvState::CRITICAL:
            triggerSafetyShutdown();
            event_log_service_.log(AgvState::SAFE_SHUTDOWN,
                                   current_measurement_,
                                   "Safety shutdown");
            current_state_ = AgvState::SAFE_SHUTDOWN;
            break;

        case AgvState::SAFE_SHUTDOWN:
            if (is_boot_button_pressed)
            {
                ESP_LOGI(kLogTag, "Reset manuel - retour INIT");
                buzzer_.stop();
                current_state_ = AgvState::INIT;
                static_cast<void>(init());
            }
            break;

        default:
            ESP_LOGE(kLogTag,
                     "Etat FSM inconnu (%d) - CRITICAL",
                     static_cast<int>(current_state_));
            current_state_ = AgvState::CRITICAL;
            break;
    }

    updateHmi();
    ble_manager_.notify(current_measurement_.temperature_degC,
                        current_measurement_.current_A,
                        current_measurement_.voltage_V,
                        current_state_);

    buzzer_.update();
}

// ============================================================
//  PROCESS NOMINAL STATE
//  Commun aux etats NORMAL et WARNING.
// ============================================================
void MainApp::processNominalState(const bool is_boot_button_pressed)
{
    current_measurement_ = monitoring_service_.acquire();

    const AgvState next_state = alarm_service_.evaluate(current_state_,
                                                        current_measurement_);

    if (next_state != current_state_)
    {
        event_log_service_.log(next_state,
                               current_measurement_,
                               stateToString(next_state));
        current_state_ = next_state;
    }

    if (is_boot_button_pressed)
    {
        current_state_ = AgvState::INIT;
    }
}

// ============================================================
//  UPDATE HMI
//  LED et buzzer selon l'etat courant.
// ============================================================
void MainApp::updateHmi()
{
    switch (current_state_)
    {
        case AgvState::INIT:
            status_led_.setOff();
            break;

        case AgvState::NORMAL:
            status_led_.setGreen();
            break;

        case AgvState::WARNING:
            status_led_.setOrange();
            buzzer_.setPattern(BuzzerPattern::WARNING_SLOW);
            break;

        case AgvState::CRITICAL:
            status_led_.setRed();
            buzzer_.setPattern(BuzzerPattern::CRITICAL_START);
            break;

        case AgvState::SAFE_SHUTDOWN:
            status_led_.setRed();
            break;

        default:
            status_led_.setRed();
            buzzer_.setPattern(BuzzerPattern::CRITICAL_START);
            break;
    }
}

// ============================================================
//  SAFETY SHUTDOWN
//  Point d'extension : couper le relais de puissance ici.
// ============================================================
void MainApp::triggerSafetyShutdown()
{
    ESP_LOGE(kLogTag, "SAFETY SHUTDOWN declenche !");
    // Exemple : digitalWrite(PIN_RELAY_POWER, LOW);
    // Ajouter la broche dans HardwareConfig.hpp si necessaire.
}

const char* MainApp::stateToString(const AgvState state)
{
    const char* state_name = "UNKNOWN";

    switch (state)
    {
        case AgvState::INIT:
            state_name = "INIT";
            break;

        case AgvState::NORMAL:
            state_name = "NORMAL";
            break;

        case AgvState::WARNING:
            state_name = "WARNING";
            break;

        case AgvState::CRITICAL:
            state_name = "CRITICAL";
            break;

        case AgvState::SAFE_SHUTDOWN:
            state_name = "SAFE_SHUTDOWN";
            break;

        default:
            state_name = "UNKNOWN";
            break;
    }

    return state_name;
}
