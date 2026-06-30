#include "app_main.h"

/**
 * @file main.c
 * @brief Point d'entree impose par le framework ESP-IDF.
 *        Se contente d'appeler la couche Application (app_main_init / app_main_run).
 */
void app_main(void)
{
    app_error_t result = app_main_init();

    if (result == APP_OK)
    {
        app_main_run();
    }
}
