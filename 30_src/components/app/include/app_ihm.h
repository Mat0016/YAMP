#ifndef APP_IHM_H
#define APP_IHM_H

#include "common_types.h"
#include "app_power_monitor.h"

/**
 * @file app_ihm.h
 * @brief Couche Application : mise en forme et affichage des mesures
 *        sur l'IHM (ecran).
 */

app_error_t app_ihm_init(void);

app_error_t app_ihm_afficher_mesure(const app_power_mesure_t *p_mesure);

#endif /* APP_IHM_H */
