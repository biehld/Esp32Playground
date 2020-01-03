#include <Arduino.h>

#include <StateMachine.h>

const int red = 25;
const int yellow = 26;
const int green = 27;

const int freq = 2000;
const int channel = 0;
const int resolution = 8;
const int buzzer = 33;

const int button = 23;

void melodyHalt(void *params)
{
  ledcAttachPin(buzzer, channel);
  ledcWriteNote(channel, NOTE_C, 5);
  delay(500);
  ledcWriteNote(channel, NOTE_D, 5);
  delay(500);
  ledcWriteNote(channel, NOTE_E, 5);
  delay(500);
  ledcWriteNote(channel, NOTE_F, 5);
  delay(500);
  ledcDetachPin(buzzer);

  vTaskDelete(nullptr);
}

void melodyGo(void *params)
{
  ledcAttachPin(buzzer, channel);
  ledcWriteNote(channel, NOTE_C, 5);
  delay(200);
  ledcWriteNote(channel, NOTE_F, 5);
  delay(500);
  ledcDetachPin(buzzer);

  vTaskDelete(nullptr);
}

StateMachine machine;

auto stateGo = machine.addState([]() {
  if (machine.executeOnce)
  {
    digitalWrite(red, LOW);
    digitalWrite(yellow, LOW);
    digitalWrite(green, HIGH);
  }
});

auto stateHaltRequested = machine.addState([]() {
  if (machine.executeOnce)
  {
    xTaskCreate(melodyGo, "melodytask", 10000, nullptr, 1, nullptr);
    delay(2000);
  }
});

auto stateAttentionHalt = machine.addState([]() {
  if (machine.executeOnce)
  {
    digitalWrite(red, LOW);
    digitalWrite(yellow, HIGH);
    digitalWrite(green, HIGH);
    delay(2000);
  }
});

auto stateHalt = machine.addState([]() {
  if (machine.executeOnce)
  {
    digitalWrite(red, HIGH);
    digitalWrite(yellow, LOW);
    digitalWrite(green, LOW);

    xTaskCreate(melodyHalt, "melodytask", 10000, nullptr, 1, nullptr);

    delay(6000);
  }
});

auto stateAttentionGo = machine.addState([]() {
  if (machine.executeOnce)
  {
    digitalWrite(red, HIGH);
    digitalWrite(yellow, HIGH);
    digitalWrite(green, LOW);

    delay(2000);
  }
});

auto lastTime = millis();

void setup()
{
  Serial.begin(115200);
  Serial.println("hi there...");

  pinMode(red, OUTPUT);
  pinMode(yellow, OUTPUT);
  pinMode(green, OUTPUT);

  ledcSetup(channel, freq, resolution);
  ledcAttachPin(buzzer, channel);

  pinMode(button, INPUT_PULLDOWN);

  stateGo->addTransition([]() { return digitalRead(button) == HIGH; }, stateHaltRequested);
  stateHaltRequested->addTransition([]() { return true; }, stateAttentionHalt);
  stateAttentionHalt->addTransition([]() { return true; }, stateHalt);
  stateHalt->addTransition([]() { return true; }, stateAttentionGo);
  stateAttentionGo->addTransition([]() { return true; }, stateGo);

  attachInterrupt(button, []() {
    static auto last = 0;

    auto v = digitalRead(button);
    auto current = millis();

    auto diff = current - lastTime;
    if (v != last && diff > 5)
    {
      Serial.printf("changed %i %s %ld\n", v, v == HIGH ? "pressed" : "released", diff);
      lastTime = current;
      last = v;
    }
  },
                  CHANGE);
}

void loop()
{
  machine.run();
  delay(100);
  // while (true) {
  //   printf("%d", digitalRead(button));
  // }
}