#include <Arduino.h>
#include "Mux.h"

Mux_::Mux_()
{
  _numPins = 0;
}

void Mux_::setSelect(uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3)
{
  _s0 = s0;
  _s1 = s1;
  _s2 = s2;
  _s3 = s3;
}

// add a pin to mux
bool Mux_::addPin(uint8_t pin)
{
  if (_numPins < MUX_MAX_NUMBER)
  {
    _pin[_numPins++] = pin;
    pinMode(pin, INPUT);
    return true;
  }
  else
  {
    return false;
  }
}

// Gets specific pin from mux, number according to initialization order 
bool Mux_::getBit(uint8_t mux, uint8_t muxpin)
{
  return bitRead(_data[mux], muxpin);
}

// read all inputs together -> base for board specific optimization by using byte read
void Mux_::handle()
{
  for (uint8_t muxpin = 0; muxpin < 16; muxpin++)
  {
    digitalWrite(_s0, bitRead(muxpin, 0));
    digitalWrite(_s1, bitRead(muxpin, 1));
    digitalWrite(_s2, bitRead(muxpin, 2));
    digitalWrite(_s3, bitRead(muxpin, 3));
    for (uint8_t mux; mux < _numPins; mux++)
    {
      bitWrite(_data[mux], muxpin, !digitalRead(_pin[mux]));
    }
  }
}

Mux_ Mux;
