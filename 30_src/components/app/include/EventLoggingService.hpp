#pragma once
#include "../../config/CommonTypes.hpp"

class PSRAMManager;

// =============================================================================
// EventLoggingService.hpp — Enregistrement des événements en PSRAM
//
// Logue les changements d'état et les dépassements de seuils.
// Les entrées sont stockées dans le buffer PSRAM via PSRAMManager.
// =============================================================================

class EventLoggingService {
public:
    explicit EventLoggingService(PSRAMManager& psram);

    Status init();

    // Logue un événement avec les données courantes
    void logEvent(SystemState state,
                  const SensorData& data,
                  const char* message = nullptr);

    // Retourne le nombre d'entrées en PSRAM
    std::size_t getLogCount() const;

    // Vide les logs (flush ou effacement)
    void clearLogs();

private:
    PSRAMManager& m_psram;
    bool          m_initialized = false;
    uint32_t      m_eventIndex  = 0U;
};
