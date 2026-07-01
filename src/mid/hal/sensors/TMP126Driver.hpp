#pragma once
#include <stdint.h>
#include <SPI.h>
#include "config/SystemConfig.hpp"
#include "config/HardwareConfig.hpp"

// ============================================================
//  TMP126Driver.hpp — Couche LOW / HAL / sensors
//
//  Capteur de température Texas Instruments TMP126.
//  Communication : SPI 4 fils via SPI.h (Arduino).
//  SPI.begin() appelé dans init().
//
//  Registres TMP126 (datasheet SNIS227C §7.6) :
//    0x00 TEMP_RESULT  — température (16 bits, R)
//    0x01 CONFIG       — configuration mode conversion (R/W)
//    0x02 TEMP_LOW_LIM — seuil bas alerte (R/W)
//    0x03 TEMP_HI_LIM  — seuil haut alerte (R/W)
//    0x04 DEVICE_ID    — 0x0126 (R)
//
//  Format trame SPI (4 fils, MSB en premier) :
//    Byte 0 : [R/W | addr[5:0] | 0]  (R=1 read, R=0 write)
//    Byte 1-2 : donnée 16 bits
//
//  Résolution température : 1 LSB = 0.03125 °C (14 bits signés,
//  bits [15:2] du registre TEMP_RESULT).
// ============================================================

class TMP126Driver {
public:
    // ---- Initialisation ------------------------------------
    // Démarre le bus SPI, vérifie DEVICE_ID, configure mode continu.
    Status init();

    // ---- Lecture --------------------------------------------
    float32_t readTemperature_degC() const;

    bool isReady() const { return m_ready; }

private:
    bool m_ready = false;

    // ---- Accès registres SPI --------------------------------
    Status   writeReg(uint8_t reg, uint16_t value) const;
    uint16_t readReg (uint8_t reg)                 const;

    // Adresses des registres internes
    static constexpr uint8_t REG_TEMP_RESULT = 0x00U;
    static constexpr uint8_t REG_CONFIG      = 0x01U;
    static constexpr uint8_t REG_DEVICE_ID   = 0x04U;

    // Octet de commande SPI
    // [7]=R/W, [6:1]=addr, [0]=0
    static constexpr uint8_t SPI_READ  = 0x80U; // bit 7 = 1
    static constexpr uint8_t SPI_WRITE = 0x00U; // bit 7 = 0

    // Résolution (14 bits signés, décalage >> 2)
    static constexpr float32_t TEMP_LSB_DEGC = 0.03125F;

    // CONFIG : mode continu, averaging x8 (voir datasheet §7.6.2)
    static constexpr uint16_t CONFIG_CONTINUOUS = 0x0600U;
};
