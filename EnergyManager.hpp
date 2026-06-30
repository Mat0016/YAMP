EnergyManager.hpp

#ifndef ENERGY_MANAGER_HPP
#define ENERGY_MANAGER_HPP

#include <stdint.h>

// Définitions des types requis par MISRA
using float32_t = float;

enum class Status : uint8_t {
    OK = 0U,
    ERROR
};

enum class AgvState : uint8_t {
    INIT = 0U,
    NORMAL,
    WARNING,
    CRITICAL,
    SAFE_SHUTDOWN
};

enum class HmiMode : uint8_t {
    SUCCESS_BEEP = 0U,
    LED_GREEN_ON,
    LED_ORANGE_BLINK_BUZZER_SLOW,
    LED_RED_ON_BUZZER_FAST
};

class EnergyManager {
public:
    EnergyManager();
    Status init();
    void process();

private:
    AgvState m_currentState;

    // Fonctions privées demandées
    void updateHmi();
    AgvState checkThresholds(float32_t current, float32_t ntcTemp, float32_t tmpTemp);
    void triggerSafetyShutdown();

    // Mock des drivers Mid-Level (à remplacer par tes vraies classes)
    struct PowerMonitor { Status configure() { return Status::OK; } float32_t getCurrent() { return 1.5f; } } m_powerMonitor;
    struct TempMonitor  { Status init() { return Status::OK; } float32_t getNtcTemp() { return 25.0f; } float32_t getTmpSensorTemp() { return 24.5f; } } m_tempMonitor;
    struct HmiController { void setNotification(HmiMode mode) { (void)mode; } bool isResetPressed() { return false; } } m_hmi;
};

#endif // ENERGY_MANAGER_HPP