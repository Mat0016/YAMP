#include "app_power_monitor_logic.h"

bool app_power_monitor_check_alarm(int32_t courant_mA, int32_t seuil_mA)
{
    bool alarme_active;

    if (courant_mA >= seuil_mA)
    {
        alarme_active = true;
    }
    else
    {
        alarme_active = false;
    }

    return alarme_active;
}
