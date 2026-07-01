#pragma once
#include <cstdint>
#include <cstddef>

// =============================================================================
// AppConfig.hpp — Seuils métier, fréquences buzzer, périodes de scrutation
// =============================================================================

namespace AppConfig {

    // -------------------------------------------------------------------------
    // Seuils de courant (INA237)
    // -------------------------------------------------------------------------
    constexpr float CURRENT_WARNING_A   = 8.0f;    // Ampères — niveau Avertissement
    constexpr float CURRENT_CRITICAL_A  = 10.0f;   // Ampères — niveau Critique

    // -------------------------------------------------------------------------
    // Seuils de température (TMP126 + NTC fusionnés, on prend le max)
    // -------------------------------------------------------------------------
    constexpr float TEMP_WARNING_C      = 50.0f;   // °C — niveau Avertissement
    constexpr float TEMP_CRITICAL_C     = 65.0f;   // °C — niveau Critique
    constexpr float TEMP_HYSTERESIS_C   = 5.0f;    // Hystérésis retour à NORMAL

    // -------------------------------------------------------------------------
    // Buzzer — fréquences LEDC (Hz)
    // -------------------------------------------------------------------------
    constexpr uint32_t BUZZER_FREQ_WARNING_HZ   = 500U;    // Ton grave — lent
    constexpr uint32_t BUZZER_FREQ_CRITICAL_HZ  = 2000U;   // Ton aigu — urgence

    // -------------------------------------------------------------------------
    // Buzzer — timings (ms)
    // WARNING : intermittent lent
    // -------------------------------------------------------------------------
    constexpr uint32_t BUZZER_WARN_ON_MS         = 500U;
    constexpr uint32_t BUZZER_WARN_OFF_MS        = 1000U;

    // CRITICAL : 2 sec continu PUIS intermittent rapide jusqu'au reset manuel
    constexpr uint32_t BUZZER_CRIT_CONTINUOUS_MS = 2000U;  // Phase 1 : continu
    constexpr uint32_t BUZZER_CRIT_ON_MS         = 300U;   // Phase 2 : bip ON
    constexpr uint32_t BUZZER_CRIT_OFF_MS        = 300U;   // Phase 2 : bip OFF

    // -------------------------------------------------------------------------
    // Périodes boucle (ms)
    // -------------------------------------------------------------------------
    constexpr uint32_t MONITORING_PERIOD_MS     = 100U;    // Scrutation capteurs
    constexpr uint32_t LOG_FLUSH_PERIOD_MS      = 5000U;   // Flush PSRAM → Flash

    // -------------------------------------------------------------------------
    // PSRAM / Logs
    // -------------------------------------------------------------------------
    constexpr std::size_t EVENT_LOG_MAX_ENTRIES  = 1000U;  // Entrées en PSRAM
    constexpr std::size_t LOG_ENTRY_MAX_LEN      = 128U;   // Caractères par log

} // namespace AppConfig
