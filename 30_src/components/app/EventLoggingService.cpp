#include "EventLoggingService.hpp"
#include "../../low/hal/memory/PSRAMManager.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstdio>
#include <cstring>

static const char* TAG = "EventLogging";

EventLoggingService::EventLoggingService(PSRAMManager& psram)
    : m_psram(psram)
{}

Status EventLoggingService::init() {
    if (!m_psram.isInitialized()) {
        ESP_LOGE(TAG, "PSRAMManager non initialisé");
        return Status::ERROR;
    }
    m_initialized = true;
    ESP_LOGI(TAG, "EventLoggingService init OK");
    return Status::OK;
}

void EventLoggingService::logEvent(SystemState state,
                                   const SensorData& data,
                                   const char* message) {
    if (!m_initialized) { return; }

    LogEntry entry = {};
    entry.timestamp_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
    entry.state        = state;
    entry.current_A    = data.current_A;
    entry.temp_C       = std::max(data.temp_tmp126, data.temp_ntc);

    if (message != nullptr) {
        (void)snprintf(entry.message, sizeof(entry.message), "%s", message);
    } else {
        static const char* stateNames[] = {
            "INIT", "NORMAL", "WARNING", "CRITICAL", "SAFE_SHUTDOWN"
        };
        const uint8_t idx = static_cast<uint8_t>(state);
        (void)snprintf(entry.message, sizeof(entry.message),
                       "[%lu] State=%s I=%.2fA T=%.1fC",
                       entry.timestamp_ms,
                       (idx < 5U) ? stateNames[idx] : "?",
                       entry.current_A,
                       entry.temp_C);
    }

    m_psram.pushLog(entry);
    ++m_eventIndex;

    ESP_LOGI(TAG, "LOG[%lu]: %s", m_eventIndex, entry.message);
}

std::size_t EventLoggingService::getLogCount() const {
    return m_psram.logCount();
}

void EventLoggingService::clearLogs() {
    // Vide le buffer en poppant toutes les entrées
    LogEntry dummy = {};
    while (m_psram.popLog(dummy)) {}
    ESP_LOGI(TAG, "Logs effacés");
}
