#pragma once
#include "../../config/CommonTypes.hpp"
#include "../../config/AppConfig.hpp"

// =============================================================================
// AlarmService.hpp — Détection des alarmes et machine d'états (FSM)
//
// C'est le cœur de la logique métier.
// Reçoit les SensorData, calcule l'état système et gère les transitions.
//
// Règles de transition :
//   INIT       → NORMAL        : si init() OK
//   INIT       → CRITICAL      : si init() ERROR
//   NORMAL     → WARNING       : courant > CURRENT_WARNING ou temp > TEMP_WARNING
//   NORMAL     → CRITICAL      : courant > CURRENT_CRITICAL ou temp > TEMP_CRITICAL
//   WARNING    → NORMAL        : courant ET temp sous seuil - hystérésis
//   WARNING    → CRITICAL      : dépassement seuil critique
//   CRITICAL   → SAFE_SHUTDOWN : immédiat (une seule transition)
//   SAFE_SHUTDOWN → INIT       : reset bouton physique uniquement
// =============================================================================

class AlarmService {
public:
    AlarmService();

    void setInitSuccess(bool success);

    // Évalue les seuils et retourne le prochain état
    SystemState evaluate(const SensorData& data);

    // Retourne l'état courant
    SystemState getState() const { return m_state; }

    // Applique le reset (bouton physique — sort de SAFE_SHUTDOWN)
    void applyReset();

    // Indique si un changement d'état vient d'avoir lieu
    bool stateChanged() const { return m_stateChanged; }

private:
    SystemState m_state        = SystemState::INIT;
    bool        m_stateChanged = false;

    SystemState checkThresholds(const SensorData& data) const;
    void        transition(SystemState next);
};
