#ifndef DRV_INA237_CONVERT_H
#define DRV_INA237_CONVERT_H

#include <stdint.h>

/**
 * @file drv_ina237_convert.h
 * @brief Fonctions de conversion "pures" des registres bruts de l'INA237
 *        vers des unites physiques (mV, mA, mW).
 *
 * Ces fonctions ne font AUCUNE entree/sortie (pas d'I2C, pas de log) :
 * elles sont donc testables sur PC, sans materiel, dans le fichier de
 * test 60_valid/test/test_main.c.
 */

/* Resolution LSB d'apres la datasheet INA237 (mode de configuration usine) */
#define INA237_VBUS_LSB_UV             (3125U) /* 1 LSB = 3.125 mV = 3125 uV */
#define INA237_DEFAULT_CURRENT_LSB_UA  (1000U) /* 1 LSB = 1 mA = 1000 uA     */

/**
 * @brief Convertit la valeur brute du registre VBUS (tension bus) en mV.
 */
uint32_t ina237_convert_vbus_raw_to_mv(uint16_t raw_value);

/**
 * @brief Convertit la valeur brute (complement a 2) du registre CURRENT en mA.
 * @param raw_value      Valeur brute signee lue dans le registre CURRENT.
 * @param current_lsb_ua Resolution du LSB courant, en microamperes.
 */
int32_t ina237_convert_current_raw_to_ma(int16_t raw_value, uint32_t current_lsb_ua);

/**
 * @brief Convertit la valeur brute du registre POWER en mW.
 *        D'apres la datasheet : Power_LSB = 25 x Current_LSB.
 */
uint32_t ina237_convert_power_raw_to_mw(uint32_t raw_value, uint32_t current_lsb_ua);

#endif /* DRV_INA237_CONVERT_H */
