#include <Arduino.h>

#include "MainApp.hpp"

namespace
{
    MainApp g_main_app;
}

void setup()
{
    g_main_app.init();
}

void loop()
{
    g_main_app.process();
}
