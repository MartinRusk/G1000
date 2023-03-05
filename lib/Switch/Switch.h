#ifndef Switch_h
#define Switch_h
#include <Mux.h>

class Switch
{
public:
  Switch(uint8_t mux, uint8_t pin);
  Switch(uint8_t pin);
  bool value();
protected:
  uint8_t _mux;
  uint8_t _pin;
  uint8_t _state;
  uint8_t _debounce;
};

class Switch2
{
public:
  Switch2(uint8_t mux, uint8_t pin1, uint8_t pin2);
  Switch2(uint8_t pin1, uint8_t pin2);
  int value();
protected:
  uint8_t _mux;
  uint8_t _pin1;
  uint8_t _pin2;
  uint8_t _state;
  uint8_t _debounce;
};

#endif
