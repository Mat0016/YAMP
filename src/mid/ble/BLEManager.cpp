#include "BLEManager.hpp"
#include <BLE2902.h>   // descripteur CCCD pour NOTIFY
#include <stdio.h>

static const char* TAG = "BLE";

bool BLEManager::init() {
    ESP_LOGI(TAG, "Demarrage BLE '%s'...", BLE_DEVICE_NAME);

    BLEDevice::init(BLE_DEVICE_NAME);
    m_server = BLEDevice::createServer();
    m_server->setCallbacks(this);

    BLEService* service = m_server->createService(SVC_UUID);

    // Caractéristique mesures — NOTIFY
    m_charNotify = service->createCharacteristic(
        CHAR_MEAS,
        BLECharacteristic::PROPERTY_NOTIFY
    );
    m_charNotify->addDescriptor(new BLE2902()); // active les notifications côté client

    // Caractéristique commandes — WRITE (placeholder pour futures commandes)
    service->createCharacteristic(CHAR_CMD, BLECharacteristic::PROPERTY_WRITE);

    service->start();
    BLEDevice::startAdvertising();

    ESP_LOGI(TAG, "BLE pret — en attente de connexion");
    return true;
}

void BLEManager::notify(float32_t temp, float32_t current, float32_t voltage, AgvState state) {
    if (!m_connected || m_charNotify == nullptr) { return; }

    // JSON compact : {"t":52.1,"i":1.23,"v":24.05,"s":"WARN"}
    char buf[80];
    snprintf(buf, sizeof(buf), "{\"t\":%.1f,\"i\":%.3f,\"v\":%.2f,\"s\":\"%s\"}",
             static_cast<double>(temp),
             static_cast<double>(current),
             static_cast<double>(voltage),
             stateToStr(state));

    m_charNotify->setValue(reinterpret_cast<uint8_t*>(buf), strlen(buf));
    m_charNotify->notify();

    ESP_LOGD(TAG, "BLE notify: %s", buf);
}
