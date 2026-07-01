#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <esp_log.h>
#include "PSRAMManager.hpp"
#include "config/SystemConfig.hpp"
#include "config/AppConfig.hpp"

// ============================================================
//  RingBuffer.hpp — Couche LOW / memory
//
//  Buffer circulaire de RING_BUFFER_SIZE entrées EventEntry,
//  alloué en PSRAM via PSRAMManager.
//
//  Usage :
//    RingBuffer log;
//    log.init();
//    log.push({millis(), AgvState::WARNING, 52.3f, 1.2f, 24.0f, "Temp haute"});
//    EventEntry e; log.peek(e);
// ============================================================

class RingBuffer {
public:
    bool init() {
        m_buffer = static_cast<EventEntry*>(
            PSRAMManager::alloc(RING_BUFFER_SIZE * sizeof(EventEntry))
        );
        if (m_buffer == nullptr) {
            ESP_LOGE("RingBuffer", "Impossible d'allouer le ring buffer en PSRAM");
            return false;
        }
        memset(m_buffer, 0, RING_BUFFER_SIZE * sizeof(EventEntry));
        m_head  = 0U;
        m_count = 0U;
        ESP_LOGI("RingBuffer", "Ring buffer alloue en PSRAM (%u entrees)", (unsigned)RING_BUFFER_SIZE);
        return true;
    }

    void push(const EventEntry& entry) {
        if (m_buffer == nullptr) { return; }
        m_buffer[m_head] = entry;
        m_head = (m_head + 1U) % RING_BUFFER_SIZE;
        if (m_count < RING_BUFFER_SIZE) { ++m_count; }
    }

    // Lit la dernière entrée sans la retirer
    bool peek(EventEntry& out) const {
        if (m_count == 0U || m_buffer == nullptr) { return false; }
        size_t last = (m_head + RING_BUFFER_SIZE - 1U) % RING_BUFFER_SIZE;
        out = m_buffer[last];
        return true;
    }

    // Accès indexé (0 = plus ancien)
    bool get(size_t index, EventEntry& out) const {
        if (index >= m_count || m_buffer == nullptr) { return false; }
        size_t realIdx = (m_head + RING_BUFFER_SIZE - m_count + index) % RING_BUFFER_SIZE;
        out = m_buffer[realIdx];
        return true;
    }

    size_t count()    const { return m_count; }
    bool   isEmpty()  const { return m_count == 0U; }
    bool   isFull()   const { return m_count == RING_BUFFER_SIZE; }

    void reset() {
        m_head  = 0U;
        m_count = 0U;
    }

private:
    EventEntry* m_buffer = nullptr;
    size_t      m_head   = 0U;
    size_t      m_count  = 0U;
};
