#ifndef DEVICES_H
#define DEVICES_H

#include <functional>
#include <sigslot/signal.h>
#include <string>

namespace devices
{

class GPIODevice
{
private:
    int _pin;
    std::string _name;

protected:
    GPIODevice(int pin, std::string name = std::string());

public:
    virtual ~GPIODevice();

    int pin() const;
    std::string name() const;
};

class OutputDevice : public GPIODevice
{
public:
    OutputDevice(int pin, std::string name = std::string());

    bool value();
    void set_value(bool v);
};

class Switch : public OutputDevice
{
public:
    Switch(int pin, std::string name = std::string());

    void on();
    void off();
    void toggle();
};

enum InputMode
{
    Normal,
    Pulldown,
    Pullup
};

class InputDevice : public GPIODevice
{
private:
    InputMode _inputMode;

public:
    InputDevice(int pin, InputMode inputMode = Normal, std::string name = std::string());

    virtual int value();
};

class DigitalInputDevice : public InputDevice
{
private:
    void on_interrupt();

protected:
    virtual void on_changed(bool newValue);

public:    
    DigitalInputDevice(int pin, InputMode inputMode, std::string name = std::string());

    ~DigitalInputDevice() override;

    sigslot::signal<bool> changed;
};

class Button : public DigitalInputDevice
{
private:
    int _lastTime = 0;

    int bounceTime = 5;
    bool _pressed = false;

protected:
    void on_changed(bool newValue) override;

    virtual void on_pressed();
    virtual void on_released();

public:
    Button(int pin, std::string name = std::string());
    Button(int pin, InputMode inputMode, std::string name = std::string());

    bool is_pressed();

    sigslot::signal<> pressed;
    sigslot::signal<> released;
};

class Led : public Switch
{
private:
public:
    Led(int pin, std::string name = std::string());

    void blink(int time = 100);
    void blinkAsync(int time = 100);
    void blink(int on_time, int off_time);
    void blinkAsync(int on_time, int off_time);
};

} // namespace devices
#endif