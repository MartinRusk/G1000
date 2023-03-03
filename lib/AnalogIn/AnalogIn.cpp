#include <Arduino.h>
#include "AnalogIn.h"

#define FULL_SCALE ((1 << AD_RES) - 1)
#define HALF_SCALE (1 << (AD_RES - 1))

AnalogIn::AnalogIn(uint8_t pin, bool bipolar)
{
  _pin = pin;
  _bipolar = bipolar;
  _filterConst = 1.0;
  _scale = 1.0;
  pinMode(_pin, INPUT);
  if (_bipolar)
  {
    _offset = HALF_SCALE;
    _scalePos = _scale / HALF_SCALE;
    _scaleNeg = _scale / HALF_SCALE;
  }
  else
  {
    _offset = 0;
    _scalePos = _scale / FULL_SCALE;
    _scaleNeg = 0.0;
  }
}

AnalogIn::AnalogIn(uint8_t pin, bool bipolar, float timeConst) : AnalogIn(pin, bipolar)
{
  _filterConst = 1.0 / timeConst;
}

float AnalogIn::value()
{
  int _raw = raw();
  _value = (_filterConst * _raw * (_raw >= 0 ? _scalePos : _scaleNeg)) + (1 - _filterConst) * _value;
  return _value;
}

int AnalogIn::raw()
{
  return analogRead(_pin) - _offset;
}

void AnalogIn::calibrate()
{
  long sum = 0;
  for (int i = 0; i < 64; i++)
  {
    sum += analogRead(_pin);
  }
  _offset = (int)(sum / 64);
  _scalePos = _scale / (float)(FULL_SCALE - _offset);
  _scaleNeg = _scale / (float)(_offset);
}