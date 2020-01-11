#ifndef _DEVICES_H
#define _DEVICES_H

#include <functional>
#include <mutex>
#include <sigslot/signal.h>
#include <string>

namespace devices {

struct GPIODevicePrivate;

class GPIODevice {
private:
    int _pin;
    int _mode;
    std::string _name;

    GPIODevicePrivate *_privateImpl;

protected:
    GPIODevice(int pin, int mode, std::string name = std::string());

    virtual void on_changed(int newValue);

public:
    virtual ~GPIODevice();

    int pin() const;
    int mode() const;
    std::string name() const;

    virtual int value() = 0;

    sigslot::signal<GPIODevice *, int> changed{};
};

class OutputDevice : public GPIODevice {
public:
    OutputDevice(int pin, std::string name = std::string());

    int value() override;
    virtual void set_value(int v) = 0;
};

class DigitalOutputDevice : public OutputDevice {
public:
    DigitalOutputDevice(int pin, int initialValue, std::string name = std::string());

    void set_value(int v) override;

    bool is_on();
    bool is_off();

    void on();
    void off();
    void toggle();
};

enum InputMode {
    Normal,
    Pulldown,
    Pullup
};

class InputDevice : public GPIODevice {
private:
    InputMode _inputMode;

public:
    InputDevice(int pin, InputMode inputMode = Normal, std::string name = std::string());
};

class DigitalInputDevice : public InputDevice {
public:
    DigitalInputDevice(int pin, InputMode inputMode, std::string name = std::string());

    int value() override;
};

struct ButtonPrivate;

class Button : public DigitalInputDevice {
private:
    int _lastTime = 0;

    const int bounceTime = 5;
    
    const int heldTickTime = 50;
    const int holdTime = 500;

    bool _pressed = false;

    ButtonPrivate *_privateImpl;
    friend ButtonPrivate;
protected:
    void on_changed(int newValue) override;

    virtual void on_pressed();
    virtual void on_released();
    virtual void on_held();

public:
    Button(int pin, std::string name = std::string());
    Button(int pin, InputMode inputMode, std::string name = std::string());
    ~Button() override;

    bool is_pressed();
    bool is_released();

    sigslot::signal<GPIODevice *> pressed{};
    sigslot::signal<GPIODevice *> released{};
    sigslot::signal<GPIODevice *> held{};
};

struct LedPrivate;

class Led : public DigitalOutputDevice {
private:
    LedPrivate *_privateImpl;

public:
    Led(int pin, std::string name = std::string());
    Led(int pin, int initialValue, std::string name = std::string());

    ~Led() override;

    void blink(int time = 100);
    void blinkAsync(int time = 100);
    void blink(int on_time, int off_time);
    void blinkAsync(int on_time, int off_time);
};

} // namespace devices
#endif