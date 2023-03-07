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
  if (_mux == NOT_USED)
  {
    pinMode(_pin, INPUT_PULLUP);
  }
}

Switch::Switch(uint8_t pin) : Switch (NOT_USED, pin)
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
    SwState_t input = eSwitchOff;
    if (DigitalIn.getBit(_mux, _pin))
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

SwState_t Switch::state()
{
  return _state;
}

void Switch::setCommand(int cmdOn, int cmdOff)
{
  _cmdOn = cmdOn;
  _cmdOff = cmdOff;
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
  if (_mux == NOT_USED)
  {
    pinMode(_pin2, INPUT_PULLUP);
  }
}

Switch2::Switch2(uint8_t pin1, uint8_t pin2) : Switch2 (NOT_USED, pin1, pin2)
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
    SwState_t input = eSwitchOff;
    if (DigitalIn.getBit(_mux, _pin))
    {
      input = eSwitchOn;
    }
    else if (DigitalIn.getBit(_mux, _pin2))
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