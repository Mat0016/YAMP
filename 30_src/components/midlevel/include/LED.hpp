#pragma once
#include <cstdint>
#include "../../../config/CommonTypes.hpp"
#include "driver/gpio.h"

// =============================================================================
// LED.hpp — Pilote de la LED bicolore (Verte / Rouge)
//
// Une seule LED bicolore à cathode commune ou deux LEDs distinctes
// sur deux GPIOs séparés (GPIO_GREEN et GPIO_RED).
// =============================================================================

enum class LedMode : uint8_t {
    OFF          = 0U,  // Toutes éteintes
    GREEN_STEADY = 1U,  // NORMAL — vert fixe
    RED_BLINK    = 2U,  // WARNING — rouge clignotant (géré dans update())
    RED_STEADY   = 3U,  // CRITICAL / SHUTDOWN — rouge fixe
};

class LED {
public:
    LED(gpio_num_t greenPin, gpio_num_t redPin);

    Status init();

    // Définit le mode d'affichage — update() doit être appelé périodiquement
    void setMode(LedMode mode);

    // Appelé dans la boucle principale — gère le clignotement
    void update(uint32_t tickMs);

    LedMode getMode() const { return m_mode; }

private:
    gpio_num_t m_greenPin;
    gpio_num_t m_redPin;
    LedMode    m_mode         = LedMode::OFF;
    bool       m_blinkState   = false;
    uint32_t   m_lastToggleMs = 0U;

    static constexpr uint32_t BLINK_PERIOD_MS = 500U; // Clignotement WARNING

    void setGreen(bool on) const;
    void setRed(bool on)   const;
    void allOff()          const;
};
