#pragma once
#include <Preferences.h>
#include <esp_log.h>
#include "config/SystemConfig.hpp"
#include "config/AppConfig.hpp"

// ============================================================
//  FlashStorage.hpp — Couche LOW / memory
//
//  Persistance des paramètres de configuration dans la flash
//  NVS de l'ESP32 via la bibliothèque Arduino Preferences.h.
//
//  Namespace NVS : "yamp"
//  Clés stockées :
//    "tWarn"  — seuil température warning (float)
//    "tCrit"  — seuil température critique (float)
//    "iCrit"  — seuil courant critique (float)
// ============================================================

class FlashStorage {
public:
    bool init() {
        bool ok = m_prefs.begin("yamp", false); // read-write
        if (!ok) {
            ESP_LOGE("Flash", "Echec ouverture namespace NVS 'yamp'");
        }
        return ok;
    }

    // --- Écriture --------------------------------------------
    void saveTempWarning (float32_t val) { m_prefs.putFloat("tWarn", val); }
    void saveTempCritical(float32_t val) { m_prefs.putFloat("tCrit", val); }
    void saveCurrentCrit (float32_t val) { m_prefs.putFloat("iCrit", val); }

    // --- Lecture (valeur par défaut = constantes AppConfig) --
    float32_t loadTempWarning()  { return m_prefs.getFloat("tWarn", TEMP_WARNING_DEGC);  }
    float32_t loadTempCritical() { return m_prefs.getFloat("tCrit", TEMP_CRITICAL_DEGC); }
    float32_t loadCurrentCrit()  { return m_prefs.getFloat("iCrit", CURRENT_CRITICAL_A); }

    // --- Reset usine -----------------------------------------
    void factoryReset() {
        m_prefs.clear();
        ESP_LOGW("Flash", "NVS efface — retour aux valeurs par defaut");
    }

    void close() { m_prefs.end(); }

private:
    Preferences m_prefs;
};
