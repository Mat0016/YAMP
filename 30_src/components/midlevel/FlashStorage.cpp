#include "FlashStorage.hpp"
#include "../../../config/HardwareConfig.hpp"
#include "esp_log.h"
#include <cstring>

static const char* TAG = "FlashStorage";

Status FlashStorage::init() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition corrompue — effacement et réinitialisation");
        (void)nvs_flash_erase();
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_flash_init FAILED: %s", esp_err_to_name(err));
        return Status::ERROR;
    }

    err = nvs_open(HardwareConfig::NVS_NAMESPACE, NVS_READWRITE, &m_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open FAILED: %s", esp_err_to_name(err));
        return Status::ERROR;
    }

    m_initialized = true;
    ESP_LOGI(TAG, "FlashStorage init OK (namespace='%s')", HardwareConfig::NVS_NAMESPACE);
    return Status::OK;
}

Status FlashStorage::saveConfig(const StoredConfig& cfg) {
    if (!m_initialized) { return Status::ERROR; }

    StoredConfig toWrite = cfg;
    toWrite.checksum     = computeChecksum(cfg);

    esp_err_t err = nvs_set_blob(m_handle, NVS_KEY_CONFIG,
                                 &toWrite, sizeof(StoredConfig));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_set_blob FAILED: %s", esp_err_to_name(err));
        return Status::ERROR;
    }

    err = nvs_commit(m_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_commit FAILED: %s", esp_err_to_name(err));
        return Status::ERROR;
    }

    ESP_LOGI(TAG, "Config sauvegardée en Flash");
    return Status::OK;
}

Status FlashStorage::loadConfig(StoredConfig& out) {
    if (!m_initialized) { return Status::ERROR; }

    size_t len = sizeof(StoredConfig);
    esp_err_t err = nvs_get_blob(m_handle, NVS_KEY_CONFIG, &out, &len);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Aucune config en Flash — valeurs par défaut utilisées");
        return Status::ERROR;
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_get_blob FAILED: %s", esp_err_to_name(err));
        return Status::ERROR;
    }

    // Validation du checksum
    const uint32_t expected = computeChecksum(out);
    if (out.checksum != expected) {
        ESP_LOGW(TAG, "Checksum invalide (0x%08lX ≠ 0x%08lX) — config ignorée",
                 out.checksum, expected);
        return Status::ERROR;
    }

    ESP_LOGI(TAG, "Config chargée depuis Flash OK");
    return Status::OK;
}

Status FlashStorage::eraseConfig() {
    if (!m_initialized) { return Status::ERROR; }
    esp_err_t err = nvs_erase_key(m_handle, NVS_KEY_CONFIG);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        return Status::ERROR;
    }
    (void)nvs_commit(m_handle);
    ESP_LOGI(TAG, "Config effacée");
    return Status::OK;
}

uint32_t FlashStorage::computeChecksum(const StoredConfig& cfg) const {
    // CRC32 simple sur les champs (sans le checksum lui-même)
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&cfg);
    const size_t   len   = sizeof(StoredConfig) - sizeof(uint32_t);
    uint32_t crc = 0xFFFFFFFFU;
    for (size_t i = 0U; i < len; ++i) {
        crc ^= static_cast<uint32_t>(bytes[i]);
        for (uint8_t bit = 0U; bit < 8U; ++bit) {
            if ((crc & 1U) != 0U) {
                crc = (crc >> 1U) ^ 0xEDB88320U;
            } else {
                crc >>= 1U;
            }
        }
    }
    return crc ^ 0xFFFFFFFFU;
}
