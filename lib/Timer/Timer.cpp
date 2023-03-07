#include <Arduino.h>
#include "Timer.h"

Timer::Timer(float cycle)
{
  setCycle(cycle);
  _lastUpdateTime = micros();
}

Timer::Timer() : Timer(0)
{
}

bool Timer::isTick()
{
  unsigned long now = micros();
  if (now > _lastUpdateTime + _cycleTime)
  {
   _lastUpdateTime = now;
   return true; 
  }
  return false;
}

float Timer::getTime()
{
  unsigned long now = micros();
  unsigned long cycle = now - _lastUpdateTime;
  _lastUpdateTime = now;
  return (float)cycle / 1000.0;
}

void Timer::setCycle(float cycle)
{  
  _cycleTime = (unsigned long)(cycle * 1000.0);
}