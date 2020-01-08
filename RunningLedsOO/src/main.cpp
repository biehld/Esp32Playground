#include <stdexcept>

#include <Arduino.h>

#include "Devices.h"
#include <functional>

devices::Led red(25);
devices::Led yellow(26);
devices::Led green(27);

auto leds = {&green, &yellow, &red};

devices::Button buttonRed(21);
devices::Button buttonYellow(23);
devices::Button buttonGreen(19);

bool running = true;
bool running1 = true;

void setup()
{
  buttonRed.released += []() { red.blinkAsync(1000); };
  buttonYellow.released += []() { yellow.toggle(); };
  buttonGreen.pressed += []() { green.toggle(); };

  Serial.begin(CONFIG_MONITOR_BAUD);
}

void loop()
{
  
}