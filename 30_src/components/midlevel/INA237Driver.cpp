#include "INA237Driver.hpp"
#include "../../../mid/i2c/I2CManager.hpp"
#include "esp_log.h"
#include <cmath>

static const char* TAG = "INA237";

// =============================================================================
// INA237Driver.cpp
// Référence datasheet : Texas Instruments SBOS924C
// =============================================================================

INA237Driver::INA237Driver(I2CManager& i2c,
                           uint8_t address,
                           float shuntOhm,
                           float maxCurrentA)
    : m_i2c(i2c)
    , m_addr(address)
    , m_shuntOhm(shuntOhm)
    , m_maxCurrentA(maxCurrentA)
    , m_currentLSB(0.0f)
    , m_initialized(false)
{}

// =============================================================================
// INIT INA237 — séquence complète de configuration des registres
// =============================================================================
Status INA237Driver::init() {
    ESP_LOGI(TAG, "=== INIT INA237 @ 0x%02X ===", m_addr);

    // -------------------------------------------------------------------------
    // Étape 1 : Vérification de la présence du composant (MANUFACTURER + DEVICE ID)
    // -------------------------------------------------------------------------
    Status idStatus = verifyDeviceId();
    if (idStatus != Status::OK) {
        ESP_LOGE(TAG, "INIT FAILED — INA237 non détecté sur le bus I2C (addr=0x%02X)", m_addr);
        return Status::ERROR;
    }
    ESP_LOGI(TAG, "  [1/5] ID vérifié OK (MANUFACTURER=0x5449, DEVICE=0x2381)");

    // -------------------------------------------------------------------------
    // Étape 2 : Reset logiciel du composant
    // Bit 15 du registre CONFIG — s'efface automatiquement après reset
    // -------------------------------------------------------------------------
    Status st = writeReg16(INA237Reg::CONFIG, INA237ConfigBits::RESET);
    if (st != Status::OK) {
        ESP_LOGE(TAG, "  [2/5] Reset logiciel FAILED");
        return Status::ERROR;
    }
    vTaskDelay(pdMS_TO_TICKS(2U)); // Attente stabilisation (datasheet : < 1 ms)
    ESP_LOGI(TAG, "  [2/5] Reset logiciel OK");

    // -------------------------------------------------------------------------
    // Étape 3 : CONFIG register (0x00)
    // ADCRANGE = 1 → ±40.96 mV (meilleure résolution pour shunt 15 mΩ)
    // -------------------------------------------------------------------------
    st = writeReg16(INA237Reg::CONFIG, INA237ConfigBits::ADCRANGE_40MV);
    if (st != Status::OK) {
        ESP_LOGE(TAG, "  [3/5] CONFIG register FAILED");
        return Status::ERROR;
    }
    ESP_LOGI(TAG, "  [3/5] CONFIG : ADCRANGE=40.96 mV OK");

    // -------------------------------------------------------------------------
    // Étape 4 : ADC_CONFIG register (0x01)
    // Mode continu sur shunt + bus + température, 16 échantillons moyennés
    // -------------------------------------------------------------------------
    st = writeReg16(INA237Reg::ADC_CONFIG, INA237ADCConfigBits::DEFAULT);
    if (st != Status::OK) {
        ESP_LOGE(TAG, "  [4/5] ADC_CONFIG register FAILED");
        return Status::ERROR;
    }
    ESP_LOGI(TAG, "  [4/5] ADC_CONFIG : continu, avg=16, Tconv=1052µs OK");

    // -------------------------------------------------------------------------
    // Étape 5 : SHUNT_CAL register (0x02)
    // Formule datasheet : SHUNT_CAL = 819.2 × 10^6 × CURRENT_LSB × R_shunt
    // CURRENT_LSB = MaxCurrent / 2^15
    // -------------------------------------------------------------------------
    m_currentLSB = m_maxCurrentA / 32768.0f;
    const float shuntCalF = 819.2e6f * m_currentLSB * m_shuntOhm;
    const uint16_t shuntCalReg = static_cast<uint16_t>(shuntCalF);

    st = writeReg16(INA237Reg::SHUNT_CAL, shuntCalReg);
    if (st != Status::OK) {
        ESP_LOGE(TAG, "  [5/5] SHUNT_CAL register FAILED");
        return Status::ERROR;
    }
    ESP_LOGI(TAG, "  [5/5] SHUNT_CAL : LSB=%.6f A, REG=0x%04X OK",
             m_currentLSB, shuntCalReg);

    // -------------------------------------------------------------------------
    // Lecture de vérification du CONFIG pour confirmer l'écriture
    // -------------------------------------------------------------------------
    uint16_t cfgReadback = 0U;
    (void)readReg16(INA237Reg::CONFIG, cfgReadback);
    ESP_LOGI(TAG, "  CONFIG readback=0x%04X (attendu=0x%04X)",
             cfgReadback, INA237ConfigBits::ADCRANGE_40MV);

    m_initialized = true;
    ESP_LOGI(TAG, "=== INA237 INIT COMPLETE — Shunt=%.3f Ω, MaxI=%.1f A ===",
             m_shuntOhm, m_maxCurrentA);
    return Status::OK;
}

