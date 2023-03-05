#include <Arduino.h>
#include <MemoryFree.h>
#include <Mux.h>
#include <Button.h>
#include <Encoder.h>
#include <Switch.h>
#include <LedShift.h>
#include <AnalogIn.h>
#include <SoftTimer.h>
#include <XPLDirect.h>

// Interface
XPLDirect XP(&Serial);

// MUX 0
Encoder encNavInner(0, 1, 2, 0, 4);
Encoder encNavOuter(0, 4, 3, 255, 4);
Encoder encComInner(0, 6, 7, 5, 4);
Encoder encComOuter(0, 9, 8, 255, 4);
Encoder encCourse(0, 11, 12, 10, 4);
Encoder encBaro(0, 14, 13, 255, 4);

// MUX 1
Encoder encAltInner(1, 1, 2, 0, 4);
Encoder encAltOuter(1, 4, 3, 255, 4);
Encoder encFMSInner(1, 6, 7, 5, 4);
Encoder encFMSOuter(1, 9, 8, 255, 4);
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
Encoder encNavVol(2, 12, 13, 14, 4);
Button btnNavFF(2,15);

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
Encoder encComVol(3, 12, 13, 14, 4);
Button btnComFF(3,15);

// MUX4 Pan/Zoom
Encoder encRange(4, 6, 5, 0, 2);
Button btnPanPush(4, 0);
RepeatButton btnPanUp(4, 1, 250);
RepeatButton btnPanLeft(4, 2, 250);
RepeatButton btnPanDown(4, 3, 250);
RepeatButton btnPanRight(4, 4, 250);
Encoder encHeading(4, 12, 13, 14, 4);

// MFD specific
Encoder encRudderTrim(4, 8, 9, 10, 4);
Switch swLightLanding(5, 0);
Switch swLightTaxi(5, 1);
Switch swLightPosition(5, 2);
Switch swLightStrobe(5, 3);
// Switch2 swFuelLeft(5, 4, 5);
// Switch2 swFuelRight(5, 6, 7);
Switch swFuelAuxLeft(5, 8);
Switch swFuelAuxRight(5, 9);
Button butGearTest(5, 10);
Switch swGear(5, 11);
// Switch2 swFlaps(5, 12, 13);

// Potentiometers
AnalogIn potInstr(A0, false, 10);
AnalogIn potFlood(A1, false, 10);

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
SoftTimer tmrMain(10.0);

// Commands MUX 0
int cmdNavInnerUp;
int cmdNavInnerDown;
int cmdNavOuterUp;
int cmdNavOuterDown;
int cmdNavToggle;
int cmdComInnerUp;
int cmdComInnerDown;
int cmdComOuterUp;
int cmdComOuterDown;
int cmdComToggle;
int cmdCourseUp;
int cmdCourseDown;
int cmdCourseSync;
int cmdBaroUp;
int cmdBaroDown;

// Commands MUX 1
int cmdAltSync;
int cmdAltInnerUp;
int cmdAltInnerDown;
int cmdAltOuterUp;
int cmdAltOuterDown;
int cmdFMSCursor;
int cmdFMSInnerUp;
int cmdFMSInnerDown;
int cmdFMSOuterUp;
int cmdFMSOuterDown;
int cmdDirect;
int cmdFPL;
int cmdCLR;
int cmdMENU;
int cmdPROC;
int cmdENT;

// Commands MUX 2
int cmdAP;
int cmdFD;
int cmdNAV;
int cmdALT;
int cmdVS;
int cmdFLC;
int cmdYD;
int cmdHDG;
int cmdAPR;
int cmdVNAV;
int cmdUP;
int cmdDN;
int cmdNavVolUp;
int cmdNavVolDown;
int cmdNavVol;
int cmdNavFF;

// Commands MUX 3
int cmdSoft1;
int cmdSoft2;
int cmdSoft3;
int cmdSoft4;
int cmdSoft5;
int cmdSoft6;
int cmdSoft7;
int cmdSoft8;
int cmdSoft9;
int cmdSoft10;
int cmdSoft11;
int cmdSoft12;
int cmdComVolUp;
int cmdComVolDown;
int cmdComVol;
int cmdComFF;

// Commands MUX 4
int cmdRangeUp;
int cmdRangeDown;
int cmdPanPush;
int cmdPanUp;
int cmdPanLeft;
int cmdPanDown;
int cmdPanRight;
int cmdHeadingUp;
int cmdHeadingDown;
int cmdHeadingSync;

// Commands MFD
int cmdRudderTrimLeft;
int cmdRudderTrimRight;
int cmdRudderTrimCenter;

// DataRefs MUX 5

