#pragma once
#include <cstdint>
#include "../../../config/CommonTypes.hpp"
#include "driver/gpio.h"

// =============================================================================
// Button.hpp — Pilote du bouton Reset/Boot
//
// Le bouton est actif bas (GPIO0 = bouton BOOT ESP32).
// Un appui valide doit durer > DEBOUNCE_MS pour éviter les rebonds.
// En état CRITICAL/SAFE_SHUTDOWN, le bouton permet de sortir de cet état.
// =============================================================================

class Button {
public:
    explicit Button(gpio_num_t pin);

    Status init();

    // Doit être appelé périodiquement dans la boucle principale
    void update(uint32_t tickMs);

    // Retourne true si un appui valide (anti-rebond) vient d'être détecté
    bool isResetPressed();

private:
    gpio_num_t m_pin;
    bool       m_lastState    = true;   // Actif bas : non enfoncé = HIGH = true
    bool       m_pressedEvent = false;
    uint32_t   m_pressStartMs = 0U;

    static constexpr uint32_t DEBOUNCE_MS = 50U;  // Anti-rebond
    static constexpr uint32_t HOLD_MS     = 200U; // Durée minimale d'appui
};
