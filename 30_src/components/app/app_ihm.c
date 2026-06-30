#include "app_ihm.h"
#include "drv_ihm.h"

#include <stdio.h>

#define APP_IHM_TAILLE_BUFFER (21U) /* CFG_IHM_NB_CARACT_LIGNE + 1 (caractere nul) */

app_error_t app_ihm_init(void)
{
    return drv_ihm_init();
}

app_error_t app_ihm_afficher_mesure(const app_power_mesure_t *p_mesure)
{
    app_error_t result;

    if (p_mesure == NULL)
    {
        result = APP_ERROR_NULL_POINTER;
    }
    else
    {
        char buffer_ligne[APP_IHM_TAILLE_BUFFER];

        result = drv_ihm_effacer_ecran();

        if (result == APP_OK)
        {
            (void)snprintf(buffer_ligne, sizeof(buffer_ligne), "V: %u mV",
                            (unsigned int)p_mesure->tension_bus_mv);
            result = drv_ihm_ecrire_ligne(0U, buffer_ligne);
        }

        if (result == APP_OK)
        {
            (void)snprintf(buffer_ligne, sizeof(buffer_ligne), "I: %d mA",
                            (int)p_mesure->courant_ma);
            result = drv_ihm_ecrire_ligne(1U, buffer_ligne);
        }

        if (result == APP_OK)
        {
            (void)snprintf(buffer_ligne, sizeof(buffer_ligne), "P: %u mW",
                            (unsigned int)p_mesure->puissance_mw);
            result = drv_ihm_ecrire_ligne(2U, buffer_ligne);
        }

        if ((result == APP_OK) && p_mesure->alarme_courant_active)
        {
            result = drv_ihm_ecrire_ligne(3U, "** ALARME COURANT **");
        }
    }

    return result;
}
