#include <Arduino.h>
#include "MainApp.hpp"

// Instance unique de l'application
static MainApp app;

void setup()
{
    // Initialisation de l'application
    (void)app.init();
}

void loop()
{
    // Boucle principale non bloquante
    app.process();
}