// =============================================================================
// Lectures
// =============================================================================

float INA237Driver::getCurrent_A() const {
    if (!m_initialized) { return 0.0f; }
    const int16_t raw = readSigned16(INA237Reg::CURRENT);
    return static_cast<float>(raw) * m_currentLSB;
}

float INA237Driver::getVoltage_V() const {
    if (!m_initialized) { return 0.0f; }
    uint16_t raw = 0U;
    (void)readReg16(INA237Reg::VBUS, raw);
    // LSB VBUS = 3.125 mV (ADCRANGE=40mV config)
    return static_cast<float>(raw) * 3.125e-3f;
}

float INA237Driver::getPower_W() const {
    if (!m_initialized) { return 0.0f; }
    uint16_t raw = 0U;
    (void)readReg16(INA237Reg::POWER, raw);
    // LSB POWER = 0.2 × CURRENT_LSB × 1000 (datasheet §7.6.1.4)
    return static_cast<float>(raw) * 0.2f * m_currentLSB * 1000.0f;
}

float INA237Driver::getDieTemp_C() const {
    if (!m_initialized) { return 0.0f; }
    uint16_t raw = 0U;
    (void)readReg16(INA237Reg::DIETEMP, raw);
    // Bits [15:4], LSB = 125 m°C (0.125°C), décalage de 4 bits
    const int16_t temp12 = static_cast<int16_t>(raw) >> 4;
    return static_cast<float>(temp12) * 0.125f;
}

// =============================================================================
// Privées — accès registres I2C
// =============================================================================

Status INA237Driver::writeReg16(uint8_t reg, uint16_t value) const {
    // INA237 : MSB en premier (big-endian)
    uint8_t buf[3] = {
        reg,
        static_cast<uint8_t>((value >> 8U) & 0xFFU),
        static_cast<uint8_t>( value        & 0xFFU)
    };
    return m_i2c.write(m_addr, buf, sizeof(buf));
}

Status INA237Driver::readReg16(uint8_t reg, uint16_t& out) const {
    uint8_t buf[2] = {0U, 0U};
    Status st = m_i2c.writeReadReg(m_addr, reg, buf, sizeof(buf));
    if (st == Status::OK) {
        out = (static_cast<uint16_t>(buf[0]) << 8U) | static_cast<uint16_t>(buf[1]);
    }
    return st;
}

int16_t INA237Driver::readSigned16(uint8_t reg) const {
    uint16_t raw = 0U;
    (void)readReg16(reg, raw);
    return static_cast<int16_t>(raw);
}

Status INA237Driver::verifyDeviceId() const {
    uint16_t mfr = 0U;
    uint16_t dev = 0U;
    if ((readReg16(INA237Reg::MANUFACTURER_ID, mfr) != Status::OK) ||
        (readReg16(INA237Reg::DEVICE_ID, dev)        != Status::OK)) {
        return Status::ERROR;
    }
    ESP_LOGI(TAG, "  MFR_ID=0x%04X DEV_ID=0x%04X", mfr, dev);
    if ((mfr != INA237IDs::MANUFACTURER) || (dev != INA237IDs::DEVICE)) {
        ESP_LOGE(TAG, "  ID mismatch ! (MFR=0x%04X≠0x%04X | DEV=0x%04X≠0x%04X)",
                 mfr, INA237IDs::MANUFACTURER, dev, INA237IDs::DEVICE);
        return Status::ERROR;
    }
    return Status::OK;
}
