#include <Arduino.h>
#include <MemoryFree.h>
#include <DigitalIn.h>
#include <Button.h>
#include <Encoder.h>
#include <Switch.h>
#include <LedShift.h>
#include <AnalogIn.h>
#include <SoftTimer.h>
#include <XPLDirect.h>

// #define DEBUG 1

// Interface
XPLDirect XP(&Serial);

// MUX 0
Encoder encNavInner(0, 1, 2, 0, eEnc4Pulse);
Encoder encNavOuter(0, 4, 3, NOT_USED, eEnc4Pulse);
Encoder encComInner(0, 6, 7, 5, eEnc4Pulse);
Encoder encComOuter(0, 9, 8, NOT_USED, eEnc4Pulse);
Encoder encCourse(0, 11, 12, 10, eEnc4Pulse);
Encoder encBaro(0, 14, 13, NOT_USED, eEnc4Pulse);
// MUX 1
Encoder encAltInner(1, 1, 2, 0, eEnc4Pulse);
Encoder encAltOuter(1, 4, 3, NOT_USED, eEnc4Pulse);
Encoder encFMSInner(1, 6, 7, 5, eEnc4Pulse);
Encoder encFMSOuter(1, 9, 8, NOT_USED, eEnc4Pulse);
Button btnDirect(1, 10);
Button btnFPL(1, 11);
Button btnCLR(1, 12);
Button btnMENU(1, 13);
Button btnPROC(1, 14);
Button btnENT(1, 15);
// MUX 2
Button btnAP(2, 0);
Button btnFD(2, 1);
Button btnNAV(2, 2);
Button btnALT(2, 3);
Button btnVS(2, 4);
Button btnFLC(2, 5);
Button btnYD(2, 6);
Button btnHDG(2, 7);
Button btnAPR(2, 8);
Button btnVNAV(2, 9);
Button btnUP(2, 10);
Button btnDN(2, 11);
Encoder encNavVol(2, 12, 13, 14, eEnc4Pulse);
Button btnNavFF(2, 15);
// MUX3
Button btnSoft1(3, 0);
Button btnSoft2(3, 1);
Button btnSoft3(3, 2);
Button btnSoft4(3, 3);
Button btnSoft5(3, 4);
Button btnSoft6(3, 5);
Button btnSoft7(3, 6);
Button btnSoft8(3, 7);
Button btnSoft9(3, 8);
Button btnSoft10(3, 9);
Button btnSoft11(3, 10);
Button btnSoft12(3, 11);
Encoder encComVol(3, 12, 13, 14, eEnc4Pulse);
Button btnComFF(3, 15);
// MUX4 Pan/Zoom
Encoder encRange(4, 6, 5, 0, eEnc2Pulse);
Button btnPanPush(4, 0);
RepeatButton btnPanUp(4, 1, 250);
RepeatButton btnPanLeft(4, 2, 250);
RepeatButton btnPanDown(4, 3, 250);
RepeatButton btnPanRight(4, 4, 250);
Encoder encHeading(4, 12, 13, 14, eEnc4Pulse);

// MFD specific
Encoder encRudderTrim(4, 8, 9, 10, eEnc4Pulse);
Switch swLightLanding(5, 0);
Switch swLightTaxi(5, 1);
Switch swLightPosition(5, 2);
Switch swLightStrobe(5, 3);
Switch2 swFuelLeft(5, 4, 5);
Switch2 swFuelRight(5, 6, 7);
Switch swFuelAuxLeft(5, 8);
Switch swFuelAuxRight(5, 9);
Button btnGearTest(5, 10);
Switch swGear(5, 11);
Switch2 swFlaps(5, 12, 13);

// Potentiometers
AnalogIn potInstr(0, eUnipolar, 10);
AnalogIn potFlood(1, eUnipolar, 10);

// LEDs
LedShift leds(16, 14, 15);
#define LED_FLAP_UP 0
#define LED_FLAP_TO 1
#define LED_FLAP_LDG 2
#define LED_GEAR_NOSE 3
#define LED_GEAR_LEFT 4
#define LED_GEAR_RIGHT 5
#define LED_GEAR_UNSAFE 6

