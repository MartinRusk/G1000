#ifndef Switch_h
#define Switch_h
#include <Mux.h>

class Switch
{
public:
  Switch(uint8_t mux, uint8_t pin);
  Switch(uint8_t pin);
  void handle();
  bool engaged();
protected:
  uint8_t _mux;
  uint8_t _pin;
  uint8_t _state;
};

// class Switch2 : Switch
// {
// public:
  
// protected:

// }

#endif
