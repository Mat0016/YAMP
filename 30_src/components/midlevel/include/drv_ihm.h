#ifndef DRV_IHM_H
#define DRV_IHM_H

#include <stdint.h>
#include "common_types.h"

/**
 * @file drv_ihm.h
 * @brief Couche Mid Level : pilote de l'IHM (ecran via SPI, boutons via GPIO).
 */

typedef enum
{
    IHM_BOUTON_NEXT = 0,
    IHM_BOUTON_OK
} ihm_bouton_id_t;

app_error_t drv_ihm_init(void);

app_error_t drv_ihm_effacer_ecran(void);

app_error_t drv_ihm_ecrire_ligne(uint8_t numero_ligne, const char *p_texte);

app_error_t drv_ihm_lire_bouton(ihm_bouton_id_t bouton_id, bool *p_est_appuye);

#endif /* DRV_IHM_H */
