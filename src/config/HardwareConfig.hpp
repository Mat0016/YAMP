#pragma once

// ============================================================
//  HardwareConfig.hpp
//  Centralise TOUTES les broches GPIO et adresses périphériques.
//  Ne jamais écrire de magic number dans le code des drivers.
// ============================================================

// --- Bus I2C (Wire.h) ----------------------------------------
static constexpr int PIN_I2C_SDA = 21;
static constexpr int PIN_I2C_SCL = 22;
static constexpr uint32_t I2C_FREQ_HZ = 400000U;   // 400 kHz fast-mode

// --- INA237 (I2C) --------------------------------------------
static constexpr uint8_t  INA237_I2C_ADDR  = 0x40U; // A0=A1=GND
static constexpr uint16_t INA237_MANUF_ID  = 0x5449U;// "TI"
static constexpr uint16_t INA237_DEVICE_ID = 0x2371U;

// --- TMP126 (SPI) --------------------------------------------
static constexpr int PIN_TMP126_CS   = 5;
static constexpr int PIN_SPI_SCLK    = 18;
static constexpr int PIN_SPI_MISO    = 19;
static constexpr int PIN_SPI_MOSI    = 23;
static constexpr uint32_t TMP126_SPI_FREQ = 4000000U; // 4 MHz max
// Device-ID attendu (registre 0x04 du TMP126)
static constexpr uint16_t TMP126_DEVICE_ID = 0x0126U;

// --- IHM : LED bicolore (verte / rouge) ----------------------
static constexpr int PIN_LED_GREEN = 14;
static constexpr int PIN_LED_RED   = 15;

// --- IHM : Buzzer (PWM ledc) ---------------------------------
static constexpr int PIN_BUZZER       = 13;
static constexpr uint8_t LEDC_CHANNEL = 0U;
static constexpr uint32_t BUZZER_FREQ_WARNING_HZ  = 500U;  // bip lent
static constexpr uint32_t BUZZER_FREQ_CRITICAL_HZ = 1500U; // bip aigu

// --- IHM : Bouton reset / boot ------------------------------
//static constexpr int PIN_BTN_RESET = EN; -> pull-up interne
static constexpr int PIN_BTN_BOOT  = 0;  // BOOT button intégré ESP32