// Timer for Main loop
SoftTimer tmrMain(1000.0);
SoftTimer tmrSync(1000.0);

// DataRefs MUX 5
float gear_ratio[3];
float flap_ratio;
long int gear_unsafe;
float light_instr = 0;
float light_flood = 0;
long int gear_handle_down;
float flap_handle_request_ratio;

long int sw_fuel_lever1 = 2;
long int sw_fuel_lever2 = 2;

// Setup
void setup()
{
  Serial.begin(XPLDIRECT_BAUDRATE);
  XP.begin("G1000 MFD");

  // Setup Multiplexers
  DigitalIn.begin(1, 0, 2, 3);
  DigitalIn.addMux(4); // MUX 0
  DigitalIn.addMux(5); // MUX 1
  DigitalIn.addMux(6); // MUX 2
  DigitalIn.addMux(7); // MUX 3
  DigitalIn.addMux(8); // MUX 4
  DigitalIn.addMux(9); // MUX 5

  // init led sequence
  leds.set_all(ledOff);
  for (int pin = 0; pin < 7; pin++)
  {
    leds.set(pin, ledOn);
    leds.handle();
    delay(100);
  }
  leds.set_all(ledOff);

  encNavInner.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_nav_inner_up")),
      XP.registerCommand(F("sim/GPS/g1000n3_nav_inner_down")),
      XP.registerCommand(F("sim/GPS/g1000n3_nav12")));
  encNavOuter.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_nav_outer_up")),
      XP.registerCommand(F("sim/GPS/g1000n3_nav_outer_down")));
  encComInner.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_com_inner_up")),
      XP.registerCommand(F("sim/GPS/g1000n3_com_inner_down")),
      XP.registerCommand(F("sim/GPS/g1000n3_com12")));
  encComOuter.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_com_outer_up")),
      XP.registerCommand(F("sim/GPS/g1000n3_com_outer_down")));
  encCourse.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_crs_up")),
      XP.registerCommand(F("sim/GPS/g1000n3_crs_down")),
      XP.registerCommand(F("sim/GPS/g1000n3_crs_sync")));
  encBaro.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_baro_up")),
      XP.registerCommand(F("sim/GPS/g1000n3_baro_down")));

  encAltInner.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_alt_inner_up")),
      XP.registerCommand(F("sim/GPS/g1000n3_alt_inner_down")),
      XP.registerCommand(F("sim/autopilot/altitude_sync")));
  encAltOuter.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_alt_outer_up")),
      XP.registerCommand(F("sim/GPS/g1000n3_alt_outer_down")));
  encFMSInner.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_fms_inner_up")),
      XP.registerCommand(F("sim/GPS/g1000n3_fms_inner_down")),
      XP.registerCommand(F("sim/GPS/g1000n3_cursor")));
  encFMSOuter.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_fms_outer_up")),
      XP.registerCommand(F("sim/GPS/g1000n3_fms_outer_down")));
  btnDirect.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_direct")));
  btnFPL.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_fpl")));
  btnCLR.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_clr")));
  btnMENU.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_menu")));
  btnPROC.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_proc")));
  btnENT.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_ent")));

  btnAP.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_ap")));
  btnFD.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_fd")));
  btnNAV.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_nav")));
  btnALT.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_alt")));
  btnVS.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_vs")));
  btnFLC.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_flc")));
  btnYD.setCommand(
      XP.registerCommand(F("sim/systems/yaw_damper_toggle")));
  btnHDG.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_hdg")));
  btnAPR.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_apr")));
  btnVNAV.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_vnv")));
  btnUP.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_nose_up")));
  btnDN.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_nose_down")));
  encNavVol.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_nvol_up")),
      XP.registerCommand(F("sim/GPS/g1000n3_nvol_dn")),
      XP.registerCommand(F("sim/GPS/g1000n3_nvol")));
  btnNavFF.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_nav_ff")));

  btnSoft1.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_softkey1")));
  btnSoft2.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_softkey2")));
  btnSoft3.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_softkey3")));
  btnSoft4.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_softkey4")));
  btnSoft5.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_softkey5")));
  btnSoft6.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_softkey6")));
  btnSoft7.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_softkey7")));
  btnSoft8.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_softkey8")));
  btnSoft9.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_softkey9")));
  btnSoft10.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_softkey10")));
  btnSoft11.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_softkey11")));
  btnSoft12.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_softkey12")));
  encComVol.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_cvol_up")),
      XP.registerCommand(F("sim/GPS/g1000n3_cvol_dn")),
      XP.registerCommand(F("sim/audio_panel/monitor_audio_com2")));
  btnComFF.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_com_ff")));

  encRange.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_range_up")),
      XP.registerCommand(F("sim/GPS/g1000n3_range_down")));
  btnPanPush.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_pan_push")));
  btnPanUp.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_pan_up")));
  btnPanLeft.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_pan_left")));
  btnPanDown.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_pan_down")));
  btnPanRight.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_pan_right")));
  encHeading.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n3_hdg_up")),
      XP.registerCommand(F("sim/GPS/g1000n3_hdg_down")),
      XP.registerCommand(F("sim/GPS/g1000n3_hdg_sync")));
  encRudderTrim.setCommand(
      XP.registerCommand(F("sim/flight_controls/rudder_trim_right")),
      XP.registerCommand(F("sim/flight_controls/rudder_trim_left")),
      XP.registerCommand(F("sim/flight_controls/rudder_trim_center")));
  btnGearTest.setCommand(
      XP.registerCommand(F("aerobask/gear_test")));

  swLightStrobe.setCommand(
      XP.registerCommand(F("sim/lights/strobe_lights_on")),
      XP.registerCommand(F("sim/lights/strobe_lights_off")));
  swLightPosition.setCommand(
      XP.registerCommand(F("sim/lights/nav_lights_on")),
      XP.registerCommand(F("sim/lights/nav_lights_off")));
  swLightTaxi.setCommand(
      XP.registerCommand(F("sim/lights/taxi_lights_on")),
      XP.registerCommand(F("sim/lights/taxi_lights_off")));
  swLightLanding.setCommand(
      XP.registerCommand(F("sim/lights/landing_lights_on")),
      XP.registerCommand(F("sim/lights/landing_lights_off")));
  swFuelAuxLeft.setCommand(
      XP.registerCommand(F("aerobask/auxfuel/pump1_up")),
      XP.registerCommand(F("aerobask/auxfuel/pump1_dn")));
  swFuelAuxRight.setCommand(
      XP.registerCommand(F("aerobask/auxfuel/pump2_up")),
      XP.registerCommand(F("aerobask/auxfuel/pump2_dn")));

  XP.registerDataRef(F("sim/flightmodel2/gear/deploy_ratio"), XPL_READ, 100, 0, &gear_ratio[0], 0);
  XP.registerDataRef(F("sim/flightmodel2/gear/deploy_ratio"), XPL_READ, 100, 0, &gear_ratio[1], 1);
  XP.registerDataRef(F("sim/flightmodel2/gear/deploy_ratio"), XPL_READ, 100, 0, &gear_ratio[2], 2);
  // XP.registerDataRef(F("sim/cockpit2/annunciators/gear_unsafe"), XPL_READ, 100, 1.0, &gear_unsafe);
  XP.registerDataRef(F("sim/cockpit/warnings/annunciators/gear_unsafe"), XPL_READ, 100, 0, &gear_unsafe);
  XP.registerDataRef(F("sim/flightmodel2/controls/flap1_deploy_ratio"), XPL_READ, 100, 0, &flap_ratio);

  XP.registerDataRef(F("sim/cockpit2/switches/instrument_brightness_ratio"), XPL_READWRITE, 100, 0, &light_instr, 0);
  XP.registerDataRef(F("sim/cockpit2/switches/panel_brightness_ratio"), XPL_READWRITE, 100, 0, &light_flood, 0);

  XP.registerDataRef(F("sim/cockpit2/controls/gear_handle_down"), XPL_READWRITE, 100, 0, &gear_handle_down);
  XP.registerDataRef(F("sim/cockpit2/controls/flap_handle_request_ratio"), XPL_READWRITE, 100, 0, &flap_handle_request_ratio);
  XP.registerDataRef(F("aerobask/eng/sw_fuel_lever1"), XPL_READWRITE, 100, 0, &sw_fuel_lever1);
  XP.registerDataRef(F("aerobask/eng/sw_fuel_lever2"), XPL_READWRITE, 100, 0, &sw_fuel_lever2);
}

