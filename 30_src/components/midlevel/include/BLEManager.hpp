#pragma once
#include <cstdint>
#include <cstring>
#include "../../config/CommonTypes.hpp"

// =============================================================================
// BLEManager.hpp — Gestionnaire Bluetooth Low Energy
//
// Expose les données de monitoring (courant, tension, temp, état FSM)
// via des caractéristiques GATT en lecture.
// Un client BLE (smartphone / PC) peut s'abonner aux notifications.
//
// UUIDs personnalisés pour le service VentecMonitoring :
//   Service    : 0x181A (Environmental Sensing — standard BLE)
//   Char État  : UUID custom
//   Char Données : UUID custom
// =============================================================================

// Payload envoyé via BLE
struct BlePayload {
    float      current_A   = 0.0f;
    float      voltage_V   = 0.0f;
    float      temp_C      = 0.0f;
    float      power_W     = 0.0f;
    uint8_t    state       = 0U;   // SystemState cast en uint8_t
    uint32_t   timestamp   = 0U;
} __attribute__((packed));

class BLEManager {
public:
    BLEManager() = default;

    Status init();

    // Mise à jour des données BLE — notifie les clients connectés
    void   updateData(const BlePayload& payload);

    bool   isConnected()    const { return m_connected;    }
    bool   isInitialized()  const { return m_initialized;  }

    // Doit être appelé dans la boucle ou depuis une tâche BLE dédiée
    void   process();

private:
    bool m_initialized = false;
    bool m_connected   = false;

    BlePayload m_lastPayload = {};
};
