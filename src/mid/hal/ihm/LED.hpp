#pragma once
#include <Arduino.h>
#include "config/HardwareConfig.hpp"

// ============================================================
//  LED.hpp — Couche LOW / HAL / IHM
//  Pilote la LED bicolore (verte + rouge sur deux GPIO séparés).
// ============================================================

class LED {
public:
    void init() {
        pinMode(PIN_LED_GREEN, OUTPUT);
        pinMode(PIN_LED_RED,   OUTPUT);
        setOff();
    }

    void setGreen()  { digitalWrite(PIN_LED_GREEN, HIGH); digitalWrite(PIN_LED_RED, LOW);  }
    void setRed()    { digitalWrite(PIN_LED_RED,   HIGH); digitalWrite(PIN_LED_GREEN, LOW); }
    void setOrange() { digitalWrite(PIN_LED_GREEN, HIGH); digitalWrite(PIN_LED_RED, HIGH);  } // les deux = orange
    void setOff()    { digitalWrite(PIN_LED_GREEN, LOW);  digitalWrite(PIN_LED_RED, LOW);   }
};
