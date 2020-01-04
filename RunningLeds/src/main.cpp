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

struct Entry
{
  int led;
  int time;
  int state;
};

auto runningLeds = {
  Entry{green, 50, HIGH},
  Entry{yellow, 50, HIGH},
  Entry{red, 50, HIGH},
  Entry{green, 50, LOW},
  Entry{yellow, 50, LOW},
  Entry{red, 50, LOW},

  Entry{none, 50, LOW},

  Entry{green, 50, HIGH},
  Entry{yellow, 50, HIGH},
  Entry{red, 50, HIGH},
  Entry{red, 50, LOW},
  Entry{yellow, 50, LOW},
  Entry{green, 50, LOW},
  
  Entry{none, 50, LOW},

  Entry{yellow, 0, LOW},
  Entry{green, 0, HIGH},  
  Entry{red, 0, HIGH},

  Entry{none, 100, LOW},

  Entry{yellow, 0, HIGH},
  Entry{green, 0, LOW},  
  Entry{red, 0, LOW},

  Entry{none, 100, LOW},

  Entry{yellow, 0, LOW},
  Entry{green, 0, HIGH},  
  Entry{red, 0, HIGH},

  Entry{none, 100, LOW},

  Entry{yellow, 0, HIGH},
  Entry{green, 0, LOW},  
  Entry{red, 0, LOW},

  Entry{yellow, 100, LOW},

  Entry{green, 100, HIGH},
  Entry{green, 100, LOW},
  Entry{yellow, 100, HIGH},
  Entry{yellow, 100, LOW},
  Entry{red, 100, HIGH},
  Entry{red, 100, LOW},
};

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
        digitalWrite(currentLed->led, currentLed->state);
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