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
LedShift ledMFD(16, 14, 15);
#define LED_FLAP_UP 0
#define LED_FLAP_TO 1
#define LED_FLAP_LDG 2
#define LED_GEAR_UNSAFE 3
#define LED_GEAR_NOSE 4
#define LED_GEAR_LEFT 5
#define LED_GEAR_RIGHT 6

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

  // Commands MUX 0
  cmdNavInnerUp   = XP.registerCommand(F("sim/GPS/g1000n3_nav_inner_up"));
  cmdNavInnerDown = XP.registerCommand(F("sim/GPS/g1000n3_nav_inner_down"));
  cmdNavOuterUp   = XP.registerCommand(F("sim/GPS/g1000n3_nav_outer_up"));
  cmdNavOuterDown = XP.registerCommand(F("sim/GPS/g1000n3_nav_outer_down"));
  cmdNavToggle    = XP.registerCommand(F("sim/GPS/g1000n3_nav12"));
  cmdComInnerUp   = XP.registerCommand(F("sim/GPS/g1000n3_com_inner_up"));
  cmdComInnerDown = XP.registerCommand(F("sim/GPS/g1000n3_com_inner_down"));
  cmdComOuterUp   = XP.registerCommand(F("sim/GPS/g1000n3_com_outer_up"));
  cmdComOuterDown = XP.registerCommand(F("sim/GPS/g1000n3_com_outer_down"));
  cmdComToggle    = XP.registerCommand(F("sim/GPS/g1000n3_com12"));
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

