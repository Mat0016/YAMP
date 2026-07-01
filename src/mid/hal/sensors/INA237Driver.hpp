#pragma once
#include <stdint.h>
#include <Wire.h>
#include "config/SystemConfig.hpp"
#include "config/HardwareConfig.hpp"
#include "config/AppConfig.hpp"

// ============================================================
//  INA237Driver.hpp — Couche LOW / HAL / sensors
//
//  Capteur de puissance Texas Instruments INA237.
//  Communication : I2C via Wire.h (Arduino).
//  Wire.begin() doit être appelé dans main.cpp avant init().
//
//  Registres utilisés (datasheet INA237, §7.6) :
//    0x00 CONFIG      — mode de conversion + moyennage
//    0x01 ADC_CONFIG  — temps de conversion
//    0x02 SHUNT_CAL   — calibration courant
//    0x05 VBUS        — tension bus (lecture)
//    0x07 CURRENT     — courant (lecture, signé)
//    0x08 POWER       — puissance (lecture)
//    0x3E MANUF_ID    — 0x5449 "TI" (vérification init)
//    0x3F DEVICE_ID   — 0x2371 (vérification init)
// ============================================================

class INA237Driver {
public:
    // ---- Initialisation ------------------------------------
    // Configure les registres et vérifie l'identité du circuit.
    // Retourne Status::ERROR si le capteur ne répond pas.
    Status init();

    // ---- Lectures physiques --------------------------------
    float32_t readCurrent_A()  const;
    float32_t readVoltage_V()  const;
    float32_t readPower_W()    const;

    bool isReady() const { return m_ready; }

private:
    bool m_ready = false;

    // ---- Accès registres I2C (Wire.h) ----------------------
    Status    writeReg(uint8_t reg, uint16_t value) const;
    uint16_t  readReg (uint8_t reg)                 const;

    // Adresses des registres internes
    static constexpr uint8_t REG_CONFIG     = 0x00U;
    static constexpr uint8_t REG_ADC_CONFIG = 0x01U;
    static constexpr uint8_t REG_SHUNT_CAL  = 0x02U;
    static constexpr uint8_t REG_VBUS       = 0x05U;
    static constexpr uint8_t REG_CURRENT    = 0x07U;
    static constexpr uint8_t REG_POWER      = 0x08U;
    static constexpr uint8_t REG_MANUF_ID   = 0x3EU;
    static constexpr uint8_t REG_DEVICE_ID  = 0x3FU;

    // Résolutions (datasheet §7.3.2)
    static constexpr float32_t VBUS_LSB_V   = 0.003125F; // 3.125 mV/LSB
    static constexpr float32_t POWER_LSB_W  = 3.2F * INA237_CURRENT_LSB_A;
};
