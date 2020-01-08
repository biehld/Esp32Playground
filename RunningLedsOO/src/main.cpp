#include <vector>

#include <Arduino.h>

#include "Devices.h"

devices::Led red(25);
devices::Led yellow(26);
devices::Led green(27);

std::vector<devices::Led *> leds = {&green, &yellow, &red};

devices::Button buttonRed(21);
devices::Button buttonYellow(23);
devices::Button buttonGreen(19);

bool running = true;
bool running1 = true;

void setup()
{
  buttonRed.released += []() { running = !running; };  

  Serial.begin(CONFIG_MONITOR_BAUD);

  srand(time(nullptr));
}

void loop()
{
  if (running)
  {
    leds[rand() % leds.size()]->blink(rand() % 500, rand() % 500);
  }
  for (auto i=0;i<leds.size();i++) {
    leds[i]->blink();
  }
}