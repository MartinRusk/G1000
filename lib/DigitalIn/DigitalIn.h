#ifndef Mux_h
#define Mux_h
#include <Adafruit_MCP23X17.h>

#define NOT_USED 255
#define MUX_MAX_NUMBER 12
#define MCP_MAX_NUMBER 0
#define MCP_TO_PIN(i2c_adress) (i2c_adress + 0xc0)

class DigitalIn_
{
public:
  DigitalIn_();
  void begin(uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3);
  bool addMux(uint8_t pin);
  bool getBit(uint8_t mux, uint8_t muxpin);
  void handle();
private:
  uint8_t _s0, _s1, _s2, _s3;
  uint8_t _numPins;
  uint8_t _pin[MUX_MAX_NUMBER];
  int16_t _data[MUX_MAX_NUMBER];
  // add MCP23017 members
#if MCP_MAX_NUMBER > 0
  uint8_t _numMCP;
  Adafruit_MCP23X17 _mcp[MCP_MAX_NUMBER];
#endif
};

extern DigitalIn_ DigitalIn;

#endif
