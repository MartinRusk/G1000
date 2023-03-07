#include <Arduino.h>
#include "Button.h"

#define DEBOUNCE_DELAY 5

enum
{
  eNone,
  ePressed,
  eReleased
};

// Buttons
Button::Button(uint8_t mux, uint8_t pin)
{
  _mux = mux;
  _pin = pin;
  _state = 0;
  _transition = 0;
  _cmdPush = -1;
  if (_mux != NOT_USED)
  {
    pinMode(_pin, INPUT_PULLUP);
  }
}

Button::Button(uint8_t pin) : Button(NOT_USED, pin)
{
}

void Button::handle()
{
  handle(Mux.getBit(_mux, _pin));
}

void Button::handle(bool input)
{
  if (input)
  {
    if (_state == 0)
    {
      _state = DEBOUNCE_DELAY;
      _transition = ePressed;
    }
  }
  else if (_state > 0)
  {
    if (--_state == 0)
    {
      _transition = eReleased;
    }
  }
}

bool Button::pressed()
{
  if (_transition == ePressed)
  {
    _transition = eNone;
    return true;
  }
  return false;
}

bool Button::released()
{
  if (_transition == eReleased)
  {
    _transition = eNone;
    return true;
  }
  return false;
}

bool Button::engaged()
{
  return _state > 0;
}

void Button::setCommand(int cmdPush)
{
  _cmdPush = cmdPush;
}

int Button::getCommand()
{
  return _cmdPush;
}

RepeatButton::RepeatButton(uint8_t mux, uint8_t pin, uint32_t delay) : Button(mux, pin)
{
  _delay = delay;
  _timer = 0;
}

RepeatButton::RepeatButton(uint8_t pin, uint32_t delay) : RepeatButton(NOT_USED, pin, delay)
{
}

void RepeatButton::handle()
{
  handle(Mux.getBit(_mux, _pin));
}

void RepeatButton::handle(bool input)
{
  if (input)
  {
    if (_state == 0)
    {
      _state = DEBOUNCE_DELAY;
      _transition = ePressed;
      _timer = millis() + _delay;
    }
    else if (_delay > 0 && (millis() >= _timer))
    {
      _state = DEBOUNCE_DELAY;
      _transition = ePressed;
      _timer += _delay;
    }
  }
  else if (_state > 0)
  {
    if (--_state == 0)
    {
      _transition = eReleased;
    }
  }
}