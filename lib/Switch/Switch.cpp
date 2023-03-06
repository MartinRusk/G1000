#include <Arduino.h>
#include "Switch.h"

#define DEBOUNCE_DELAY 10;

Switch::Switch(uint8_t mux, uint8_t pin)
{
  _mux = mux;
  _pin = pin;
  _state = eSwitchOff;
  _cmdOff = -1;
  _cmdOn = -1;
  if (_mux == 255)
  {
    pinMode(_pin, INPUT_PULLUP);
  }
}

Switch::Switch(uint8_t pin) : Switch (255, pin)
{
}

bool Switch::handle()
{
  if (_debounce > 0)
  {
    _debounce--;
  }
  else 
  {
    eSwitch_t input = eSwitchOff;
    if (Mux.getBit(_mux, _pin))
    {
      input = eSwitchOn;
    }
    if (input != _state)
    {
      _debounce = DEBOUNCE_DELAY;
      _state = input;
      return true;
    }
  }
  return false;
}

eSwitch_t Switch::state()
{
  if (_state)
  {
    return eSwitchOn;
  }
  return eSwitchOff;
}

void Switch::setCommand(int cmdOff, int cmdOn)
{
  _cmdOff = cmdOff;
  _cmdOn = cmdOn;
}

int Switch::getCommand()
{
  switch (_state)
  {
  case eSwitchOff:
    return _cmdOff;
    break;
  case eSwitchOn:
    return _cmdOn;
    break;
  default:
    return -1;
    break;
  }
}

Switch2::Switch2(uint8_t mux, uint8_t pin1, uint8_t pin2) : Switch(mux, pin1)
{
  _pin2 = pin2;
  _cmdOn2 = -1;
  if (_mux == 255)
  {
    pinMode(_pin2, INPUT_PULLUP);
  }
}

Switch2::Switch2(uint8_t pin1, uint8_t pin2) : Switch2 (255, pin1, pin2)
{
}

bool Switch2::handle()
{
  if (_debounce > 0)
  {
    _debounce--;
  }
  else
  {
    eSwitch_t input = eSwitchOff;
    if (Mux.getBit(_mux, _pin))
    {
      input = eSwitchOn;
    }
    else if (Mux.getBit(_mux, _pin2))
    {
      input = eSwitchOn2;
    }
    if (input != _state)
    {
      _debounce = DEBOUNCE_DELAY;
      _state = input;
      return true;
    }
  }
  return false;
}

void Switch2::setCommand(int cmdOff, int cmdOn, int cmdOn2)
{
  _cmdOff = cmdOff;
  _cmdOn = cmdOn;
  _cmdOn2 = cmdOn2;
}

int Switch2::getCommand()
{
  switch (_state)
  {
  case eSwitchOff:
    return _cmdOff;
    break;
  case eSwitchOn:
    return _cmdOn;
    break;
  case eSwitchOn2:
    return _cmdOn2;
    break;
  default:
    return -1;
    break;
  }
}