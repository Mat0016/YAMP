#include "TMP126Driver.hpp"
#include <Arduino.h>
#include <esp_log.h>

static const char* TAG = "TMP126";

// ============================================================
//  INIT — Séquence d'initialisation complète
//  1. Démarrage du bus SPI (broche CS gérée en manuel)
//  2. Vérification DEVICE_ID (0x0126 pour TMP126)
//  3. Écriture CONFIG : mode conversion continu
// ============================================================
Status TMP126Driver::init() {
    ESP_LOGI(TAG, "Initialisation TMP126 (SPI CS=GPIO%d)...", PIN_TMP126_CS);

    // CS en sortie, inactif haut
    pinMode(PIN_TMP126_CS, OUTPUT);
    digitalWrite(PIN_TMP126_CS, HIGH);

    // Démarrage bus SPI (SCLK, MISO, MOSI déjà câblés)
    SPI.begin(PIN_SPI_SCLK, PIN_SPI_MISO, PIN_SPI_MOSI, PIN_TMP126_CS);

    // Bref délai de démarrage requis par le TMP126 (Power-on = 1 ms typ.)
    delay(2U);

    // --- 1. Vérification DEVICE_ID ---------------------------
    uint16_t devId = readReg(REG_DEVICE_ID);
    if (devId != TMP126_DEVICE_ID) {
        ESP_LOGE(TAG, "DEVICE_ID invalide : 0x%04X (attendu 0x%04X)", devId, TMP126_DEVICE_ID);
        return Status::ERROR;
    }
    ESP_LOGI(TAG, "Identite OK — DEVICE_ID=0x%04X", devId);

    // --- 2. CONFIG : mode continu, moyennage x8 --------------
    Status st = writeReg(REG_CONFIG, CONFIG_CONTINUOUS);
    if (st != Status::OK) {
        ESP_LOGE(TAG, "Erreur ecriture CONFIG");
        return Status::ERROR;
    }

    m_ready = true;
    ESP_LOGI(TAG, "TMP126 pret. Resolution = %.5f degC/LSB", TEMP_LSB_DEGC);
    return Status::OK;
}

// ============================================================
//  LECTURE TEMPÉRATURE
//  TEMP_RESULT[15:2] = valeur signée 14 bits (>> 2 + cast int16)
// ============================================================
float32_t TMP126Driver::readTemperature_degC() const {
    if (!m_ready) { return -999.0F; }

    uint16_t raw    = readReg(REG_TEMP_RESULT);
    int16_t  signed_raw = static_cast<int16_t>(raw) >> 2; // décalage arithmétique
    return static_cast<float32_t>(signed_raw) * TEMP_LSB_DEGC;
}

// ============================================================
//  ACCÈS REGISTRES — SPI.h (Arduino)
//  Trame : [cmd byte][MSB data][LSB data]
//  cmd = SPI_READ | (addr << 1)  ou  SPI_WRITE | (addr << 1)
// ============================================================
uint16_t TMP126Driver::readReg(uint8_t reg) const {
    uint8_t cmd = SPI_READ | static_cast<uint8_t>(reg << 1);

    SPI.beginTransaction(SPISettings(TMP126_SPI_FREQ, MSBFIRST, SPI_MODE0));
    digitalWrite(PIN_TMP126_CS, LOW);

    SPI.transfer(cmd);
    uint8_t msb = SPI.transfer(0x00U);
    uint8_t lsb = SPI.transfer(0x00U);

    digitalWrite(PIN_TMP126_CS, HIGH);
    SPI.endTransaction();

    return (static_cast<uint16_t>(msb) << 8) | static_cast<uint16_t>(lsb);
}

Status TMP126Driver::writeReg(uint8_t reg, uint16_t value) const {
    uint8_t cmd = SPI_WRITE | static_cast<uint8_t>(reg << 1);

    SPI.beginTransaction(SPISettings(TMP126_SPI_FREQ, MSBFIRST, SPI_MODE0));
    digitalWrite(PIN_TMP126_CS, LOW);

    SPI.transfer(cmd);
    SPI.transfer(static_cast<uint8_t>(value >> 8));
    SPI.transfer(static_cast<uint8_t>(value & 0xFFU));

    digitalWrite(PIN_TMP126_CS, HIGH);
    SPI.endTransaction();

    return Status::OK;
}
