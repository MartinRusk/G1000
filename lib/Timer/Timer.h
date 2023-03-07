#ifndef SoftTimer_h
#define SoftTimer_h

class Timer
{
  public: 
    Timer(float cycle); // ms
    Timer();
    bool isTick();
    float getTime(); // ms
    void setCycle(float cycle); // ms
  private:
    unsigned long _cycleTime;
    unsigned long _lastUpdateTime;
};

#endif
