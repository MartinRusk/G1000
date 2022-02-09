// mrusk-G1000 device handler for RealSimGear Driver
#include "Arduino.h"

// configuration
#define VERSION "1.3.3"
#define UNIT_PFD 1

// Select unit
#if UNIT_PFD
#define BOARD_ID "0001"
// autopilot layout (set both to 0 if no AP connected)
#define AP_NXI 0
#define AP_STD 0
#define LEDS_AVAILABLE 1
#define NUM_LEDS 3
#else
#define BOARD_ID "0002"
// autopilot layout (set both to 0 if no AP connected)
#define AP_NXI 1
#define AP_STD 0
#define LEDS_AVAILABLE 1
#define NUM_LEDS 7
#endif

// printout debug data
#define DEBUG 0
// reserve space for input devices
#define MAX_BUTTONS 47
#define MAX_SWITCHES 13
#define MAX_ENCODERS 15
// optional if LED outputs are connected
#define DM13A_DAI 10
#define DM13A_DCK 13
#define DM13A_LAT 12

// storage for input devices
struct button_t
{
    bool _state;
} Buttons[MAX_BUTTONS];
struct switch_t
{
    bool _state;
} Switches[MAX_SWITCHES];
struct encoder_t
{
    int8_t _count, _mark;
    uint8_t _state;
} Encoders[MAX_ENCODERS];

// storage for virtual inputs from MUX
uint8_t mux1[16];
uint8_t mux2[16];
// keep alive timer
uint32_t next = 0;
#if DEBUG
// counter to check runtime behavior
uint16_t count = 0;
#endif

// setup multiplexer pins as outputs
void setupMux()
{
#ifdef ARDUINO_AVR_NANO
    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);
    pinMode(5, OUTPUT);
#else
    pinMode(22, OUTPUT);
    pinMode(23, OUTPUT);
    pinMode(24, OUTPUT);
    pinMode(25, OUTPUT);
#endif
}

// scan all multiplexers simultaneously into virtual inputs
void handleMux()
{
    for (uint8_t channel = 0; channel < 16; channel++)
    {
#ifdef ARDUINO_AVR_NANO
        digitalWrite(4, channel & 1);
        digitalWrite(5, channel & 2);
        digitalWrite(2, channel & 4);
        digitalWrite(3, channel & 8);
        for (uint8_t i = 0; i < 100; i++)
            ;
        mux1[channel] = (~PINC & 0x3F);
        mux2[channel] = (~PINB & 0x3F);
#else
        digitalWrite(22, channel & 1);
        digitalWrite(23, channel & 2);
        digitalWrite(24, channel & 4);
        digitalWrite(25, channel & 8);
        mux1[channel] = 0;
        mux2[channel] = 0;
        // scan all physical inputs into virtual inputs
        for (uint8_t input = 0; input < 6; input++)
        {
            // invert signal here b/c switches are pull down
            // mux inputs start at PIN26
            mux1[channel] |= !digitalRead(input + 26) << input;
            mux2[channel] |= !digitalRead(input + 32) << input;
        }
#endif
    }
}

// Buttons
void initButton(button_t *but)
{
    but->_state = false;
}
void handleButton(button_t *but, const char *name, bool repeat, bool input)
{
    if (!input && but->_state)
    {
        Serial.write(name);
        Serial.write("=0\n");
    }
    if (repeat)
    {
        if (input)
        {
            Serial.write(name);
            Serial.write("=1\n");
            delay(100);
        }
    }
    else
    {
        if (input && !but->_state)
        {
            Serial.write(name);
            Serial.write("=1\n");
        }
    }
    but->_state = input;
}

// Switches
void initSwitch(switch_t *sw)
{
    sw->_state = false;
}
void handleSwitch(switch_t *sw, const char *name, bool input)
{
    if (input && !sw->_state)
    {
        Serial.write(name);
        Serial.write(".SW.ON\n");
    }    
    if (!input && sw->_state)
    {
        Serial.write(name);
        Serial.write(".SW.OFF\n");
    }
    sw->_state = input;
}

