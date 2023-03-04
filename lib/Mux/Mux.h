#ifndef Mux_h
#define Mux_h

#define MUX_MAX_NUMBER 6

class Mux_
{
public:
  Mux_();
  void begin(uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3);
  bool addMux(uint8_t pin);
  bool getBit(uint8_t mux, uint8_t muxpin);
  void handle();
private:
  uint8_t _s0, _s1, _s2, _s3;
  uint8_t _numPins;
  uint8_t _pin[MUX_MAX_NUMBER];
  int16_t _data[MUX_MAX_NUMBER];
};

extern Mux_ Mux;

#endif
