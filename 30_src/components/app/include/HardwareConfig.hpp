#pragma once
#include <cstdint>

// =============================================================================
// HardwareConfig.hpp — Pins GPIO, adresses I2C, paramètres SPI
// Modifier ici pour adapter au PCB physique.
// =============================================================================

namespace HardwareConfig {

    // -------------------------------------------------------------------------
    // Bus I2C (INA237 + I2CManager)
    // -------------------------------------------------------------------------
    constexpr uint8_t  I2C_PORT        = 0U;          // I2C_NUM_0
    constexpr uint8_t  I2C_SDA_PIN     = 21U;
    constexpr uint8_t  I2C_SCL_PIN     = 22U;
    constexpr uint32_t I2C_FREQ_HZ     = 400000U;     // Fast Mode 400 kHz

    // -------------------------------------------------------------------------
    // INA237 — Capteur courant / tension (I2C)
    // A0=GND, A1=GND → adresse 0x40 (datasheet TI SBOS924)
    // -------------------------------------------------------------------------
    constexpr uint8_t  INA237_ADDR     = 0x40U;
    constexpr float    INA237_SHUNT_OHM = 0.015f;     // Résistance shunt 15 mΩ
    constexpr float    INA237_MAX_A     = 10.0f;       // Courant max attendu

    // -------------------------------------------------------------------------
    // TMP126 — Capteur température numérique (SPI Mode 1)
    // (datasheet TI SBOS811)
    // -------------------------------------------------------------------------
    constexpr uint8_t  TMP126_CS_PIN   = 5U;
    constexpr uint8_t  TMP126_MISO_PIN = 19U;
    constexpr uint8_t  TMP126_MOSI_PIN = 23U;  // Non utilisé (read-only)
    constexpr uint8_t  TMP126_CLK_PIN  = 18U;
    constexpr uint32_t TMP126_SPI_HZ   = 8000000U;   // 8 MHz (max 10 MHz)
    constexpr uint8_t  TMP126_SPI_HOST = 2U;          // VSPI_HOST = 3 ou HSPI = 2

    // -------------------------------------------------------------------------
    // IHM — LED bicolore (verte / rouge), Buzzer, Bouton
    // -------------------------------------------------------------------------
    constexpr uint8_t  LED_GREEN_PIN   = 25U;
    constexpr uint8_t  LED_RED_PIN     = 26U;
    constexpr uint8_t  BUZZER_PIN      = 27U;
    constexpr uint8_t  BUTTON_PIN      = 0U;           // Boot/Reset button (actif bas)

    // -------------------------------------------------------------------------
    // BLE
    // -------------------------------------------------------------------------
    constexpr char     BLE_DEVICE_NAME[] = "VentecMon";

    // -------------------------------------------------------------------------
    // Mémoire — PSRAM (Logs) + Flash NVS (Config)
    // -------------------------------------------------------------------------
    constexpr char     NVS_NAMESPACE[]  = "ventec_cfg";

} // namespace HardwareConfig
