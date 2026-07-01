#pragma once
#include <cstdint>
#include <cstring>
#include "../../../config/CommonTypes.hpp"
#include "nvs_flash.h"
#include "nvs.h"

// =============================================================================
// FlashStorage.hpp — Stockage de la configuration en Flash NVS (Non-Volatile)
//
// Utilise l'API NVS (Non-Volatile Storage) d'ESP-IDF.
// Permet de persister la config même après redémarrage.
// =============================================================================

struct StoredConfig {
    float    currentWarningA  = 8.0f;
    float    currentCriticalA = 10.0f;
    float    tempWarningC     = 50.0f;
    float    tempCriticalC    = 65.0f;
    uint32_t checksum         = 0U;   // CRC32 simple pour validation
};

class FlashStorage {
public:
    FlashStorage() = default;

    Status init();

    // Sauvegarde la configuration en Flash
    Status saveConfig(const StoredConfig& cfg);

    // Charge la configuration depuis la Flash
    // Retourne Status::ERROR si la Flash est vide ou corrompue → utiliser défauts
    Status loadConfig(StoredConfig& out);

    // Efface la configuration (retour aux valeurs par défaut)
    Status eraseConfig();

    bool isInitialized() const { return m_initialized; }

private:
    bool        m_initialized = false;
    nvs_handle_t m_handle     = 0U;

    static constexpr char NVS_KEY_CONFIG[] = "sys_config";

    uint32_t computeChecksum(const StoredConfig& cfg) const;
};
