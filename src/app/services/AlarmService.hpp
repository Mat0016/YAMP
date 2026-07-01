#pragma once
#include "config/SystemConfig.hpp"
#include "config/AppConfig.hpp"
#include "MonitoringService.hpp"

// ============================================================
//  AlarmService.hpp — Couche APP / services
//
//  Logique PURE de calcul du prochain état FSM à partir des
//  mesures et de l'état courant.
//  Aucune E/S : testable sur PC (unit tests).
//
//  Seuils configurables à l'exécution (chargés depuis FlashStorage).
// ============================================================

class AlarmService {
public:
    // Seuils modifiables (chargés depuis FlashStorage au démarrage)
    float32_t tempWarning_degC  = TEMP_WARNING_DEGC;
    float32_t tempCritical_degC = TEMP_CRITICAL_DEGC;
    float32_t currentCritical_A = CURRENT_CRITICAL_A;
    float32_t hysteresis        = TEMP_HYSTERESIS;

    // Calcule le prochain état FSM
    // currentState : état courant de la FSM
    // m            : mesures acquises
    AgvState evaluate(AgvState currentState, const Measurement& m) const;

    // Vrai si les deux capteurs ont retourné des données valides
    static bool isMeasurementValid(const Measurement& m) { return m.valid; }
};
