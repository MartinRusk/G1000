#include <Arduino.h>
#include "Switch.h"

#define DEBOUNCE_DELAY 100;

// Buttons
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

void Switch::handle()
{
  if ((_mux == 255) ? !digitalRead(_pin) : Mux.getBit(_mux, _pin))
  {
    if (_state == 0)
    {
      _state = DEBOUNCE_DELAY;
    }
  }
  else if (_state > 0)
  {
    _state--;
  }
}

bool Switch::engaged()
{
  return _state > 0;
}
