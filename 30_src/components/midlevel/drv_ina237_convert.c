#include "drv_ina237_convert.h"

#define INA237_POWER_LSB_FACTOR (25U)
#define MICRO_TO_MILLI_DIVIDER  (1000U)

uint32_t ina237_convert_vbus_raw_to_mv(uint16_t raw_value)
{
    /* Registre VBUS : valeur sur 16 bits non signee, LSB = 3.125 mV */
    uint32_t voltage_uv = (uint32_t)raw_value * INA237_VBUS_LSB_UV;
    uint32_t voltage_mv = voltage_uv / MICRO_TO_MILLI_DIVIDER;

    return voltage_mv;
}

int32_t ina237_convert_current_raw_to_ma(int16_t raw_value, uint32_t current_lsb_ua)
{
    int32_t current_ua = (int32_t)raw_value * (int32_t)current_lsb_ua;
    int32_t current_ma = current_ua / (int32_t)MICRO_TO_MILLI_DIVIDER;

    return current_ma;
}

uint32_t ina237_convert_power_raw_to_mw(uint32_t raw_value, uint32_t current_lsb_ua)
{
    uint32_t power_lsb_ua = current_lsb_ua * INA237_POWER_LSB_FACTOR;
    uint32_t power_uw = raw_value * power_lsb_ua;
    uint32_t power_mw = power_uw / MICRO_TO_MILLI_DIVIDER;

    return power_mw;
}
