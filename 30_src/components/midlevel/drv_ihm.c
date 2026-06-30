#include "drv_ihm.h"
#include "drv_spi.h"
#include "common_config.h"

#include "driver/gpio.h" /* Module ESP32 (ESP-IDF) : lecture des boutons */

#include <string.h>

static bool g_ihm_is_initialized = false;

app_error_t drv_ihm_init(void)
{
    app_error_t result;

    result = drv_spi_init();

    if (result == APP_OK)
    {
        gpio_config_t gpio_conf;

        gpio_conf.pin_bit_mask = (1ULL << CFG_IHM_GPIO_BOUTON_NEXT) |
                                  (1ULL << CFG_IHM_GPIO_BOUTON_OK);
        gpio_conf.mode = GPIO_MODE_INPUT;
        gpio_conf.pull_up_en = GPIO_PULLUP_ENABLE;
        gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        gpio_conf.intr_type = GPIO_INTR_DISABLE;

        (void)gpio_config(&gpio_conf);

        g_ihm_is_initialized = true;
    }

    return result;
}

app_error_t drv_ihm_effacer_ecran(void)
{
    app_error_t result;

    if (!g_ihm_is_initialized)
    {
        result = APP_ERROR_NOT_INITIALIZED;
    }
    else
    {
        static const uint8_t CMD_CLEAR_SCREEN = 0x01U;

        result = drv_spi_write(&CMD_CLEAR_SCREEN, sizeof(CMD_CLEAR_SCREEN), true);
    }

    return result;
}

app_error_t drv_ihm_ecrire_ligne(uint8_t numero_ligne, const char *p_texte)
{
    app_error_t result;

    if (p_texte == NULL)
    {
        result = APP_ERROR_NULL_POINTER;
    }
    else if (!g_ihm_is_initialized)
    {
        result = APP_ERROR_NOT_INITIALIZED;
    }
    else if (numero_ligne >= CFG_IHM_NB_LIGNES)
    {
        result = APP_ERROR_INVALID_PARAM;
    }
    else
    {
        size_t longueur_texte = strnlen(p_texte, (size_t)CFG_IHM_NB_CARACT_LIGNE);

        result = drv_spi_write((const uint8_t *)p_texte, longueur_texte, false);
    }

    return result;
}

app_error_t drv_ihm_lire_bouton(ihm_bouton_id_t bouton_id, bool *p_est_appuye)
{
    app_error_t result;

    if (p_est_appuye == NULL)
    {
        result = APP_ERROR_NULL_POINTER;
    }
    else if (!g_ihm_is_initialized)
    {
        result = APP_ERROR_NOT_INITIALIZED;
    }
    else
    {
        gpio_num_t gpio_num;
        int niveau_logique;

        if (bouton_id == IHM_BOUTON_NEXT)
        {
            gpio_num = (gpio_num_t)CFG_IHM_GPIO_BOUTON_NEXT;
        }
        else
        {
            gpio_num = (gpio_num_t)CFG_IHM_GPIO_BOUTON_OK;
        }

        niveau_logique = gpio_get_level(gpio_num);

        /* Boutons cables en pull-up : "appuye" correspond au niveau bas (0) */
        *p_est_appuye = (niveau_logique == 0);

        result = APP_OK;
    }

    return result;
}
