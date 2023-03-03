#ifndef SoftTimer_h
#define SoftTimer_h

class SoftTimer
{
  public: 
    SoftTimer(float cycle); // ms
    SoftTimer();
    bool isTick();
    float getTime(); // ms
    void setCycle(float cycle); // ms
  private:
    unsigned long _cycleTime;
    unsigned long _lastUpdateTime;
};

#endif
