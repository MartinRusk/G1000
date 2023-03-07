#include <Arduino.h>
#include "DigitalIn.h"

#define IS_MUX_PIN(pin) (pin < 0xf0)
#define IS_MCP_PIN(pin) (pin >= 0xf0 && pin < 0xf8)
#define PIN_TO_MCP(pin) (pin - 0xc0)

DigitalIn_::DigitalIn_()
{
  _numPins = 0;
  for (uint8_t mux = 0; mux < MUX_MAX_NUMBER; mux++)
  {
    _pin[mux] = NOT_USED;
  }
}

void DigitalIn_::begin(uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3)
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

// add a pin to mux
bool DigitalIn_::addMux(uint8_t pin)
{
  if (_numPins >= MUX_MAX_NUMBER)
  {
    return false;
  }
  // 74HC4067?
  if (pin < 0xf0)
  { 
    _pin[_numPins++] = pin;
    pinMode(pin, INPUT);
    return true;
  }
#if MCP_MAX_NUMBER > 0
  if (_numMCP >= MCP_MAX_NUMBER)
  {
    return false;
  }
  // MCP23017 on i2c?
  // _mcp[_numMCP] = new Adafruit_MCP23X17;
  if (!_mcp[_numMCP].begin_I2C(PIN_TO_MCP(pin), &Wire))
  {
    // delete &_mcp[_numMCP];
    return false;
  }
  for (int i = 0; i < 16; i++)
  {
    // TODO: register write
    _mcp[_numMCP].pinMode(i, INPUT_PULLUP);
  }
  _numMCP++;
  _pin[_numPins++] = pin;
  return true;
#else
  return false;
#endif
}

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
  for (uint8_t muxpin = 0; muxpin < 16; muxpin++)
  {
    digitalWrite(_s0, bitRead(muxpin, 0));
    digitalWrite(_s1, bitRead(muxpin, 1));
    digitalWrite(_s2, bitRead(muxpin, 2));
    digitalWrite(_s3, bitRead(muxpin, 3));
    for (uint8_t mux = 0; mux < _numPins; mux++)
    { 
      if (_pin[mux] < 0xf0)
      {
        bitWrite(_data[mux], muxpin, !digitalRead(_pin[mux]));
      }
    }
  }
#if MCP_MAX_NUMBER > 0
  int mcp = 0;
  for (uint8_t mux = 0; mux < _numPins; mux++)
  {
    if (_pin[mux] >= 0xf0)
    {
      _data[mux] = _mcp[mcp++].readGPIOAB();
    }
  }
#endif
}

DigitalIn_ DigitalIn;
