#include "PSRAMManager.hpp"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include <cstring>

static const char* TAG = "PSRAMManager";

// =============================================================================
// PSRAMManager.cpp
// =============================================================================

Status PSRAMManager::init() {
    ESP_LOGI(TAG, "=== INIT PSRAM Manager ===");

    // -------------------------------------------------------------------------
    // Vérification de la présence de la PSRAM
    // -------------------------------------------------------------------------
    const size_t psramTotal = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    if (psramTotal == 0U) {
        ESP_LOGW(TAG, "PSRAM non détectée — logs stockés en RAM interne (capacité réduite)");
        m_psramAvailable = false;
        // Fallback : allocation en RAM interne (capacité réduite)
        m_logBuffer = static_cast<LogEntry*>(
            heap_caps_malloc(LOG_CAPACITY * sizeof(LogEntry), MALLOC_CAP_8BIT));
    } else {
        ESP_LOGI(TAG, "PSRAM détectée : %zu octets disponibles", psramTotal);
        m_psramAvailable = true;
        // Allocation en PSRAM
        m_logBuffer = static_cast<LogEntry*>(
            heap_caps_malloc(LOG_CAPACITY * sizeof(LogEntry), MALLOC_CAP_SPIRAM));
    }

    if (m_logBuffer == nullptr) {
        ESP_LOGE(TAG, "Allocation du buffer de logs FAILED (%zu × %zu octets)",
                 LOG_CAPACITY, sizeof(LogEntry));
        return Status::ERROR;
    }

    // Mise à zéro du buffer
    (void)memset(m_logBuffer, 0, LOG_CAPACITY * sizeof(LogEntry));

    m_head        = 0U;
    m_tail        = 0U;
    m_full        = false;
    m_logCount    = 0U;
    m_initialized = true;

    ESP_LOGI(TAG, "Buffer logs alloué : %zu entrées × %zu octets = %zu octets (%s)",
             LOG_CAPACITY, sizeof(LogEntry),
             LOG_CAPACITY * sizeof(LogEntry),
             m_psramAvailable ? "PSRAM" : "SRAM interne");
    ESP_LOGI(TAG, "=== PSRAMManager INIT COMPLETE ===");
    return Status::OK;
}

void PSRAMManager::pushLog(const LogEntry& entry) {
    if (m_logBuffer == nullptr) { return; }

    m_logBuffer[m_head] = entry;
    m_head = (m_head + 1U) % LOG_CAPACITY;

    if (m_full) {
        // Écrase le plus ancien
        m_tail = (m_tail + 1U) % LOG_CAPACITY;
    } else {
        ++m_logCount;
    }

    m_full = (m_head == m_tail);
}

bool PSRAMManager::popLog(LogEntry& out) {
    if (m_logBuffer == nullptr || (m_logCount == 0U && !m_full)) {
        return false;
    }

    out    = m_logBuffer[m_tail];
    m_tail = (m_tail + 1U) % LOG_CAPACITY;
    m_full = false;

    if (m_logCount > 0U) {
        --m_logCount;
    }
    return true;
}

std::size_t PSRAMManager::psramFreeBytes() const {
    return heap_caps_get_free_size(
        m_psramAvailable ? MALLOC_CAP_SPIRAM : MALLOC_CAP_8BIT);
}
