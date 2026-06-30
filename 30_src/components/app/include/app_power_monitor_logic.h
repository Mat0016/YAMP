#ifndef APP_POWER_MONITOR_LOGIC_H
#define APP_POWER_MONITOR_LOGIC_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @file app_power_monitor_logic.h
 * @brief Logique applicative "pure" (sans E/S) de surveillance de puissance.
 *
 * Separer cette logique de app_power_monitor.c permet de la tester sur PC
 * sans materiel ESP32, dans 60_valid/test/test_main.c.
 */

/**
 * @brief Determine si le courant mesure depasse le seuil d'alarme configure.
 *
 * Regle MISRA (niveau debutant) : un seul point de sortie (return), pas
 * d'effet de bord, parametres explicitement types.
 *
 * @param courant_mA Courant mesure, en milliamperes.
 * @param seuil_mA   Seuil d'alarme, en milliamperes.
 * @return true si courant_ma >= seuil_ma, false sinon.
 */
bool app_power_monitor_check_alarm(int32_t courant_mA, int32_t seuil_mA);

#endif /* APP_POWER_MONITOR_LOGIC_H */
