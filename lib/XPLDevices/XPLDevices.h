#ifndef XPLDevices_h
#define XPLDevices_h

#include <AnalogIn.h>
#include <DigitalIn.h>
#include <Button.h>
#include <Encoder.h>
#include <Switch.h>
#include <LedShift.h>
#include <Timer.h>
#include <XPLDirect.h>

extern XPLDirect XP;
void XPsetup(const char *devicename);
void XPloop();
void handleCommand(Button *btn);
void handleCommand(Encoder *btn);
void handleCommand(Switch *btn);

#endif