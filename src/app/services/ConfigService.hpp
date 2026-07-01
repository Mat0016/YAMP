#pragma once
#include "config/SystemConfig.hpp"
#include "mid/memory/FlashStorage.hpp"
#include "AlarmService.hpp"

// ============================================================
//  ConfigService.hpp — Couche APP / services
//  Charge et sauvegarde les seuils d'alarme depuis la flash.
// ============================================================

class ConfigService {
public:
    explicit ConfigService(FlashStorage& flash, AlarmService& alarm)
        : m_flash(flash), m_alarm(alarm) {}

    Status init();

    // Charge les seuils depuis la flash vers AlarmService
    void loadThresholds();

    // Sauvegarde les seuils courants d'AlarmService en flash
    void saveThresholds();

    void factoryReset();

private:
    FlashStorage& m_flash;
    AlarmService& m_alarm;
};
