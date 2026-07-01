#include "MonitoringService.hpp"
#include "../../low/hal/sensors/INA237Driver.hpp"
#include "../../low/hal/sensors/TMP126Driver.hpp"
#include "esp_log.h"
#include <algorithm>

static const char* TAG = "MonitoringService";

MonitoringService::MonitoringService(INA237Driver& ina237, TMP126Driver& tmp126)
    : m_ina237(ina237)
    , m_tmp126(tmp126)
{}

Status MonitoringService::init() {
    if (!m_ina237.isInitialized() || !m_tmp126.isInitialized()) {
        ESP_LOGE(TAG, "Drivers non initialisés — appeler INA237.init() et TMP126.init() avant");
        return Status::ERROR;
    }
    m_initialized = true;
    ESP_LOGI(TAG, "MonitoringService init OK");
    return Status::OK;
}

Status MonitoringService::measure() {
    if (!m_initialized) { return Status::ERROR; }

    m_data.current_A   = m_ina237.getCurrent_A();
    m_data.voltage_V   = m_ina237.getVoltage_V();
    m_data.power_W     = m_ina237.getPower_W();
    m_data.temp_tmp126 = m_tmp126.getTemp_C();
    m_data.temp_ntc    = 0.0f;   // NTC non utilisé dans cette architecture
    m_data.valid       = true;

    return Status::OK;
}

float MonitoringService::getMaxTemp_C() const {
    return std::max(m_data.temp_tmp126, m_data.temp_ntc);
}
