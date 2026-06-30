#ifndef APP_POWER_MONITOR_H
#define APP_POWER_MONITOR_H

#include <stdint.h>
#include "common_types.h"

/**
 * @file app_power_monitor.h
 * @brief Couche Application : acquisition des mesures de puissance
 *        (tension, courant, puissance) via le pilote INA237, et calcul
 *        de l'etat d'alarme.
 */

typedef struct
{
    uint32_t tension_bus_mV;       /**< Tension bus mesuree, en mV          */
    int32_t  courant_mA;           /**< Courant mesure, en mA               */
    uint32_t puissance_mW;         /**< Puissance mesuree, en mW            */
    bool     alarme_courant_active;/**< Vrai si le seuil d'alarme est atteint */
} app_power_mesure_t;

app_error_t app_power_monitor_init(void);

app_error_t app_power_monitor_process(app_power_mesure_t *p_mesure);

#endif /* APP_POWER_MONITOR_H */
