#pragma once
#include "config/SystemConfig.hpp"
#include "mid/hal/sensors/INA237Driver.hpp"
#include "mid/hal/sensors/TMP126Driver.hpp"

// ============================================================
//  MonitoringService.hpp — Couche APP / services
//
//  Agrège les lectures de l'INA237 (I2C) et du TMP126 (SPI).
//  Résultat exposé via la structure Measurement.
// ============================================================

struct Measurement {
    float32_t temperature_degC = 0.0F;
    float32_t current_A        = 0.0F;
    float32_t voltage_V        = 0.0F;
    float32_t power_W          = 0.0F;
    bool      valid            = false;
};

class MonitoringService {
public:
    MonitoringService(INA237Driver& ina, TMP126Driver& tmp)
        : m_ina(ina), m_tmp(tmp) {}

    Status init();

    // Lit tous les capteurs et retourne la mesure agrégée
    Measurement acquire();

private:
    INA237Driver& m_ina;
    TMP126Driver& m_tmp;
};
