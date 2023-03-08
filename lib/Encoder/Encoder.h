#ifndef Encoder_h
#define Encoder_h
#include <DigitalIn.h>

enum EncCmd_t
{
  eEncCmdUp,
  eEncCmdDown,
  eEncCmdPush
};

enum EncPulse_t
{
  eEnc1Pulse = 1,
  eEnc2Pulse = 2,
  eEnc4Pulse = 4
};

class Encoder
{
public:
  Encoder(uint8_t mux, uint8_t pin1, uint8_t pin2, uint8_t pin3, EncPulse_t pulses);
  Encoder(uint8_t pin1, uint8_t pin2, uint8_t pin3, EncPulse_t pulses);  
  void handle();
  int16_t pos();
  bool up();
  bool down();
  bool pressed();
  bool released();
  bool engaged();
  void setCommand(int cmdUp, int cmdDown, int cmdPush);
  void setCommand(int cmdUp, int cmdDown);
  int getCommand(EncCmd_t cmd);
  void handleCommand();
private:
  uint8_t _mux;
  uint8_t _pin1, _pin2, _pin3;
  int8_t _count;
  uint8_t _pulses;
  uint8_t _state;
  uint8_t _debounce;
  uint8_t _transition;
  int _cmdUp;
  int _cmdDown;
  int _cmdPush;
};

#endif
