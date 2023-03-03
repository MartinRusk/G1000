#ifndef AnalogIn_h
#define AnalogIn_h

#define AD_RES 10

class AnalogIn
{
public:
  AnalogIn(uint8_t pin, bool bipolar, float timeConst);
  AnalogIn(uint8_t pin, bool bipolar);
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
  bool _bipolar;
  uint8_t _pin;
};

#endif
