#include "AlarmService.hpp"

AgvState AlarmService::evaluate(AgvState currentState, const Measurement& m) const {
    AgvState next = currentState;

    // --- Cas 1 : seuils critiques (température OU surintensité) ---
    if ((m.temperature_degC >= tempCritical_degC) || (m.current_A >= currentCritical_A)) {
        next = AgvState::CRITICAL;
    }
    // --- Cas 2 : seuil warning ---
    else if (m.temperature_degC >= tempWarning_degC) {
        next = AgvState::WARNING;
    }
    // --- Cas 3 : retour NORMAL avec hystérésis ---
    else if (currentState == AgvState::WARNING) {
        // On ne repasse NORMAL que si la temp redescend sous (seuil - hystérésis)
        if (m.temperature_degC < (tempWarning_degC - hysteresis)) {
            next = AgvState::NORMAL;
        }
        // Sinon on reste en WARNING (pas d'oscillation)
    }
    else {
        next = AgvState::NORMAL;
    }

    return next;
}
