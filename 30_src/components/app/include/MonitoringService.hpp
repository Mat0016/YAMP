#pragma once
#include "../../config/CommonTypes.hpp"

// Forward declarations
class INA237Driver;
class TMP126Driver;

// =============================================================================
// MonitoringService.hpp — Service de mesure (courant, tension, température)
//
// Couche APP — Logique Métier
// Interroge les drivers HAL et produit une structure SensorData cohérente.
// C'est le seul endroit où les capteurs sont lus.
// =============================================================================

class MonitoringService {
public:
    MonitoringService(INA237Driver& ina237,
                      TMP126Driver& tmp126);

    Status init();

    // Lit tous les capteurs et met à jour les données internes
    Status measure();

    // Accès aux dernières données mesurées
    const SensorData& getData() const { return m_data; }

    // Température maximale parmi toutes les sondes (TMP126 + NTC si dispo)
    float getMaxTemp_C() const;

private:
    INA237Driver& m_ina237;
    TMP126Driver& m_tmp126;
    SensorData    m_data  = {};
    bool          m_initialized = false;
};
