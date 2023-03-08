#ifndef Switch_h
#define Switch_h
#include <DigitalIn.h>

enum SwState_t
{
  eSwitchOff,
  eSwitchOn,
  eSwitchOn2
};

class Switch
{
public:
  Switch(uint8_t mux, uint8_t pin);
  Switch(uint8_t pin);
  bool handle();
  SwState_t state();
  void setCommand(int cmdOn, int cmdOff);
  int getCommand();
  void handleCommand();
protected:
  uint8_t _mux;
  uint8_t _pin;
  uint8_t _debounce;
  SwState_t _state;
  int _cmdOff;
  int _cmdOn;
};

class Switch2 : public Switch
{
public:
  Switch2(uint8_t mux, uint8_t pin1, uint8_t pin2);
  Switch2(uint8_t pin1, uint8_t pin2);
  bool handle();
  void setCommand(int cmdOn, int cmdOff, int cmdOn2, int cmdOff2);
  int getCommand();
protected:
  uint8_t _pin2;
  SwState_t _lastState;
  int _cmdOff2;
  int _cmdOn2;
};

#endif
