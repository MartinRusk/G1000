#ifndef Switch_h
#define Switch_h
#include <Mux.h>

enum eSwitch_t
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
  eSwitch_t state();
  void setCommand(int cmdOff, int cmdOn);
  int getCommand();
protected:
  uint8_t _mux;
  uint8_t _pin;
  uint8_t _debounce;
  eSwitch_t _state;
  int _cmdOff;
  int _cmdOn;
};

class Switch2 : public Switch
{
public:
  Switch2(uint8_t mux, uint8_t pin1, uint8_t pin2);
  Switch2(uint8_t pin1, uint8_t pin2);
  bool handle();
  void setCommand(int cmdOff, int cmdOn, int cmdOn2);
  int getCommand();
protected:
  uint8_t _pin2;
  int _cmdOn2;
};

#endif
