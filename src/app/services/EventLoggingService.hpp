#pragma once
#include <Arduino.h>
#include <esp_log.h>
#include "config/SystemConfig.hpp"
#include "mid/memory/RingBuffer.hpp"
#include "MonitoringService.hpp"

// ============================================================
//  EventLoggingService.hpp — Couche APP / services
//  Journalise les transitions d'état et les mesures en PSRAM.
// ============================================================

class EventLoggingService {
public:
    explicit EventLoggingService(RingBuffer& ring) : m_ring(ring) {}

    // Ajoute une entrée au log
    void log(AgvState state, const Measurement& m, const char* msg);

    // Dump des N dernières entrées via ESP_LOG
    void dumpLast(size_t n) const;

    size_t count() const { return m_ring.count(); }

private:
    RingBuffer& m_ring;
};
