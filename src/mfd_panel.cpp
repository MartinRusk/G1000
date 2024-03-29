#include <Arduino.h>
#include <XPLDevices.h>

#if G1000_MFD

// MUX 0
Encoder encNavInner(0, 1, 2, 0, enc4Pulse);
Encoder encNavOuter(0, 4, 3, NOT_USED, enc4Pulse);
Encoder encComInner(0, 6, 7, 5, enc4Pulse);
Encoder encComOuter(0, 9, 8, NOT_USED, enc4Pulse);
Encoder encCourse(0, 11, 12, 10, enc4Pulse);
Encoder encBaro(0, 14, 13, NOT_USED, enc4Pulse);
// MUX 1
Encoder encAltInner(1, 1, 2, 0, enc4Pulse);
Encoder encAltOuter(1, 4, 3, NOT_USED, enc4Pulse);
Encoder encFMSInner(1, 6, 7, 5, enc4Pulse);
Encoder encFMSOuter(1, 9, 8, NOT_USED, enc4Pulse);
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
Encoder encNavVol(2, 12, 13, 14, enc4Pulse);
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
Encoder encComVol(3, 12, 13, 14, enc4Pulse);
Button btnComFF(3, 15);
// MUX4 Pan/Zoom
Encoder encRange(4, 6, 5, 0, enc2Pulse);
Button btnPanPush(4, 0);
RepeatButton btnPanUp(4, 1, 250);
RepeatButton btnPanLeft(4, 2, 250);
RepeatButton btnPanDown(4, 3, 250);
RepeatButton btnPanRight(4, 4, 250);
Encoder encHeading(4, 12, 13, 14, enc4Pulse);

// MFD specific
Encoder encRudderTrim(4, 8, 9, 10, enc4Pulse);
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
AnalogIn potInstr(0, unipolar, 10);
AnalogIn potFlood(1, unipolar, 10);

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
Timer tmrMain(1000.0);
Timer tmrSync(1000.0);

// DataRefs MUX 5
float gear_ratio[3];
float flap_ratio;
long gear_unsafe;
float light_instr = 0;
float light_flood = 0;
long gear_handle_down;
float flap_handle_request_ratio;

