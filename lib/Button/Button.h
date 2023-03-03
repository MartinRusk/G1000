#ifndef Button_h
#define Button_h

#include <Mux.h>

class Button
{
public:
  Button(uint8_t pin);
  Button(uint8_t pin, uint32_t delay);
  void handle();
  bool pressed();
  bool released();

private:
  uint8_t _pin;
  uint8_t _state;
  uint8_t _transition;
  uint32_t _delay;
  uint32_t _timer;
};

#endif