// Encoders
void initEncoder(encoder_t *enc)
{
    enc->_count = 0;
    enc->_mark = 0;
    enc->_state = 0;
}
void handleEncoder(encoder_t *enc, const char *up, const char *dn, bool input1, bool input2, uint8_t pulses)
{
    // collect new state
    enc->_state = ((enc->_state & 0x03) << 2) | input1 | (input2 << 1);
    // evaluate state change
    switch (enc->_state)
    {
    case 0:
    case 5:
    case 10:
    case 15:
        break;
    case 1:
    case 7:
    case 8:
    case 14:
        enc->_count++;
        break;
    case 2:
    case 4:
    case 11:
    case 13:
        enc->_count--;
        break;
    case 3:
    case 12:
        enc->_count += 2;
        break;
    default:
        enc->_count -= 2;
        break;
    }
    // evaluate counter with individual pulses per detent
    if ((enc->_count - enc->_mark) >= pulses)
    {
        Serial.write(up);
        Serial.write("\n");
        enc->_mark = enc->_count;
    }
    if ((enc->_count - enc->_mark) <= -pulses)
    {
        Serial.write(dn);
        Serial.write("\n");
        enc->_mark = enc->_count;
    }
}

// LEDs
#if LEDS_AVAILABLE
void writeLEDs(uint16_t leds)
{
    shiftOut(DM13A_DAI, DM13A_DCK, MSBFIRST, (leds & 0xFF00) >> 8);
    shiftOut(DM13A_DAI, DM13A_DCK, MSBFIRST, (leds & 0x00FF));
    digitalWrite(DM13A_LAT, HIGH);
    digitalWrite(DM13A_LAT, LOW);
}
void setupLEDs()
{
    pinMode(DM13A_LAT, OUTPUT);
    pinMode(DM13A_DCK, OUTPUT);
    pinMode(DM13A_DAI, OUTPUT);
    digitalWrite(DM13A_LAT, LOW);
    digitalWrite(DM13A_DAI, LOW);
    uint16_t leds = 0;
    for (uint8_t i = 0; i < NUM_LEDS; ++i)
    {
        leds = (1 << i);
        writeLEDs(leds);
        delay(100);
    }
    writeLEDs(0x0000);
}
void handleLEDs()
{
    if (Serial.available() > NUM_LEDS)
    { // full dataword available from RSG driver
        // capture full databurst and parse through it
        String leddata = Serial.readStringUntil('\n');
        uint16_t leds = 0;
        for (uint8_t i = 0; i < NUM_LEDS; ++i)
        {
            if (leddata.charAt(i + 1) == '1')
            {
                leds |= (1 << i);
            }
        }
        writeLEDs(leds);
    }
}
#endif

int8_t sw1_old = 0;
int8_t sw2_old = 0;

void handlePoti()
{
    int8_t sw1 = analogRead(6) >> 6;
    int8_t sw2 = analogRead(7) >> 6;
    if (sw1 != sw1_old)
    {
        sw1_old = sw1;
        Serial.write("SW_INSTR_");
        Serial.print(sw1);
        Serial.write("=1\n");
    }
    if (sw2 != sw2_old)
    {
        sw2_old = sw2;
        Serial.write("SW_FLOOD_");
        Serial.print(sw2);
        Serial.write("=1\n");
    }
}

// helper
bool getBit(uint8_t val, uint8_t bit)
{
    return ((val >> bit) & 1);
}

// main setup routine
void setup()
{
    // setup interface
    Serial.begin(115200);

    // setup MUX pins
    setupMux();

#if LEDS_AVAILABLE
    // setup LED driver
    setupLEDs();
#endif

    // initialize data structures for buttons and encoders
    for (uint8_t but = 0; but < MAX_BUTTONS; but++)
    {
        initButton(&Buttons[but]);
    }
    for (uint8_t sw = 0; sw < MAX_SWITCHES; sw++)
    {
        initSwitch(&Switches[sw]);
    }
    for (uint8_t enc = 0; enc < MAX_ENCODERS; enc++)
    {
        initEncoder(&Encoders[enc]);
    }
}

