#include "ConfigService.hpp"
#include "esp_log.h"

static const char* TAG = "ConfigService";

ConfigService::ConfigService(FlashStorage& flash) : m_flash(flash) {}

Status ConfigService::init() {
    if (m_flash.loadConfig(m_config) != Status::OK) {
        ESP_LOGW(TAG, "Config non trouvée en Flash — valeurs par défaut appliquées");
        resetToDefaults();
    } else {
        ESP_LOGI(TAG, "Config chargée : I_warn=%.1fA I_crit=%.1fA T_warn=%.1f°C T_crit=%.1f°C",
                 m_config.currentWarningA, m_config.currentCriticalA,
                 m_config.tempWarningC,    m_config.tempCriticalC);
    }
    m_initialized = true;
    return Status::OK;
}

Status ConfigService::saveConfig(const StoredConfig& cfg) {
    m_config = cfg;
    return m_flash.saveConfig(cfg);
}

void ConfigService::resetToDefaults() {
    m_config.currentWarningA  = 8.0f;
    m_config.currentCriticalA = 10.0f;
    m_config.tempWarningC     = 50.0f;
    m_config.tempCriticalC    = 65.0f;
    m_config.checksum         = 0U;
    ESP_LOGI(TAG, "Config par défaut appliquée");
}
