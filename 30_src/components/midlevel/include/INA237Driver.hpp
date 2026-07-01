#pragma once
#include <cstdint>
#include "../../../config/CommonTypes.hpp"

// Forward declaration — évite l'inclusion circulaire
class I2CManager;

// =============================================================================
// INA237Driver.hpp — Pilote du capteur de courant / tension INA237 (I2C)
// Référence : Texas Instruments SBOS924 / SBOA379
//
// Adresse I2C : dépend des pins A0/A1 (cf. HardwareConfig.hpp)
// Interface   : I2C jusqu'à 400 kHz (Fast Mode)
// Résolution  : 16 bits (courant/tension), 12 bits (température die)
// =============================================================================

// ----------------------------------------------------------------------------
// Carte des registres INA237 (adresses 8 bits)
// ----------------------------------------------------------------------------
namespace INA237Reg {
    constexpr uint8_t CONFIG          = 0x00U;  // Configuration générale
    constexpr uint8_t ADC_CONFIG      = 0x01U;  // Configuration ADC (mode, oversampling)
    constexpr uint8_t SHUNT_CAL       = 0x02U;  // Calibration shunt (SHUNT_CAL)
    constexpr uint8_t SHUNT_TEMPCO    = 0x03U;  // Coeff. tempér. shunt (optionnel)
    constexpr uint8_t VSHUNT          = 0x04U;  // Tension shunt (signé, 16 bits)
    constexpr uint8_t VBUS            = 0x05U;  // Tension bus (non signé)
    constexpr uint8_t DIETEMP         = 0x06U;  // Température interne du die
    constexpr uint8_t CURRENT         = 0x07U;  // Courant calculé (signé)
    constexpr uint8_t POWER           = 0x08U;  // Puissance calculée
    constexpr uint8_t DIAG_ALRT       = 0x0BU;  // Diagnostics & alertes
    constexpr uint8_t SOVL            = 0x0CU;  // Seuil overvoltage shunt
    constexpr uint8_t SUVL            = 0x0DU;  // Seuil undervoltage shunt
    constexpr uint8_t BOVL            = 0x0EU;  // Seuil overvoltage bus
    constexpr uint8_t BUVL            = 0x0FU;  // Seuil undervoltage bus
    constexpr uint8_t TEMP_LIMIT      = 0x10U;  // Seuil température die
    constexpr uint8_t PWR_LIMIT       = 0x11U;  // Seuil puissance
    constexpr uint8_t MANUFACTURER_ID = 0x3EU;  // Doit lire 0x5449 ("TI")
    constexpr uint8_t DEVICE_ID       = 0x3FU;  // Doit lire 0x2381
}

// ----------------------------------------------------------------------------
// Valeurs CONFIG register (0x00)
// Bit 4 : ADCRANGE — 0=±163.84 mV, 1=±40.96 mV (résolution accrue)
// ----------------------------------------------------------------------------
namespace INA237ConfigBits {
    constexpr uint16_t RESET          = 0x8000U; // Reset logiciel (auto-clear)
    constexpr uint16_t ADCRANGE_163MV = 0x0000U; // Gamme shunt large
    constexpr uint16_t ADCRANGE_40MV  = 0x0010U; // Gamme shunt étroite (précision)
}

// ----------------------------------------------------------------------------
// Valeurs ADC_CONFIG register (0x01)
// Bits [15:12] MODE, [11:9] VBUSCT, [8:6] VSHCT, [5:3] VTCT, [2:0] AVG
// ----------------------------------------------------------------------------
namespace INA237ADCConfigBits {
    // Mode continu sur toutes les mesures (shunt + bus + température)
    constexpr uint16_t MODE_CONT_ALL   = 0xF000U;
    // Temps de conversion 1052 µs pour bus et shunt
    constexpr uint16_t VBUSCT_1052US   = 0x0200U;
    constexpr uint16_t VSHCT_1052US    = 0x0040U;
    constexpr uint16_t VTCT_1052US     = 0x0008U;
    // Moyenne sur 16 échantillons
    constexpr uint16_t AVG_16          = 0x0004U;
    // Configuration par défaut recommandée
    constexpr uint16_t DEFAULT         = MODE_CONT_ALL | VBUSCT_1052US |
                                         VSHCT_1052US  | VTCT_1052US   | AVG_16;
}

// IDs attendus pour validation de présence
namespace INA237IDs {
    constexpr uint16_t MANUFACTURER    = 0x5449U; // "TI"
    constexpr uint16_t DEVICE          = 0x2381U;
}


class INA237Driver {
public:
    // -------------------------------------------------------------------------
    // Construction avec le gestionnaire I2C et l'adresse du composant
    // -------------------------------------------------------------------------
    explicit INA237Driver(I2CManager& i2c,
                          uint8_t address,
                          float   shuntOhm,
                          float   maxCurrentA);

    // =========================================================================
    // INIT — Configuration complète des registres de l'INA237
    // Doit être appelé UNE FOIS avant tout appel à getCurrent/getVoltage.
    // =========================================================================
    Status init();

    // -------------------------------------------------------------------------
    // Lectures — disponibles uniquement après init() == Status::OK
    // -------------------------------------------------------------------------
    float  getCurrent_A()  const;    // Courant en Ampères
    float  getVoltage_V()  const;    // Tension bus en Volts
    float  getPower_W()    const;    // Puissance en Watts
    float  getDieTemp_C()  const;    // Température die interne (°C)

    bool   isInitialized() const { return m_initialized; }

private:
    I2CManager& m_i2c;
    uint8_t     m_addr;
    float       m_shuntOhm;
    float       m_maxCurrentA;
    float       m_currentLSB;        // LSB courant calculé à l'init
    bool        m_initialized = false;

    // Écriture / lecture registre 16 bits (big-endian sur le bus)
    Status    writeReg16(uint8_t reg, uint16_t value) const;
    Status    readReg16(uint8_t reg, uint16_t& out)   const;
    int16_t   readSigned16(uint8_t reg)               const;

    // Validation de l'ID du composant (anti-câblage erroné)
    Status    verifyDeviceId() const;
};
