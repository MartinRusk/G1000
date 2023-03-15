#include <Arduino.h>
#include <MemoryFree.h>
#include <XPLDevices.h>

#if G1000_PFD

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

// PFD specific
// MUX2
Switch swMaster(2, 0);
Switch swAVMaster(2, 1);
Switch swPitot(2, 2);
Switch swOxygen(2, 3);
Button btnDeiceMax(2, 5);
Switch2 swDeice(2, 6, 7);
Button btnDeiceWS(2, 8);
Switch2 swDeiceAnnun(2, 9, 10);
Switch swBackup(2, 11);
// MUX 5
Switch swAltLeft(5, 0);
Button btnStartLeft(5, 1);
Button btnStartRight(5, 2);
Switch swAltRight(5, 3);
Switch swPumpLeft(5, 4);
Switch swEngineMasterLeft(5, 5);
Switch swEngineMasterRight(5, 6);
Switch swPumpRight(5, 7);
Switch swVoteLeftA(5, 8);
Switch swVoteLeftB(5, 9);
Button btnECUTestLeft(5, 10);
Button btnECUTestRight(5, 11);
Switch swVoteRightA(5, 12);
Switch swVoteRightB(5, 13);

// Datarefs
long deice_norm;
long deice_high;
long deice_max;

// LEDs
LedShift leds(16, 14, 15);
#define LED_DEICE_MAX 0
#define LED_DEICE_HIGH 1
#define LED_DEICE_NORM 2

// Timer for Main loop
Timer tmrMain(1000.0);
Timer tmrSync(1000.0);