// Setup
void setup()
{
// Setup XP interface
  Serial.begin(XPLDIRECT_BAUDRATE);
  XP.begin("G1000 MFD");

  // Setup Multiplexers
  DigitalIn.setMux(1, 0, 2, 3);
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

  encNavInner.setCommand(F("sim/GPS/g1000n3_nav_inner_up"), F("sim/GPS/g1000n3_nav_inner_down"), F("sim/GPS/g1000n3_nav12"));
  encNavOuter.setCommand(F("sim/GPS/g1000n3_nav_outer_up"), F("sim/GPS/g1000n3_nav_outer_down"));
  encComInner.setCommand(F("sim/GPS/g1000n3_com_inner_up"), F("sim/GPS/g1000n3_com_inner_down"), F("sim/GPS/g1000n3_com12"));
  encComOuter.setCommand(F("sim/GPS/g1000n3_com_outer_up"), F("sim/GPS/g1000n3_com_outer_down"));
  encCourse.setCommand(F("sim/GPS/g1000n3_crs_up"), F("sim/GPS/g1000n3_crs_down"), F("sim/GPS/g1000n3_crs_sync"));
  encBaro.setCommand(F("sim/GPS/g1000n3_baro_up"), F("sim/GPS/g1000n3_baro_down"));
  encAltInner.setCommand(F("sim/GPS/g1000n3_alt_inner_up"), F("sim/GPS/g1000n3_alt_inner_down"), F("sim/autopilot/altitude_sync"));
  encAltOuter.setCommand(F("sim/GPS/g1000n3_alt_outer_up"), F("sim/GPS/g1000n3_alt_outer_down"));
  encFMSInner.setCommand(F("sim/GPS/g1000n3_fms_inner_up"), F("sim/GPS/g1000n3_fms_inner_down"), F("sim/GPS/g1000n3_cursor"));
  encFMSOuter.setCommand(F("sim/GPS/g1000n3_fms_outer_up"), F("sim/GPS/g1000n3_fms_outer_down"));
  btnDirect.setCommand(F("sim/GPS/g1000n3_direct"));
  btnFPL.setCommand(F("sim/GPS/g1000n3_fpl"));
  btnCLR.setCommand(F("sim/GPS/g1000n3_clr"));
  btnMENU.setCommand(F("sim/GPS/g1000n3_menu"));
  btnPROC.setCommand(F("sim/GPS/g1000n3_proc"));
  btnENT.setCommand(F("sim/GPS/g1000n3_ent"));
  btnAP.setCommand(F("sim/GPS/g1000n3_ap"));
  btnFD.setCommand(F("sim/GPS/g1000n3_fd"));
  btnNAV.setCommand(F("sim/GPS/g1000n3_nav"));
  btnALT.setCommand(F("sim/GPS/g1000n3_alt"));
  btnVS.setCommand(F("sim/GPS/g1000n3_vs"));
  btnFLC.setCommand(F("sim/GPS/g1000n3_flc"));
  btnYD.setCommand(F("sim/systems/yaw_damper_toggle"));
  btnHDG.setCommand(F("sim/GPS/g1000n3_hdg"));
  btnAPR.setCommand(F("sim/GPS/g1000n3_apr"));
  btnVNAV.setCommand(F("sim/GPS/g1000n3_vnv"));
  btnUP.setCommand(F("sim/GPS/g1000n3_nose_up"));
  btnDN.setCommand(F("sim/GPS/g1000n3_nose_down"));
  encNavVol.setCommand(F("sim/GPS/g1000n3_nvol_up"), F("sim/GPS/g1000n3_nvol_dn"), F("sim/GPS/g1000n3_nvol"));
  btnNavFF.setCommand(F("sim/GPS/g1000n3_nav_ff"));
  btnSoft1.setCommand(F("sim/GPS/g1000n3_softkey1"));
  btnSoft2.setCommand(F("sim/GPS/g1000n3_softkey2"));
  btnSoft3.setCommand(F("sim/GPS/g1000n3_softkey3"));
  btnSoft4.setCommand(F("sim/GPS/g1000n3_softkey4"));
  btnSoft5.setCommand(F("sim/GPS/g1000n3_softkey5"));
  btnSoft6.setCommand(F("sim/GPS/g1000n3_softkey6"));
  btnSoft7.setCommand(F("sim/GPS/g1000n3_softkey7"));
  btnSoft8.setCommand(F("sim/GPS/g1000n3_softkey8"));
  btnSoft9.setCommand(F("sim/GPS/g1000n3_softkey9"));
  btnSoft10.setCommand(F("sim/GPS/g1000n3_softkey10"));
  btnSoft11.setCommand(F("sim/GPS/g1000n3_softkey11"));
  btnSoft12.setCommand(F("sim/GPS/g1000n3_softkey12"));
  encComVol.setCommand(F("sim/GPS/g1000n3_cvol_up"), F("sim/GPS/g1000n3_cvol_dn"), F("sim/audio_panel/monitor_audio_com2"));
  btnComFF.setCommand(F("sim/GPS/g1000n3_com_ff"));
  encRange.setCommand(F("sim/GPS/g1000n3_range_up"), F("sim/GPS/g1000n3_range_down"));
  btnPanPush.setCommand(F("sim/GPS/g1000n3_pan_push"));
  btnPanUp.setCommand(F("sim/GPS/g1000n3_pan_up"));
  btnPanLeft.setCommand(F("sim/GPS/g1000n3_pan_left"));
  btnPanDown.setCommand(F("sim/GPS/g1000n3_pan_down"));
  btnPanRight.setCommand(F("sim/GPS/g1000n3_pan_right"));
  encHeading.setCommand(F("sim/GPS/g1000n3_hdg_up"), F("sim/GPS/g1000n3_hdg_down"), F("sim/GPS/g1000n3_hdg_sync"));
  encRudderTrim.setCommand(F("sim/flight_controls/rudder_trim_right"), F("sim/flight_controls/rudder_trim_left"), F("sim/flight_controls/rudder_trim_center"));
  btnGearTest.setCommand(F("aerobask/gear_test"));
  swLightStrobe.setCommand(F("sim/lights/strobe_lights_on"), F("sim/lights/strobe_lights_off"));
  swLightPosition.setCommand(F("sim/lights/nav_lights_on"), F("sim/lights/nav_lights_off"));
  swLightTaxi.setCommand(F("sim/lights/taxi_lights_on"), F("sim/lights/taxi_lights_off"));
  swLightLanding.setCommand(F("sim/lights/landing_lights_on"), F("sim/lights/landing_lights_off"));
  swFuelAuxLeft.setCommand(F("aerobask/auxfuel/pump1_up"), F("aerobask/auxfuel/pump1_dn"));
  swFuelAuxRight.setCommand(F("aerobask/auxfuel/pump2_up"), F("aerobask/auxfuel/pump2_dn"));
  swFuelLeft.setCommand(F("aerobask/eng/fuel_lever1_dn"), F("aerobask/eng/fuel_lever1_up"));
  swFuelRight.setCommand(F("aerobask/eng/fuel_lever2_dn"), F("aerobask/eng/fuel_lever2_up"));

  XP.registerDataRef(F("sim/flightmodel2/gear/deploy_ratio"), XPL_READ, 100, 0, &gear_ratio[0], 0);
  XP.registerDataRef(F("sim/flightmodel2/gear/deploy_ratio"), XPL_READ, 100, 0, &gear_ratio[1], 1);
  XP.registerDataRef(F("sim/flightmodel2/gear/deploy_ratio"), XPL_READ, 100, 0, &gear_ratio[2], 2);
  XP.registerDataRef(F("sim/cockpit/warnings/annunciators/gear_unsafe"), XPL_READ, 100, 1, &gear_unsafe);
  XP.registerDataRef(F("sim/flightmodel2/controls/flap1_deploy_ratio"), XPL_READ, 100, 0, &flap_ratio);
  XP.registerDataRef(F("sim/cockpit2/switches/instrument_brightness_ratio"), XPL_WRITE, 100, 0, &light_instr, 0);
  XP.registerDataRef(F("aerobask/knob_inst"), XPL_WRITE, 100, 0, &light_instr);
  XP.registerDataRef(F("sim/cockpit2/switches/panel_brightness_ratio"), XPL_WRITE, 100, 0, &light_flood, 0);
  XP.registerDataRef(F("aerobask/knob_flood"), XPL_WRITE, 100, 0, &light_flood);
  XP.registerDataRef(F("sim/cockpit2/controls/gear_handle_down"), XPL_READWRITE, 100, 0, &gear_handle_down);
  XP.registerDataRef(F("sim/cockpit2/controls/flap_handle_request_ratio"), XPL_READWRITE, 100, 0, &flap_handle_request_ratio);
}

