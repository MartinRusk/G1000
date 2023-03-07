#ifndef AnalogIn_h
#define AnalogIn_h

#define AD_RES 10

enum Analog_t
{
  eUnipolar,
  eBipolar
};

class AnalogIn
{
public:
  AnalogIn(uint8_t pin, Analog_t type, float timeConst);
  AnalogIn(uint8_t pin, Analog_t type);
  float value();
  int raw();
  void calibrate();
private:
  float _filterConst;
  float _value;
  float _scale;
  float _scalePos;
  float _scaleNeg;
  int _offset;
  uint8_t _pin;
};

#endif
