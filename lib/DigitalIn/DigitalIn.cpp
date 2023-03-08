#include <Arduino.h>
#include "DigitalIn.h"

#define MCP_PIN 254

// constructor
DigitalIn_::DigitalIn_()
{
  _numPins = 0;
  for (uint8_t mux = 0; mux < MUX_MAX_NUMBER; mux++)
  {
    _pin[mux] = NOT_USED;
  }
}

// configure 74HC4067 adress pins S0-S3
void DigitalIn_::setMux(uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3)
{
  _s0 = s0;
  _s1 = s1;
  _s2 = s2;
  _s3 = s3;    
  pinMode(_s0, OUTPUT);
  pinMode(_s1, OUTPUT);
  pinMode(_s2, OUTPUT);
  pinMode(_s3, OUTPUT);
}

// Add a 74HC4067
bool DigitalIn_::addMux(uint8_t pin)
{
  if (_numPins >= MUX_MAX_NUMBER)
  {
    return false;
  }
  _pin[_numPins++] = pin;
  pinMode(pin, INPUT);
  return true;
}

#if MCP_MAX_NUMBER > 0
// Add a MCP23017
bool DigitalIn_::addMCP(uint8_t adress)
{
  if (_numMCP >= MCP_MAX_NUMBER)
  {
    return false;
  }
  if (!_mcp[_numMCP].begin_I2C(adress, &Wire))
  {
    return false;
  }
  for (int i = 0; i < 16; i++)
  {
    // TODO: register write iodir = 0xffff, ipol = 0xffff, gppu = 0xffff
    _mcp[_numMCP].pinMode(i, INPUT_PULLUP);
  }
  _numMCP++;
  _pin[_numPins++] = MCP_PIN;
  return true;
}
#endif

// Gets specific pin from mux, number according to initialization order 
bool DigitalIn_::getBit(uint8_t mux, uint8_t pin)
{
  if (mux == NOT_USED)
  {
    return !digitalRead(pin);
  } 
  return bitRead(_data[mux], pin);
}

// read all inputs together -> base for board specific optimization by using byte read
void DigitalIn_::handle()
{
  // only if Mux Pins present
#if MCP_MAX_NUMBER > 0  
  if (_numPins > _numMCP)
#endif
  {
    for (uint8_t muxpin = 0; muxpin < 16; muxpin++)
    {
      digitalWrite(_s0, bitRead(muxpin, 0));
      digitalWrite(_s1, bitRead(muxpin, 1));
      digitalWrite(_s2, bitRead(muxpin, 2));
      digitalWrite(_s3, bitRead(muxpin, 3));
      for (uint8_t mux = 0; mux < _numPins; mux++)
      {
        if (_pin[mux] != MCP_PIN)
        {
          bitWrite(_data[mux], muxpin, !digitalRead(_pin[mux]));
        }
      }
    }
  }
#if MCP_MAX_NUMBER > 0
  int mcp = 0;
  for (uint8_t mux = 0; mux < _numPins; mux++)
  {
    if (_pin[mux] == MCP_PIN)
    {
      _data[mux] = ~_mcp[mcp++].readGPIOAB();
    }
  }
#endif
}

DigitalIn_ DigitalIn;
