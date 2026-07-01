#pragma once
#include <stdint.h>
#include <stddef.h>
#include <esp_heap_caps.h>
#include <esp_log.h>

// ============================================================
//  PSRAMManager.hpp — Couche LOW / memory
//
//  Fournit une API d'allocation dans la PSRAM (SPI RAM).
//  Utilise esp_heap_caps.h (ESP-IDF, disponible sous Arduino).
//
//  Pré-requis platformio.ini :
//    build_flags = -DBOARD_HAS_PSRAM
//    board_build.f_flash = 80000000L  (optionnel)
// ============================================================

class PSRAMManager {
public:
    // ---- Vérification disponibilité -------------------------
    static bool isAvailable() {
        return (heap_caps_get_total_size(MALLOC_CAP_SPIRAM) > 0U);
    }

    static size_t getFreeBytes() {
        return heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    }

    static size_t getTotalBytes() {
        return heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    }

    // ---- Allocation / libération ----------------------------
    // Alloue `size` octets en PSRAM. Retourne nullptr si échec.
    static void* alloc(size_t size) {
        void* ptr = heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
        if (ptr == nullptr) {
            ESP_LOGE("PSRAM", "Echec allocation %u octets en PSRAM", (unsigned)size);
        }
        return ptr;
    }

    static void free(void* ptr) {
        heap_caps_free(ptr);
    }

    // ---- Rapport -----------------------------------------------
    static void logInfo() {
        ESP_LOGI("PSRAM", "PSRAM disponible : %u / %u octets libres",
                 (unsigned)getFreeBytes(), (unsigned)getTotalBytes());
    }
};
