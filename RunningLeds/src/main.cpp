#include <Arduino.h>
#include <list>
#include <vector>

const int red = 25;
const int yellow = 26;
const int green = 27;

const int button = 23;

void setup()
{
  Serial.begin(115200);

  pinMode(red, OUTPUT);
  pinMode(yellow, OUTPUT);
  pinMode(green, OUTPUT);

  pinMode(button, INPUT_PULLDOWN);
}

std::list<int> leds = {red, yellow, green};

auto currentLed = leds.begin();
auto state = false;

auto lastButtonTime = millis();
auto lastButtonValue = LOW;

auto lastLedSwitchTime = millis();
auto ledState = false;

void loop()
{
  auto buttonValue = digitalRead(button);
  auto currentButtonTime = millis();
  auto timeDiff = currentButtonTime - lastButtonTime;
  if (buttonValue != lastButtonValue && timeDiff > 5)
  {
    if (buttonValue == LOW)
    {
      state = !state;
    }

    lastButtonTime = currentButtonTime;
    lastButtonValue = buttonValue;
  }

  auto currentLedSwitchTime = millis();
  if (state && (currentLedSwitchTime - lastLedSwitchTime) > 100)
  {
    ledState = !ledState;

    lastLedSwitchTime = currentLedSwitchTime;
    digitalWrite(*currentLed, digitalRead(*currentLed) == HIGH ? LOW : HIGH);

    if (!ledState)
    {
      currentLed++;
      if (currentLed == leds.end())
      {
        leds.reverse();
        currentLed = leds.begin();
        currentLed++;
      }
    }
  }
}