// Main loop routine
void loop()
{
    // scan Multiplexers into virtual inputs
    handleMux();

    // keep alive for RSG connection
    if (millis() >= next)
    { // timer interval for keepalive
        Serial.write("\\####RealSimGear#mrusk-G1000XFD1#1#");
        Serial.write(VERSION);
        Serial.write("#");
        Serial.write(BOARD_ID);
        Serial.write("\n");
#if DEBUG
        // print out number of cycles per 500ms to verify runtime
        Serial.println(count);
        count = 0;
#endif
        next = millis() + 500;
    }

#if UNIT_PFD == 0
    // handle analog inputs
    handlePoti();
#endif

#if DEBUG
    count++;
#endif

#if LEDS_AVAILABLE
    // handle incoming LED data
    handleLEDs();
#endif

    uint8_t btn = 0;
    uint8_t sw = 0;
    uint8_t enc = 0;
    // MUX 0
    handleButton(&Buttons[btn++], "BTN_NAV_TOG", false, getBit(mux1[0], 0));
    handleEncoder(&Encoders[enc++], "ENC_NAV_INNER_UP", "ENC_NAV_INNER_DN", getBit(mux1[1], 0), getBit(mux1[2], 0), 4);
    handleEncoder(&Encoders[enc++], "ENC_NAV_OUTER_UP", "ENC_NAV_OUTER_DN", getBit(mux1[4], 0), getBit(mux1[3], 0), 4);
    handleButton(&Buttons[btn++], "BTN_COM_TOG", false, mux1[5] & (1 << 0));
    handleEncoder(&Encoders[enc++], "ENC_COM_INNER_UP", "ENC_COM_INNER_DN", getBit(mux1[6], 0), getBit(mux1[7], 0), 4);
    handleEncoder(&Encoders[enc++], "ENC_COM_OUTER_UP", "ENC_COM_OUTER_DN", getBit(mux1[9], 0), getBit(mux1[8], 0), 4);
    handleButton(&Buttons[btn++], "BTN_CRS_SYNC", false, getBit(mux1[10], 0));
    handleEncoder(&Encoders[enc++], "ENC_CRS_UP", "ENC_CRS_DN", getBit(mux1[11], 0), getBit(mux1[12], 0), 4);
    handleEncoder(&Encoders[enc++], "ENC_BARO_UP", "ENC_BARO_DN", getBit(mux1[14], 0), getBit(mux1[13], 0), 4);
    // MUX 1
    handleButton(&Buttons[btn++], "BTN_ALT_SEL", false, getBit(mux1[0], 1));
    handleEncoder(&Encoders[enc++], "ENC_ALT_INNER_UP", "ENC_ALT_INNER_DN", getBit(mux1[1], 1), getBit(mux1[2], 1), 4);
    handleEncoder(&Encoders[enc++], "ENC_ALT_OUTER_UP", "ENC_ALT_OUTER_DN", getBit(mux1[4], 1), getBit(mux1[3], 1), 4);
    handleButton(&Buttons[btn++], "BTN_FMS", false, getBit(mux1[5], 1));
    handleEncoder(&Encoders[enc++], "ENC_FMS_INNER_UP", "ENC_FMS_INNER_DN", getBit(mux1[6], 1), getBit(mux1[7], 1), 4);
    handleEncoder(&Encoders[enc++], "ENC_FMS_OUTER_UP", "ENC_FMS_OUTER_DN", getBit(mux1[9], 1), getBit(mux1[8], 1), 4);
    handleButton(&Buttons[btn++], "BTN_DIRECT", false, getBit(mux1[10], 1));
    handleButton(&Buttons[btn++], "BTN_FPL", false, getBit(mux1[11], 1));
    handleButton(&Buttons[btn++], "BTN_CLR", false, getBit(mux1[12], 1));
    handleButton(&Buttons[btn++], "BTN_MENU", false, getBit(mux1[13], 1));
    handleButton(&Buttons[btn++], "BTN_PROC", false, getBit(mux1[14], 1));
    handleButton(&Buttons[btn++], "BTN_ENT", false, getBit(mux1[15], 1));
    // MUX 2
#if AP_NXI
    handleButton(&Buttons[btn++], "BTN_AP", false, getBit(mux1[0], 2));
    handleButton(&Buttons[btn++], "BTN_FD", false, getBit(mux1[1], 2));
    handleButton(&Buttons[btn++], "BTN_NAV", false, getBit(mux1[2], 2));
    handleButton(&Buttons[btn++], "BTN_ALT", false, getBit(mux1[3], 2));
    handleButton(&Buttons[btn++], "BTN_VS", false, getBit(mux1[4], 2));
    handleButton(&Buttons[btn++], "BTN_FLC", false, getBit(mux1[5], 2));
    handleButton(&Buttons[btn++], "BTN_YD", false, getBit(mux1[6], 2));
    handleButton(&Buttons[btn++], "BTN_HDG", false, getBit(mux1[7], 2));
    handleButton(&Buttons[btn++], "BTN_APR", false, getBit(mux1[8], 2));
    handleButton(&Buttons[btn++], "BTN_VNAV", false, getBit(mux1[9], 2));
    handleButton(&Buttons[btn++], "BTN_NOSE_UP", false, getBit(mux1[10], 2));
    handleButton(&Buttons[btn++], "BTN_NOSE_DN", false, getBit(mux1[11], 2));
#endif
#if AP_STD
    handleButton(&Buttons[btn++], "BTN_AP", false, getBit(mux1[0], 2));
    handleButton(&Buttons[btn++], "BTN_HDG", false, getBit(mux1[1], 2));
    handleButton(&Buttons[btn++], "BTN_NAV", false, getBit(mux1[2], 2));
    handleButton(&Buttons[btn++], "BTN_APR", false, getBit(mux1[3], 2));
    handleButton(&Buttons[btn++], "BTN_VS", false, getBit(mux1[4], 2));
    handleButton(&Buttons[btn++], "BTN_FLC", false, getBit(mux1[5], 2));
    handleButton(&Buttons[btn++], "BTN_FD", false, getBit(mux1[6], 2));
    handleButton(&Buttons[btn++], "BTN_ALT", false, getBit(mux1[7], 2));
    handleButton(&Buttons[btn++], "BTN_VNAV", false, getBit(mux1[8], 2));
    handleButton(&Buttons[btn++], "BTN_BC", false, getBit(mux1[9], 2));
    handleButton(&Buttons[btn++], "BTN_NOSE_UP", false, getBit(mux1[10], 2));
    handleButton(&Buttons[btn++], "BTN_NOSE_DN", false, getBit(mux1[11], 2));
#endif
    handleEncoder(&Encoders[enc++], "ENC_NAV_VOL_UP", "ENC_NAV_VOL_DN", getBit(mux1[12], 2), getBit(mux1[13], 2), 4);
    handleButton(&Buttons[btn++], "BTN_NAV_VOL", false, getBit(mux1[14], 2));
    handleButton(&Buttons[btn++], "BTN_NAV_FF", false, getBit(mux1[15], 2));
    // MUX 3
    handleButton(&Buttons[btn++], "BTN_SOFT_1", false, getBit(mux1[0], 3));
    handleButton(&Buttons[btn++], "BTN_SOFT_2", false, getBit(mux1[1], 3));
    handleButton(&Buttons[btn++], "BTN_SOFT_3", false, getBit(mux1[2], 3));
    handleButton(&Buttons[btn++], "BTN_SOFT_4", false, getBit(mux1[3], 3));
    handleButton(&Buttons[btn++], "BTN_SOFT_5", false, getBit(mux1[4], 3));
    handleButton(&Buttons[btn++], "BTN_SOFT_6", false, getBit(mux1[5], 3));
    handleButton(&Buttons[btn++], "BTN_SOFT_7", false, getBit(mux1[6], 3));
    handleButton(&Buttons[btn++], "BTN_SOFT_8", false, getBit(mux1[7], 3));
    handleButton(&Buttons[btn++], "BTN_SOFT_9", false, getBit(mux1[8], 3));
    handleButton(&Buttons[btn++], "BTN_SOFT_10", false, getBit(mux1[9], 3));
    handleButton(&Buttons[btn++], "BTN_SOFT_11", false, getBit(mux1[10], 3));
    handleButton(&Buttons[btn++], "BTN_SOFT_12", false, getBit(mux1[11], 3));
    handleEncoder(&Encoders[enc++], "ENC_COM_VOL_UP", "ENC_COM_VOL_DN", getBit(mux1[12], 3), getBit(mux1[13], 3), 4);
    handleButton(&Buttons[btn++], "BTN_COM_VOL", false, getBit(mux1[14], 3));
    handleButton(&Buttons[btn++], "BTN_COM_FF", false, getBit(mux1[15], 3));
    // MUX 4
    handleButton(&Buttons[btn++], "BTN_PAN_SYNC", false, getBit(mux1[0], 4));
    handleButton(&Buttons[btn++], "BTN_PAN_UP", true, getBit(mux1[1], 4));
    handleButton(&Buttons[btn++], "BTN_PAN_LEFT", true, getBit(mux1[2], 4));
    handleButton(&Buttons[btn++], "BTN_PAN_DN", true, getBit(mux1[3], 4));
    handleButton(&Buttons[btn++], "BTN_PAN_RIGHT", true, getBit(mux1[4], 4));
    handleEncoder(&Encoders[enc++], "ENC_RANGE_UP", "ENC_RANGE_DN", getBit(mux1[6], 4), getBit(mux1[5], 4), 2);
    handleEncoder(&Encoders[enc++], "ENC_HDG_UP", "ENC_HDG_DN", getBit(mux1[12], 4), getBit(mux1[13], 4), 4);
    handleButton(&Buttons[btn++], "BTN_HDG_SYNC", false, getBit(mux1[14], 4));

#if UNIT_PFD == 1
    sw++; // to avoid warning
#endif

#if UNIT_PFD == 0
    handleButton(&Buttons[btn++], "BTN_TRIM_CENTER", false, getBit(mux1[10], 4));
    handleEncoder(&Encoders[enc++], "ENC_TRIM_RIGHT", "ENC_TRIM_LEFT", getBit(mux1[8], 4), getBit(mux1[9], 4), 4);
    // MUX 5
    handleSwitch(&Switches[sw++], "SW_LIGHT_LDG", getBit(mux1[0], 5));
    handleSwitch(&Switches[sw++], "SW_LIGHT_TAXI", getBit(mux1[1], 5));
    handleSwitch(&Switches[sw++], "SW_LIGHT_POS", getBit(mux1[2], 5));
    handleSwitch(&Switches[sw++], "SW_LIGHT_STRB", getBit(mux1[3], 5));
    handleSwitch(&Switches[sw++], "SW_FUEL_L_DN", getBit(mux1[4], 5));
    handleSwitch(&Switches[sw++], "SW_FUEL_L_UP", getBit(mux1[5], 5));
    handleSwitch(&Switches[sw++], "SW_FUEL_R_DN", getBit(mux1[6], 5));
    handleSwitch(&Switches[sw++], "SW_FUEL_R_UP", getBit(mux1[7], 5));
    handleSwitch(&Switches[sw++], "SW_FUEL_AUX_L", getBit(mux1[8], 5));
    handleSwitch(&Switches[sw++], "SW_FUEL_AUX_R", getBit(mux1[9], 5));
    handleButton(&Buttons[btn++], "BUT_GEAR_TEST", false, getBit(mux1[10], 5));
    handleSwitch(&Switches[sw++], "SW_GEAR", getBit(mux1[11], 5));
    handleSwitch(&Switches[sw++], "SW_FLAP_DN", getBit(mux1[12], 5));
    handleSwitch(&Switches[sw++], "SW_FLAP_UP", getBit(mux1[13], 5));
#endif

#if DEBUG
    // halt program in case of error since memory is corrupted
    if (btn > MAX_BUTTONS)
    {
        Serial.println("ERROR: Too many buttons used");
        while (true)
            ;
    }
    if (sw > MAX_SWITCHES)
    {
        Serial.println("ERROR: Too many Switches used");
        while (true)
            ;
    }
    if (enc > MAX_ENCODERS)
    {
        Serial.println("ERROR: Too many encoders used");
        while (true)
            ;
    }
#endif
}
