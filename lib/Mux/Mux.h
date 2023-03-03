#ifndef Mux_h
#define Mux_h

#define MUX_MAX_NUMBER 6

class Mux
{
public:
  Mux(int8_t s0, int8_t s1, int8_t s2, int8_t s3);
  bool addPin(int8_t pin);
  bool getBit(int8_t module, int8_t input);
  void handle();
private:
  int8_t _s0, _s1, _s2, _s3;
  int8_t _numPins;
  int8_t _pin[MUX_MAX_NUMBER];
  int16_t _data[MUX_MAX_NUMBER];
}

#endif
