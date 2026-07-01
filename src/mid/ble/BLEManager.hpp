#pragma once
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLECharacteristic.h>
#include <esp_log.h>
#include "config/SystemConfig.hpp"
#include "config/AppConfig.hpp"

// ============================================================
//  BLEManager.hpp — Couche MID / ble
//
//  Expose une caractéristique BLE NOTIFY contenant un JSON
//  compact des mesures : {"t":52.1,"i":1.23,"v":24.0,"s":"W"}
//
//  UUID service   : 12345678-1234-1234-1234-123456789ABC
//  UUID mesures   : 12345678-1234-1234-1234-123456789ABD (NOTIFY)
//  UUID commandes : 12345678-1234-1234-1234-123456789ABE (WRITE)
// ============================================================

class BLEManager : public BLEServerCallbacks {
public:
    bool init();

    // Envoie une notification avec les mesures courantes
    void notify(float32_t temp, float32_t current, float32_t voltage, AgvState state);

    bool isConnected() const { return m_connected; }

private:
    // Callbacks connexion BLE
    void onConnect   (BLEServer*) override { m_connected = true;  ESP_LOGI("BLE", "Client connecte");    }
    void onDisconnect(BLEServer* s) override {
        m_connected = false;
        ESP_LOGI("BLE", "Client deconnecte — redemarrage advertising");
        s->startAdvertising();
    }

    BLEServer*          m_server     = nullptr;
    BLECharacteristic*  m_charNotify = nullptr;
    bool                m_connected  = false;

    static constexpr const char* SVC_UUID  = "12345678-1234-1234-1234-123456789ABC";
    static constexpr const char* CHAR_MEAS = "12345678-1234-1234-1234-123456789ABD";
    static constexpr const char* CHAR_CMD  = "12345678-1234-1234-1234-123456789ABE";

    static const char* stateToStr(AgvState s) {
        switch (s) {
            case AgvState::INIT:          return "INIT";
            case AgvState::NORMAL:        return "OK";
            case AgvState::WARNING:       return "WARN";
            case AgvState::CRITICAL:      return "CRIT";
            case AgvState::SAFE_SHUTDOWN: return "SHUT";
            default:                      return "UNK";
        }
    }
};
