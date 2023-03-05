#ifndef Button_h
#define Button_h
#include <Mux.h>

// simple Button
class Button
{
public:
  Button(uint8_t pin);
  Button(uint8_t mux, uint8_t muxpin);
  void handle();
  void handle(bool input);
  bool pressed();
  bool released();
  bool engaged();
  void setCmd(int cmdPush);
  int getCmd();
protected:
  uint8_t _mux;
  uint8_t _pin;
  uint8_t _state;
  uint8_t _transition;
  int _cmdPush;
};

// Button with automatic Repeat
class RepeatButton : public Button
{
public:
  RepeatButton(uint8_t pin, uint32_t delay);
  RepeatButton(uint8_t mux, uint8_t muxpin, uint32_t delay);
  void handle();
  void handle(bool input);
private:
  uint32_t _delay;
  uint32_t _timer;
};

#endif