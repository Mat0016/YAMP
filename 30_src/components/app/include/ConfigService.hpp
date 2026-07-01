#pragma once
#include "../../config/CommonTypes.hpp"
#include "../../low/hal/memory/FlashStorage.hpp"

// =============================================================================
// ConfigService.hpp — Gestion de la configuration (lecture / écriture Flash)
// =============================================================================

class ConfigService {
public:
    explicit ConfigService(FlashStorage& flash);

    Status init();

    // Charge depuis Flash (ou valeurs par défaut si absent/corrompu)
    const StoredConfig& getConfig() const { return m_config; }

    // Sauvegarde la configuration en Flash
    Status saveConfig(const StoredConfig& cfg);

    // Remet les valeurs par défaut sans écrire en Flash
    void resetToDefaults();

private:
    FlashStorage& m_flash;
    StoredConfig  m_config       = {};
    bool          m_initialized  = false;
};