void handleCommand(Button *btn)
{
  btn->handle();
  int cmdPush = btn->getCommand();
  if (btn->pressed())
  {
    XP.commandStart(cmdPush);
  }
  if (btn->released())
  {
    XP.commandEnd(cmdPush);
  }
}

void handleCommand(Button *btn, bool input)
{
  btn->handle(input);
  int cmdPush = btn->getCommand();
  if (btn->pressed())
  {
    XP.commandStart(cmdPush);
  }
  if (btn->released())
  {
    XP.commandEnd(cmdPush);
  }
}

void handleCommand(Encoder *enc)
{
  enc->handle();
  if (enc->up())
  {
    XP.commandTrigger(enc->getCommand(eEncCmdUp));
  }
  if (enc->down())
  {
    XP.commandTrigger(enc->getCommand(eEncCmdDown));
  }
  int cmdPush = enc->getCommand(eEncCmdPush);
  if (cmdPush >= 0)
  {
    if (enc->pressed())
    {
      XP.commandStart(cmdPush);
    }
    if (enc->released())
    {
      XP.commandEnd(cmdPush);
    }
  }
}

void handleCommand(Switch *sw)
{
  if (sw->handle())
  {
    XP.commandTrigger(sw->getCommand());
  }
}

// Main loop
void loop()
{
  XP.xloop();
  DigitalIn.handle();
  leds.handle();

  handleCommand(&encNavInner);
  handleCommand(&encNavOuter);
  handleCommand(&encComInner);
  handleCommand(&encComOuter);
  handleCommand(&encCourse);
  handleCommand(&encBaro);
  handleCommand(&encAltInner);
  handleCommand(&encAltOuter);
  handleCommand(&encFMSInner);
  handleCommand(&encFMSOuter);
  handleCommand(&btnDirect);
  handleCommand(&btnFPL);
  handleCommand(&btnCLR);
  handleCommand(&btnMENU);
  handleCommand(&btnPROC);
  handleCommand(&btnENT);
  handleCommand(&btnAP);
  handleCommand(&btnFD);
  handleCommand(&btnNAV);
  handleCommand(&btnALT);
  handleCommand(&btnVS);
  handleCommand(&btnFLC);
  handleCommand(&btnYD);
  handleCommand(&btnHDG);
  handleCommand(&btnAPR);
  handleCommand(&btnVNAV);
  handleCommand(&btnUP);
  handleCommand(&btnDN);
  handleCommand(&encNavVol);
  handleCommand(&btnNavFF);
  handleCommand(&btnSoft1);
  handleCommand(&btnSoft2);
  handleCommand(&btnSoft3);
  handleCommand(&btnSoft4);
  handleCommand(&btnSoft5);
  handleCommand(&btnSoft6);
  handleCommand(&btnSoft7);
  handleCommand(&btnSoft8);
  handleCommand(&btnSoft9);
  handleCommand(&btnSoft10);
  handleCommand(&btnSoft11);
  handleCommand(&btnSoft12);
  handleCommand(&encComVol);
  handleCommand(&btnComFF);
  handleCommand(&encRange);
  // handle pan stick manually due to logical operations for inputs
  handleCommand(&btnPanPush, DigitalIn.getBit(4, 0) &&
                                 !DigitalIn.getBit(4, 1) && !DigitalIn.getBit(4, 2) &&
                                 !DigitalIn.getBit(4, 3) && !DigitalIn.getBit(4, 4));
  handleCommand(&btnPanUp, DigitalIn.getBit(4, 0) && DigitalIn.getBit(4, 1));
  handleCommand(&btnPanLeft, DigitalIn.getBit(4, 0) && DigitalIn.getBit(4, 2));
  handleCommand(&btnPanDown, DigitalIn.getBit(4, 0) && DigitalIn.getBit(4, 3));
  handleCommand(&btnPanRight, DigitalIn.getBit(4, 0) && DigitalIn.getBit(4, 4));

  handleCommand(&encRudderTrim);
  handleCommand(&btnGearTest);

  handleCommand(&swLightStrobe);
  handleCommand(&swLightPosition);
  handleCommand(&swLightTaxi);
  handleCommand(&swLightLanding);

  // Sync Switches 
  if (tmrSync.isTick())
  {
    XP.commandTrigger(swLightStrobe.getCommand());
    XP.commandTrigger(swLightPosition.getCommand());
    XP.commandTrigger(swLightTaxi.getCommand());
    XP.commandTrigger(swLightLanding.getCommand());
    XP.commandTrigger(swFuelAuxLeft.getCommand());
    XP.commandTrigger(swFuelAuxRight.getCommand());
  }

  swFlaps.handle();
  SwState_t state = swFlaps.state();
  flap_handle_request_ratio = (state == eSwitchOn) ? 1.0 : (state == eSwitchOn2) ? 0.0 : 0.5;
  swGear.handle();
  gear_handle_down = (swGear.state() == eSwitchOn) ? 0.0 : 1.0;
  swFuelLeft.handle();
  state = swFuelLeft.state();
  sw_fuel_lever1 = (state == eSwitchOn) ? 2 : (state == eSwitchOn2) ? 0 : 1;
  swFuelRight.handle();
  state = swFuelRight.state();
  sw_fuel_lever2 = (state == eSwitchOn) ? 2 : (state == eSwitchOn2) ? 0 : 1;
  
  light_instr = potInstr.value();
  light_flood = potFlood.value();

  // Update LEDs
  if (flap_ratio < 0.05)
  {
    leds.set(LED_FLAP_UP, ledOn);
    leds.set(LED_FLAP_TO, ledOff);
    leds.set(LED_FLAP_LDG, ledOff);
  }
  else if (flap_ratio < 0.45)
  {
    leds.set(LED_FLAP_UP, ledOn);
    leds.set(LED_FLAP_TO, ledOn);
    leds.set(LED_FLAP_LDG, ledOff);
  }
  else if (flap_ratio < 0.55)
  {
    leds.set(LED_FLAP_UP, ledOff);
    leds.set(LED_FLAP_TO, ledOn);
    leds.set(LED_FLAP_LDG, ledOff);
  }
  else if (flap_ratio < 0.95)
  {
    leds.set(LED_FLAP_UP, ledOff);
    leds.set(LED_FLAP_TO, ledOn);
    leds.set(LED_FLAP_LDG, ledOn);
  }
  else
  {
    leds.set(LED_FLAP_UP, ledOff);
    leds.set(LED_FLAP_TO, ledOff);
    leds.set(LED_FLAP_LDG, ledOn);
  }

  leds.set(LED_GEAR_NOSE, (gear_ratio[0] > 0.99) ? ledOn : ledOff);
  leds.set(LED_GEAR_LEFT, (gear_ratio[1] > 0.99) ? ledOn : ledOff);
  leds.set(LED_GEAR_RIGHT, (gear_ratio[2] > 0.99) ? ledOn : ledOff);
  leds.set(LED_GEAR_UNSAFE, (gear_unsafe > 0) ? ledMedium : ledOff);

#if DEBUG
  static int count = 0;
  if (tmrMain.isTick())
  {
    Serial.print("Flap: ");
    Serial.print(swFlaps.state());
    Serial.print(" Flap: ");
    Serial.print(flap_handle_request_ratio);
    Serial.print(" Instr: ");
    Serial.print(light_instr);
    Serial.print(" Flood: ");
    Serial.print(light_flood);
    Serial.print(" Memory: ");
    Serial.print(freeMemory());
    Serial.print(" Cycle count: ");
    Serial.println(count);
    count = 0;
  }
  else
  {
    count++;
  }
#endif
}