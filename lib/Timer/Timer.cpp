#include <Arduino.h>
#include "Timer.h"

// cyclic timer
Timer::Timer(float cycle)
{
  setCycle(cycle);
  _lastUpdateTime = micros();
}

// measurement timer
Timer::Timer() : Timer(0)
{
}

// check if cyclic timer interval reached
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

// measure time and reset timer
float Timer::getTime()
{
  unsigned long now = micros();
  unsigned long cycle = now - _lastUpdateTime;
  _lastUpdateTime = now;
  return (float)cycle / 1000.0;
}

// set new cycle time
void Timer::setCycle(float cycle)
{  
  _cycleTime = (unsigned long)(cycle * 1000.0);
}