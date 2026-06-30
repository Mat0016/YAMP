# Architecture logicielle — projet_esp32_src

## 1. Vue d'ensemble

Le projet est decoupe en **3 couches** strictement separees, chacune dans son
propre composant ESP-IDF (`30_src/components/...`) :

```
30_src/
├── main/                     Point d'entree ESP-IDF (app_main())
├── components/
│   ├── app/                  Couche APPLICATION
│   │   ├── include/
│   │   │   ├── app_main.h
│   │   │   ├── app_power_monitor.h
│   │   │   ├── app_power_monitor_logic.h
│   │   │   └── app_ihm.h
│   │   ├── app_main.c
│   │   ├── app_power_monitor.c
│   │   ├── app_power_monitor_logic.c   (logique pure, testable sans materiel)
│   │   └── app_ihm.c
│   │
│   ├── midlevel/              Couche MID LEVEL (pilotes)
│   │   ├── include/
│   │   │   ├── drv_i2c.h
│   │   │   ├── drv_spi.h
│   │   │   ├── drv_ina237.h
│   │   │   ├── drv_ina237_convert.h    (conversions pures, testables)
│   │   │   └── drv_ihm.h
│   │   ├── drv_i2c.c          (module ESP32 : driver/i2c.h)
│   │   ├── drv_spi.c          (module ESP32 : driver/spi_master.h)
│   │   ├── drv_ina237.c
│   │   ├── drv_ina237_convert.c
│   │   └── drv_ihm.c          (module ESP32 : driver/gpio.h pour les boutons)
│   │
│   └── common/                 Types et configuration partages
│       └── include/
│           ├── common_types.h
│           └── common_config.h

60_valid/
└── test/
    ├── test_main.c            Tests unitaires (fonctions pures, hors cible)
    ├── Makefile
    └── README.md
```

## 2. Role de chaque couche

### Couche Application (`components/app`)
Contient la logique metier : que faire des mesures, quand declencher une
alarme, comment les presenter a l'utilisateur. Cette couche **ne parle
jamais directement au materiel** (pas d'appel I2C/SPI/GPIO direct) : elle
passe systematiquement par la couche Mid Level.

| Fichier                        | Role                                                          |
|---------------------------------|----------------------------------------------------------------|
| `app_main.c` / `.h`             | Orchestrateur : initialise les bus puis les modules, boucle principale |
| `app_power_monitor.c` / `.h`    | Acquiert les mesures INA237, calcule l'etat d'alarme           |
| `app_power_monitor_logic.c/.h`  | Fonction pure de comparaison au seuil (testable sans materiel) |
| `app_ihm.c` / `.h`              | Met en forme et envoie les mesures a l'IHM                     |

### Couche Mid Level (`components/midlevel`)
Contient les pilotes : chaque fichier encapsule un seul peripherique ou un
seul bus de communication. C'est la seule couche qui inclut les modules
ESP32 fournis par ESP-IDF (`driver/i2c.h`, `driver/spi_master.h`,
`driver/gpio.h`).

| Fichier                       | Role                                                            |
|---------------------------------|------------------------------------------------------------------|
| `drv_i2c.c` / `.h`              | Pilote generique du bus I2C de l'ESP32                          |
| `drv_spi.c` / `.h`              | Pilote generique du bus SPI de l'ESP32                          |
| `drv_ina237.c` / `.h`           | Lecture/ecriture des registres du capteur INA237 (via drv_i2c)  |
| `drv_ina237_convert.c/.h`       | Conversions registres bruts -> mV / mA / mW (fonctions pures)   |
| `drv_ihm.c` / `.h`              | Ecran (via drv_spi) et boutons (via GPIO ESP32)                 |

### Couche commune (`components/common`)
Types d'erreur (`app_error_t`) et constantes de configuration (broches,
adresses, seuils), partages par toutes les couches afin d'eviter toute
duplication et tout "magic number" disperse dans le code.

## 3. Flux de donnees (resume)

1. `app_main_init()` initialise le bus I2C (`drv_i2c_init`), puis le module
   `app_power_monitor` (qui initialise le capteur `INA237` via `ina237_init`),
   puis le module `app_ihm` (qui initialise le SPI et les boutons via
   `drv_ihm_init`).
2. `app_main_run()` boucle toutes les `CFG_APP_PERIODE_MS` millisecondes :
   - `app_power_monitor_process()` lit tension/courant/puissance sur
     l'INA237 (I2C) et determine l'etat d'alarme.
   - `app_ihm_afficher_mesure()` envoie le resultat a l'ecran (SPI).

## 4. Pourquoi separer la "logique pure" du materiel ?

`app_power_monitor_logic.c` et `drv_ina237_convert.c` ne font **aucune**
entree/sortie (pas d'I2C, pas de SPI, pas de log). Elles ne dependent que de
`<stdint.h>` / `<stdbool.h>`. Cela permet de les compiler et de les tester
directement sur un PC, sans materiel ESP32 ni framework ESP-IDF — voir
`60_valid/test/test_main.c`.

## 5. Construire le projet ESP-IDF

```bash
cd 30_src
idf.py set-target esp32
idf.py build
idf.py -p /dev/ttyUSBx flash monitor
```
