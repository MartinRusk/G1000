#include <Arduino.h>
#include "Mux.h"


Mux::Mux(int8_t s0, int8_t s1, int8_t s2, int8_t s3)
{
  _s0 = s0;
  _s1 = s1;
  _s2 = s2;
  _s3 = s3;
  _numPins = 0;
}

// add a pin to mux
bool Mux::addPin(int8_t pin)
{
  if (_numPins < MUX_MAX_NUMBER)
  {
    _pin[_numPins++] = pin;
    return true;
  }
  else
  {
    return false;
  }
}

// Gets specific pin from mux, number according to initialization order 
bool Mux::getBit(int8_t module, int8_t input)
{
  return bitRead(data[module], input);
}

// read all inputs together -> base for board specific optimization by using byte read
void Mux::handle()
for (uint8_t input = 0; input < 16; input++)
{
  digitalWrite(_s0, bitRead(input, 0));
  digitalWrite(_s1, bitRead(input, 1));
  digitalWrite(_s2, bitRead(input, 2));
  digitalWrite(_s3, bitRead(input, 3));
  for (uint8_t i; i < _numPins; i++)
  {
    bitWrite(data[i], input, !digitalRead(_pin[i]);
  }
}
