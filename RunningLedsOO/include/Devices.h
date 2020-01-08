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
    std::string _name;
    int _pin;

protected:
    GPIODevice(std::string name, int pin);

public:
    virtual ~GPIODevice();

    int pin() const;
    std::string name() const;
};

class OutputDevice : public GPIODevice
{
public:
    OutputDevice(std::string name, int pin);

    bool value();
    void set_value(bool v);
};

class Switch : public OutputDevice
{
public:
    Switch(std::string name, int pin);

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
    InputDevice(std::string name, int pin, InputMode inputMode = Normal);

    virtual int value();
};

class DigitalInputDevice : public InputDevice
{
private:
    void on_interrupt();

protected:
    virtual void on_changed(bool newValue);

public:
    DigitalInputDevice(std::string name, int pin, InputMode inputMode = Normal);

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
    Button(std::string name, int pin, InputMode inputMode = Normal);

    bool is_pressed();

    sigslot::signal<> pressed;
    sigslot::signal<> released;
};


class Led: public Switch {
private:
public:
    Led(std::string name, int pin);

    void blink(int time=100);
    void blinkAsync(int time=100);
    void blink(int on_time, int off_time);
    void blinkAsync(int on_time, int off_time);
};

} // namespace devices
#endif