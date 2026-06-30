# Tests unitaires — projet_esp32_src

Ce dossier contient les tests d'ecriture des fonctions purement logicielles
du projet (sans dependance au materiel ESP32 ni au framework ESP-IDF).

## Pourquoi ces fonctions sont testables sans materiel ?

Deux fichiers du projet sont volontairement ecrits sans aucune
entree/sortie (pas d'I2C, pas de SPI, pas de log) :

- `30_src/components/midlevel/drv_ina237_convert.c` : conversions des
  registres bruts de l'INA237 vers des unites physiques (mV, mA, mW).
- `30_src/components/app/app_power_monitor_logic.c` : decision de
  declenchement de l'alarme courant.

Ce sont des fonctions "pures" : meme entree -> toujours la meme sortie,
aucun effet de bord. Elles sont donc testables directement sur PC avec un
compilateur C standard, sans avoir besoin d'un ESP32 connecte ni d'ESP-IDF
installe.

## Compiler et lancer les tests

```bash
cd 60_valid/test
make test
```

Sortie attendue :

```
=== Tests unitaires - modules logiciels purs (sans materiel) ===

[OK]   ina237_convert_vbus_raw_to_mv(1000) == 3125 mV
[OK]   ina237_convert_vbus_raw_to_mv(0) == 0 mV
[OK]   ina237_convert_current_raw_to_ma(500, 1000) == 500 mA
[OK]   ina237_convert_current_raw_to_ma(-200, 1000) == -200 mA
[OK]   ina237_convert_power_raw_to_mw(100, 1000) == 2500 mW
[OK]   check_alarm(5000 < seuil 8000) -> pas d'alarme
[OK]   check_alarm(8000 == seuil 8000) -> alarme (limite incluse)
[OK]   check_alarm(9000 > seuil 8000) -> alarme
[OK]   check_alarm(courant negatif) -> pas d'alarme

=== Resultat : 9 test(s) OK / 0 test(s) en echec ===
```

Le code de retour du programme est `0` si tous les tests passent, `1`
sinon — il peut donc etre utilise directement dans une chaine
d'integration continue (CI).

## Nettoyer

```bash
make clean
```

## Ajouter un nouveau test

1. Ecrire une fonction `static void test_xxx(void)` qui appelle une ou
   plusieurs fois `test_verifier_u32`, `test_verifier_i32` ou
   `test_verifier_bool` avec le resultat attendu et le resultat obtenu.
2. Appeler cette fonction depuis `main()`.
3. Si le test porte sur une nouvelle fonction non encore "pure"
   (dependant du materiel), envisager d'abord d'en extraire la partie
   logique dans un fichier dedie (voir `app_power_monitor_logic.c` comme
   modele), afin de garder ce dossier de test independant du materiel.
