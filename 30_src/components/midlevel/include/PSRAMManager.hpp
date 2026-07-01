#pragma once
#include <cstdint>
#include <cstddef>
#include "../../../config/CommonTypes.hpp"
#include "../../../config/AppConfig.hpp"
#include "RingBuffer.hpp"
#include "esp_heap_caps.h"

// =============================================================================
// PSRAMManager.hpp — Gestionnaire de la PSRAM (mémoire externe)
//
// La PSRAM (SPIRAM) de l'ESP32 permet de stocker un grand nombre de logs
// sans surcharger la SRAM interne (limitée à ~300 KB).
//
// Rôle :
//   - Allouer un buffer circulaire de LogEntry en PSRAM
//   - Fournir push / pop pour EventLoggingService
//   - Gérer l'état d'occupation et les statistiques
//
// Prérequis sdkconfig :
//   CONFIG_ESP32_SPIRAM_SUPPORT=y
//   CONFIG_SPIRAM_USE_MALLOC=y  (ou CAPS_MALLOC)
// =============================================================================

class PSRAMManager {
public:
    PSRAMManager() = default;

    // Initialise la PSRAM et alloue le buffer de logs
    Status init();

    // Pousse une entrée de log en PSRAM (écrase le plus ancien si plein)
    void   pushLog(const LogEntry& entry);

    // Retire l'entrée la plus ancienne
    bool   popLog(LogEntry& out);

    // Statistiques
    std::size_t logCount()       const { return m_logCount;                      }
    std::size_t logCapacity()    const { return AppConfig::EVENT_LOG_MAX_ENTRIES; }
    bool        isPsramPresent() const { return m_psramAvailable;                }
    std::size_t psramFreeBytes() const;

    bool isInitialized() const { return m_initialized; }

private:
    bool        m_initialized    = false;
    bool        m_psramAvailable = false;
    std::size_t m_logCount       = 0U;

    // Buffer alloué dynamiquement en PSRAM
    LogEntry*   m_logBuffer      = nullptr;
    std::size_t m_head           = 0U;
    std::size_t m_tail           = 0U;
    bool        m_full           = false;

    static constexpr std::size_t LOG_CAPACITY = AppConfig::EVENT_LOG_MAX_ENTRIES;
};
