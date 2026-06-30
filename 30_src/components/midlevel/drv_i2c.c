#include "drv_i2c.h"
#include "common_config.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"   /* Module ESP32 (ESP-IDF) : pilote materiel I2C */

static bool g_i2c_is_initialized = false;

app_error_t drv_i2c_init(void)
{
    app_error_t result;
    i2c_config_t i2c_conf;
    esp_err_t esp_result;

    i2c_conf.mode = I2C_MODE_MASTER;
    i2c_conf.sda_io_num = CFG_I2C_SDA_GPIO;
    i2c_conf.scl_io_num = CFG_I2C_SCL_GPIO;
    i2c_conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_conf.master.clk_speed = CFG_I2C_FREQ_HZ;

    esp_result = i2c_param_config((i2c_port_t)CFG_I2C_PORT_NUM, &i2c_conf);

    if (esp_result == ESP_OK)
    {
        esp_result = i2c_driver_install((i2c_port_t)CFG_I2C_PORT_NUM,
                                         i2c_conf.mode, 0, 0, 0);
    }

    if (esp_result == ESP_OK)
    {
        g_i2c_is_initialized = true;
        result = APP_OK;
    }
    else
    {
        result = APP_ERROR_I2C;
    }

    return result;
}

app_error_t drv_i2c_write_reg(uint8_t device_addr,
                               uint8_t reg_addr,
                               const uint8_t *p_data,
                               size_t data_len)
{
    app_error_t result;

    if ((p_data == NULL) || (data_len == 0U))
    {
        result = APP_ERROR_NULL_POINTER;
    }
    else if (!g_i2c_is_initialized)
    {
        result = APP_ERROR_NOT_INITIALIZED;
    }
    else
    {
        i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
        esp_err_t esp_result;

        (void)i2c_master_start(cmd_handle);
        (void)i2c_master_write_byte(cmd_handle,
            (uint8_t)((device_addr << 1) | I2C_MASTER_WRITE), true);
        (void)i2c_master_write_byte(cmd_handle, reg_addr, true);
        (void)i2c_master_write(cmd_handle, p_data, data_len, true);
        (void)i2c_master_stop(cmd_handle);

        esp_result = i2c_master_cmd_begin((i2c_port_t)CFG_I2C_PORT_NUM,
                                           cmd_handle,
                                           pdMS_TO_TICKS(CFG_I2C_TIMEOUT_MS));
        i2c_cmd_link_delete(cmd_handle);

        result = (esp_result == ESP_OK) ? APP_OK : APP_ERROR_I2C;
    }

    return result;
}

app_error_t drv_i2c_read_reg(uint8_t device_addr,
                              uint8_t reg_addr,
                              uint8_t *p_data,
                              size_t data_len)
{
    app_error_t result;

    if ((p_data == NULL) || (data_len == 0U))
    {
        result = APP_ERROR_NULL_POINTER;
    }
    else if (!g_i2c_is_initialized)
    {
        result = APP_ERROR_NOT_INITIALIZED;
    }
    else
    {
        i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
        esp_err_t esp_result;

        /* Phase 1 : ecriture de l'adresse du registre a lire */
        (void)i2c_master_start(cmd_handle);
        (void)i2c_master_write_byte(cmd_handle,
            (uint8_t)((device_addr << 1) | I2C_MASTER_WRITE), true);
        (void)i2c_master_write_byte(cmd_handle, reg_addr, true);

        /* Phase 2 : redemarrage puis lecture des donnees */
        (void)i2c_master_start(cmd_handle);
        (void)i2c_master_write_byte(cmd_handle,
            (uint8_t)((device_addr << 1) | I2C_MASTER_READ), true);
        (void)i2c_master_read(cmd_handle, p_data, data_len, I2C_MASTER_LAST_NACK);
        (void)i2c_master_stop(cmd_handle);

        esp_result = i2c_master_cmd_begin((i2c_port_t)CFG_I2C_PORT_NUM,
                                           cmd_handle,
                                           pdMS_TO_TICKS(CFG_I2C_TIMEOUT_MS));
        i2c_cmd_link_delete(cmd_handle);

        result = (esp_result == ESP_OK) ? APP_OK : APP_ERROR_I2C;
    }

    return result;
}
