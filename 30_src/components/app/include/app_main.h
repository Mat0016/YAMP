#ifndef APP_MAIN_H
#define APP_MAIN_H

#include "common_types.h"

/**
 * @file app_main.h
 * @brief Couche Application : orchestrateur principal. Initialise les
 *        bus, les pilotes et les modules applicatifs, puis lance la
 *        boucle principale.
 */

app_error_t app_main_init(void);

void app_main_run(void);

#endif /* APP_MAIN_H */