// Main loop
void loop()
{
  Mux.handle();

  encNavInner.handle();
  encNavOuter.handle();
  encComInner.handle();
  encComOuter.handle();
  encCourse.handle();
  encBaro.handle();
  encAltInner.handle();
  encAltOuter.handle();
  encFMSInner.handle();
  encFMSOuter.handle();
  btnDirect.handle();
  btnFPL.handle();
  btnCLR.handle();
  btnMENU.handle();
  btnPROC.handle();
  btnENT.handle();
  btnAP.handle();
  btnFD.handle();
  btnNAV.handle();
  btnALT.handle();
  btnVS.handle();
  btnFLC.handle();
  btnYD.handle();
  btnHDG.handle();
  btnAPR.handle();
  btnVNAV.handle();
  btnUP.handle();
  btnDN.handle();
  encNavVol.handle();
  btnNavFF.handle();

  if (tmrMain.isTick())
  {
    XP.xloop();
    
    // MUX 0
    if (encNavInner.pressed())  XP.commandTrigger(cmdNavToggle);
    if (encNavInner.up())       XP.commandTrigger(cmdNavInnerUp);
    if (encNavInner.down())     XP.commandTrigger(cmdNavInnerDown);
    if (encNavOuter.up())       XP.commandTrigger(cmdNavOuterUp);
    if (encNavOuter.down())     XP.commandTrigger(cmdNavOuterDown);
    if (encComInner.pressed())  XP.commandTrigger(cmdComToggle);
    if (encComInner.up())       XP.commandTrigger(cmdComInnerUp);
    if (encComInner.down())     XP.commandTrigger(cmdComInnerDown);
    if (encComOuter.up())       XP.commandTrigger(cmdComOuterUp);
    if (encComOuter.down())     XP.commandTrigger(cmdComOuterDown);
    if (encCourse.pressed())    XP.commandTrigger(cmdCourseSync);
    if (encCourse.up())         XP.commandTrigger(cmdCourseUp);
    if (encCourse.down())       XP.commandTrigger(cmdCourseDown);
    if (encBaro.up())           XP.commandTrigger(cmdBaroUp);
    if (encBaro.down())         XP.commandTrigger(cmdBaroDown);

    // MUX 1
    if (encAltInner.pressed())  XP.commandTrigger(cmdAltSync);
    if (encAltInner.up())       XP.commandTrigger(cmdAltInnerUp);
    if (encAltInner.down())     XP.commandTrigger(cmdAltInnerDown);
    if (encAltOuter.up())       XP.commandTrigger(cmdAltOuterUp);
    if (encAltOuter.down())     XP.commandTrigger(cmdAltOuterDown);
    if (encFMSInner.pressed())  XP.commandTrigger(cmdFMSCursor);
    if (encFMSInner.up())       XP.commandTrigger(cmdFMSInnerDown);
    if (encFMSInner.down())     XP.commandTrigger(cmdFMSInnerDown);
    if (encFMSOuter.up())       XP.commandTrigger(cmdFMSOuterUp);
    if (encFMSOuter.down())     XP.commandTrigger(cmdFMSOuterDown);
    if (btnDirect.pressed())    XP.commandTrigger(cmdDirect);
    if (btnFPL.pressed())       XP.commandTrigger(cmdFPL);
    if (btnCLR.pressed())       XP.commandTrigger(cmdCLR);
    if (btnMENU.pressed())      XP.commandTrigger(cmdMENU);
    if (btnPROC.pressed())      XP.commandTrigger(cmdPROC);
    if (btnENT.pressed())       XP.commandTrigger(cmdENT);

    // MUX 2
    if (btnAP.pressed())        XP.commandTrigger(cmdAP);
    if (btnFD.pressed())        XP.commandTrigger(cmdFD);
    if (btnNAV.pressed())       XP.commandTrigger(cmdNAV);
    if (btnALT.pressed())       XP.commandTrigger(cmdALT);
    if (btnVS.pressed())        XP.commandTrigger(cmdVS);
    if (btnFLC.pressed())       XP.commandTrigger(cmdFLC);
    if (btnYD.pressed())        XP.commandTrigger(cmdYD);
    if (btnHDG.pressed())       XP.commandTrigger(cmdHDG);
    if (btnAPR.pressed())       XP.commandTrigger(cmdAPR);
    if (btnVNAV.pressed())      XP.commandTrigger(cmdVNAV);
    if (btnUP.pressed())        XP.commandTrigger(cmdUP);
    if (btnDN.pressed())        XP.commandTrigger(cmdDN);
    if (encNavVol.up())         XP.commandTrigger(cmdNavVolUp);
    if (encNavVol.down())       XP.commandTrigger(cmdNavVolDown);
    if (encNavVol.pressed())    XP.commandTrigger(cmdNavVol);
    if (btnNavFF.pressed())     XP.commandTrigger(cmdNavFF);

    // MUX 3
    if (btnSoft1.pressed())     XP.commandTrigger(cmdSoft1);
    if (btnSoft2.pressed())     XP.commandTrigger(cmdSoft2);
    if (btnSoft3.pressed())     XP.commandTrigger(cmdSoft3);
    if (btnSoft4.pressed())     XP.commandTrigger(cmdSoft4);
    if (btnSoft5.pressed())     XP.commandTrigger(cmdSoft5);
    if (btnSoft6.pressed())     XP.commandTrigger(cmdSoft6);
    if (btnSoft7.pressed())     XP.commandTrigger(cmdSoft7);
    if (btnSoft8.pressed())     XP.commandTrigger(cmdSoft8);
    if (btnSoft9.pressed())     XP.commandTrigger(cmdSoft9);
    if (btnSoft10.pressed())    XP.commandTrigger(cmdSoft10);
    if (btnSoft11.pressed())    XP.commandTrigger(cmdSoft11);
    if (btnSoft12.pressed())    XP.commandTrigger(cmdSoft12);
    if (encComVol.up())         XP.commandTrigger(cmdComVolUp);
    if (encComVol.down())       XP.commandTrigger(cmdComVolDown);
    if (encComVol.pressed())    XP.commandTrigger(cmdComVol);
    if (btnComFF.pressed())     XP.commandTrigger(cmdComFF);

    // MUX 4 
    if (encRange.up())          XP.commandTrigger(cmdRangeUp);
    if (encRange.down())        XP.commandTrigger(cmdRangeDown);
    // TODO -> validate!
    if (encRange.pressed() && !btnPanUp.engaged() && !btnPanLeft.engaged() && 
        !btnPanDown.engaged() && !btnPanRight.engaged()) XP.commandTrigger(cmdPanPush);
    if (btnPanUp.pressed() && encRange.engaged())    XP.commandTrigger(cmdPanUp);
    if (btnPanLeft.pressed() && encRange.engaged())  XP.commandTrigger(cmdPanLeft);
    if (btnPanDown.pressed() && encRange.engaged())  XP.commandTrigger(cmdPanDown);
    if (btnPanRight.pressed() && encRange.engaged()) XP.commandTrigger(cmdPanRight);
    if (encHeading.up())        XP.commandTrigger(cmdHeadingUp);
    if (encHeading.down())      XP.commandTrigger(cmdHeadingDown);
    if (encHeading.pressed())   XP.commandTrigger(cmdHeadingSync);

    if (encRudderTrim.up())     XP.commandTrigger(cmdRudderTrimRight);
    if (encRudderTrim.down())   XP.commandTrigger(cmdRudderTrimLeft);
    if (encRudderTrim.pressed())XP.commandTrigger(cmdRudderTrimCenter);

  }
}