/**
 * @file test_main.c
 * @brief Fichier de test d'ecriture des fonctions "pures" du projet
 *        (sans dependance materielle ESP32), compilable et executable
 *        directement sur PC avec un compilateur C standard (gcc).
 *
 * Modules testes :
 *  - drv_ina237_convert.c : conversions registres bruts -> unites physiques
 *  - app_power_monitor_logic.c : logique de declenchement de l'alarme
 *
 * Compilation (depuis le dossier 60_valid/test) :
 *   gcc -Wall -Wextra -std=c99 \
 *       -I ../../30_src/components/midlevel/include \
 *       -I ../../30_src/components/app/include \
 *       test_main.c \
 *       ../../30_src/components/midlevel/drv_ina237_convert.c \
 *       ../../30_src/components/app/app_power_monitor_logic.c \
 *       -o test_runner
 *
 * Execution :
 *   ./test_runner
 *
 * Code de retour : 0 si tous les tests passent, 1 sinon (utilisable en CI).
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "drv_ina237_convert.h"
#include "app_power_monitor_logic.h"

static uint32_t g_nb_tests_ok = 0U;
static uint32_t g_nb_tests_ko = 0U;

static void test_verifier_u32(uint32_t attendu, uint32_t obtenu, const char *p_nom_test)
{
    if (attendu == obtenu)
    {
        g_nb_tests_ok++;
        (void)printf("[OK]   %s\n", p_nom_test);
    }
    else
    {
        g_nb_tests_ko++;
        (void)printf("[FAIL] %s (attendu=%u, obtenu=%u)\n",
                      p_nom_test, (unsigned int)attendu, (unsigned int)obtenu);
    }
}

static void test_verifier_i32(int32_t attendu, int32_t obtenu, const char *p_nom_test)
{
    if (attendu == obtenu)
    {
        g_nb_tests_ok++;
        (void)printf("[OK]   %s\n", p_nom_test);
    }
    else
    {
        g_nb_tests_ko++;
        (void)printf("[FAIL] %s (attendu=%d, obtenu=%d)\n", p_nom_test, attendu, obtenu);
    }
}

static void test_verifier_bool(bool attendu, bool obtenu, const char *p_nom_test)
{
    if (attendu == obtenu)
    {
        g_nb_tests_ok++;
        (void)printf("[OK]   %s\n", p_nom_test);
    }
    else
    {
        g_nb_tests_ko++;
        (void)printf("[FAIL] %s (attendu=%d, obtenu=%d)\n",
                      p_nom_test, (int)attendu, (int)obtenu);
    }
}

/* --------------------------------------------------------------------- */
/* Tests du module drv_ina237_convert                                    */
/* --------------------------------------------------------------------- */
static void test_ina237_convert_vbus(void)
{
    uint32_t resultat;

    /* 1 LSB = 3.125 mV. Pour raw = 1000 -> 3125 mV (3.125 V) */
    resultat = ina237_convert_vbus_raw_to_mv(1000U);
    test_verifier_u32(3125U, resultat, "ina237_convert_vbus_raw_to_mv(1000) == 3125 mV");

    resultat = ina237_convert_vbus_raw_to_mv(0U);
    test_verifier_u32(0U, resultat, "ina237_convert_vbus_raw_to_mv(0) == 0 mV");
}

static void test_ina237_convert_current(void)
{
    int32_t resultat;

    /* LSB = 1000 uA (1 mA). raw = 500 -> 500 mA */
    resultat = ina237_convert_current_raw_to_ma(500, 1000U);
    test_verifier_i32(500, resultat, "ina237_convert_current_raw_to_ma(500, 1000) == 500 mA");

    /* Valeur negative : sens inverse du courant (decharge) */
    resultat = ina237_convert_current_raw_to_ma(-200, 1000U);
    test_verifier_i32(-200, resultat, "ina237_convert_current_raw_to_ma(-200, 1000) == -200 mA");
}

static void test_ina237_convert_power(void)
{
    uint32_t resultat;

    /* Power_LSB = 25 x Current_LSB. raw=100, current_lsb=1000 uA
       -> power_lsb = 25000 uA -> 100 * 25000 = 2 500 000 uW = 2500 mW */
    resultat = ina237_convert_power_raw_to_mw(100U, 1000U);
    test_verifier_u32(2500U, resultat, "ina237_convert_power_raw_to_mw(100, 1000) == 2500 mW");
}

/* --------------------------------------------------------------------- */
/* Tests du module app_power_monitor_logic                               */
/* --------------------------------------------------------------------- */
static void test_app_power_monitor_check_alarm(void)
{
    bool resultat;

    resultat = app_power_monitor_check_alarm(5000, 8000);
    test_verifier_bool(false, resultat, "check_alarm(5000 < seuil 8000) -> pas d'alarme");

    resultat = app_power_monitor_check_alarm(8000, 8000);
    test_verifier_bool(true, resultat, "check_alarm(8000 == seuil 8000) -> alarme (limite incluse)");

    resultat = app_power_monitor_check_alarm(9000, 8000);
    test_verifier_bool(true, resultat, "check_alarm(9000 > seuil 8000) -> alarme");

    resultat = app_power_monitor_check_alarm(-100, 8000);
    test_verifier_bool(false, resultat, "check_alarm(courant negatif) -> pas d'alarme");
}

int main(void)
{
    (void)printf("=== Tests unitaires - modules logiciels purs (sans materiel) ===\n\n");

    test_ina237_convert_vbus();
    test_ina237_convert_current();
    test_ina237_convert_power();
    test_app_power_monitor_check_alarm();

    (void)printf("\n=== Resultat : %u test(s) OK / %u test(s) en echec ===\n",
                  (unsigned int)g_nb_tests_ok, (unsigned int)g_nb_tests_ko);

    return (g_nb_tests_ko == 0U) ? 0 : 1;
}
