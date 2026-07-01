#pragma once
#include <cstdint>
#include "../../config/CommonTypes.hpp"
#include "driver/i2c.h"

// =============================================================================
// I2CManager.hpp — Gestionnaire du bus I2C
// Couche MID : abstraction du bus partagé entre les drivers (INA237...)
// Les drivers HAL l'utilisent via référence, il n'y a qu'une instance.
// =============================================================================

class I2CManager {
public:
    I2CManager() = default;
    ~I2CManager();

    // Non-copiable (ressource hardware unique)
    I2CManager(const I2CManager&)            = delete;
    I2CManager& operator=(const I2CManager&) = delete;

    // Initialise le bus I2C (appel unique au démarrage)
    Status init();

    // Écriture d'un ou plusieurs octets vers un esclave
    Status write(uint8_t addr, const uint8_t* data, size_t len);

    // Lecture d'un ou plusieurs octets depuis un esclave
    Status read(uint8_t addr, uint8_t* buf, size_t len);

    // Écriture d'un registre puis lecture (opération atomique)
    Status writeReadReg(uint8_t addr, uint8_t reg,
                        uint8_t* buf, size_t len);

    bool isInitialized() const { return m_initialized; }

private:
    bool          m_initialized = false;
    i2c_port_t    m_port        = I2C_NUM_0;

    static constexpr uint32_t I2C_TIMEOUT_MS = 100U;
};
