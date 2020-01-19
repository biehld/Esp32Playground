#include <iostream>
#include <vector>

#include <Arduino.h>
#include <driver/i2s.h>

#include "Devices.h"

using devices::Button;
using devices::GPIODevice;
using devices::Led;

Led ledRed(25, "red");
Led ledYellow(26, "yellow");
Led ledGreen(27, "green");

std::vector<devices::Led *> leds = {&ledGreen, &ledYellow, &ledRed};

Button buttonRed(21, "buttonRed");

Button buttonYellow(23, "buttonYellow");
Button buttonGreen(19, "buttonGreen");

bool running = true;

void on_led_changed(devices::GPIODevice *sender, int newValue) {
    Serial.printf("led %s is switched to %d\n", sender->name().c_str(), newValue);
}

int blinkTime = 0;

void setup() {
    Serial.begin(CONFIG_MONITOR_BAUD);

    ledRed.changed += &on_led_changed;
    ledYellow.changed += &on_led_changed;
    ledGreen.changed += &on_led_changed;

    buttonRed.released += [](auto) { ledRed.toggle(); };
    buttonYellow.pressed += [](auto) { ledYellow.blinkAsync(1000); };
    buttonGreen.released += [](auto) { ledGreen.toggle(); };
}

void loop() {
}
