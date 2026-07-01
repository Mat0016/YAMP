#pragma once

// ============================================================
//  AppConfig.hpp
//  Seuils métier, fréquences d'acquisition, tailles de buffer.
//  Modifiable sans toucher au code des drivers ni de la FSM.
// ============================================================

// --- Seuils thermiques (°C) ----------------------------------
static constexpr float TEMP_WARNING_DEGC  = 50.0F;
static constexpr float TEMP_CRITICAL_DEGC = 65.0F;
static constexpr float TEMP_HYSTERESIS    = 5.0F;  // retour NORMAL depuis WARNING

// --- Seuil courant (A) ---------------------------------------
static constexpr float CURRENT_CRITICAL_A = 10.0F;

// --- Calibration INA237 --------------------------------------
// R_shunt = 10 mΩ, courant max = 10 A
static constexpr float  INA237_RSHUNT_OHM     = 0.01F;
static constexpr float  INA237_MAX_CURRENT_A  = 10.0F;
// CURRENT_LSB = MAX / 2^15
static constexpr float  INA237_CURRENT_LSB_A  = INA237_MAX_CURRENT_A / 32768.0F;
// SHUNT_CAL = 819.2e6 × CURRENT_LSB × R_shunt
static constexpr uint16_t INA237_SHUNT_CAL   = 2500U;

// --- Boucle principale ---------------------------------------
static constexpr uint32_t APP_LOOP_PERIOD_MS = 200U;

// --- PSRAM / RingBuffer --------------------------------------
static constexpr int16_t RING_BUFFER_SIZE     = 512U;  // nb d'entrées de log

// --- BLE -----------------------------------------------------
static constexpr const char* BLE_DEVICE_NAME = "YAMP-Monitor";
