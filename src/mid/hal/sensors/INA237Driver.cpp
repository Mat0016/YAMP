#include "INA237Driver.hpp"
#include <esp_log.h>

static const char* TAG = "INA237";

// ============================================================
//  INIT — Séquence d'initialisation complète
//  1. Vérification MANUF_ID + DEVICE_ID
//  2. Écriture CONFIG  : reset soft + mode continu
//  3. Écriture SHUNT_CAL
// ============================================================
Status INA237Driver::init() {
    ESP_LOGI(TAG, "Initialisation INA237 @ I2C 0x%02X ...", INA237_I2C_ADDR);

    // --- 1. Vérification identité du circuit -----------------
    uint16_t manufId   = readReg(REG_MANUF_ID);
    uint16_t deviceId  = readReg(REG_DEVICE_ID);

    if (manufId != INA237_MANUF_ID) {
        ESP_LOGE(TAG, "MANUF_ID invalide : 0x%04X (attendu 0x%04X)", manufId, INA237_MANUF_ID);
        return Status::ERROR;
    }
    if (deviceId != INA237_DEVICE_ID) {
        ESP_LOGE(TAG, "DEVICE_ID invalide : 0x%04X (attendu 0x%04X)", deviceId, INA237_DEVICE_ID);
        return Status::ERROR;
    }
    ESP_LOGI(TAG, "Identite OK — MANUF=0x%04X  DEVICE=0x%04X", manufId, deviceId);

    // --- 2. CONFIG : mode continu toutes mesures (I+V+P), moyenne 16x
    //    Bit 15-12 : mode = 0xF (continuous shunt+bus+temp)
    //    Bit 11-9  : VBUSCT = 100 (1.1 ms)
    //    Bit 8-6   : VSHCT  = 100 (1.1 ms)
    //    Bit 5-0   : AVG    = 010 (16 samples)
    Status st = writeReg(REG_CONFIG, 0x4127U);
    if (st != Status::OK) {
        ESP_LOGE(TAG, "Erreur ecriture CONFIG");
        return Status::ERROR;
    }

    // --- 3. ADC_CONFIG par défaut (on garde la valeur reset 0xFB68)
    //    ici on force explicitement pour clarté
    st = writeReg(REG_ADC_CONFIG, 0xFB68U);
    if (st != Status::OK) {
        ESP_LOGE(TAG, "Erreur ecriture ADC_CONFIG");
        return Status::ERROR;
    }

    // --- 4. SHUNT_CAL : calibration courant
    //    = 819.2e6 × CURRENT_LSB × R_shunt (voir AppConfig.hpp)
    st = writeReg(REG_SHUNT_CAL, INA237_SHUNT_CAL);
    if (st != Status::OK) {
        ESP_LOGE(TAG, "Erreur ecriture SHUNT_CAL");
        return Status::ERROR;
    }

    m_ready = true;
    ESP_LOGI(TAG, "INA237 pret. CURRENT_LSB = %.6f A", INA237_CURRENT_LSB_A);
    return Status::OK;
}

// ============================================================
//  LECTURES
// ============================================================
float32_t INA237Driver::readCurrent_A() const {
    if (!m_ready) { return 0.0F; }
    // Registre CURRENT : entier signé 16 bits (complément à 2)
    int16_t raw = static_cast<int16_t>(readReg(REG_CURRENT));
    return static_cast<float32_t>(raw) * INA237_CURRENT_LSB_A;
}

float32_t INA237Driver::readVoltage_V() const {
    if (!m_ready) { return 0.0F; }
    uint16_t raw = readReg(REG_VBUS);
    return static_cast<float32_t>(raw) * VBUS_LSB_V;
}

float32_t INA237Driver::readPower_W() const {
    if (!m_ready) { return 0.0F; }
    // Registre POWER : non signé 24 bits → on lit seulement les 16 MSB ici
    // (INA237 renvoie en réalité 3 octets; simplifié pour ESP32/Wire)
    uint16_t raw = readReg(REG_POWER);
    return static_cast<float32_t>(raw) * POWER_LSB_W;
}

// ============================================================
//  ACCÈS REGISTRES — Wire.h (I2C Arduino)
//  Pas de classe manager : on utilise Wire directement.
// ============================================================
Status INA237Driver::writeReg(uint8_t reg, uint16_t value) const {
    Wire.beginTransmission(INA237_I2C_ADDR);
    Wire.write(reg);
    Wire.write(static_cast<uint8_t>(value >> 8));    // MSB en premier (big-endian)
    Wire.write(static_cast<uint8_t>(value & 0xFFU));
    uint8_t err = Wire.endTransmission();
    return (err == 0U) ? Status::OK : Status::ERROR;
}

uint16_t INA237Driver::readReg(uint8_t reg) const {
    // Pointer write
    Wire.beginTransmission(INA237_I2C_ADDR);
    Wire.write(reg);
    Wire.endTransmission(false);  // repeated start

    // Read 2 bytes
    Wire.requestFrom(static_cast<uint8_t>(INA237_I2C_ADDR), static_cast<uint8_t>(2U));
    if (Wire.available() < 2) { return 0U; }
    uint16_t msb = static_cast<uint16_t>(Wire.read()) << 8;
    uint16_t lsb = static_cast<uint16_t>(Wire.read());
    return msb | lsb;
}
