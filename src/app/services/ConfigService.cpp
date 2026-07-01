#include "ConfigService.hpp"
#include <esp_log.h>

static const char* TAG = "Config";

Status ConfigService::init() {
    bool ok = m_flash.init();
    if (ok) { loadThresholds(); }
    return ok ? Status::OK : Status::ERROR;
}

void ConfigService::loadThresholds() {
    m_alarm.tempWarning_degC  = m_flash.loadTempWarning();
    m_alarm.tempCritical_degC = m_flash.loadTempCritical();
    m_alarm.currentCritical_A = m_flash.loadCurrentCrit();
    ESP_LOGI(TAG, "Seuils charges: tWarn=%.1f tCrit=%.1f iCrit=%.1f",
             static_cast<double>(m_alarm.tempWarning_degC),
             static_cast<double>(m_alarm.tempCritical_degC),
             static_cast<double>(m_alarm.currentCritical_A));
}

void ConfigService::saveThresholds() {
    m_flash.saveTempWarning (m_alarm.tempWarning_degC);
    m_flash.saveTempCritical(m_alarm.tempCritical_degC);
    m_flash.saveCurrentCrit (m_alarm.currentCritical_A);
    ESP_LOGI(TAG, "Seuils sauvegardes en flash");
}

void ConfigService::factoryReset() {
    m_flash.factoryReset();
    loadThresholds(); // recharge les valeurs par défaut
}
