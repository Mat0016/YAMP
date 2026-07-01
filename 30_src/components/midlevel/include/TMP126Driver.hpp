#pragma once
#include <cstdint>
#include "../../../config/CommonTypes.hpp"
#include "driver/spi_master.h"
#include "driver/gpio.h"

// =============================================================================
// TMP126Driver.hpp — Pilote du capteur de température numérique TMP126 (SPI)
// Référence : Texas Instruments SBOS811
//
// Interface  : SPI Mode 1 (CPOL=0, CPHA=1) — lecture seule
// Résolution : 14 bits — 1 LSB = 0.03125 °C (1/32 °C)
// Plage      : -40 °C à +125 °C
// CS actif   : bas (actif low)
// =============================================================================

// ----------------------------------------------------------------------------
// Format du mot de données TMP126 (16 bits reçus sur MISO)
// Bit 15    : NRDY — 0 = donnée prête, 1 = conversion en cours
// Bit 14    : AL   — Alarm bit
// Bit 13    : CR   — Configuration Register (ignoré en lecture simple)
// Bits [12:1] : Température sur 12 bits signés + résolution 0.03125°C
// Bit 0     : Bit de signe étendu (pour température négative)
//
// Note : le TMP126 sort en réalité 14 bits de température [13:0] après
// décalage du mot 16 bits de 2 positions vers la droite.
// ----------------------------------------------------------------------------

namespace TMP126Bits {
    constexpr uint16_t NRDY_MASK  = 0x8000U;  // Bit 15 : donnée non prête
    constexpr uint16_t ALARM_MASK = 0x4000U;  // Bit 14 : alarme
    constexpr float    TEMP_LSB_C = 0.03125f; // 1 LSB = 1/32 °C
    constexpr int16_t  TEMP_SHIFT = 2;        // Décalage pour obtenir les 14 bits
}

class TMP126Driver {
public:
    TMP126Driver() = default;
    ~TMP126Driver();

    // Non-copiable (ressource SPI unique)
    TMP126Driver(const TMP126Driver&)            = delete;
    TMP126Driver& operator=(const TMP126Driver&) = delete;

    // =========================================================================
    // INIT TMP126 — Initialisation du bus SPI et du device handle
    // Le TMP126 n'a pas de registre de configuration à écrire (read-only).
    // L'init consiste à :
    //   1. Configurer le bus SPI (si pas déjà fait)
    //   2. Ajouter le device TMP126 sur ce bus
    //   3. Effectuer une lecture de validation (NRDY bit doit être 0)
    // =========================================================================
    Status init();

    // -------------------------------------------------------------------------
    // Lecture de la température en degrés Celsius
    // -------------------------------------------------------------------------
    float  getTemp_C() const;

    // Indique si la dernière lecture a déclenché une alarme interne
    bool   isAlarmActive() const { return m_lastAlarm; }

    bool   isInitialized() const { return m_initialized; }

private:
    spi_device_handle_t m_spi        = nullptr;
    bool                m_initialized = false;
    mutable bool        m_lastAlarm   = false;

    // Lecture brute du mot 16 bits SPI
    Status    readRaw(uint16_t& out) const;
};
