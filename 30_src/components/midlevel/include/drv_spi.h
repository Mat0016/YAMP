#ifndef DRV_SPI_H
#define DRV_SPI_H

#include <stdint.h>
#include <stddef.h>
#include "common_types.h"

/**
 * @file drv_spi.h
 * @brief Couche Mid Level : pilote generique du bus SPI de l'ESP32.
 *
 * Ce module encapsule le module ESP-IDF "driver/spi_master.h" afin que
 * le pilote IHM (drv_ihm) ne depende jamais directement du SDK Espressif.
 */

/**
 * @brief Initialise le bus SPI de l'ESP32 (broches, frequence, peripherique).
 * @return APP_OK si succes, APP_ERROR_SPI sinon.
 */
app_error_t drv_spi_init(void);

/**
 * @brief Envoie data_len octets sur le bus SPI.
 * @param p_data     Pointeur vers les donnees a transmettre.
 * @param data_len   Nombre d'octets a transmettre.
 * @param is_command true si l'octet est une commande, false si c'est une donnee
 *                    (utilise la broche Data/Command de l'afficheur).
 */
app_error_t drv_spi_write(const uint8_t *p_data, size_t data_len, bool is_command);

#endif /* DRV_SPI_H */
