#include "TMP126Driver.hpp"
#include "../../../config/HardwareConfig.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstring>

static const char* TAG = "TMP126";

// =============================================================================
// TMP126Driver.cpp
// Référence datasheet : Texas Instruments SBOS811
//
// Le TMP126 est un capteur SPI en LECTURE SEULE.
// Aucun registre d'écriture n'existe — la config se fait par câblage (ADDR pin).
// =============================================================================

TMP126Driver::~TMP126Driver() {
    if (m_spi != nullptr) {
        spi_bus_remove_device(m_spi);
    }
}

// =============================================================================
// INIT TMP126 — Initialisation SPI et validation de présence
// =============================================================================
Status TMP126Driver::init() {
    ESP_LOGI(TAG, "=== INIT TMP126 (SPI) ===");

    // -------------------------------------------------------------------------
    // Étape 1 : Configuration du bus SPI
    // Mode 1 (CPOL=0, CPHA=1) requis par le TMP126
    // -------------------------------------------------------------------------
    spi_bus_config_t busConfig = {};
    busConfig.mosi_io_num     = HardwareConfig::TMP126_MOSI_PIN; // Non utilisé (lecture seule)
    busConfig.miso_io_num     = HardwareConfig::TMP126_MISO_PIN;
    busConfig.sclk_io_num     = HardwareConfig::TMP126_CLK_PIN;
    busConfig.quadwp_io_num   = -1;
    busConfig.quadhd_io_num   = -1;
    busConfig.max_transfer_sz = 2U; // 16 bits max par transaction

    const spi_host_device_t host =
        static_cast<spi_host_device_t>(HardwareConfig::TMP126_SPI_HOST);

    esp_err_t err = spi_bus_initialize(host, &busConfig, SPI_DMA_CH_AUTO);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        // ESP_ERR_INVALID_STATE = bus déjà initialisé, toléré
        ESP_LOGE(TAG, "  [1/3] spi_bus_initialize FAILED: %s", esp_err_to_name(err));
        return Status::ERROR;
    }
    ESP_LOGI(TAG, "  [1/3] Bus SPI%d initialisé (MISO=%d, CLK=%d, Mode1)",
             HardwareConfig::TMP126_SPI_HOST,
             HardwareConfig::TMP126_MISO_PIN,
             HardwareConfig::TMP126_CLK_PIN);

    // -------------------------------------------------------------------------
    // Étape 2 : Ajout du device TMP126 sur le bus
    // SPI Mode 1 : CPOL=0, CPHA=1
    // -------------------------------------------------------------------------
    spi_device_interface_config_t devConfig = {};
    devConfig.clock_speed_hz = static_cast<int>(HardwareConfig::TMP126_SPI_HZ);
    devConfig.mode           = 1U;   // Mode SPI 1 (CPOL=0, CPHA=1)
    devConfig.spics_io_num   = static_cast<int>(HardwareConfig::TMP126_CS_PIN);
    devConfig.queue_size     = 1U;
    devConfig.flags          = 0U;
    devConfig.pre_cb         = nullptr;
    devConfig.post_cb        = nullptr;

    err = spi_bus_add_device(host, &devConfig, &m_spi);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "  [2/3] spi_bus_add_device FAILED: %s", esp_err_to_name(err));
        return Status::ERROR;
    }
    ESP_LOGI(TAG, "  [2/3] Device TMP126 ajouté (CS=GPIO%d, freq=%lu Hz)",
             HardwareConfig::TMP126_CS_PIN,
             HardwareConfig::TMP126_SPI_HZ);

    // -------------------------------------------------------------------------
    // Étape 3 : Lecture de validation
    // Le TMP126 démarre une conversion dès la mise sous tension (typ. 6 ms).
    // On lit jusqu'à ce que NRDY=0 (données prêtes) avec timeout.
    // -------------------------------------------------------------------------
    ESP_LOGI(TAG, "  [3/3] Attente conversion initiale (NRDY → 0)...");
    constexpr uint8_t  MAX_RETRIES    = 10U;
    constexpr uint32_t RETRY_DELAY_MS = 10U;

    uint16_t rawVal = 0U;
    bool     ready  = false;

    for (uint8_t attempt = 0U; attempt < MAX_RETRIES; ++attempt) {
        vTaskDelay(pdMS_TO_TICKS(RETRY_DELAY_MS));
        Status st = readRaw(rawVal);
        if (st != Status::OK) {
            ESP_LOGE(TAG, "  Lecture SPI FAILED (tentative %d)", attempt + 1);
            continue;
        }
        if ((rawVal & TMP126Bits::NRDY_MASK) == 0U) {
            ready = true;
            // Décodage pour log
            const int16_t  raw14   = static_cast<int16_t>(rawVal) >> TMP126Bits::TEMP_SHIFT;
            const float    tempC   = static_cast<float>(raw14) * TMP126Bits::TEMP_LSB_C;
            ESP_LOGI(TAG, "  [3/3] NRDY=0 — Temp initiale = %.2f °C (raw=0x%04X)",
                     tempC, rawVal);
            break;
        }
        ESP_LOGD(TAG, "  NRDY=1, retry %d/%d...", attempt + 1, MAX_RETRIES);
    }

    if (!ready) {
        ESP_LOGE(TAG, "  [3/3] Timeout — TMP126 non prêt après %d tentatives",
                 MAX_RETRIES);
        return Status::ERROR;
    }

    m_initialized = true;
    ESP_LOGI(TAG, "=== TMP126 INIT COMPLETE ===");
    return Status::OK;
}

// =============================================================================
// Lecture température
// =============================================================================

float TMP126Driver::getTemp_C() const {
    if (!m_initialized) { return 0.0f; }

    uint16_t raw = 0U;
    if (readRaw(raw) != Status::OK) {
        return 0.0f;
    }

    // Vérification NRDY — si conversion en cours, on retourne la dernière valeur connue
    if ((raw & TMP126Bits::NRDY_MASK) != 0U) {
        ESP_LOGW(TAG, "getTemp : NRDY=1 (conversion en cours)");
        return 0.0f;
    }

    m_lastAlarm = ((raw & TMP126Bits::ALARM_MASK) != 0U);

    // Décalage de 2 bits pour obtenir la valeur 14 bits signée
    const int16_t temp14 = static_cast<int16_t>(raw) >> TMP126Bits::TEMP_SHIFT;
    return static_cast<float>(temp14) * TMP126Bits::TEMP_LSB_C;
}

// =============================================================================
// Lecture SPI brute (16 bits)
// =============================================================================

Status TMP126Driver::readRaw(uint16_t& out) const {
    uint8_t rxBuf[2] = {0U, 0U};

    spi_transaction_t trans = {};
    trans.length    = 16U; // 16 bits
    trans.rxlength  = 16U;
    trans.rx_buffer = rxBuf;
    trans.tx_buffer = nullptr;
    trans.flags     = 0U;

    esp_err_t err = spi_device_transmit(m_spi, &trans);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "SPI transmit FAILED: %s", esp_err_to_name(err));
        return Status::ERROR;
    }

    // TMP126 : MSB en premier
    out = (static_cast<uint16_t>(rxBuf[0]) << 8U) | static_cast<uint16_t>(rxBuf[1]);
    return Status::OK;
}
