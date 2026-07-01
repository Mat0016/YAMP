#pragma once
#include <cstdint>
#include "../../../config/CommonTypes.hpp"
#include "driver/ledc.h"

// =============================================================================
// Buzzer.hpp — Pilote du buzzer avec gestion des fréquences et modes
//
// Comportements :
//   NORMAL  → Silence
//   WARNING → Buzzer intermittent LENT (500 Hz, 500ms ON / 1000ms OFF)
//   CRITICAL → Phase 1 : 2 sec continus (2 kHz)
//              Phase 2 : intermittent rapide (2 kHz, 300ms ON / 300ms OFF)
//              → Actif jusqu'au reset/reboot manuel
//
// Le buzzer est piloté par le timer LEDC de l'ESP32 (PWM hardware).
// =============================================================================

enum class BuzzerMode : uint8_t {
    SILENT          = 0U,  // Silence
    WARNING_SLOW    = 1U,  // Bip lent — avertissement
    CRITICAL_ACTIVE = 2U,  // Phase active (2 sec continu + intermittent)
};

class Buzzer {
public:
    explicit Buzzer(uint8_t pin);

    Status init();

    // Définit le mode du buzzer
    void setMode(BuzzerMode mode);

    // Appelé périodiquement dans la boucle principale — gère les timings
    void update(uint32_t tickMs);

    BuzzerMode getMode() const { return m_mode; }

    // Reset du buzzer CRITICAL (déclenché par appui bouton)
    void reset();

private:
    uint8_t    m_pin;
    BuzzerMode m_mode            = BuzzerMode::SILENT;
    uint32_t   m_modeStartMs     = 0U;
    uint32_t   m_lastToggleMs    = 0U;
    bool       m_buzzOn          = false;
    bool       m_critContinuDone = false; // Phase 1 terminée ?

    // LEDC
    static constexpr ledc_timer_t   LEDC_TIMER   = LEDC_TIMER_0;
    static constexpr ledc_channel_t LEDC_CHANNEL = LEDC_CHANNEL_0;
    static constexpr ledc_mode_t    LEDC_MODE    = LEDC_LOW_SPEED_MODE;
    static constexpr uint32_t       DUTY_50PCT   = 512U; // 50% duty sur 10 bits

    void toneOn(uint32_t freqHz)  const;
    void toneOff()                const;
};
