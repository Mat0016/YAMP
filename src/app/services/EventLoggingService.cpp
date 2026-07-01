#include "EventLoggingService.hpp"
#include <string.h>

static const char* TAG = "EventLog";

void EventLoggingService::log(AgvState state, const Measurement& m, const char* msg) {
    EventEntry e;
    e.timestamp_ms    = millis();
    e.state           = state;
    e.temperature_degC = m.temperature_degC;
    e.current_A       = m.current_A;
    e.voltage_V       = m.voltage_V;
    strncpy(e.message, msg, sizeof(e.message) - 1U);
    e.message[sizeof(e.message) - 1U] = '\0';

    m_ring.push(e);
    ESP_LOGI(TAG, "[%lu ms] state=%d  T=%.1f  I=%.3f  V=%.2f  %s",
             (unsigned long)e.timestamp_ms, static_cast<int>(state),
             static_cast<double>(m.temperature_degC),
             static_cast<double>(m.current_A),
             static_cast<double>(m.voltage_V),
             msg);
}

void EventLoggingService::dumpLast(size_t n) const {
    size_t total = m_ring.count();
    size_t start = (total > n) ? (total - n) : 0U;

    ESP_LOGI(TAG, "--- Dump %u dernieres entrees ---", (unsigned)n);
    for (size_t i = start; i < total; ++i) {
        EventEntry e;
        if (m_ring.get(i, e)) {
            ESP_LOGI(TAG, "[%lu] st=%d T=%.1f I=%.3f V=%.2f | %s",
                     (unsigned long)e.timestamp_ms,
                     static_cast<int>(e.state),
                     static_cast<double>(e.temperature_degC),
                     static_cast<double>(e.current_A),
                     static_cast<double>(e.voltage_V),
                     e.message);
        }
    }
}
