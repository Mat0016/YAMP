#ifndef COMMON_CONFIG_H
#define COMMON_CONFIG_H

#include <stdint.h>

/**
 * @file common_config.h
 * @brief Constantes de configuration materielle du projet.
 *
 * Regle MISRA (niveau debutant) : centraliser ici toutes les valeurs
 * numeriques (broches, adresses, seuils, frequences) sous forme de
 * #define nommes, afin de ne jamais ecrire de "magic numbers" dans
 * le code des pilotes ou de l'application.
 */

/* ----------------------------------------------------------------------- */
/* Bus I2C (utilise par le pilote du capteur INA237)                       */
/* ----------------------------------------------------------------------- */
#define CFG_I2C_PORT_NUM        (0U)
#define CFG_I2C_SDA_GPIO        (21)
#define CFG_I2C_SCL_GPIO        (22)
#define CFG_I2C_FREQ_HZ         (400000U)   /* 400 kHz : mode rapide I2C   */
#define CFG_I2C_TIMEOUT_MS      (100U)

/* ----------------------------------------------------------------------- */
/* Bus SPI (utilise par le pilote de l'IHM : afficheur)                    */
/* ----------------------------------------------------------------------- */
#define CFG_SPI_HOST_NUM        (1)         /* HSPI_HOST sur ESP32        */
#define CFG_SPI_MOSI_GPIO       (23)
#define CFG_SPI_MISO_GPIO       (19)
#define CFG_SPI_SCLK_GPIO       (18)
#define CFG_SPI_CS_GPIO         (5)
#define CFG_SPI_DC_GPIO         (4)         /* Broche Data/Command         */
#define CFG_SPI_FREQ_HZ         (10000000U) /* 10 MHz                      */
#define CFG_SPI_TIMEOUT_MS      (100U)

/* ----------------------------------------------------------------------- */
/* Capteur de puissance INA237                                             */
/* ----------------------------------------------------------------------- */
#define CFG_INA237_I2C_ADDR          (0x40U)
#define CFG_INA237_ALARM_CURRENT_MA  (8000)  /* Seuil d'alarme courant     */

/* ----------------------------------------------------------------------- */
/* IHM (ecran + boutons)                                                   */
/* ----------------------------------------------------------------------- */
#define CFG_IHM_NB_LIGNES           (4U)
#define CFG_IHM_NB_CARACT_LIGNE     (20U)
#define CFG_IHM_GPIO_BOUTON_NEXT    (25)
#define CFG_IHM_GPIO_BOUTON_OK      (26)

/* ----------------------------------------------------------------------- */
/* Boucle applicative principale                                           */
/* ----------------------------------------------------------------------- */
#define CFG_APP_PERIODE_MS          (200U)

#endif /* COMMON_CONFIG_H */
