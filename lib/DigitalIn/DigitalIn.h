#ifndef Mux_h
#define Mux_h

#define NOT_USED 255
#define MUX_MAX_NUMBER 6
#define MCP_MAX_NUMBER 0

#if MCP_MAX_NUMBER > 0
#include <Adafruit_MCP23X17.h>
#endif

class DigitalIn_
{
public:
  DigitalIn_();
  void setMux(uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3);
  bool addMux(uint8_t pin);
#if MCP_MAX_NUMBER > 0
  bool addMCP(uint8_t adress);
#endif
  bool getBit(uint8_t mux, uint8_t muxpin);
  void handle();
private:
  uint8_t _s0, _s1, _s2, _s3;
  uint8_t _numPins;
  uint8_t _pin[MUX_MAX_NUMBER + MCP_MAX_NUMBER];
  int16_t _data[MUX_MAX_NUMBER + MCP_MAX_NUMBER];
#if MCP_MAX_NUMBER > 0
  uint8_t _numMCP;
  Adafruit_MCP23X17 _mcp[MCP_MAX_NUMBER];
#endif
};

extern DigitalIn_ DigitalIn;

#endif
