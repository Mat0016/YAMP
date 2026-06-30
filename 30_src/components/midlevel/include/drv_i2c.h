#ifndef DRV_I2C_H
#define DRV_I2C_H

#include <stdint.h>
#include <stddef.h>
#include "common_types.h"

/**
 * @file drv_i2c.h
 * @brief Couche Mid Level : pilote generique du bus I2C de l'ESP32.
 *
 * Ce module encapsule le module ESP-IDF "driver/i2c.h" afin que les
 * pilotes de la couche superieure (drv_ina237) ne dependent jamais
 * directement du SDK Espressif : portabilite et testabilite accrues.
 */

/**
 * @brief Initialise le bus I2C de l'ESP32 (broches, frequence, mode maitre).
 * @return APP_OK si succes, APP_ERROR_I2C sinon.
 */
app_error_t drv_i2c_init(void);

/**
 * @brief Ecrit data_len octets a partir du registre reg_addr d'un esclave I2C.
 */
app_error_t drv_i2c_write_reg(uint8_t device_addr,
                               uint8_t reg_addr,
                               const uint8_t *p_data,
                               size_t data_len);

/**
 * @brief Lit data_len octets a partir du registre reg_addr d'un esclave I2C.
 */
app_error_t drv_i2c_read_reg(uint8_t device_addr,
                              uint8_t reg_addr,
                              uint8_t *p_data,
                              size_t data_len);

#endif /* DRV_I2C_H */
