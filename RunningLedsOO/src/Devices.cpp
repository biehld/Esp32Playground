#include "Devices.h"

#include <Arduino.h>

#include <Ticker.h>

#include <functional>

namespace devices {

class xSemaphoreLockGuard {
public:
    explicit xSemaphoreLockGuard(SemaphoreHandle_t &semHandle, TickType_t blockTime = portMAX_DELAY) : _semHandle(semHandle) {
        xSemaphoreTake(_semHandle, portMAX_DELAY);
    }

    xSemaphoreLockGuard(const xSemaphoreLockGuard &other) = delete;
    xSemaphoreLockGuard(xSemaphoreLockGuard &&other) = delete;

    ~xSemaphoreLockGuard() {
        xSemaphoreGive(_semHandle);
    }

private:
    SemaphoreHandle_t &_semHandle;
};

struct GPIODevicePrivate {
    portMUX_TYPE mutex = portMUX_INITIALIZER_UNLOCKED;
};

GPIODevice::GPIODevice(int pin, int mode, std::string name) : _pin(pin), _mode(mode), _name(name), _privateImpl(new GPIODevicePrivate()) {
    pinMode(pin, OUTPUT);

    attachInterruptArg(pin,
                       [](void *arg) IRAM_ATTR {
                           auto d = static_cast<GPIODevice *>(arg);
                           portENTER_CRITICAL_ISR(&d->_privateImpl->mutex);

                           xTaskCreate(
                               [](void *arg) IRAM_ATTR {
                                   auto d = static_cast<GPIODevice *>(arg);

                                   d->on_changed(d->value());

                                   vTaskDelete(nullptr);
                               },
                               "change_interrupt", 2000, arg, 0, nullptr);

                           portEXIT_CRITICAL_ISR(&static_cast<GPIODevice *>(arg)->_privateImpl->mutex);
                       },
                       this, CHANGE);
}

GPIODevice::~GPIODevice() {
    detachInterrupt(pin());
    delete _privateImpl;
}

int GPIODevice::pin() const {
    return _pin;
}

int GPIODevice::mode() const {
    return _mode;
}

std::string GPIODevice::name() const {
    return _name;
}

void GPIODevice::on_changed(int newValue) {
    changed(this, newValue);
}

OutputDevice::OutputDevice(int pin, std::string name) : GPIODevice(pin, OUTPUT, name) {
}

int OutputDevice::value() {
    return digitalRead(pin());
}

DigitalOutputDevice::DigitalOutputDevice(int pin, int initialValue, std::string name) : OutputDevice(pin, name) {
    set_value(initialValue);
}

void DigitalOutputDevice::set_value(int v) {
    digitalWrite(pin(), v);
}

bool DigitalOutputDevice::is_on() {
    return value() == HIGH;
}
bool DigitalOutputDevice::is_off() {
    return value() == LOW;
}

void DigitalOutputDevice::on() {
    set_value(HIGH);
}

void DigitalOutputDevice::off() {
    set_value(LOW);
}

void DigitalOutputDevice::toggle() {
    set_value(value() == HIGH ? LOW : HIGH);
}

inline int inputModeToPinMode(InputMode inputMode) {
    switch (inputMode) {
    case Pulldown:
        return INPUT_PULLDOWN;
    case Pullup:
        return INPUT_PULLUP;
    default:
        return INPUT;
    }
}

InputDevice::InputDevice(int pin, InputMode inputMode, std::string name) : GPIODevice(pin, inputModeToPinMode(inputMode), name), _inputMode(inputMode) {
}

DigitalInputDevice::DigitalInputDevice(int pin, InputMode inputMode, std::string name) : InputDevice(pin, inputMode, name) {
}

int DigitalInputDevice::value() {
    return digitalRead(pin());
}

struct ButtonPrivate {
private:
    hw_timer_t *timer = {};
    Button *_button = {};
    portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
    Ticker *ticker{};
    unsigned long timerStartTime;

public:
    ButtonPrivate(Button *button) : _button(button) {
    }

    ~ButtonPrivate() {
        timerStop();
    }
    std::function<void(void)> func;

    void timerStart() {

        timerStartTime = millis() + _button->holdTime;

        assert(ticker == nullptr);

        ticker = new Ticker();
        ticker->attach_ms<ButtonPrivate *>(_button->heldTickTime,
                                           [](ButtonPrivate *b) IRAM_ATTR{
                                               if (millis() > b->timerStartTime) {
                                                   b->_button->on_held();
                                               }
                                           },
                                           this);
    }

    void timerStop() {
        ticker->detach();
        delete ticker;
        ticker = nullptr;
    }
};

Button::Button(int pin, std::string name) : Button(pin, InputMode::Pulldown, name) {
}

Button::Button(int pin, InputMode inputMode, std::string name) : DigitalInputDevice(pin, inputMode, name), _privateImpl(new ButtonPrivate(this)) {
}

Button::~Button() {
    delete _privateImpl;
}

void Button::on_changed(int newValue) {
    DigitalInputDevice::on_changed(newValue);

    auto currentTime = millis();
    if ((newValue == HIGH) != _pressed && currentTime - _lastTime > bounceTime) {
        if (newValue == HIGH) {
            on_pressed();
        } else {
            on_released();
        }

        _pressed = newValue == HIGH;
        _lastTime = currentTime;
    }
}

bool Button::is_pressed() {
    return _pressed;
}

bool Button::is_released() {
    return !_pressed;
}

void Button::on_pressed() {
    _privateImpl->timerStart();

    pressed(this);
}

void Button::on_released() {
    _privateImpl->timerStop();
    released(this);
}

void Button::on_held() {
    held(this);
}

struct LedPrivate {
    SemaphoreHandle_t blinkMutex = xSemaphoreCreateMutex();
};

Led::Led(int pin, std::string name) : Led(pin, LOW, name) {
}

Led::Led(int pin, int initialValue, std::string name) : DigitalOutputDevice(pin, initialValue, name), _privateImpl(new LedPrivate()) {
}

Led::~Led() {
    delete _privateImpl;
}

void Led::blink(int time) {
    blink(time, time);
}

void Led::blink(int on_time, int off_time) {
    xSemaphoreLockGuard lock_guard(_privateImpl->blinkMutex);

    on();
    if (on_time > 0)
        delay(on_time);

    off();
    if (off_time > 0)
        delay(off_time);
}

void Led::blinkAsync(int time) {
    blinkAsync(time, time);
}

struct TaskBlinkParams {
    Led *led;
    int on_time;
    int off_time;
};

void Led::blinkAsync(int on_time, int off_time) {
    xSemaphoreLockGuard lock_guard(_privateImpl->blinkMutex);

    xTaskCreate(
        [](void *params) {
            auto blinkParams = static_cast<TaskBlinkParams *>(params);

            blinkParams->led->blink(blinkParams->on_time, blinkParams->off_time);

            delete blinkParams;

            vTaskDelete(nullptr);
        },
        (std::string("blink") + name()).c_str(), 1000, new TaskBlinkParams{this, on_time, off_time}, 1, nullptr);
}

} // namespace devices