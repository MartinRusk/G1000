#include <Arduino.h>
#include "Switch.h"

#define DEBOUNCE_DELAY 10;

Switch::Switch(uint8_t mux, uint8_t pin)
{
  _mux = mux;
  _pin = pin;
  _state = 0;
  if (_mux == 255)
  {
    pinMode(_pin, INPUT_PULLUP);
  }
}

Switch::Switch(uint8_t pin) : Switch (255, pin)
{
}

bool Switch::value()
{
  if (_debounce > 0)
  {
    _debounce--;
  }
  else 
  {
    bool input = ((_mux == 255) ? !digitalRead(_pin) : Mux.getBit(_mux, _pin));
    if (input != _state)
    {
      _debounce = DEBOUNCE_DELAY;
      _state = input;
    }
  }
  return _state;
}

Switch2::Switch2(uint8_t mux, uint8_t pin1, uint8_t pin2)
{
  _mux = mux;
  _pin1 = pin1;
  _pin2 = pin2;
  _state = 1;
  if (_mux == 255)
  {
    pinMode(_pin1, INPUT_PULLUP);
    pinMode(_pin2, INPUT_PULLUP);
  }
}

Switch2::Switch2(uint8_t pin1, uint8_t pin2) : Switch2 (255, pin1, pin2)
{
}

int Switch2::value()
{
  if (_debounce > 0)
  {
    _debounce--;
  }
  else
  {
    int input;
    if ((_mux == 255) ? !digitalRead(_pin1) : Mux.getBit(_mux, _pin1))
    {
      input = 0;
    }
    else if ((_mux == 255) ? !digitalRead(_pin2) : Mux.getBit(_mux, _pin2))
    {
      input = 2;
    }
    else
    {
      input = 1;
    }
    if (input != _state)
    {
      _debounce = DEBOUNCE_DELAY;
      _state = input;
    }
  }
  return _state;
}
