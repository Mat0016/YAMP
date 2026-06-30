#ifndef DRV_INA237_H
#define DRV_INA237_H

#include <stdint.h>
#include "common_types.h"

/**
 * @file drv_ina237.h
 * @brief Couche Mid Level : pilote du capteur de puissance INA237
 *        (lecture/ecriture des registres via le bus I2C).
 */

/* Adresses des registres internes de l'INA237 (cf. datasheet Texas Instruments) */
#define INA237_REG_CONFIG       (0x00U)
#define INA237_REG_ADC_CONFIG   (0x01U)
#define INA237_REG_SHUNT_CAL    (0x02U)
#define INA237_REG_VSHUNT       (0x04U)
#define INA237_REG_VBUS         (0x05U)
#define INA237_REG_DIETEMP      (0x06U)
#define INA237_REG_CURRENT      (0x07U)
#define INA237_REG_POWER        (0x08U)
#define INA237_REG_DIAG_ALRT    (0x0BU)
#define INA237_REG_MANUF_ID     (0x3EU)
#define INA237_REG_DEVICE_ID    (0x3FU)

#define INA237_MANUFACTURER_ID  (0x5449U) /* Code constructeur "TI" */

/**
 * @brief Handle (contexte) d'une instance du capteur INA237.
 */
typedef struct
{
    uint8_t i2c_addr;        /**< Adresse I2C 7 bits du capteur            */
    bool    is_initialized;  /**< Vrai si ina237_init() a reussi            */
} ina237_handle_t;

app_error_t ina237_init(ina237_handle_t *p_handle, uint8_t i2c_addr);

app_error_t ina237_read_register(const ina237_handle_t *p_handle,
                                  uint8_t reg_addr,
                                  uint16_t *p_value);

app_error_t ina237_write_register(const ina237_handle_t *p_handle,
                                   uint8_t reg_addr,
                                   uint16_t value);

app_error_t ina237_read_bus_voltage_mv(const ina237_handle_t *p_handle,
                                        uint32_t *p_voltage_mv);

app_error_t ina237_read_current_ma(const ina237_handle_t *p_handle,
                                    int32_t *p_current_ma);

app_error_t ina237_read_power_mw(const ina237_handle_t *p_handle,
                                  uint32_t *p_power_mw);

#endif /* DRV_INA237_H */
