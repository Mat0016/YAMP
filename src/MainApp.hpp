#pragma once
#include <Arduino.h>
#include "config/SystemConfig.hpp"
#include "config/AppConfig.hpp"

// Couche MID — matériel
#include "mid/hal/sensors/INA237Driver.hpp"
#include "mid/hal/sensors/TMP126Driver.hpp"
#include "mid/hal/ihm/LED.hpp"
#include "mid/hal/ihm/Buzzer.hpp"
#include "mid/hal/ihm/Button.hpp"
#include "mid/memory/RingBuffer.hpp"
#include "mid/memory/FlashStorage.hpp"

// Couche MID — communication
#include "mid/ble/BLEManager.hpp"

// Couche APP — services
#include "app/services/MonitoringService.hpp"
#include "app/services/AlarmService.hpp"
#include "app/services/EventLoggingService.hpp"
#include "app/services/ConfigService.hpp"

// ============================================================
//  MainApp.hpp — Couche APP
//
//  Orchestre la machine à états (FSM) et tous les services.
//  Inspiré de l'EnergyManager fourni — adapté Arduino/PlatformIO.
//
//  FSM :
//    INIT  ──OK──► NORMAL ──seuil W──► WARNING ──seuil C──► CRITICAL
//                    ▲                   │ (hystérésis)         │
//                    └───────────────────┘                      ▼
//                  (reset)                               SAFE_SHUTDOWN
//                    ▲                                          │
//                    └──────────────────────────────────────────┘ (reset)
// ============================================================

class MainApp {
public:
    // setup() doit appeler init() UNE SEULE FOIS
    Status init();

    // loop() appelle process() à chaque cycle
    void process();

private:
    // ---- Instances drivers (ownership ici) ------------------
    INA237Driver  m_ina237;
    TMP126Driver  m_tmp126;
    LED           m_led;
    Buzzer        m_buzzer;
    Button        m_button;
    RingBuffer    m_ringBuffer;
    FlashStorage  m_flash;
    BLEManager    m_ble;

    // ---- Services applicatifs -------------------------------
    MonitoringService    m_monitoring {m_ina237, m_tmp126};
    AlarmService         m_alarm;
    EventLoggingService  m_eventLog   {m_ringBuffer};
    ConfigService        m_config     {m_flash, m_alarm};

    // ---- État FSM -------------------------------------------
    AgvState m_state = AgvState::INIT;

    // ---- Mesures courantes ----------------------------------
    Measurement m_meas;

    // ---- Timing boucle principale ---------------------------
    uint32_t m_lastLoopMs = 0U;

    // ---- Méthodes privées -----------------------------------
    void updateHmi();
    void triggerSafetyShutdown();
    const char* stateToStr(AgvState s) const;
};
