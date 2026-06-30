#include "app_power_monitor.h"
#include "app_power_monitor_logic.h"
#include "drv_ina237.h"
#include "common_config.h"

static ina237_handle_t g_ina237_handle;
static bool g_module_is_initialized = false;

app_error_t app_power_monitor_init(void)
{
    app_error_t result;

    result = ina237_init(&g_ina237_handle, (uint8_t)CFG_INA237_I2C_ADDR);

    if (result == APP_OK)
    {
        g_module_is_initialized = true;
    }

    return result;
}

app_error_t app_power_monitor_process(app_power_mesure_t *p_mesure)
{
    app_error_t result;

    if (p_mesure == NULL)
    {
        result = APP_ERROR_NULL_POINTER;
    }
    else if (!g_module_is_initialized)
    {
        result = APP_ERROR_NOT_INITIALIZED;
    }
    else
    {
        result = ina237_read_bus_voltage_mv(&g_ina237_handle, &p_mesure->tension_bus_mv);

        if (result == APP_OK)
        {
            result = ina237_read_current_ma(&g_ina237_handle, &p_mesure->courant_ma);
        }

        if (result == APP_OK)
        {
            result = ina237_read_power_mw(&g_ina237_handle, &p_mesure->puissance_mw);
        }

        if (result == APP_OK)
        {
            p_mesure->alarme_courant_active =
                app_power_monitor_check_alarm(p_mesure->courant_ma,
                                               (int32_t)CFG_INA237_ALARM_CURRENT_MA);
        }
    }

    return result;
}
