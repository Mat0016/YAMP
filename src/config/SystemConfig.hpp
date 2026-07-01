#pragma once
#include <stdint.h>

// ============================================================
//  SystemConfig.hpp
//  Types partagés par toutes les couches (Status, états FSM…).
//  Aucune dépendance Arduino ici : fichier testable sur PC.
// ============================================================

using float32_t = float;

// --- Code de retour générique --------------------------------
enum class Status : uint8_t {
    OK    = 0U,
    ERROR = 1U
};

// --- États de la machine à états (FSM) -----------------------
enum class AgvState : uint8_t {
    INIT          = 0U,
    NORMAL        = 1U,
    WARNING       = 2U,
    CRITICAL      = 3U,
    SAFE_SHUTDOWN = 4U
};

// --- Modes IHM (LED + Buzzer) --------------------------------
enum class HmiMode : uint8_t {
    OFF                         = 0U,
    SUCCESS_BEEP                = 1U, // Bip court d'init OK
    LED_GREEN                   = 2U, // Normal
    LED_GREEN_BLINK             = 3U, // Réservé
    LED_ORANGE_BUZZER_SLOW      = 4U, // Warning : LED rouge+verte, buzzer lent
    LED_RED_BUZZER_CRITICAL     = 5U  // Critical : LED rouge fixe, buzzer critique
};

// --- Entrée du log d'événement (stockée en PSRAM) ------------
struct EventEntry {
    uint32_t  timestamp_ms;
    AgvState  state;
    float32_t temperature_degC;
    float32_t current_A;
    float32_t voltage_V;
    char      message[40];
};