// Main loop
void loop()
{
  XP.xloop();
  DigitalIn.handle();
  leds.handle();

  encNavInner.handleXP();
  encNavOuter.handleXP();
  encComInner.handleXP();
  encComOuter.handleXP();
  encCourse.handleXP();
  encBaro.handleXP();
  encAltInner.handleXP();
  encAltOuter.handleXP();
  encFMSInner.handleXP();
  encFMSOuter.handleXP();
  btnDirect.handleXP();
  btnFPL.handleXP();
  btnCLR.handleXP();
  btnMENU.handleXP();
  btnPROC.handleXP();
  btnENT.handleXP();
  btnAP.handleXP();
  btnFD.handleXP();
  btnNAV.handleXP();
  btnALT.handleXP();
  btnVS.handleXP();
  btnFLC.handleXP();
  btnYD.handleXP();
  btnHDG.handleXP();
  btnAPR.handleXP();
  btnVNAV.handleXP();
  btnUP.handleXP();
  btnDN.handleXP();
  encNavVol.handleXP();
  btnNavFF.handleXP();
  btnSoft1.handleXP();
  btnSoft2.handleXP();
  btnSoft3.handleXP();
  btnSoft4.handleXP();
  btnSoft5.handleXP();
  btnSoft6.handleXP();
  btnSoft7.handleXP();
  btnSoft8.handleXP();
  btnSoft9.handleXP();
  btnSoft10.handleXP();
  btnSoft11.handleXP();
  btnSoft12.handleXP();
  encComVol.handleXP();
  btnComFF.handleXP();
  encRange.handleXP();
  encHeading.handleXP();

  // handle pan stick with logical dependecies
  btnPanPush.handleXP(!DigitalIn.getBit(4, 1) && !DigitalIn.getBit(4, 2) && !DigitalIn.getBit(4, 3) && !DigitalIn.getBit(4, 4));
  btnPanUp.handleXP(DigitalIn.getBit(4, 0));
  btnPanLeft.handleXP(DigitalIn.getBit(4, 0));
  btnPanDown.handleXP(DigitalIn.getBit(4, 0));
  btnPanRight.handleXP(DigitalIn.getBit(4, 0));

  encRudderTrim.handleXP();
  btnGearTest.handleXP();

  swLightStrobe.handleXP();
  swLightPosition.handleXP();
  swLightTaxi.handleXP();
  swLightLanding.handleXP();

  swFuelLeft.handleXP();
  swFuelRight.handleXP();
  swFuelAuxLeft.handleXP();
  swFuelAuxRight.handleXP();

  // Sync Switches 
  if (tmrSync.elapsed())
  {
    XP.commandTrigger(swLightStrobe.getCommand());
    XP.commandTrigger(swLightPosition.getCommand());
    XP.commandTrigger(swLightTaxi.getCommand());
    XP.commandTrigger(swLightLanding.getCommand());
    XP.commandTrigger(swFuelAuxLeft.getCommand());
    XP.commandTrigger(swFuelAuxRight.getCommand());
    if (swFuelLeft.on1() || swFuelLeft.on2())
    {
      XP.commandTrigger(swFuelLeft.getCommand(), 2);
    }
    if (swFuelRight.on1() || swFuelRight.on2())
    {
      XP.commandTrigger(swFuelRight.getCommand(), 2);
    }
  }

  swFlaps.handle();
  flap_handle_request_ratio = swFlaps.value(1.0, 0.5, 0.0);
  swGear.handle();
  gear_handle_down = swGear.value(0.0, 1.0);

  potInstr.handle();
  light_instr = 0.01 * round(100.0 * potInstr.value());
  potFlood.handle();
  light_flood = 0.01 * round(100.0 * potFlood.value());

  // Update flap LEDs
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

  // gear status LEDs (yellow simulated by flashing)
  leds.set(LED_GEAR_NOSE, (gear_ratio[0] > 0.99) ? ledOn : (gear_ratio[0] < 0.01) ? ledOff : ledFast);
  leds.set(LED_GEAR_LEFT, (gear_ratio[1] > 0.99) ? ledOn : (gear_ratio[1] < 0.01) ? ledOff : ledFast);
  leds.set(LED_GEAR_RIGHT, (gear_ratio[2] > 0.99) ? ledOn : (gear_ratio[2] < 0.01) ? ledOff : ledFast);
  
  // gear unsafe LED
  leds.set(LED_GEAR_UNSAFE, (gear_unsafe != 0) ? ledOn : ledOff);

  // if (tmrMain.elapsed())
  // {
  //   char tmp[16];
  //   sprintf(tmp, " %ld Cycles/s", tmrMain.count());
  //   XP.sendDebugMessage(tmp);
  // }
}

#endif