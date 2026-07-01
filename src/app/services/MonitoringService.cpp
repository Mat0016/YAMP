#include "MonitoringService.hpp"
#include <Wire.h>
#include <esp_log.h>

static const char* TAG = "Monitoring";

Status MonitoringService::init() {
    // Wire.begin() est appelé dans main.cpp avant tout
    Status sIna = m_ina.init();
    Status sTmp = m_tmp.init();

    if (sIna != Status::OK) { ESP_LOGE(TAG, "INA237 init ECHEC"); }
    if (sTmp != Status::OK) { ESP_LOGE(TAG, "TMP126 init ECHEC"); }

    return (sIna == Status::OK && sTmp == Status::OK) ? Status::OK : Status::ERROR;
}

Measurement MonitoringService::acquire() {
    Measurement m;
    m.temperature_degC = m_tmp.readTemperature_degC();
    m.current_A        = m_ina.readCurrent_A();
    m.voltage_V        = m_ina.readVoltage_V();
    m.power_W          = m_ina.readPower_W();
    m.valid            = m_ina.isReady() && m_tmp.isReady();

    ESP_LOGD(TAG, "T=%.1f°C  I=%.3fA  V=%.2fV  P=%.2fW",
             static_cast<double>(m.temperature_degC),
             static_cast<double>(m.current_A),
             static_cast<double>(m.voltage_V),
             static_cast<double>(m.power_W));
    return m;
}
