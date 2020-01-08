#include <stdexcept>

#include <Arduino.h>

#include "Devices.h"
#include <functional>

devices::Led red("red", 25);
devices::Led yellow("yellow", 26);
devices::Led green("green", 27);

auto leds = {&green, &yellow, &red};

devices::Button buttonRed("button", 21, devices::Pulldown);
devices::Button buttonYellow("button", 23, devices::Pulldown);
devices::Button buttonGreen("button", 19, devices::Pulldown);

bool running = true;
bool running1 = true;

void setup()
{
  buttonRed.released += []() { red.blinkAsync(1000);};
  buttonYellow.released += []() { yellow.toggle(); };
  buttonGreen.pressed += []() { green.toggle(); };

  Serial.begin(CONFIG_MONITOR_BAUD);  
}

void loop()
{
  
}