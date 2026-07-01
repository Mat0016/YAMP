#pragma once
#include <Arduino.h>
#include "config/HardwareConfig.hpp"

// ============================================================
//  Buzzer.hpp — Couche LOW / HAL / IHM
//
//  Comportement selon la spécification :
//  - WARNING  : bip lent intermittent (500 Hz, 200 ms ON / 800 ms OFF)
//  - CRITICAL : 2 s continu (1500 Hz), puis intermittent rapide
//               (1500 Hz, 300 ms ON / 300 ms OFF).
//               Le buzzer RESTE actif jusqu'à l'appel de stop().
//
//  Utilise le timer LEDC de l'ESP32 pour générer la fréquence.
//  update() doit être appelé à chaque cycle de la boucle principale.
// ============================================================

enum class BuzzerPattern : uint8_t {
    OFF             = 0U,
    SUCCESS_BEEP    = 1U, // bip unique 100 ms
    WARNING_SLOW    = 2U, // intermittent lent
    CRITICAL_START  = 3U, // phase 1 : 2 s continu
    CRITICAL_BEEP   = 4U  // phase 2 : intermittent rapide indéfini
};

class Buzzer {
public:
    void init();
    void setPattern(BuzzerPattern pattern);
    void stop();

    // À appeler dans loop() — gère les timings sans delay()
    void update();

private:
    void toneOn(uint32_t freqHz);
    void toneOff();

    BuzzerPattern m_pattern    = BuzzerPattern::OFF;
    bool          m_buzzerOn   = false;
    uint32_t      m_lastToggle = 0U;

    // Durées pour SUCCESS_BEEP
    uint32_t m_beepStart       = 0U;
    static constexpr uint32_t SUCCESS_DURATION_MS = 200U;

    // Durées pour WARNING_SLOW
    static constexpr uint32_t WARNING_ON_MS  = 200U;
    static constexpr uint32_t WARNING_OFF_MS = 800U;

    // Durées pour CRITICAL
    static constexpr uint32_t CRITICAL_CONT_MS = 2000U;  // phase continu
    static constexpr uint32_t CRITICAL_ON_MS   = 300U;
    static constexpr uint32_t CRITICAL_OFF_MS  = 300U;
    uint32_t m_criticalStart    = 0U;
};
