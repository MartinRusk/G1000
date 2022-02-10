// mrusk-G1000 device handler for RealSimGear Driver
#include "Arduino.h"

// configuration
#define VERSION "1.3.5"
#define UNIT_PFD 1
// printout debug data
#define DEBUG 0

// Select unit
#if UNIT_PFD
#define BOARD_ID "0001"
// autopilot layout (set both to 0 if no AP connected)
#define AP_NXI 0
#define AP_STD 0
#define NUM_LEDS 3
#else
#define BOARD_ID "0002"
// autopilot layout (set both to 0 if no AP connected)
#define AP_NXI 1
#define AP_STD 0
#define NUM_LEDS 7
#endif

// reserve space for input devices
#define MAX_BUTTONS 50
#define MAX_SWITCHES 20
#define MAX_ENCODERS 15
#define MAX_POTIS 2
// pins for LED driver
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
    uint8_t _state;
    int8_t _count, _mark;
} Encoders[MAX_ENCODERS];

struct poti_t
{
    uint8_t _state;
} Potis[MAX_POTIS];

// storage for virtual inputs from MUX
uint8_t mux1[16];
uint8_t mux2[16];
// keep alive timer
uint32_t next = 0;
#if DEBUG
// counter to check runtime behavior
uint16_t count = 0;
bool init_mark = false;
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
    for (uint8_t pin = 0; pin < 16; pin++)
    {
#ifdef ARDUINO_AVR_NANO
        // remap pins 0+1 to 4+5 due to bug on PCB
        PORTD = (PIND & 0xc3) | (pin & 0x0c) | ((pin & 0x3) << 4);
        // delay to settle mux and avoid bouncing
        delayMicroseconds(10);
        mux1[pin] = (~PINC & 0x3F);
        mux2[pin] = (~PINB & 0x3F);
#else
        // TODO: Rewrite write and read for direct port manipulation
        // PORTA = (PINA & 0xF0) | pin; // TODO: verify port
        digitalWrite(22, pin & 1);
        digitalWrite(23, pin & 2);
        digitalWrite(24, pin & 4);
        digitalWrite(25, pin & 8);
        mux1[pin] = 0;
        mux2[pin] = 0;
        // scan all physical inputs into virtual inputs
        for (uint8_t input = 0; input < 6; input++)
        {
            // invert signal here b/c switches are pull down
            // mux inputs start at PIN26
            // __BUILTIN_AVR_INSERT_BITS (0x01234567, bits, 0); // flip bit order
            mux1[pin] |= !digitalRead(input + 26) << input;
            mux2[pin] |= !digitalRead(input + 32) << input;
        }
#endif
    }
}
// get state of a specific input pin
bool getMux(uint8_t *mux, uint8_t channel, uint8_t pin)
{
    // channel: mux module (0-5)
    // pin: pin of the module (0-15)
    return ((mux[pin] >> channel) & 1);
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
void initSwitch(switch_t *swi)
{
    swi->_state = false;
}
void handleSwitch(switch_t *swi, const char *name, bool input)
{
    if (input && !swi->_state)
    {
        Serial.write(name);
        Serial.write(".SW.ON\n");
    }    
    if (!input && swi->_state)
    {
        Serial.write(name);
        Serial.write(".SW.OFF\n");
    }
    swi->_state = input;
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
#if NUM_LEDS > 0
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
    // init loop thru all LEDs
    for (uint8_t i = 0; i < NUM_LEDS; ++i)
    {
        writeLEDs(1 << i);
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

// analog inputs with 16 steps
void initPoti(poti_t *pot)
{
    pot->_state = 0;
}
void handlePoti(poti_t *pot, const char *name, int16_t input)
{
    int8_t swi = input >> 6;
    if (swi != pot->_state)
    {
        pot->_state = swi;
        Serial.write(name);
        Serial.write("_");
        Serial.print(pot->_state);
        Serial.write("=1\n");
    }
}

// main setup routine
void setup()
{
    // setup interface
    Serial.begin(115200);

    // setup MUX pins
    setupMux();

#if NUM_LEDS > 0
    // setup LED driver
    setupLEDs();
#endif

    // initialize data structures for input devices
    for (uint8_t but = 0; but < MAX_BUTTONS; but++)
    {
        initButton(&Buttons[but]);
    }
    for (uint8_t swi = 0; swi < MAX_SWITCHES; swi++)
    {
        initSwitch(&Switches[swi]);
    }
    for (uint8_t enc = 0; enc < MAX_ENCODERS; enc++)
    {
        initEncoder(&Encoders[enc]);
    }
    for (uint8_t pot = 0; pot < MAX_POTIS; pot++)
    {
        initPoti(&Potis[pot]);
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
        Serial.write("Loop count: ");
        Serial.println(count);
        count = 0;
#endif
        next = millis() + 500;
    }

#if DEBUG
    count++;
#endif

#if NUM_LEDS > 0
    // handle incoming LED data
    handleLEDs();
#endif

    // avoid compiler warning if a device type is not used
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
    uint8_t btn = 0;
    uint8_t swi = 0;
    uint8_t enc = 0;
    uint8_t pot = 0;
#pragma GCC diagnostic pop

    // MUX 0
    handleButton(&Buttons[btn++], "BTN_NAV_TOG", false, getMux(mux1, 0, 0));
    handleEncoder(&Encoders[enc++], "ENC_NAV_INNER_UP", "ENC_NAV_INNER_DN", getMux(mux1, 0, 1), getMux(mux1, 0, 2), 4);
    handleEncoder(&Encoders[enc++], "ENC_NAV_OUTER_UP", "ENC_NAV_OUTER_DN", getMux(mux1, 0, 4), getMux(mux1, 0, 3), 4);
    handleButton(&Buttons[btn++], "BTN_COM_TOG", false, getMux(mux1, 0, 5));
    handleEncoder(&Encoders[enc++], "ENC_COM_INNER_UP", "ENC_COM_INNER_DN", getMux(mux1, 0, 6), getMux(mux1, 0, 7), 4);
    handleEncoder(&Encoders[enc++], "ENC_COM_OUTER_UP", "ENC_COM_OUTER_DN", getMux(mux1, 0, 9), getMux(mux1, 0, 8), 4);
    handleButton(&Buttons[btn++], "BTN_CRS_SYNC", false, getMux(mux1, 0, 10));
    handleEncoder(&Encoders[enc++], "ENC_CRS_UP", "ENC_CRS_DN", getMux(mux1, 0, 11), getMux(mux1, 0, 12), 4);
    handleEncoder(&Encoders[enc++], "ENC_BARO_UP", "ENC_BARO_DN", getMux(mux1, 0, 14), getMux(mux1, 0, 13), 4);
    // MUX 1
    handleButton(&Buttons[btn++], "BTN_ALT_SEL", false, getMux(mux1, 1, 0));
    handleEncoder(&Encoders[enc++], "ENC_ALT_INNER_UP", "ENC_ALT_INNER_DN", getMux(mux1, 1, 1), getMux(mux1, 1, 2), 4);
    handleEncoder(&Encoders[enc++], "ENC_ALT_OUTER_UP", "ENC_ALT_OUTER_DN", getMux(mux1, 1, 4), getMux(mux1, 1, 3), 4);
    handleButton(&Buttons[btn++], "BTN_FMS", false, getMux(mux1, 1, 5));
    handleEncoder(&Encoders[enc++], "ENC_FMS_INNER_UP", "ENC_FMS_INNER_DN", getMux(mux1, 1, 6), getMux(mux1, 1, 7), 4);
    handleEncoder(&Encoders[enc++], "ENC_FMS_OUTER_UP", "ENC_FMS_OUTER_DN", getMux(mux1, 1, 9), getMux(mux1, 1, 8), 4);
    handleButton(&Buttons[btn++], "BTN_DIRECT", false, getMux(mux1, 1, 10));
    handleButton(&Buttons[btn++], "BTN_FPL", false, getMux(mux1, 1, 11));
    handleButton(&Buttons[btn++], "BTN_CLR", false, getMux(mux1, 1, 12));
    handleButton(&Buttons[btn++], "BTN_MENU", false, getMux(mux1, 1, 13));
    handleButton(&Buttons[btn++], "BTN_PROC", false, getMux(mux1, 1, 14));
    handleButton(&Buttons[btn++], "BTN_ENT", false, getMux(mux1, 1, 15));
    // MUX 2
#if AP_NXI
    handleButton(&Buttons[btn++], "BTN_AP", false, getMux(mux1, 2, 0));
    handleButton(&Buttons[btn++], "BTN_FD", false, getMux(mux1, 2, 1));
    handleButton(&Buttons[btn++], "BTN_NAV", false, getMux(mux1, 2, 2));
    handleButton(&Buttons[btn++], "BTN_ALT", false, getMux(mux1, 2, 3));
    handleButton(&Buttons[btn++], "BTN_VS", false, getMux(mux1, 2, 4));
    handleButton(&Buttons[btn++], "BTN_FLC", false, getMux(mux1, 2, 5));
    handleButton(&Buttons[btn++], "BTN_YD", false, getMux(mux1, 2, 6));
    handleButton(&Buttons[btn++], "BTN_HDG", false, getMux(mux1, 2, 7));
    handleButton(&Buttons[btn++], "BTN_APR", false, getMux(mux1, 2, 8));
    handleButton(&Buttons[btn++], "BTN_VNAV", false, getMux(mux1, 2, 9));
    handleButton(&Buttons[btn++], "BTN_NOSE_UP", false, getMux(mux1, 2, 10));
    handleButton(&Buttons[btn++], "BTN_NOSE_DN", false, getMux(mux1, 2, 11));
#endif
#if AP_STD
    handleButton(&Buttons[btn++], "BTN_AP", false, getMux(mux1, 2, 0));
    handleButton(&Buttons[btn++], "BTN_HDG", false, getMux(mux1, 2, 1));
    handleButton(&Buttons[btn++], "BTN_NAV", false, getMux(mux1, 2, 2));
    handleButton(&Buttons[btn++], "BTN_APR", false, getMux(mux1, 2, 3));
    handleButton(&Buttons[btn++], "BTN_VS", false, getMux(mux1, 2, 4));
    handleButton(&Buttons[btn++], "BTN_FLC", false, getMux(mux1, 2, 5));
    handleButton(&Buttons[btn++], "BTN_FD", false, getMux(mux1, 2, 6));
    handleButton(&Buttons[btn++], "BTN_ALT", false, getMux(mux1, 2, 7));
    handleButton(&Buttons[btn++], "BTN_VNAV", false, getMux(mux1, 2, 8));
    handleButton(&Buttons[btn++], "BTN_BC", false, getMux(mux1, 2, 9));
    handleButton(&Buttons[btn++], "BTN_NOSE_UP", false, getMux(mux1, 2, 10));
    handleButton(&Buttons[btn++], "BTN_NOSE_DN", false, getMux(mux1, 2, 11));
#endif
    handleEncoder(&Encoders[enc++], "ENC_NAV_VOL_UP", "ENC_NAV_VOL_DN", getMux(mux1, 2, 12), getMux(mux1, 2, 13), 4);
    handleButton(&Buttons[btn++], "BTN_NAV_VOL", false, getMux(mux1, 2, 14));
    handleButton(&Buttons[btn++], "BTN_NAV_FF", false, getMux(mux1, 2, 15));
    // MUX 3
    handleButton(&Buttons[btn++], "BTN_SOFT_1", false, getMux(mux1, 3, 0));
    handleButton(&Buttons[btn++], "BTN_SOFT_2", false, getMux(mux1, 3, 1));
    handleButton(&Buttons[btn++], "BTN_SOFT_3", false, getMux(mux1, 3, 2));
    handleButton(&Buttons[btn++], "BTN_SOFT_4", false, getMux(mux1, 3, 3));
    handleButton(&Buttons[btn++], "BTN_SOFT_5", false, getMux(mux1, 3, 4));
    handleButton(&Buttons[btn++], "BTN_SOFT_6", false, getMux(mux1, 3, 5));
    handleButton(&Buttons[btn++], "BTN_SOFT_7", false, getMux(mux1, 3, 6));
    handleButton(&Buttons[btn++], "BTN_SOFT_8", false, getMux(mux1, 3, 7));
    handleButton(&Buttons[btn++], "BTN_SOFT_9", false, getMux(mux1, 3, 8));
    handleButton(&Buttons[btn++], "BTN_SOFT_10", false, getMux(mux1, 3, 9));
    handleButton(&Buttons[btn++], "BTN_SOFT_11", false, getMux(mux1, 3, 10));
    handleButton(&Buttons[btn++], "BTN_SOFT_12", false, getMux(mux1, 3, 11));
    handleEncoder(&Encoders[enc++], "ENC_COM_VOL_UP", "ENC_COM_VOL_DN", getMux(mux1, 3, 12), getMux(mux1, 3, 13), 4);
    handleButton(&Buttons[btn++], "BTN_COM_VOL", false, getMux(mux1, 3, 14));
    handleButton(&Buttons[btn++], "BTN_COM_FF", false, getMux(mux1, 3, 15));
    // MUX 4
    handleButton(&Buttons[btn++], "BTN_PAN_SYNC", false, getMux(mux1, 4, 0));
    handleButton(&Buttons[btn++], "BTN_PAN_UP", true, getMux(mux1, 4, 1));
    handleButton(&Buttons[btn++], "BTN_PAN_LEFT", true, getMux(mux1, 4, 2));
    handleButton(&Buttons[btn++], "BTN_PAN_DN", true, getMux(mux1, 4, 3));
    handleButton(&Buttons[btn++], "BTN_PAN_RIGHT", true, getMux(mux1, 4, 4));
    handleEncoder(&Encoders[enc++], "ENC_RANGE_UP", "ENC_RANGE_DN", getMux(mux1, 4, 6), getMux(mux1, 4, 5), 2);
    handleEncoder(&Encoders[enc++], "ENC_HDG_UP", "ENC_HDG_DN", getMux(mux1, 4, 12), getMux(mux1, 4, 13), 4);
    handleButton(&Buttons[btn++], "BTN_HDG_SYNC", false, getMux(mux1, 4, 14));

#if UNIT_PFD == 1
    // handleSwitch(&Switches[swi++], "SW_MASTER", getMux(mux1, 5, 0));
    // handleSwitch(&Switches[swi++], "SW_AV_MASTER", getMux(mux1, 5, 1));
    // handleSwitch(&Switches[swi++], "SW_PITOT", getMux(mux1, 5, 2));
    // handleSwitch(&Switches[swi++], "SW_BRAKE", getMux(mux1, 5, 3));
#endif

#if UNIT_PFD == 0
    handleButton(&Buttons[btn++], "BTN_TRIM_CENTER", false, getMux(mux1, 4, 10));
    handleEncoder(&Encoders[enc++], "ENC_TRIM_RIGHT", "ENC_TRIM_LEFT", getMux(mux1, 4, 8), getMux(mux1, 4, 9), 4);
    // MUX 5
    handleSwitch(&Switches[swi++], "SW_LIGHT_LDG", getMux(mux1, 5, 0));
    handleSwitch(&Switches[swi++], "SW_LIGHT_TAXI", getMux(mux1, 5, 1));
    handleSwitch(&Switches[swi++], "SW_LIGHT_POS", getMux(mux1, 5, 2));
    handleSwitch(&Switches[swi++], "SW_LIGHT_STRB", getMux(mux1, 5, 3));
    handleSwitch(&Switches[swi++], "SW_FUEL_L_DN", getMux(mux1, 5, 4));
    handleSwitch(&Switches[swi++], "SW_FUEL_L_UP", getMux(mux1, 5, 5));
    handleSwitch(&Switches[swi++], "SW_FUEL_R_DN", getMux(mux1, 5, 6));
    handleSwitch(&Switches[swi++], "SW_FUEL_R_UP", getMux(mux1, 5, 7));
    handleSwitch(&Switches[swi++], "SW_FUEL_AUX_L", getMux(mux1, 5, 8));
    handleSwitch(&Switches[swi++], "SW_FUEL_AUX_R", getMux(mux1, 5, 9));
    handleButton(&Buttons[btn++], "BUT_GEAR_TEST", false, getMux(mux1, 5, 10));
    handleSwitch(&Switches[swi++], "SW_GEAR", getMux(mux1, 5, 11));
    handleSwitch(&Switches[swi++], "SW_FLAP_DN", getMux(mux1, 5, 12));
    handleSwitch(&Switches[swi++], "SW_FLAP_UP", getMux(mux1, 5, 13));
    // analog inputs
    handlePoti(&Potis[pot++], "SW_INSTR", analogRead(6));
    handlePoti(&Potis[pot++], "SW_FLOOD", analogRead(7));

#endif

#if DEBUG
    // show number of used input devices
    if (!init_mark)
    {
        init_mark = true;
        Serial.print("Buttons:  ");
        Serial.println(btn);
        Serial.print("Switches: ");
        Serial.println(swi);
        Serial.print("Encoders: ");
        Serial.println(enc);
        Serial.print("Potis:    ");
        Serial.println(pot);
    }
    // halt program in case of error since memory is corrupted
    if (btn > MAX_BUTTONS)
    {
        Serial.println("ERROR: Too many buttons used");
        while (true)
            ;
    }
    if (swi > MAX_SWITCHES)
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
    if (pot > MAX_POTIS)
    {
        Serial.println("ERROR: Too many potis used");
        while (true)
            ;
    }
#endif
}
