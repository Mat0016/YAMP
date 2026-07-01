#pragma once
#include <cstdint>

// =============================================================================
// CommonTypes.hpp — Types partagés entre toutes les couches
// =============================================================================

// Statut générique de retour (inspiré MISRA / AUTOSAR)
enum class Status : uint8_t {
    OK    = 0U,
    ERROR = 1U,
};

// États de la machine d'état principale (FSM)
enum class SystemState : uint8_t {
    INIT          = 0U,  // Démarrage / init des périphériques
    NORMAL        = 1U,  // Fonctionnement nominal — LED verte
    WARNING       = 2U,  // Seuil d'alerte dépassé — LED rouge clignotante + buzzer lent
    CRITICAL      = 3U,  // Seuil critique — LED rouge fixe + buzzer continu puis intermittent
    SAFE_SHUTDOWN = 4U,  // Système figé en sécurité — attend reset manuel
};

// Données mesurées par les capteurs (transmises entre couches)
struct SensorData {
    float   current_A   = 0.0f;
    float   voltage_V   = 0.0f;
    float   power_W     = 0.0f;
    float   temp_tmp126 = 0.0f;   // TMP126 (numérique SPI)
    float   temp_ntc    = 0.0f;   // Capteur NTC (analogique — non utilisé ici)
    bool    valid       = false;
};

// Entrée de log événement
struct LogEntry {
    uint32_t timestamp_ms = 0U;
    SystemState state     = SystemState::INIT;
    float current_A       = 0.0f;
    float temp_C          = 0.0f;
    char  message[64]     = {};
};
