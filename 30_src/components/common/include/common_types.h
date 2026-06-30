#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @file common_types.h
 * @brief Types et codes d'erreur communs a tous les modules du projet.
 *
 * Regle MISRA (niveau debutant) appliquee ici :
 *  - Utilisation d'une enumeration nommee plutot que des "magic numbers"
 *    pour representer un statut de retour de fonction.
 *  - Toutes les fonctions du projet retournent un app_error_t : un seul
 *    type de retour homogene, plus facile a verifier systematiquement.
 */

typedef enum
{
    APP_OK = 0,                /**< Operation reussie */
    APP_ERROR_NULL_POINTER,    /**< Pointeur NULL passe en parametre */
    APP_ERROR_INVALID_PARAM,   /**< Parametre hors plage autorisee */
    APP_ERROR_I2C,              /**< Erreur sur le bus I2C */
    APP_ERROR_SPI,              /**< Erreur sur le bus SPI */
    APP_ERROR_TIMEOUT,          /**< Depassement de delai */
    APP_ERROR_NOT_INITIALIZED   /**< Module utilise avant son initialisation */
} app_error_t;

#endif /* COMMON_TYPES_H */
