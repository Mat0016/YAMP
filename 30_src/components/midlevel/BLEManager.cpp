#include "BLEManager.hpp"
#include "../../config/HardwareConfig.hpp"
#include "../../config/SystemConfig.hpp"
#include "esp_log.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include <cstring>

static const char* TAG = "BLEManager";

// =============================================================================
// BLEManager.cpp — Implémentation NimBLE (ESP-IDF)
//
// NimBLE est le stack BLE recommandé pour l'ESP-IDF (plus léger que Bluedroid).
// Activer dans sdkconfig : CONFIG_BT_NIMBLE_ENABLED=y
// =============================================================================

// UUID du service et des caractéristiques (UUIDs 16 bits custom)
static constexpr uint16_t GATT_SVR_SVC_UUID       = 0x181AU; // Environmental Sensing
static constexpr uint16_t GATT_CHR_DATA_UUID       = 0x2BA6U; // Custom : données capteurs
static constexpr uint16_t GATT_CHR_STATE_UUID      = 0x2BA7U; // Custom : état FSM

// Buffer de données partagé (accès depuis callback GATT)
static BlePayload s_sharedPayload = {};
static uint8_t    s_connHandle    = 0xFFU;

// ---------------------------------------------------------------------------
// Callbacks GATT
// ---------------------------------------------------------------------------

static int gattChrAccess(uint16_t              connHandle,
                         uint16_t              attrHandle,
                         struct ble_gatt_access_ctxt* ctxt,
                         void*                 arg)
{
    (void)connHandle;
    (void)attrHandle;
    (void)arg;

    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
        int rc = os_mbuf_append(ctxt->om, &s_sharedPayload, sizeof(BlePayload));
        return (rc == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    return BLE_ATT_ERR_UNLIKELY;
}

// Table GATT statique
static const struct ble_gatt_svc_def s_gattServices[] = {
    {
        .type            = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid            = BLE_UUID16_DECLARE(GATT_SVR_SVC_UUID),
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                .uuid       = BLE_UUID16_DECLARE(GATT_CHR_DATA_UUID),
                .access_cb  = gattChrAccess,
                .flags      = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
            },
            { 0 } // Terminateur
        },
    },
    { 0 } // Terminateur
};

// ---------------------------------------------------------------------------
// Callbacks GAP
// ---------------------------------------------------------------------------

static int gapEventCallback(struct ble_gap_event* event, void* arg) {
    (void)arg;
    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            if (event->connect.status == 0) {
                s_connHandle = event->connect.conn_handle;
                ESP_LOGI(TAG, "BLE client connecté (handle=%d)", s_connHandle);
            } else {
                s_connHandle = 0xFFU;
            }
            break;
        case BLE_GAP_EVENT_DISCONNECT:
            ESP_LOGI(TAG, "BLE client déconnecté");
            s_connHandle = 0xFFU;
            // Relancer la publicité
            ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, nullptr, BLE_HS_FOREVER,
                              nullptr, gapEventCallback, nullptr);
            break;
        default:
            break;
    }
    return 0;
}

static void bleHostTask(void* param) {
    (void)param;
    nimble_port_run(); // Boucle NimBLE — bloquant
    nimble_port_freertos_deinit();
}

static void bleOnSync() {
    // Démarrer la publicité BLE
    struct ble_gap_adv_params advParams = {};
    advParams.conn_mode = BLE_GAP_CONN_MODE_UND;
    advParams.disc_mode = BLE_GAP_DISC_MODE_GEN;

    int rc = ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, nullptr, BLE_HS_FOREVER,
                                &advParams, gapEventCallback, nullptr);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_gap_adv_start FAILED: %d", rc);
    } else {
        ESP_LOGI(TAG, "BLE publicité démarrée ('%s')",
                 HardwareConfig::BLE_DEVICE_NAME);
    }
}

// =============================================================================
// BLEManager
// =============================================================================

Status BLEManager::init() {
    if (!SystemConfig::ENABLE_BLE) {
        ESP_LOGI(TAG, "BLE désactivé (SystemConfig)");
        return Status::OK;
    }

    esp_err_t err = nimble_port_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nimble_port_init FAILED: %s", esp_err_to_name(err));
        return Status::ERROR;
    }

    ble_hs_cfg.sync_cb = bleOnSync;

    // Nom du device BLE
    int rc = ble_svc_gap_device_name_set(HardwareConfig::BLE_DEVICE_NAME);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_svc_gap_device_name_set FAILED: %d", rc);
        return Status::ERROR;
    }

    // Enregistrement des services GATT
    ble_svc_gap_init();
    ble_svc_gatt_init();

    rc = ble_gatts_count_cfg(s_gattServices);
    if (rc != 0) { return Status::ERROR; }
    rc = ble_gatts_add_svcs(s_gattServices);
    if (rc != 0) { return Status::ERROR; }

    // Lancer la tâche BLE
    nimble_port_freertos_init(bleHostTask);

    m_initialized = true;
    ESP_LOGI(TAG, "BLEManager init OK");
    return Status::OK;
}

void BLEManager::updateData(const BlePayload& payload) {
    if (!m_initialized) { return; }
    m_lastPayload  = payload;
    s_sharedPayload = payload;
    m_connected    = (s_connHandle != 0xFFU);
}

void BLEManager::process() {
    // Les notifications BLE sont gérées par la tâche NimBLE en arrière-plan.
    // Cette méthode peut être étendue pour des envois actifs (notify).
}
