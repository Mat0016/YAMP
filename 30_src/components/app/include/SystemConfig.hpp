#pragma once
#include <cstdint>

// =============================================================================
// SystemConfig.hpp — Configuration globale de l'application
// =============================================================================

namespace SystemConfig {

    constexpr char     APP_NAME[]          = "VentecMonitoring";
    constexpr char     APP_VERSION[]       = "1.0.0";

    constexpr bool     ENABLE_BLE          = true;
    constexpr bool     ENABLE_LOGGING      = true;
    constexpr bool     ENABLE_WATCHDOG     = true;

    constexpr uint32_t WATCHDOG_TIMEOUT_MS = 10000U;   // 10 s

    // Stack sizes FreeRTOS (mots 32 bits)
    constexpr uint32_t STACK_MAIN_TASK     = 4096U;
    constexpr uint32_t STACK_BLE_TASK      = 4096U;
    constexpr uint32_t STACK_LOG_TASK      = 2048U;

    // Priorités FreeRTOS
    constexpr uint8_t  PRIO_MAIN_TASK      = 5U;
    constexpr uint8_t  PRIO_BLE_TASK       = 3U;
    constexpr uint8_t  PRIO_LOG_TASK       = 2U;

} // namespace SystemConfig
