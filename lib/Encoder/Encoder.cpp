#include <Arduino.h>
#include <XPLDirect.h>
#include "Encoder.h"

#define DEBOUNCE_DELAY 5

enum
{
  eNone,
  ePressed,
  eReleased
};

// Encoder with button functionality on MUX
Encoder::Encoder(uint8_t mux, uint8_t pin1, uint8_t pin2, uint8_t pin3, EncPulse_t pulses)
{
  _mux = mux;
  _pin1 = pin1;
  _pin2 = pin2;
  _pin3 = pin3;
  _pulses = pulses;
  _count = 0;
  _state = 0;
  _transition = eNone;
  _cmdUp = -1;
  _cmdDown = -1;
  _cmdPush = -1;
  if (_mux != NOT_USED)
  {
    pinMode(_pin1, INPUT_PULLUP);
    pinMode(_pin2, INPUT_PULLUP);
    if (_pin3 != NOT_USED)
    {
      pinMode(_pin3, INPUT_PULLUP);
    }
  }
}

// Encoder with Button funktionality directly on pins
Encoder::Encoder(uint8_t pin1, uint8_t pin2, uint8_t pin3, EncPulse_t pulses) : Encoder(NOT_USED, pin1, pin2, pin3, pulses)
{
}

// real time handling
void Encoder::handle()
{
  // collect new state
  _state = ((_state & 0x03) << 2) | (DigitalIn.getBit(_mux, _pin2) << 1) | (DigitalIn.getBit(_mux, _pin1));
  // evaluate state change
  switch (_state)
  {
  case 1:
  case 7:
  case 8:
  case 14:
    _count++;
    break;
  case 2:
  case 4:
  case 11:
  case 13:
    _count--;
    break;
  case 3:
  case 12:
    _count += 2;
    break;
  case 6:
  case 9:
    _count -= 2;
    break;
  default:
    break;
  }

  // optional button functionality
  if (_pin3 != NOT_USED)
  {
    if (DigitalIn.getBit(_mux, _pin3))
    {
      if (_debounce == 0)
      {
        _debounce = DEBOUNCE_DELAY;
        _transition = ePressed;
      }
    }
    else if (_debounce > 0)
    {
      if (--_debounce == 0)
      {
        _transition = eReleased;
      }
    }
  }
}

// Return counter
int16_t Encoder::pos()
{
  return _count;
}

// consume up event
bool Encoder::up()
{
  if (_count >= _pulses)
  {
    _count -= _pulses;
    return true;
  }
  return false;
}

// consume down event
bool Encoder::down()
{
  if (_count <= -_pulses)
  {
    _count += _pulses;
    return true;
  }
  return false;
}

// consume pressed event
bool Encoder::pressed()
{
  if (_transition == ePressed)
  {
    _transition = eNone;
    return true;
  }
  return false;
}

// consume released event
bool Encoder::released()
{
  if (_transition == eReleased)
  {
    _transition = eNone;
    return true;
  }
  return false;
}

bool Encoder::engaged()
{
  return _state > 0;
}

void Encoder::setCommand(int cmdUp, int cmdDown, int cmdPush)
{
  _cmdUp = cmdUp;
  _cmdDown = cmdDown;
  _cmdPush = cmdPush;
}

void Encoder::setCommand(int cmdUp, int cmdDown)
{
  setCommand(cmdUp, cmdDown, -1);
}

int Encoder::getCommand(EncCmd_t cmd)
{
  switch (cmd)
  {
  case eEncCmdUp:
    return _cmdUp;
    break;
  case eEncCmdDown:
    return _cmdDown;
    break;
  case eEncCmdPush:
    return _cmdPush;
    break;
  default:
    return -1;
    break;
  }
}

void Encoder::handleCommand()
{
  handle();
  if (up())
  {
    XP.commandTrigger(_cmdUp);
  }
  if (down())
  {
    XP.commandTrigger(_cmdDown);
  }
  if (_cmdPush >= 0)
  {
    if (pressed())
    {
      XP.commandStart(_cmdPush);
    }
    if (released())
    {
      XP.commandEnd(_cmdPush);
    }
  }
}
