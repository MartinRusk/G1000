#include <Arduino.h>
#include <Mux.h>
#include <Button.h>
#include <Encoder.h>
#include <Switch.h>

Button but1(0, 1);
RepeatButton but2(0, 1, 500);
Encoder enc1(0, 2, 3, 4, 4);
Switch sw1(0, 8);

void setup()
{
  Serial.begin(115200);
  Mux.setSelect(1, 0, 2, 3);
  Mux.addPin(4);
  Mux.addPin(5);
  Mux.addPin(6);
  Mux.addPin(7);
  Mux.addPin(8);
  Mux.addPin(9);
}

void loop()
{
  long time = millis();
  for (int i = 0; i < 1000; i++)
  {
    Mux.handle();
    but1.handle();
    but2.handle();
    sw1.handle();
    for (int j = 0; j<200; j++)
    {
      enc1.handle();
    }
  }
  Serial.println(millis() - time);
}