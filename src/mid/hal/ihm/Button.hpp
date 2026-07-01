#pragma once
#include <Arduino.h>
#include "config/HardwareConfig.hpp"

// ============================================================
//  Button.hpp — Couche LOW / HAL / IHM
//  Bouton Reset et bouton Boot de l'ESP32.
//  Câblés en pull-up : appui = niveau bas (LOW).
//  Anti-rebond logiciel (50 ms).
// ============================================================

class Button {
public:
    void init() {
        // PIN_BTN_BOOT (GPIO0) a déjà un pull-up hardware sur la carte
        pinMode(PIN_BTN_BOOT, INPUT_PULLUP);
    }

    // Retourne true si le bouton BOOT est pressé
    bool isBootPressed() {
        return isPressed(PIN_BTN_BOOT, m_lastBootMs);
    }

private:
    static constexpr uint32_t DEBOUNCE_MS = 50U;


    uint32_t m_lastBootMs  = 0U;

    bool isPressed(int pin, uint32_t& lastMs) {
        if (digitalRead(pin) == LOW) {
            uint32_t now = millis();
            if (now - lastMs > DEBOUNCE_MS) {
                lastMs = now;
                return true;
            }
        }
        return false;
    }
};
