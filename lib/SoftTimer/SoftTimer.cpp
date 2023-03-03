#include <Arduino.h>
#include "SoftTimer.h"

SoftTimer::SoftTimer(float cycle)
{
  setCycle(cycle);
  _lastUpdateTime = micros();
}

SoftTimer::SoftTimer() : SoftTimer(0)
{
}

bool SoftTimer::isTick()
{
  unsigned long now = micros();
  if (now > _lastUpdateTime + _cycleTime)
  {
   _lastUpdateTime = now;
   return true; 
  }
  return false;
}

float SoftTimer::getTime()
{
  unsigned long now = micros();
  unsigned long cycle = now - _lastUpdateTime;
  _lastUpdateTime = now;
  return (float)cycle / 1000.0;
}

void SoftTimer::setCycle(float cycle)
{  
  _cycleTime = (unsigned long)(cycle * 1000.0);
}