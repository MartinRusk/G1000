#include <Arduino.h>
#include "Button.h"

#define DEBOUNCE_DELAY 100

enum
{
  eNone,
  ePressed,
  eReleased
};

// Buttons
Button::Button(uint8_t pin, uint32_t delay)
{
  _pin = pin;
  _state = 0;
  _transition = 0;
  _delay = delay;
  _timer = 0;
  pinMode(_pin, INPUT_PULLUP);
}

Button::Button(uint8_t pin) : Button(pin, 0)
{
}

void Button::handle()
{
  if (!digitalRead(_pin))
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