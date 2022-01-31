// G1000 handler
// mrusk
#include "Arduino.h"

// configuration
#define VERSION "1.2.0"
#define BOARD_ID "0001"
#define AP_NXI 1
#define AP_STD 0
#define DEBUG 0
#define MFD_AVAILABLE 0
#define MAX_BUTTONS 45
#define MAX_ENCODERS 14
#define PULSES_PER_DETENT 4

// storage for virtual inputs from MUX
uint8_t mux1[16];
uint8_t mux2[16];

// https://arduino-projekte.webnode.at/registerprogrammierung/ein-ausgangsports/

// setup multiplexer pins as outputs
void muxSetup()
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
void muxScan()
{
    for (uint8_t channel = 0; channel < 16; channel++)
    {
#ifdef ARDUINO_AVR_NANO
        digitalWrite(4, channel & 1);
        digitalWrite(5, channel & 2);
        digitalWrite(2, channel & 4);
        digitalWrite(3, channel & 8);
        // scan all physical inputs into virtual inputs
        for (uint8_t input = 0; input < 6; input++)
        {
            // invert signal here b/c switches are pull down
            // mux inputs start at PIN8
            mux1[channel] |= !digitalRead(input + 8) << input;
            mux2[channel] |= !digitalRead(input + 14) << input;
        }
        // delay
        for (uint8_t i=0; i<100; i++);
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

struct Button
{
    bool _state;
};
void initButton(Button *but)
{
    but->_state = false;
}
void handleButton(Button *but, const char *name, bool repeat, bool input)
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

struct Encoder
{
    int8_t _count, _mark;
    uint8_t _state;
};
void initEncoder(Encoder *enc) {
    enc->_count = 0;
    enc->_mark = 0;
    enc->_state = 0;
}
void handleEncoder(Encoder *enc, const char* up, const char* dn, bool input1, bool input2) {
    // collect new state
    enc->_state = ((enc->_state & 0x03) << 2) | input1 | (input2<<1);
    // evaluate state change
	switch (enc->_state) {
		case 0: case 5: case 10: case 15:
			break;
		case 1: case 7: case 8: case 14:
			enc->_count++; break;
		case 2: case 4: case 11: case 13:
			enc->_count--; break;
		case 3: case 12:
			enc->_count += 2; break;
		default:
			enc->_count -= 2; break;
	}
    // evaluate counter
    if ((enc->_count - enc->_mark) >= PULSES_PER_DETENT) {
        Serial.write(up);
        Serial.write("\n");
        enc->_mark = enc->_count;
    }
    if ((enc->_count - enc->_mark) <= -PULSES_PER_DETENT) {
        Serial.write(dn);
        Serial.write("\n");
        enc->_mark = enc->_count;
    }
}

bool GetBit(uint8_t val, uint8_t bit) 
{
    return((val >> bit) & 1);
}

static Button Buttons[MAX_BUTTONS];
static Encoder Encoders[MAX_ENCODERS];

void setup() {
    // setup interface
    Serial.begin(115200);
    // setup MUX pins
    muxSetup();
    // initialize data structures for buttons and encoders
    for (uint8_t but = 0; but < MAX_BUTTONS; but++){
        initButton(&Buttons[but]);
    }
    for (uint8_t enc = 0; enc < MAX_ENCODERS; enc++){
        initEncoder(&Encoders[enc]);
    }
}

#if DEBUG
// counter to check runtime behavior
uint16_t count = 0;
#endif
// keep alive timer
uint32_t next = 0;

void loop() {  
    // keep alive for RSG connection
    if(millis() >= next) { //timer interval for keepalive       
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
        next = millis() + 1000;
    }

#if DEBUG
    count++;
#endif

    // scan Multiplexers into virtual inputs
    muxScan();

    uint8_t btn = 0;
    uint8_t enc = 0;
    // MUX 0
    handleButton(&Buttons[btn++],"BTN_NAV_TOG", false, GetBit(mux1[0], 0));
    handleEncoder(&Encoders[enc++],"ENC_NAV_INNER_UP", "ENC_NAV_INNER_DN", GetBit(mux1[1], 0), GetBit(mux1[2], 0));
    handleEncoder(&Encoders[enc++],"ENC_NAV_OUTER_UP", "ENC_NAV_OUTER_DN", GetBit(mux1[4], 0), GetBit(mux1[3], 0));
    handleButton(&Buttons[btn++],"BTN_COM_TOG", false, mux1[5] & (1 << 0));
    handleEncoder(&Encoders[enc++],"ENC_COM_INNER_UP", "ENC_COM_INNER_DN", GetBit(mux1[6], 0), GetBit(mux1[7], 0));
    handleEncoder(&Encoders[enc++],"ENC_COM_OUTER_UP", "ENC_COM_OUTER_DN", GetBit(mux1[9], 0), GetBit(mux1[8], 0));
    handleButton(&Buttons[btn++],"BTN_CRS_SYNC", false, GetBit(mux1[10], 0));
    handleEncoder(&Encoders[enc++],"ENC_CRS_UP", "ENC_CRS_DN", GetBit(mux1[11], 0), GetBit(mux1[12], 0));
    handleEncoder(&Encoders[enc++],"ENC_BARO_UP", "ENC_BARO_DN", GetBit(mux1[14], 0), GetBit(mux1[13], 0));
    // MUX 1
    handleButton(&Buttons[btn++],"BTN_ALT_SEL", false, GetBit(mux1[0], 1));
    handleEncoder(&Encoders[enc++],"ENC_ALT_INNER_UP", "ENC_ALT_INNER_DN", GetBit(mux1[1], 1), GetBit(mux1[2], 1));
    handleEncoder(&Encoders[enc++],"ENC_ALT_OUTER_UP", "ENC_ALT_OUTER_DN", GetBit(mux1[4], 1), GetBit(mux1[3], 1));
    handleButton(&Buttons[btn++],"BTN_FMS", false, GetBit(mux1[5], 1));
    handleEncoder(&Encoders[enc++],"ENC_FMS_INNER_UP", "ENC_FMS_INNER_DN", GetBit(mux1[6], 1), GetBit(mux1[7], 1));
    handleEncoder(&Encoders[enc++],"ENC_FMS_OUTER_UP", "ENC_FMS_OUTER_DN", GetBit(mux1[9], 1), GetBit(mux1[8], 1));
    handleButton(&Buttons[btn++],"BTN_DIRECT", false, GetBit(mux1[10], 1));
    handleButton(&Buttons[btn++],"BTN_FPL", false, GetBit(mux1[11], 1));
    handleButton(&Buttons[btn++],"BTN_CLR", false, GetBit(mux1[12], 1));
    handleButton(&Buttons[btn++],"BTN_MENU", false, GetBit(mux1[13], 1));
    handleButton(&Buttons[btn++],"BTN_PROC", false, GetBit(mux1[14], 1));
    handleButton(&Buttons[btn++],"BTN_ENT", false, GetBit(mux1[15], 1));
    // MUX 2
#if AP_NXI
    handleButton(&Buttons[btn++],"BTN_AP", false, GetBit(mux1[0], 2));
    handleButton(&Buttons[btn++],"BTN_FD", false, GetBit(mux1[1], 2));
    handleButton(&Buttons[btn++],"BTN_NAV", false, GetBit(mux1[2], 2));
    handleButton(&Buttons[btn++],"BTN_ALT", false, GetBit(mux1[3], 2));
    handleButton(&Buttons[btn++],"BTN_VS", false, GetBit(mux1[4], 2));
    handleButton(&Buttons[btn++],"BTN_FLC", false, GetBit(mux1[5], 2));
    handleButton(&Buttons[btn++],"BTN_YD", false, GetBit(mux1[6], 2));
    handleButton(&Buttons[btn++],"BTN_HDG", false, GetBit(mux1[7], 2));
    handleButton(&Buttons[btn++],"BTN_APR", false, GetBit(mux1[8], 2));
    handleButton(&Buttons[btn++],"BTN_VNAV", false, GetBit(mux1[9], 2));
    handleButton(&Buttons[btn++],"BTN_NOSE_UP", false, GetBit(mux1[10], 2));
    handleButton(&Buttons[btn++],"BTN_NOSE_DN", false, GetBit(mux1[11], 2));
#endif
#if AP_STD
    handleButton(&Buttons[btn++],"BTN_AP", false, GetBit(mux1[0], 2));
    handleButton(&Buttons[btn++],"BTN_HDG", false, GetBit(mux1[1], 2));
    handleButton(&Buttons[btn++],"BTN_NAV", false, GetBit(mux1[2], 2));
    handleButton(&Buttons[btn++],"BTN_APR", false, GetBit(mux1[3], 2));
    handleButton(&Buttons[btn++],"BTN_VS", false, GetBit(mux1[4], 2));
    handleButton(&Buttons[btn++],"BTN_FLC", false, GetBit(mux1[5], 2));
    handleButton(&Buttons[btn++],"BTN_FD", false, GetBit(mux1[6], 2));
    handleButton(&Buttons[btn++],"BTN_ALT", false, GetBit(mux1[7], 2));
    handleButton(&Buttons[btn++],"BTN_VNAV", false, GetBit(mux1[8], 2));
    handleButton(&Buttons[btn++],"BTN_BC", false, GetBit(mux1[9], 2));
    handleButton(&Buttons[btn++],"BTN_NOSE_UP", false, GetBit(mux1[10], 2));
    handleButton(&Buttons[btn++],"BTN_NOSE_DN", false, GetBit(mux1[11], 2));
#endif    
    handleEncoder(&Encoders[enc++],"ENC_NAV_VOL_UP", "ENC_NAV_VOL_DN", GetBit(mux1[12], 2), GetBit(mux1[13], 2));
    handleButton(&Buttons[btn++],"BTN_NAV_VOL", false, GetBit(mux1[14], 2));
    handleButton(&Buttons[btn++],"BTN_NAV_FF", false, GetBit(mux1[15], 2));
    // MUX 3
    handleButton(&Buttons[btn++],"BTN_SOFT_1", false, GetBit(mux1[0], 3));
    handleButton(&Buttons[btn++],"BTN_SOFT_2", false, GetBit(mux1[1], 3));
    handleButton(&Buttons[btn++],"BTN_SOFT_3", false, GetBit(mux1[2], 3));
    handleButton(&Buttons[btn++],"BTN_SOFT_4", false, GetBit(mux1[3], 3));
    handleButton(&Buttons[btn++],"BTN_SOFT_5", false, GetBit(mux1[4], 3));
    handleButton(&Buttons[btn++],"BTN_SOFT_6", false, GetBit(mux1[5], 3));
    handleButton(&Buttons[btn++],"BTN_SOFT_7", false, GetBit(mux1[6], 3));
    handleButton(&Buttons[btn++],"BTN_SOFT_8", false, GetBit(mux1[7], 3));
    handleButton(&Buttons[btn++],"BTN_SOFT_9", false, GetBit(mux1[8], 3));
    handleButton(&Buttons[btn++],"BTN_SOFT_10", false, GetBit(mux1[9], 3));
    handleButton(&Buttons[btn++],"BTN_SOFT_11", false, GetBit(mux1[10], 3));
    handleButton(&Buttons[btn++],"BTN_SOFT_12", false, GetBit(mux1[11], 3));
    handleEncoder(&Encoders[enc++],"ENC_COM_VOL_UP", "ENC_COM_VOL_DN", GetBit(mux1[12], 3), GetBit(mux1[13], 3));
    handleButton(&Buttons[btn++],"BTN_COM_VOL", false, GetBit(mux1[14], 3));
    handleButton(&Buttons[btn++],"BTN_COM_FF", false, GetBit(mux1[15], 3));
    // MUX 4
    handleButton(&Buttons[btn++],"BTN_PAN_SYNC", false, GetBit(mux1[0], 4));
    handleButton(&Buttons[btn++],"BTN_PAN_UP", true, GetBit(mux1[1], 4));
    handleButton(&Buttons[btn++],"BTN_PAN_LEFT", true, GetBit(mux1[2], 4));
    handleButton(&Buttons[btn++],"BTN_PAN_DN", true, GetBit(mux1[3], 4));
    handleButton(&Buttons[btn++],"BTN_PAN_RIGHT", true, GetBit(mux1[4], 4));
    handleEncoder(&Encoders[enc++],"ENC_RANGE_UP", "ENC_RANGE_DN", GetBit(mux1[6], 4), GetBit(mux1[5], 4));
    handleEncoder(&Encoders[enc++],"ENC_HDG_UP", "ENC_HDG_DN", GetBit(mux1[12], 4), GetBit(mux1[13], 4));
    handleButton(&Buttons[btn++],"BTN_HDG_SYNC", false, GetBit(mux1[14], 4));

#if DEBUG
    // halt program in case of error since memory is corrupted
    if (btn > MAX_BUTTONS){
        Serial.println("ERROR: Too many buttons used");
        while (true);
    }
    if (enc > MAX_ENCODERS){
        Serial.println("ERROR: Too many encoders used");
        while (true);
    }
#endif
}
