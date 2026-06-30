#include "app_main.h"
#include "app_power_monitor.h"
#include "app_ihm.h"
#include "drv_i2c.h"
#include "common_config.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "include/app_power_monitor.h"
#include <projet_esp32_src/30_src/components/common/include/common_types.h>

app_error_t app_main_init(void)
{
    app_error_t result;

    /* 1. Initialisation du module ESP32 de bus I2C (utilise par l'INA237) */
    result = drv_i2c_init();

    /* 2. Initialisation du module applicatif de mesure de puissance */
    if (result == APP_OK)
    {
        result = app_power_monitor_init();
    }

    /* 3. Initialisation du module applicatif IHM (initialise le SPI en interne) */
    if (result == APP_OK)
    {
        result = app_ihm_init();
    }

    return result;
}

void app_main_run(void)
{
    app_power_mesure_t mesure_courante;
    app_error_t result;

    for (;;)
    {
        result = app_power_monitor_process(&mesure_courante);

        if (result == APP_OK)
        {
            (void)app_ihm_afficher_mesure(&mesure_courante);
        }

        vTaskDelay(pdMS_TO_TICKS(CFG_APP_PERIODE_MS));
    }
}
