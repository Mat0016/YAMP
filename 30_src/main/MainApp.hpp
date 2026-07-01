#pragma once
#include "../../config/CommonTypes.hpp"

// Drivers HAL
#include "../../low/hal/sensors/INA237Driver.hpp"
#include "../../low/hal/sensors/TMP126Driver.hpp"
#include "../../low/hal/IHM/LED.hpp"
#include "../../low/hal/IHM/Buzzer.hpp"
#include "../../low/hal/IHM/Button.hpp"
#include "../../low/hal/memory/PSRAMManager.hpp"
#include "../../low/hal/memory/FlashStorage.hpp"

// Couche MID
#include "../../mid/i2c/I2CManager.hpp"
#include "../../mid/ble/BLEManager.hpp"

// Services APP
#include "../services/MonitoringService.hpp"
#include "../services/AlarmService.hpp"
#include "../services/EventLoggingService.hpp"
#include "../services/ConfigService.hpp"

// =============================================================================
// MainApp.hpp — Orchestrateur principal
//
// Instancie tous les objets, enchaîne l'init et exécute la boucle principale.
// Appelé depuis app_main() de l'ESP-IDF.
// =============================================================================

class MainApp {
public:
    MainApp();

    // Initialise tous les sous-systèmes dans le bon ordre
    Status init();

    // Boucle principale (appeler indéfiniment depuis app_main)
    void run();

private:
    // -----------------------------------------------------------------------
    // Couche LOW — Drivers HAL
    // -----------------------------------------------------------------------
    PSRAMManager     m_psram;
    FlashStorage     m_flash;

    // -----------------------------------------------------------------------
    // Couche MID — Communication
    // -----------------------------------------------------------------------
    I2CManager       m_i2c;
    BLEManager       m_ble;

    // -----------------------------------------------------------------------
    // Drivers capteurs et IHM (dépendent de I2CManager)
    // -----------------------------------------------------------------------
    INA237Driver     m_ina237;
    TMP126Driver     m_tmp126;
    LED              m_led;
    Buzzer           m_buzzer;
    Button           m_button;

    // -----------------------------------------------------------------------
    // Couche APP — Services métier
    // -----------------------------------------------------------------------
    MonitoringService    m_monitoring;
    AlarmService         m_alarm;
    EventLoggingService  m_logging;
    ConfigService        m_config;

    // -----------------------------------------------------------------------
    // État interne
    // -----------------------------------------------------------------------
    SystemState  m_prevState    = SystemState::INIT;
    uint32_t     m_tickMs       = 0U;
    bool         m_initialized  = false;

    // Mise à jour de l'IHM en fonction de l'état FSM
    void updateIHM(SystemState state);

    // Mise à jour BLE avec les dernières données
    void updateBLE(const SensorData& data, SystemState state);
};