// Setup
void setup()
{
  Serial.begin(XPLDIRECT_BAUDRATE);
  XP.begin("G1000 PFD");

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
  for (int pin = 0; pin < 3; pin++)
  {
    leds.set(pin, ledOn);
    leds.handle();
    delay(100);
  }
  leds.set_all(ledOff);

  encNavInner.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_nav_inner_up")),
      XP.registerCommand(F("sim/GPS/g1000n1_nav_inner_down")),
      XP.registerCommand(F("sim/GPS/g1000n1_nav12")));
  encNavOuter.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_nav_outer_up")),
      XP.registerCommand(F("sim/GPS/g1000n1_nav_outer_down")));
  encComInner.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_com_inner_up")),
      XP.registerCommand(F("sim/GPS/g1000n1_com_inner_down")),
      XP.registerCommand(F("sim/GPS/g1000n1_com12")));
  encComOuter.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_com_outer_up")),
      XP.registerCommand(F("sim/GPS/g1000n1_com_outer_down")));
  encCourse.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_crs_up")),
      XP.registerCommand(F("sim/GPS/g1000n1_crs_down")),
      XP.registerCommand(F("sim/GPS/g1000n1_crs_sync")));
  encBaro.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_baro_up")),
      XP.registerCommand(F("sim/GPS/g1000n1_baro_down")));

  encAltInner.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_alt_inner_up")),
      XP.registerCommand(F("sim/GPS/g1000n1_alt_inner_down")),
      XP.registerCommand(F("sim/autopilot/altitude_sync")));
  encAltOuter.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_alt_outer_up")),
      XP.registerCommand(F("sim/GPS/g1000n1_alt_outer_down")));
  encFMSInner.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_fms_inner_up")),
      XP.registerCommand(F("sim/GPS/g1000n1_fms_inner_down")),
      XP.registerCommand(F("sim/GPS/g1000n1_cursor")));
  encFMSOuter.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_fms_outer_up")),
      XP.registerCommand(F("sim/GPS/g1000n1_fms_outer_down")));
  btnDirect.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_direct")));
  btnFPL.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_fpl")));
  btnCLR.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_clr")));
  btnMENU.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_menu")));
  btnPROC.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_proc")));
  btnENT.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_ent")));

  encNavVol.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_nvol_up")),
      XP.registerCommand(F("sim/GPS/g1000n1_nvol_dn")),
      XP.registerCommand(F("sim/GPS/g1000n1_nvol")));
  btnNavFF.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_nav_ff")));

  btnSoft1.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_softkey1")));
  btnSoft2.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_softkey2")));
  btnSoft3.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_softkey3")));
  btnSoft4.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_softkey4")));
  btnSoft5.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_softkey5")));
  btnSoft6.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_softkey6")));
  btnSoft7.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_softkey7")));
  btnSoft8.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_softkey8")));
  btnSoft9.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_softkey9")));
  btnSoft10.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_softkey10")));
  btnSoft11.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_softkey11")));
  btnSoft12.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_softkey12")));
  encComVol.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_cvol_up")),
      XP.registerCommand(F("sim/GPS/g1000n1_cvol_dn")),
      XP.registerCommand(F("sim/audio_panel/monitor_audio_com2")));
  btnComFF.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_com_ff")));

  encRange.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_range_up")),
      XP.registerCommand(F("sim/GPS/g1000n1_range_down")));
  btnPanPush.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_pan_push")));
  btnPanUp.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_pan_up")));
  btnPanLeft.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_pan_left")));
  btnPanDown.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_pan_down")));
  btnPanRight.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_pan_right")));
  encHeading.setCommand(
      XP.registerCommand(F("sim/GPS/g1000n1_hdg_up")),
      XP.registerCommand(F("sim/GPS/g1000n1_hdg_down")),
      XP.registerCommand(F("sim/GPS/g1000n1_hdg_sync")));

  swMaster.setCommand(
      XP.registerCommand(F("sim/electrical/battery_1_on")),
      XP.registerCommand(F("sim/electrical/battery_1_off")));
  swAVMaster.setCommand(
      XP.registerCommand(F("sim/systems/avionics_on")),
      XP.registerCommand(F("sim/systems/avionics_off")));
  swPitot.setCommand(
      XP.registerCommand(F("sim/ice/pitot_heat0_on")),
      XP.registerCommand(F("sim/ice/pitot_heat0_off")));
  // swOxygen.setCommand(());
  btnDeiceMax.setCommand(
      XP.registerCommand(F("aerobask/deice/max")));
  swDeice.setCommand(
      XP.registerCommand(F("aerobask/deice/main_up")),
      XP.registerCommand(F("aerobask/deice/main_dn")));
  btnDeiceWS.setCommand(
      XP.registerCommand(F("aerobask/deice/wshld")));
  swDeiceAnnun.setCommand(
      XP.registerCommand(F("aerobask/deice/annun_up")),
      XP.registerCommand(F("aerobask/deice/annun_dn")));
  // swBackup.setCommand(());
  // MUX 5
  swAltLeft.setCommand(
      XP.registerCommand(F("sim/electrical/generator_1_on")),
      XP.registerCommand(F("sim/electrical/generator_1_off")));
  btnStartLeft.setCommand(
      XP.registerCommand(F("sim/starters/engage_starter_1")));
  btnStartRight.setCommand(
      XP.registerCommand(F("sim/starters/engage_starter_2")));
  swAltRight.setCommand(
      XP.registerCommand(F("sim/electrical/generator_2_on")),
      XP.registerCommand(F("sim/electrical/generator_2_off")));
  swPumpLeft.setCommand(
      XP.registerCommand(F("aerobask/eng/fuel_pump1_on")),
      XP.registerCommand(F("aerobask/eng/fuel_pump1_off")));
  swEngineMasterLeft.setCommand(
      XP.registerCommand(F("aerobask/eng/master1_up")),
      XP.registerCommand(F("aerobask/eng/master1_dn")));
  swEngineMasterRight.setCommand(
      XP.registerCommand(F("aerobask/eng/master2_up")),
      XP.registerCommand(F("aerobask/eng/master2_dn")));
  swPumpRight.setCommand(
      XP.registerCommand(F("aerobask/eng/fuel_pump2_on")),
      XP.registerCommand(F("aerobask/eng/fuel_pump2_off")));
  btnECUTestLeft.setCommand(
      XP.registerCommand(F("aerobask/eng/ecu_test1")));
  btnECUTestRight.setCommand(
      XP.registerCommand(F("aerobask/eng/ecu_test2")));

  int cmd = XP.registerCommand(F("aerobask/eng/ecu_ab1_auto"));
  swVoteLeftA.setCommand(
      XP.registerCommand(F("aerobask/eng/ecu_ab1_voter_a")), cmd);
  swVoteLeftB.setCommand(
      XP.registerCommand(F("aerobask/eng/ecu_ab1_voter_b")), cmd);
  cmd = XP.registerCommand(F("aerobask/eng/ecu_ab2_auto"));
  swVoteRightA.setCommand(
      XP.registerCommand(F("aerobask/eng/ecu_ab2_voter_a")), cmd);
  swVoteRightB.setCommand(
      XP.registerCommand(F("aerobask/eng/ecu_ab2_voter_b")), cmd);

  // DataRefs
  XP.registerDataRef(F("aerobask/deice/lt_norm"), XPL_READ, 100, 0, &deice_norm);
  XP.registerDataRef(F("aerobask/deice/lt_high"), XPL_READ, 100, 0, &deice_high);
  XP.registerDataRef(F("aerobask/deice/lt_max"), XPL_READ, 100, 0, &deice_max);
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
  btnPanPush.handleXP(!DigitalIn.getBit(4, 1) && !DigitalIn.getBit(4, 2) &&
                      !DigitalIn.getBit(4, 3) && !DigitalIn.getBit(4, 4));
  btnPanUp.handleXP(DigitalIn.getBit(4, 0));
  btnPanLeft.handleXP(DigitalIn.getBit(4, 0));
  btnPanDown.handleXP(DigitalIn.getBit(4, 0));
  btnPanRight.handleXP(DigitalIn.getBit(4, 0));

  swMaster.handleXP();
  swAVMaster.handleXP();
  swPitot.handleXP();
  // swOxygen.handleXP();
  btnDeiceMax.handleXP();
  swDeice.handleXP();
  btnDeiceWS.handleXP();
  swDeiceAnnun.handleXP();
  // swBackup.handleXP();
  swAltLeft.handleXP();
  btnStartLeft.handleXP();
  btnStartRight.handleXP();
  swAltRight.handleXP();
  swPumpLeft.handleXP();
  swEngineMasterLeft.handleXP();
  swEngineMasterRight.handleXP();
  swPumpRight.handleXP();
  swVoteLeftA.handleXP();
  swVoteLeftB.handleXP();
  btnECUTestLeft.handleXP();
  btnECUTestRight.handleXP();
  swVoteRightA.handleXP();
  swVoteRightB.handleXP();

  // handle pan stick with logical dependecies
  btnPanPush.handleXP(!DigitalIn.getBit(4, 1) && !DigitalIn.getBit(4, 2) &&
                      !DigitalIn.getBit(4, 3) && !DigitalIn.getBit(4, 4));
  btnPanUp.handleXP(DigitalIn.getBit(4, 0));
  btnPanLeft.handleXP(DigitalIn.getBit(4, 0));
  btnPanDown.handleXP(DigitalIn.getBit(4, 0));
  btnPanRight.handleXP(DigitalIn.getBit(4, 0));

  // Sync Switches
  if (tmrSync.isTick())
  {
    XP.commandTrigger(swMaster.getCommand());
    XP.commandTrigger(swAVMaster.getCommand());
    XP.commandTrigger(swPitot.getCommand());
    // XP.commandTrigger(swOxygen.getCommand());
    // XP.commandTrigger(swBackup.getCommand());
    XP.commandTrigger(swAltLeft.getCommand());
    XP.commandTrigger(swAltRight.getCommand());
    XP.commandTrigger(swPumpLeft.getCommand());
    XP.commandTrigger(swEngineMasterLeft.getCommand());
    XP.commandTrigger(swEngineMasterRight.getCommand());
    XP.commandTrigger(swPumpRight.getCommand());
    if (swDeice.on() || swDeice.on2())
    {
      XP.commandTrigger(swDeice.getCommand(), 2);
    }
    if (swDeiceAnnun.on() || swDeiceAnnun.on2())
    {
      XP.commandTrigger(swDeiceAnnun.getCommand(), 2);
    }
    if (swVoteLeftA.on())
    {
      XP.commandTrigger(swVoteLeftA.getCommand());
    }
    else
    {
      XP.commandTrigger(swVoteLeftB.getCommand());
    }
    if (swVoteRightA.on())
    {
      XP.commandTrigger(swVoteRightA.getCommand());
    }
    else
    {
      XP.commandTrigger(swVoteRightB.getCommand());
    }
  }

  leds.set(LED_DEICE_NORM, deice_norm ? ledOn : ledOff);
  leds.set(LED_DEICE_HIGH, deice_high ? ledOn : ledOff);
  leds.set(LED_DEICE_MAX, deice_max ? ledOn : ledOff);
}

#endif