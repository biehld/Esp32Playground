#include "Devices.h"

#include <Arduino.h>
#include <functional>

namespace devices
{

GPIODevice::GPIODevice(std::string name, int pin) : _name(std::move(name)), _pin(pin)
{
}

GPIODevice::~GPIODevice()
{
}

int GPIODevice::pin() const
{
    return _pin;
}

std::string GPIODevice::name() const
{
    return _name;
}

OutputDevice::OutputDevice(std::string name, int pin) : GPIODevice(name, pin)
{
    pinMode(pin, OUTPUT);
}

bool OutputDevice::value()
{
    return digitalRead(pin()) == HIGH;
}

void OutputDevice::set_value(bool v)
{
    digitalWrite(pin(), v ? HIGH : LOW);
}

Switch::Switch(std::string name, int pin) : OutputDevice(name, pin)
{
}

void Switch::on()
{
    set_value(true);
}

void Switch::off()
{
    set_value(false);
}

void Switch::toggle()
{
    set_value(!value());
}

InputDevice::InputDevice(std::string name, int pin, InputMode inputMode) : GPIODevice(name, pin), _inputMode(inputMode)
{
    switch (inputMode)
    {
    case Normal:
        pinMode(pin, INPUT);
        break;
    case Pulldown:
        pinMode(pin, INPUT_PULLDOWN);
        break;
    case Pullup:
        pinMode(pin, INPUT_PULLUP);
        break;
    default:
        pinMode(pin, INPUT);
        break;
    }
}

int InputDevice::value()
{
    return digitalRead(pin());
}

DigitalInputDevice::DigitalInputDevice(std::string name, int pin, InputMode inputMode) : InputDevice(name, pin, inputMode)
{
    attachInterruptArg(pin, [](void *arg) IRAM_ATTR { static_cast<DigitalInputDevice *>(arg)->on_interrupt(); }, this, CHANGE);
}

DigitalInputDevice::~DigitalInputDevice()
{
    detachInterrupt(pin());
}

IRAM_ATTR void DigitalInputDevice::on_interrupt()
{
    on_changed(value());
}

void DigitalInputDevice::on_changed(bool newValue)
{
    changed(newValue);
}

Button::Button(std::string name, int pin, InputMode inputMode) : DigitalInputDevice(name, pin, inputMode)
{
}

void Button::on_changed(bool newValue)
{
    DigitalInputDevice::on_changed(newValue);

    auto currentTime = millis();
    if (newValue != _pressed && currentTime - _lastTime > bounceTime)
    {
        if (newValue)
            on_pressed();
        else
            on_released();

        _pressed = newValue;
        _lastTime = currentTime;
    }
}

bool Button::is_pressed()
{
    return _pressed;
}

void Button::on_pressed()
{
    pressed();
}

void Button::on_released()
{
    released();
}

Led::Led(std::string name, int pin) : Switch(name, pin)
{
}

void Led::blink(int time)
{
    blink(time, time);
}

void Led::blink(int on_time, int off_time)
{
    on();
    delay(on_time);
    off();
    delay(off_time);
}

void Led::blinkAsync(int time)
{
    blinkAsync(time, time);
}

struct TaskBlinkParams
{
    Led *led;
    int on_time;
    int off_time;
};

void task_blink_led(void *params)
{
    auto blinkParams = static_cast<TaskBlinkParams *>(params);

    blinkParams->led->blink(blinkParams->on_time, blinkParams->off_time);

    vTaskDelete(nullptr);
}

void Led::blinkAsync(int on_time, int off_time)
{
    auto params = new TaskBlinkParams{this, on_time, off_time};

    xTaskCreate(&task_blink_led, "blink", 10000, params, 0, nullptr);
}

} // namespace devices