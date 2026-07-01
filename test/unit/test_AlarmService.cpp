// ============================================================
//  test_AlarmService.cpp — Tests unitaires PlatformIO (native)
//
//  Compile et s'exécute sur PC avec : pio test -e native
//  Teste la logique pure d'AlarmService (aucun hardware requis).
// ============================================================

// Stubs minimaux pour compiler sans Arduino.h
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

// Inclure uniquement les headers "purs" (pas de Wire/SPI/Arduino)
#include "../../src/config/SystemConfig.hpp"
#include "../../src/config/AppConfig.hpp"
#include "app/services/MonitoringService.hpp"
#include "app/services/AlarmService.hpp"


// Mini-framework de test maison (pas de dépendance Unity ici)
static int g_ok = 0, g_fail = 0;
#define ASSERT_EQ(a, b, msg) \
    do { if ((a)==(b)) { printf("[OK]   " msg "\n"); g_ok++; } \
         else { printf("[FAIL] " msg " — attendu %d obtenu %d\n", (int)(a), (int)(b)); g_fail++; } } while(0)

int main() {
    printf("=== Tests AlarmService ===\n\n");

    AlarmService alarm;
    Measurement m;
    m.valid = true;

    // ---- 1. Nominal → NORMAL --------------------------------
    m.temperature_degC = 30.0F;
    m.current_A        = 1.0F;
    ASSERT_EQ(AgvState::NORMAL, alarm.evaluate(AgvState::NORMAL, m),
              "T=30 I=1 → NORMAL");

    // ---- 2. Dépassement warning → WARNING -------------------
    m.temperature_degC = 55.0F;
    m.current_A        = 1.0F;
    ASSERT_EQ(AgvState::WARNING, alarm.evaluate(AgvState::NORMAL, m),
              "T=55 → WARNING");

    // ---- 3. Dépassement courant → CRITICAL ------------------
    m.temperature_degC = 30.0F;
    m.current_A        = 12.0F;
    ASSERT_EQ(AgvState::CRITICAL, alarm.evaluate(AgvState::NORMAL, m),
              "I=12 > seuil → CRITICAL");

    // ---- 4. Température critique → CRITICAL -----------------
    m.temperature_degC = 70.0F;
    m.current_A        = 1.0F;
    ASSERT_EQ(AgvState::CRITICAL, alarm.evaluate(AgvState::WARNING, m),
              "T=70 > 65 → CRITICAL");

    // ---- 5. Retour NORMAL depuis WARNING (sous hystérésis) --
    m.temperature_degC = 44.0F; // < 50 - 5 = 45 : OK → NORMAL
    m.current_A        = 1.0F;
    ASSERT_EQ(AgvState::NORMAL, alarm.evaluate(AgvState::WARNING, m),
              "T=44 < (50-5) depuis WARNING → NORMAL");

    // ---- 6. Pas de retour NORMAL avant hystérésis -----------
    m.temperature_degC = 47.0F; // entre 45 et 50 : on reste en WARNING
    m.current_A        = 1.0F;
    ASSERT_EQ(AgvState::WARNING, alarm.evaluate(AgvState::WARNING, m),
              "T=47 dans zone hysteresis → reste WARNING");

    // ---- 7. Seuils à la limite exacte → CRITICAL ------------
    m.temperature_degC = TEMP_CRITICAL_DEGC;
    m.current_A        = 1.0F;
    ASSERT_EQ(AgvState::CRITICAL, alarm.evaluate(AgvState::NORMAL, m),
              "T exactement == critique → CRITICAL");

    printf("\n=== Resultat : %d OK / %d FAIL ===\n", g_ok, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
