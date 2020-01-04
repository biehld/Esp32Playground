#include <Arduino.h>

const int red = 25;
const int yellow = 26;
const int green = 27;

auto leds = {green, yellow, red};

const int button = 23;

void setup()
{
  Serial.begin(CONFIG_MONITOR_BAUD);

  for (auto l : leds)
  {
    pinMode(l, OUTPUT);
  }

  pinMode(button, INPUT_PULLDOWN);
}

struct Entry
{
  uint8_t led;
  int time;
};

auto runningLeds = {Entry{green, 50}, Entry{yellow, 50}, Entry{red, 50}};

auto currentLed = runningLeds.begin();
auto isRunning = false;

auto lastButtonTime = 0;
auto lastButtonValue = LOW;

auto lastLedSwitchTime = 0;
auto ledState = false;

const auto ButtonBounceTime = 5;

void loop()
{
  auto buttonValue = digitalRead(button);
  auto currentButtonTime = millis();
  auto timeDiff = currentButtonTime - lastButtonTime;
  if (buttonValue != lastButtonValue && timeDiff > ButtonBounceTime)
  {
    if (buttonValue == LOW)
    {
      isRunning = !isRunning;
    }

    lastButtonTime = currentButtonTime;
    lastButtonValue = buttonValue;

    if (!isRunning)
    {
      // switch all off
      for (auto l : leds)
      {
        digitalWrite(l, LOW);
      }
      currentLed = runningLeds.begin();
      lastLedSwitchTime = 0;
      ledState = false;
    }
  }

  auto currentLedSwitchTime = millis();
  if (isRunning)
  {
    if (currentLedSwitchTime - lastLedSwitchTime > currentLed->time)
    {
      ledState = !ledState;

      lastLedSwitchTime = currentLedSwitchTime;
      if (currentLed->led >= 0)
      {
        digitalWrite(currentLed->led, digitalRead(currentLed->led) == HIGH ? LOW : HIGH);
      }
      if (!ledState)
      {
        currentLed++;
        if (currentLed == runningLeds.end())
        {
          currentLed = runningLeds.begin();
        }
      }
    }
  }
}