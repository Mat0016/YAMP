#include "AlarmService.hpp"
#include "esp_log.h"
#include <algorithm>

static const char* TAG = "AlarmService";

static const char* stateToStr(SystemState s) {
    switch (s) {
        case SystemState::INIT:          return "INIT";
        case SystemState::NORMAL:        return "NORMAL";
        case SystemState::WARNING:       return "WARNING";
        case SystemState::CRITICAL:      return "CRITICAL";
        case SystemState::SAFE_SHUTDOWN: return "SAFE_SHUTDOWN";
        default:                         return "UNKNOWN";
    }
}

AlarmService::AlarmService() : m_state(SystemState::INIT), m_stateChanged(false) {}

void AlarmService::setInitSuccess(bool success) {
    transition(success ? SystemState::NORMAL : SystemState::CRITICAL);
}

SystemState AlarmService::evaluate(const SensorData& data) {
    m_stateChanged = false;

    // En SAFE_SHUTDOWN, seul applyReset() peut changer l'état
    if (m_state == SystemState::SAFE_SHUTDOWN) {
        return m_state;
    }

    // En CRITICAL, on passe immédiatement à SAFE_SHUTDOWN
    if (m_state == SystemState::CRITICAL) {
        transition(SystemState::SAFE_SHUTDOWN);
        return m_state;
    }

    // Évaluation des seuils en NORMAL et WARNING
    if ((m_state == SystemState::NORMAL) || (m_state == SystemState::WARNING)) {
        const SystemState next = checkThresholds(data);
        if (next != m_state) {
            transition(next);
        }
    }

    return m_state;
}

void AlarmService::applyReset() {
    if (m_state == SystemState::SAFE_SHUTDOWN) {
        ESP_LOGI(TAG, "Reset manuel appliqué — retour à INIT");
        transition(SystemState::INIT);
    }
}

// =============================================================================
// Logique de seuils (privé)
// =============================================================================

SystemState AlarmService::checkThresholds(const SensorData& data) const {
    const float current = data.current_A;
    const float temp    = std::max(data.temp_tmp126, data.temp_ntc);

    // Seuils critiques
    if ((current >= AppConfig::CURRENT_CRITICAL_A) ||
        (temp    >= AppConfig::TEMP_CRITICAL_C)) {
        return SystemState::CRITICAL;
    }

    // Seuils avertissement
    if ((current >= AppConfig::CURRENT_WARNING_A) ||
        (temp    >= AppConfig::TEMP_WARNING_C)) {
        return SystemState::WARNING;
    }

    // Retour à NORMAL avec hystérésis (évite oscillations)
    if (m_state == SystemState::WARNING) {
        const bool currentOk = (current < (AppConfig::CURRENT_WARNING_A  - 0.5f));
        const bool tempOk    = (temp    < (AppConfig::TEMP_WARNING_C - AppConfig::TEMP_HYSTERESIS_C));
        if (currentOk && tempOk) {
            return SystemState::NORMAL;
        }
        return SystemState::WARNING; // Maintien
    }

    return SystemState::NORMAL;
}

void AlarmService::transition(SystemState next) {
    if (next == m_state) { return; }
    ESP_LOGW(TAG, "FSM : %s → %s", stateToStr(m_state), stateToStr(next));
    m_state        = next;
    m_stateChanged = true;
}