// Setup 
void setup()
{
  Serial.begin(XPLDIRECT_BAUDRATE);
  XP.begin("G1000 MFD");

  // Setup Multiplexers
  Mux.begin(1, 0, 2, 3); 
  Mux.addMux(4); // MUX 0
  Mux.addMux(5); // MUX 1
  Mux.addMux(6); // MUX 2
  Mux.addMux(7); // MUX 3
  Mux.addMux(8); // MUX 4
  Mux.addMux(9); // MUX 5
  
  // init led sequence
  leds.set_all(ledOff);
  for (int pin = 0; pin < 7; pin++)
  {
    leds.set(pin, ledOn);
    leds.handle();
    delay(200);
  }
  leds.set_all(ledOff);

  // Commands MUX 0
  // implicit method
  encNavInner.setCmdUp(XP.registerCommand(F("sim/GPS/g1000n3_nav_inner_up")));
  encNavInner.setCmdDown(XP.registerCommand(F("sim/GPS/g1000n3_nav_inner_down")));
  encNavOuter.setCmdUp(XP.registerCommand(F("sim/GPS/g1000n3_nav_outer_up")));
  encNavOuter.setCmdDown(XP.registerCommand(F("sim/GPS/g1000n3_nav_outer_down")));
  encNavInner.setCmdPush(XP.registerCommand(F("sim/GPS/g1000n3_nav12")));
  encComInner.setCmdUp(XP.registerCommand(F("sim/GPS/g1000n3_com_inner_up")));
  encComInner.setCmdDown(XP.registerCommand(F("sim/GPS/g1000n3_com_inner_down")));
  encComOuter.setCmdUp(XP.registerCommand(F("sim/GPS/g1000n3_com_outer_up")));
  encComOuter.setCmdDown(XP.registerCommand(F("sim/GPS/g1000n3_com_outer_down")));
  encComInner.setCmdPush(XP.registerCommand(F("sim/GPS/g1000n3_com12")));
  // explicit method
  cmdCourseUp     = XP.registerCommand(F("sim/GPS/g1000n3_crs_up"));
  cmdCourseDown   = XP.registerCommand(F("sim/GPS/g1000n3_crs_down"));
  cmdCourseSync   = XP.registerCommand(F("sim/GPS/g1000n3_crs_sync"));
  cmdBaroUp       = XP.registerCommand(F("sim/GPS/g1000n3_baro_up"));
  cmdBaroDown     = XP.registerCommand(F("sim/GPS/g1000n3_baro_down"));
  
  // Commands MUX 1
  cmdAltSync      = XP.registerCommand(F("sim/autopilot/altitude_sync"));
  cmdAltInnerUp   = XP.registerCommand(F("sim/GPS/g1000n3_alt_inner_up"));
  cmdAltInnerDown = XP.registerCommand(F("sim/GPS/g1000n3_alt_inner_down"));
  cmdAltOuterUp   = XP.registerCommand(F("sim/GPS/g1000n3_alt_outer_up"));
  cmdAltOuterDown = XP.registerCommand(F("sim/GPS/g1000n3_alt_outer_down"));
  cmdFMSCursor    = XP.registerCommand(F("sim/GPS/g1000n3_cursor"));
  cmdFMSInnerUp   = XP.registerCommand(F("sim/GPS/g1000n3_fms_inner_up"));
  cmdFMSInnerDown = XP.registerCommand(F("sim/GPS/g1000n3_fms_inner_down"));
  cmdFMSOuterUp   = XP.registerCommand(F("sim/GPS/g1000n3_fms_outer_up"));
  cmdFMSOuterDown = XP.registerCommand(F("sim/GPS/g1000n3_fms_outer_down"));
  cmdDirect       = XP.registerCommand(F("sim/GPS/g1000n3_direct"));
  cmdFPL          = XP.registerCommand(F("sim/GPS/g1000n3_fpl"));
  cmdCLR          = XP.registerCommand(F("sim/GPS/g1000n3_clr"));
  cmdMENU         = XP.registerCommand(F("sim/GPS/g1000n3_menu"));
  cmdPROC         = XP.registerCommand(F("sim/GPS/g1000n3_proc"));
  cmdENT          = XP.registerCommand(F("sim/GPS/g1000n3_ent"));
  
  // Commands MUX 2
  cmdAP           = XP.registerCommand(F("sim/GPS/g1000n3_ap"));
  cmdFD           = XP.registerCommand(F("sim/GPS/g1000n3_fd"));
  cmdNAV          = XP.registerCommand(F("sim/GPS/g1000n3_nav"));
  cmdALT          = XP.registerCommand(F("sim/GPS/g1000n3_alt"));
  cmdVS           = XP.registerCommand(F("sim/GPS/g1000n3_vs"));
  cmdFLC          = XP.registerCommand(F("sim/GPS/g1000n3_flc"));
  cmdYD           = XP.registerCommand(F("sim/systems/yaw_damper_toggle"));
  cmdHDG          = XP.registerCommand(F("sim/GPS/g1000n3_hdg"));
  cmdAPR          = XP.registerCommand(F("sim/GPS/g1000n3_apr"));
  cmdVNAV         = XP.registerCommand(F("sim/GPS/g1000n3_vnv"));
  cmdUP           = XP.registerCommand(F("sim/GPS/g1000n3_nose_up"));
  cmdDN           = XP.registerCommand(F("sim/GPS/g1000n3_nose_down"));
  cmdNavVolUp     = XP.registerCommand(F("sim/GPS/g1000n3_nvol_up"));
  cmdNavVolDown   = XP.registerCommand(F("sim/GPS/g1000n3_nvol_dn"));
  cmdNavVol       = XP.registerCommand(F("sim/GPS/g1000n3_nvol"));
  cmdNavFF        = XP.registerCommand(F("sim/GPS/g1000n3_nav_ff"));

  // Commands MUX 3
  cmdSoft1        = XP.registerCommand(F("sim/GPS/g1000n3_softkey1"));
  cmdSoft2        = XP.registerCommand(F("sim/GPS/g1000n3_softkey2"));
  cmdSoft3        = XP.registerCommand(F("sim/GPS/g1000n3_softkey3"));
  cmdSoft4        = XP.registerCommand(F("sim/GPS/g1000n3_softkey4"));
  cmdSoft5        = XP.registerCommand(F("sim/GPS/g1000n3_softkey5"));
  cmdSoft6        = XP.registerCommand(F("sim/GPS/g1000n3_softkey6"));
  cmdSoft7        = XP.registerCommand(F("sim/GPS/g1000n3_softkey7"));
  cmdSoft8        = XP.registerCommand(F("sim/GPS/g1000n3_softkey8"));
  cmdSoft9        = XP.registerCommand(F("sim/GPS/g1000n3_softkey9"));
  cmdSoft10       = XP.registerCommand(F("sim/GPS/g1000n3_softkey10"));
  cmdSoft11       = XP.registerCommand(F("sim/GPS/g1000n3_softkey11"));
  cmdSoft12       = XP.registerCommand(F("sim/GPS/g1000n3_softkey12"));
  cmdComVolUp     = XP.registerCommand(F("sim/GPS/g1000n3_cvol_up"));
  cmdComVolDown   = XP.registerCommand(F("sim/GPS/g1000n3_cvol_dn"));
  cmdComVol       = XP.registerCommand(F("sim/audio_panel/monitor_audio_com2"));
  cmdComFF        = XP.registerCommand(F("sim/GPS/g1000n3_com_ff"));

  // Commands MUX 4
  cmdRangeUp      = XP.registerCommand(F("sim/GPS/g1000n3_range_up"));
  cmdRangeDown    = XP.registerCommand(F("sim/GPS/g1000n3_range_down"));
  cmdPanPush      = XP.registerCommand(F("sim/GPS/g1000n3_pan_push"));
  cmdPanUp        = XP.registerCommand(F("sim/GPS/g1000n3_pan_up"));
  cmdPanLeft      = XP.registerCommand(F("sim/GPS/g1000n3_pan_left"));
  cmdPanDown      = XP.registerCommand(F("sim/GPS/g1000n3_pan_down"));
  cmdPanRight     = XP.registerCommand(F("sim/GPS/g1000n3_pan_right"));
  cmdHeadingUp    = XP.registerCommand(F("sim/GPS/g1000n3_hdg_up"));
  cmdHeadingDown  = XP.registerCommand(F("sim/GPS/g1000n3_hdg_down"));
  cmdHeadingSync  = XP.registerCommand(F("sim/GPS/g1000n3_hdg_sync"));

  // MFD

  // delay(5000);
  // Serial.println(freeMemory(), DEC);
}

