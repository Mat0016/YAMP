# Regles MISRA C appliquees (niveau debutant)

Ce projet n'implemente pas une conformite MISRA-C complete (cela demande un
analyseur statique dedie type PC-lint, Polyspace ou Cppcheck/MISRA addon),
mais applique manuellement les regles les plus importantes et les plus
accessibles pour un debutant. Chacune est illustree par un exemple tire du
code du projet.

## 1. Types explicites (pas de `int` ou `char` nu)
On utilise systematiquement les types de `<stdint.h>` (`uint8_t`, `int16_t`,
`uint32_t`...) plutot que `int`, `short`, `long`, dont la taille depend du
compilateur/de la cible.

```c
uint32_t tension_bus_mv;
int32_t  courant_ma;
```

## 2. Pas de "magic numbers"
Toute valeur numerique significative (broche, adresse, seuil, frequence)
est definie par une constante nommee dans `common_config.h`, jamais ecrite
en clair dans le code metier.

```c
#define CFG_INA237_I2C_ADDR (0x40U)
...
result = ina237_init(&g_ina237_handle, (uint8_t)CFG_INA237_I2C_ADDR);
```

## 3. Accolades obligatoires, meme pour un bloc d'une ligne
Toute structure `if`/`else`/`for`/`while` utilise des accolades, meme si le
corps ne contient qu'une instruction, pour eviter les erreurs d'indentation
trompeuse.

```c
if (courant_ma >= seuil_ma)
{
    alarme_active = true;
}
else
{
    alarme_active = false;
}
```

## 4. Un seul point de sortie par fonction
Chaque fonction declare sa variable de retour en debut de fonction et la
renseigne au fil des branches, avec un unique `return` final. Cela facilite
la relecture et la preuve de couverture de test.

```c
app_error_t result;
if (p_handle == NULL)
{
    result = APP_ERROR_NULL_POINTER;
}
else
{
    ...
    result = APP_OK;
}
return result;
```

## 5. Verification systematique des pointeurs avant utilisation
Toute fonction recevant un pointeur en parametre verifie qu'il n'est pas
`NULL` avant de le dereferencer.

```c
if ((p_handle == NULL) || (p_value == NULL))
{
    result = APP_ERROR_NULL_POINTER;
}
```

## 6. Initialisation systematique des variables
Toute variable locale est initialisee a sa declaration (jamais de valeur
indeterminee lue par erreur).

```c
uint16_t raw_value = 0U;
uint8_t raw_bytes[2] = {0U, 0U};
```

## 7. Garde d'inclusion (`#ifndef` / `#define` / `#endif`) sur tous les headers
Chaque fichier `.h` est protege contre les inclusions multiples.

```c
#ifndef DRV_INA237_H
#define DRV_INA237_H
...
#endif /* DRV_INA237_H */
```

## 8. Separation stricte des couches (pas de dependance "remontante")
La couche Mid Level ne connait jamais la couche Application. La couche
Application n'inclut jamais directement les headers ESP-IDF
(`driver/i2c.h`, `driver/spi_master.h`) : elle passe toujours par les
pilotes `drv_*`. Cela limite le couplage et facilite le portage /
les tests.

## 9. Pas d'allocation dynamique de memoire
Aucun `malloc`/`free` n'est utilise : toutes les structures (handles,
buffers) sont allouees statiquement ou sur la pile, ce qui est la pratique
standard en embarque critique.

## 10. Fonctions courtes et a responsabilite unique
Chaque fichier `.c` adresse un seul peripherique ou une seule
responsabilite (ex. : `drv_ina237_convert.c` ne fait que des conversions,
`drv_i2c.c` ne fait que parler au bus I2C). Cela limite la complexite
cyclomatique de chaque fonction et facilite les tests unitaires cibles.

## Limites de cette demarche
Ces regles sont appliquees manuellement et au niveau debutant. Pour un
projet industriel necessitant une certification MISRA-C, il est recommande
d'utiliser un outil d'analyse statique dedie (ex. : Cppcheck avec
l'addon MISRA, PC-lint Plus, Polyspace) integre a la chaine d'integration
continue.
