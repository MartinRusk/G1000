#ifndef Led_h
#define Led_h

enum led_t
{
    ledOff,
    ledSlow,
    ledMedium,
    ledFast,
    ledOn
};

class LedShift
{
public:
  LedShift(uint8_t pin_DAI, uint8_t pin_DCK, uint8_t pin_LAT);
  void set(uint8_t pin, led_t mode);
  void set_all(led_t mode);
  void handle();
private:
  void _send();
  void _update(uint8_t pin);
  uint8_t _pin_DAI;
  uint8_t _pin_DCK;
  uint8_t _pin_LAT;
  uint16_t _state;
  led_t _mode[16];
  uint8_t _count;
  unsigned long _timer;
};

#endif
