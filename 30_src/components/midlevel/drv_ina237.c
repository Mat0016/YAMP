#include "drv_ina237.h"
#include "drv_ina237_convert.h"
#include "drv_i2c.h"

#define INA237_REG_SIZE_BYTES (2U)

app_error_t ina237_init(ina237_handle_t *p_handle, uint8_t i2c_addr)
{
    app_error_t result;
    uint16_t manufacturer_id = 0U;

    if (p_handle == NULL)
    {
        result = APP_ERROR_NULL_POINTER;
    }
    else
    {
        p_handle->i2c_addr = i2c_addr;
        /* Necessaire pour pouvoir appeler ina237_read_register() ci-dessous */
        p_handle->is_initialized = true;

        result = ina237_read_register(p_handle, INA237_REG_MANUF_ID, &manufacturer_id);

        if ((result == APP_OK) && (manufacturer_id != INA237_MANUFACTURER_ID))
        {
            p_handle->is_initialized = false;
            result = APP_ERROR_I2C;
        }
    }

    return result;
}

app_error_t ina237_read_register(const ina237_handle_t *p_handle,
                                  uint8_t reg_addr,
                                  uint16_t *p_value)
{
    app_error_t result;
    uint8_t raw_bytes[INA237_REG_SIZE_BYTES] = {0U, 0U};

    if ((p_handle == NULL) || (p_value == NULL))
    {
        result = APP_ERROR_NULL_POINTER;
    }
    else if (!p_handle->is_initialized)
    {
        result = APP_ERROR_NOT_INITIALIZED;
    }
    else
    {
        result = drv_i2c_read_reg(p_handle->i2c_addr, reg_addr,
                                   raw_bytes, sizeof(raw_bytes));

        if (result == APP_OK)
        {
            /* L'INA237 transmet les registres en "big endian" (MSB en premier) */
            *p_value = ((uint16_t)raw_bytes[0] << 8) | (uint16_t)raw_bytes[1];
        }
    }

    return result;
}

app_error_t ina237_write_register(const ina237_handle_t *p_handle,
                                   uint8_t reg_addr,
                                   uint16_t value)
{
    app_error_t result;

    if (p_handle == NULL)
    {
        result = APP_ERROR_NULL_POINTER;
    }
    else if (!p_handle->is_initialized)
    {
        result = APP_ERROR_NOT_INITIALIZED;
    }
    else
    {
        uint8_t raw_bytes[INA237_REG_SIZE_BYTES];

        raw_bytes[0] = (uint8_t)(value >> 8);
        raw_bytes[1] = (uint8_t)(value & 0xFFU);

        result = drv_i2c_write_reg(p_handle->i2c_addr, reg_addr,
                                    raw_bytes, sizeof(raw_bytes));
    }

    return result;
}

app_error_t ina237_read_bus_voltage_mv(const ina237_handle_t *p_handle, uint32_t *p_voltage_mv)
{
    app_error_t result;
    uint16_t raw_value = 0U;

    if (p_voltage_mv == NULL)
    {
        result = APP_ERROR_NULL_POINTER;
    }
    else
    {
        result = ina237_read_register(p_handle, INA237_REG_VBUS, &raw_value);

        if (result == APP_OK)
        {
            *p_voltage_mv = ina237_convert_vbus_raw_to_mv(raw_value);
        }
    }

    return result;
}

app_error_t ina237_read_current_ma(const ina237_handle_t *p_handle, int32_t *p_current_ma)
{
    app_error_t result;
    uint16_t raw_value = 0U;

    if (p_current_ma == NULL)
    {
        result = APP_ERROR_NULL_POINTER;
    }
    else
    {
        result = ina237_read_register(p_handle, INA237_REG_CURRENT, &raw_value);

        if (result == APP_OK)
        {
            *p_current_ma = ina237_convert_current_raw_to_ma((int16_t)raw_value,
                                                               INA237_DEFAULT_CURRENT_LSB_UA);
        }
    }

    return result;
}

app_error_t ina237_read_power_mw(const ina237_handle_t *p_handle, uint32_t *p_power_mw)
{
    app_error_t result;
    uint16_t raw_value = 0U;

    if (p_power_mw == NULL)
    {
        result = APP_ERROR_NULL_POINTER;
    }
    else
    {
        result = ina237_read_register(p_handle, INA237_REG_POWER, &raw_value);

        if (result == APP_OK)
        {
            *p_power_mw = ina237_convert_power_raw_to_mw((uint32_t)raw_value,
                                                           INA237_DEFAULT_CURRENT_LSB_UA);
        }
    }

    return result;
}
