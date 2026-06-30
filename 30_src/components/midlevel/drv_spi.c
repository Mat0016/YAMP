#include "drv_spi.h"
#include "common_config.h"

#include "driver/spi_master.h" /* Module ESP32 (ESP-IDF) : pilote materiel SPI */
#include "driver/gpio.h"

static spi_device_handle_t g_spi_device_handle;
static bool g_spi_is_initialized = false;

app_error_t drv_spi_init(void)
{
    app_error_t result;
    spi_bus_config_t bus_conf;
    spi_device_interface_config_t dev_conf;
    esp_err_t esp_result;

    bus_conf.mosi_io_num = CFG_SPI_MOSI_GPIO;
    bus_conf.miso_io_num = CFG_SPI_MISO_GPIO;
    bus_conf.sclk_io_num = CFG_SPI_SCLK_GPIO;
    bus_conf.quadwp_io_num = -1;
    bus_conf.quadhd_io_num = -1;
    bus_conf.max_transfer_sz = 0; /* Valeur par defaut du SDK */

    esp_result = spi_bus_initialize((spi_host_device_t)CFG_SPI_HOST_NUM,
                                     &bus_conf, SPI_DMA_CH_AUTO);

    if (esp_result == ESP_OK)
    {
        dev_conf.clock_speed_hz = (int)CFG_SPI_FREQ_HZ;
        dev_conf.mode = 0;
        dev_conf.spics_io_num = CFG_SPI_CS_GPIO;
        dev_conf.queue_size = 1;
        dev_conf.command_bits = 0;
        dev_conf.address_bits = 0;

        esp_result = spi_bus_add_device((spi_host_device_t)CFG_SPI_HOST_NUM,
                                         &dev_conf, &g_spi_device_handle);
    }

    if (esp_result == ESP_OK)
    {
        gpio_config_t dc_gpio_conf;

        dc_gpio_conf.pin_bit_mask = (1ULL << CFG_SPI_DC_GPIO);
        dc_gpio_conf.mode = GPIO_MODE_OUTPUT;
        dc_gpio_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        dc_gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        dc_gpio_conf.intr_type = GPIO_INTR_DISABLE;

        esp_result = gpio_config(&dc_gpio_conf);
    }

    if (esp_result == ESP_OK)
    {
        g_spi_is_initialized = true;
        result = APP_OK;
    }
    else
    {
        result = APP_ERROR_SPI;
    }

    return result;
}

app_error_t drv_spi_write(const uint8_t *p_data, size_t data_len, bool is_command)
{
    app_error_t result;

    if ((p_data == NULL) || (data_len == 0U))
    {
        result = APP_ERROR_NULL_POINTER;
    }
    else if (!g_spi_is_initialized)
    {
        result = APP_ERROR_NOT_INITIALIZED;
    }
    else
    {
        spi_transaction_t transaction;
        esp_err_t esp_result;

        (void)gpio_set_level((gpio_num_t)CFG_SPI_DC_GPIO, is_command ? 0 : 1);

        transaction.length = data_len * 8U; /* Longueur exprimee en bits */
        transaction.tx_buffer = p_data;
        transaction.rx_buffer = NULL;
        transaction.rxlength = 0;
        transaction.flags = 0;
        transaction.user = NULL;
        transaction.cmd = 0;
        transaction.addr = 0;

        esp_result = spi_device_transmit(g_spi_device_handle, &transaction);

        result = (esp_result == ESP_OK) ? APP_OK : APP_ERROR_SPI;
    }

    return result;
}
