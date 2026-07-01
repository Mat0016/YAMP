#include "I2CManager.hpp"
#include "../../config/HardwareConfig.hpp"
#include "esp_log.h"

static const char* TAG = "I2CManager";

// =============================================================================
// I2CManager.cpp
// =============================================================================

I2CManager::~I2CManager() {
    if (m_initialized) {
        i2c_driver_delete(m_port);
    }
}

Status I2CManager::init() {
    m_port = static_cast<i2c_port_t>(HardwareConfig::I2C_PORT);

    i2c_config_t conf = {};
    conf.mode             = I2C_MODE_MASTER;
    conf.sda_io_num       = static_cast<gpio_num_t>(HardwareConfig::I2C_SDA_PIN);
    conf.scl_io_num       = static_cast<gpio_num_t>(HardwareConfig::I2C_SCL_PIN);
    conf.sda_pullup_en    = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en    = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = HardwareConfig::I2C_FREQ_HZ;
    conf.clk_flags        = 0U;

    esp_err_t err = i2c_param_config(m_port, &conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2c_param_config failed: %s", esp_err_to_name(err));
        return Status::ERROR;
    }

    err = i2c_driver_install(m_port, I2C_MODE_MASTER, 0U, 0U, 0U);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2c_driver_install failed: %s", esp_err_to_name(err));
        return Status::ERROR;
    }

    m_initialized = true;
    ESP_LOGI(TAG, "I2C init OK — SDA:%d SCL:%d @ %lu Hz",
             HardwareConfig::I2C_SDA_PIN,
             HardwareConfig::I2C_SCL_PIN,
             HardwareConfig::I2C_FREQ_HZ);
    return Status::OK;
}

Status I2CManager::write(uint8_t addr, const uint8_t* data, size_t len) {
    esp_err_t err = i2c_master_write_to_device(
        m_port, addr, data, len,
        pdMS_TO_TICKS(I2C_TIMEOUT_MS));
    return (err == ESP_OK) ? Status::OK : Status::ERROR;
}

Status I2CManager::read(uint8_t addr, uint8_t* buf, size_t len) {
    esp_err_t err = i2c_master_read_from_device(
        m_port, addr, buf, len,
        pdMS_TO_TICKS(I2C_TIMEOUT_MS));
    return (err == ESP_OK) ? Status::OK : Status::ERROR;
}

Status I2CManager::writeReadReg(uint8_t addr, uint8_t reg,
                                uint8_t* buf, size_t len) {
    esp_err_t err = i2c_master_write_read_device(
        m_port, addr, &reg, 1U, buf, len,
        pdMS_TO_TICKS(I2C_TIMEOUT_MS));
    return (err == ESP_OK) ? Status::OK : Status::ERROR;
}
