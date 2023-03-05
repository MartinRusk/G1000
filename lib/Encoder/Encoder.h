#ifndef Encoder_h
#define Encoder_h
#include <Mux.h>

class Encoder
{
public:
  Encoder(uint8_t mux, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pulses);
  Encoder(uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pulses);  
  void handle();
  int16_t pos();
  bool up();
  bool down();
  bool pressed();
  bool released();
  bool engaged();
  void setCmdUp(int cmdUp);
  int getCmdUp();
  void setCmdDown(int cmdDown);
  int getCmdDown();
  void setCmdPush(int cmdPush);
  int getCmdPush();
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
