#include <Arduino.h>

const auto none = -1;
const auto red = 25;
const auto yellow = 26;
const auto green = 27;

auto leds = {green, yellow, red};

const auto button = 23;

void setup()
{
  Serial.begin(CONFIG_MONITOR_BAUD);

  for (auto l : leds)
  {
    pinMode(l, OUTPUT);
  }

  pinMode(button, INPUT_PULLDOWN);
}

struct StateEntry
{
  int led;
  int time;
  int state;
};

auto runningLedsStates = {
    StateEntry{green, 50, HIGH},
    StateEntry{yellow, 50, HIGH},
    StateEntry{red, 50, HIGH},
    StateEntry{green, 50, LOW},
    StateEntry{yellow, 50, LOW},
    StateEntry{red, 50, LOW},

    StateEntry{none, 50, LOW},

    StateEntry{green, 50, HIGH},
    StateEntry{yellow, 50, HIGH},
    StateEntry{red, 50, HIGH},
    StateEntry{red, 50, LOW},
    StateEntry{yellow, 50, LOW},
    StateEntry{green, 50, LOW},

    StateEntry{none, 50, LOW},

    StateEntry{yellow, 0, LOW},
    StateEntry{green, 0, HIGH},
    StateEntry{red, 0, HIGH},

    StateEntry{none, 100, LOW},

    StateEntry{yellow, 0, HIGH},
    StateEntry{green, 0, LOW},
    StateEntry{red, 0, LOW},

    StateEntry{none, 100, LOW},

    StateEntry{yellow, 0, LOW},
    StateEntry{green, 0, HIGH},
    StateEntry{red, 0, HIGH},

    StateEntry{none, 100, LOW},

    StateEntry{yellow, 0, HIGH},
    StateEntry{green, 0, LOW},
    StateEntry{red, 0, LOW},

    StateEntry{yellow, 100, LOW},

    StateEntry{green, 100, HIGH},
    StateEntry{green, 100, LOW},
    StateEntry{yellow, 100, HIGH},
    StateEntry{yellow, 100, LOW},
    StateEntry{red, 100, HIGH},
    StateEntry{red, 100, LOW},
};

auto currentLedState = runningLedsStates.begin();
auto isRunning = false;

auto lastButtonTime = 0;
auto lastButtonValue = LOW;

auto lastLedSwitchTime = 0;

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
      currentLedState = runningLedsStates.begin();
      lastLedSwitchTime = 0;      
    }
  }

  auto currentLedSwitchTime = millis();
  if (isRunning)
  {
    if (currentLedSwitchTime - lastLedSwitchTime > currentLedState->time)
    {
      lastLedSwitchTime = currentLedSwitchTime;
      if (currentLedState->led >= 0)
      {
        digitalWrite(currentLedState->led, currentLedState->state);
      }

      currentLedState++;
      if (currentLedState == runningLedsStates.end())
      {
        currentLedState = runningLedsStates.begin();
      }
    }
  }
}