void handleDevice(Button *btn, int cmd)
{
  btn->handle();
  if (btn->pressed())
  {
    XP.commandStart(cmd);
  }
  if (btn->released())
  {
    XP.commandEnd(cmd);
  }
}

void handleDevice(Encoder *enc, int cmdUp, int cmdDown)
{
  enc->handle();
  if (enc->up())
  {
    XP.commandTrigger(cmdUp);
  }
  if (enc->down())
  {
    XP.commandTrigger(cmdDown);
  }
}

void handleDevice(Encoder *enc, int cmdUp, int cmdDown, int cmdPress)
{
  handleDevice(enc, cmdUp, cmdDown);
  if (enc->pressed())
  {
    XP.commandStart(cmdPress);
  }
  if (enc->released())
  {
    XP.commandEnd(cmdPress);
  }
}

void handleDevice(Encoder *enc)
{
  enc->handle();
  if (enc->up())
  {
    XP.commandTrigger(enc->getCmdUp());
  }
  if (enc->down())
  {
    XP.commandTrigger(enc->getCmdDown());
  }
  int cmdPush = enc->getCmdPush();
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

// Main loop
void loop()
{
  XP.xloop();
  Mux.handle();
  leds.handle();

  // handle all input devices directly connected to commands
  // handleDevice(&encNavInner, cmdNavInnerUp, cmdNavInnerDown, cmdNavToggle);
  // handleDevice(&encNavOuter, cmdNavOuterUp, cmdNavOuterDown);
  // handleDevice(&encComInner, cmdComInnerUp, cmdComInnerDown, cmdComToggle);
  // handleDevice(&encComOuter, cmdComOuterUp, cmdComOuterDown);

  handleDevice(&encNavInner);
  handleDevice(&encNavOuter);
  handleDevice(&encComInner);
  handleDevice(&encComOuter);

  handleDevice(&encCourse, cmdCourseUp, cmdCourseDown, cmdCourseSync);
  handleDevice(&encBaro, cmdBaroUp, cmdBaroDown);
  handleDevice(&encAltInner, cmdAltInnerUp, cmdAltInnerDown, cmdAltSync);
  handleDevice(&encAltOuter, cmdAltOuterUp, cmdAltOuterDown);
  handleDevice(&encFMSInner, cmdFMSInnerUp, cmdFMSInnerDown, cmdFMSCursor);
  handleDevice(&encFMSOuter, cmdFMSOuterUp, cmdFMSOuterDown);
  handleDevice(&btnDirect, cmdDirect);
  handleDevice(&btnFPL, cmdFPL);
  handleDevice(&btnCLR, cmdCLR);
  handleDevice(&btnMENU, cmdMENU);
  handleDevice(&btnPROC, cmdPROC);
  handleDevice(&btnENT, cmdENT);
  handleDevice(&btnAP, cmdAP);
  handleDevice(&btnFD, cmdFD);
  handleDevice(&btnNAV, cmdNAV);
  handleDevice(&btnALT, cmdALT);
  handleDevice(&btnVS, cmdVS);
  handleDevice(&btnFLC, cmdFLC);
  handleDevice(&btnYD, cmdYD);
  handleDevice(&btnHDG, cmdHDG);
  handleDevice(&btnAPR, cmdAPR);
  handleDevice(&btnVNAV, cmdVNAV);
  handleDevice(&btnUP, cmdUP);
  handleDevice(&btnDN, cmdDN);
  handleDevice(&encNavVol, cmdNavVolUp, cmdNavVolDown, cmdNavVol);
  handleDevice(&btnNavFF, cmdNavFF);
  handleDevice(&btnSoft1, cmdSoft1);
  handleDevice(&btnSoft2, cmdSoft2);
  handleDevice(&btnSoft3, cmdSoft3);
  handleDevice(&btnSoft4, cmdSoft4);
  handleDevice(&btnSoft5, cmdSoft5);
  handleDevice(&btnSoft6, cmdSoft6);
  handleDevice(&btnSoft7, cmdSoft7);
  handleDevice(&btnSoft8, cmdSoft8);
  handleDevice(&btnSoft9, cmdSoft9);
  handleDevice(&btnSoft10, cmdSoft10);
  handleDevice(&btnSoft11, cmdSoft11);
  handleDevice(&btnSoft12, cmdSoft12);
  handleDevice(&encComVol, cmdComVolUp, cmdComVolDown, cmdComVol);
  handleDevice(&btnComFF, cmdComFF);
  handleDevice(&encRange, cmdRangeUp, cmdRangeDown);
  // handle pan stick manually due to logical operations for inputs
  btnPanPush.handle(Mux.getBit(4, 0) && !Mux.getBit(4, 1) &&!Mux.getBit(4, 2) &&!Mux.getBit(4, 3) &&!Mux.getBit(4, 4));
  if (btnPanPush.pressed())
  {
    XP.commandTrigger(cmdPanPush);
  }
  btnPanUp.handle(Mux.getBit(4, 0) && Mux.getBit(4, 1));
  if (btnPanUp.pressed())
  {
    XP.commandTrigger(cmdPanUp);
  }
  btnPanLeft.handle(Mux.getBit(4, 0) && Mux.getBit(4, 2));
  if (btnPanLeft.pressed())
  {
    XP.commandTrigger(cmdPanLeft);
  }
  btnPanDown.handle(Mux.getBit(4, 0) && Mux.getBit(4, 3));
  if (btnPanDown.pressed())
  {
    XP.commandTrigger(cmdPanDown);
  }
  btnPanRight.handle(Mux.getBit(4, 0) && Mux.getBit(4, 4));
  if (btnPanRight.pressed())
  {
    XP.commandTrigger(cmdPanRight);
  }

  handleDevice(&encRudderTrim, cmdRudderTrimRight, cmdRudderTrimLeft, cmdRudderTrimCenter);
}