// mrusk-G1000 device handler for RealSimGear Driver
#include "Arduino.h"

// configuration
#define VERSION "3.1.1"
#define XFD_UNIT 11

// printout debug data
#define DEBUG 0

// supported board types
#define BOARD_NANO 0
#define BOARD_MICRO 1
#define BOARD_MEGA 2

// Select unit
#if XFD_UNIT == 1
// mrusk PFD
#define BOARD_ID "0001"
#define BOARD_TYPE BOARD_NANO
#define AP_NXI 0
#define AP_STD 0
#define MFD_PANEL 0
#define LEFT_PANEL 1
#define RIGHT_PANEL 0
#define MAX_BUTTONS 50
#define MAX_SWITCHES 20
#define MAX_ENCODERS 15
#define MAX_POTIS 0
#define NUM_LEDS 3
#define DM13A_DAI 8
#define DM13A_DCK 10
#define DM13A_LAT 12
#elif XFD_UNIT == 2
// mrusk MFD
#define BOARD_ID "0002"
#define BOARD_TYPE BOARD_NANO
#define AP_NXI 1
#define AP_STD 0
#define MFD_PANEL 0
#define LEFT_PANEL 0
#define RIGHT_PANEL 1
#define MAX_BUTTONS 47
#define MAX_SWITCHES 13
#define MAX_ENCODERS 15
#define MAX_POTIS 2
#define ANALOG_BASE 6
#define NUM_LEDS 7
#define DM13A_DAI 10
#define DM13A_DCK 13
#define DM13A_LAT 12
#elif XFD_UNIT == 3
// mrusk PFD #2
#define BOARD_ID "0003"
#define BOARD_TYPE BOARD_MEGA
#define AP_NXI 0
#define AP_STD 1
#define MFD_PANEL 0
#define LEFT_PANEL 0
#define RIGHT_PANEL 0
#define MAX_BUTTONS 100
#define MAX_SWITCHES 40
#define MAX_ENCODERS 30
#define MAX_POTIS 0
#define NUM_LEDS 0
#define DM13A_DAI 10
#define DM13A_DCK 13
#define DM13A_LAT 12
#elif XFD_UNIT == 4
// fzahn PFD+MFD unit
#define BOARD_ID "0004"
#define BOARD_TYPE BOARD_MEGA
#define AP_NXI 0
#define AP_STD 1
#define MFD_PANEL 1
#define LEFT_PANEL 0
#define RIGHT_PANEL 0
#define MAX_BUTTONS 100
#define MAX_SWITCHES 30
#define MAX_ENCODERS 30
#define MAX_POTIS 0
#define NUM_LEDS 0
#define DM13A_DAI 8
#define DM13A_DCK 10
#define DM13A_LAT 12
#elif XFD_UNIT == 11
// mrusk PFD micro
#define BOARD_ID "0011"
#define BOARD_TYPE BOARD_MICRO
#define AP_NXI 0
#define AP_STD 0
#define MFD_PANEL 0
#define LEFT_PANEL 1
#define RIGHT_PANEL 0
#define MAX_BUTTONS 50
#define MAX_SWITCHES 20
#define MAX_ENCODERS 15
#define MAX_POTIS 0
#define NUM_LEDS 3
#define DM13A_DAI 16
#define DM13A_DCK 14
#define DM13A_LAT 15
#elif XFD_UNIT == 12
// mrusk MFD micro
#define BOARD_ID "0012"
#define BOARD_TYPE BOARD_MICRO
#define AP_NXI 1
#define AP_STD 0
#define MFD_PANEL 0
#define LEFT_PANEL 0
#define RIGHT_PANEL 1
#define MAX_BUTTONS 47
#define MAX_SWITCHES 13
#define MAX_ENCODERS 15
#define MAX_POTIS 2
#define ANALOG_BASE 0
#define NUM_LEDS 7
#define DM13A_DAI 16
#define DM13A_DCK 14
#define DM13A_LAT 15
#else
#endif

// delay for keep alive message [ms]
#define KEEPALIVE_DELAY 500
// delay for key repeat [ms]
#define REPEAT_DELAY 250
// delay for key debouncing [cycles]
#define DEBOUNCE_DELAY 50
// delay for MUX settling [us]
#define MUX_DELAY 10

// storage for input devices
struct button_t
{
    uint8_t _state;
} Buttons[MAX_BUTTONS];

struct switch_t
{
    uint8_t _state;
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

enum repeat_t
{
    single,
    repeat
};

// storage for virtual inputs from MUX
// packed format to save memory space
uint16_t Mux[16];
// keep alive timer
uint32_t tmr_next = 0;
// repeat timer
uint32_t tmr_rep = 0;

#if DEBUG
// counter to check runtime behavior
uint16_t count = 0;
bool init_mark = false;
#endif

// setup multiplexer pins as outputs
void setupMux()
{
#if BOARD_TYPE == BOARD_NANO
    // set bits 2-5 Port D to output for mux adress (Pin 2-5)
    DDRD = (DDRD & 0xfc) | 0x3c;
#elif BOARD_TYPE == BOARD_MICRO
    // set bits 0-3 Port D to output for mux adress
    DDRD = (DDRD & 0xf0) | 0x0f;
#else
    // set bits 0-3 Port A to output for mux adress (Pin 22-25)
    DDRA = 0x0f;
    DDRC = 0x00;
#endif
}
// scan all multiplexers simultaneously into virtual inputs
void handleMux()
{
    // loop over all pins
    for (uint8_t pin = 0; pin < 16; pin++)
    {
#if BOARD_TYPE == BOARD_NANO
        // remap pins 0+1 to 4+5 due to bug on PCB
        PORTD = (PIND & 0xc3) | (pin & 0x0c) | ((pin & 0x03) << 4);
        // delay to settle mux
        delayMicroseconds(MUX_DELAY);
        // join inputs from all boards into channels
        // P1 and P2 are exchanged on HAT board
        Mux[pin] = (~PINC & 0x3F);
#elif BOARD_TYPE == BOARD_MICRO
        // select MUX pin
        // MSB         LSB
        // PD0 PD1 PD2 PD3
        PORTD = (PIND & 0xf0) | (pin & 0x08) >> 3 | ((pin & 0x04) >> 1) | (pin & 0x02) << 1 | ((pin & 0x01) << 3);
        // delay to settle mux
        delayMicroseconds(MUX_DELAY);
        // join inputs from all boards into channels
        // MSB                 LSB
        // PB5 PB4 PE6 PD7 PC6 PD4
        Mux[pin] = (~PIND & 0x10) >> 4 | (~PINC & 0x40) >> 5 | (~PIND & 0x80) >> 5 | (~PINE & 0x40) >> 3 | (~PINB & 0x30);
#else
        PORTA = (PINA & 0xF0) | pin;
        // delay to settle mux
        delayMicroseconds(MUX_DELAY);
        // scan inputs from first two Mux boards (P1 & P2)
        //     MSB                 LSB
        // P1: PC6 PC7 PA7 PA6 PA5 PA4
        // P2: PC0 PC1 PC2 PC3 PC4 PC5
        Mux[pin] = (uint16_t)((~PINA & 0xF0) >> 4) | ((uint16_t)__builtin_avr_insert_bits(0x01234567, ~PINC, 0) << 4);
#endif
    }
}
// get state of a specific input pin
bool getMux(uint16_t *mux, uint8_t module, uint8_t pin)
{
    // module: mux module (0-11)
    // pin: pin on the module (0-15)
    return ((mux[pin] >> module) & 1);
}

// Buttons
void initButton(button_t *but)
{
    but->_state = 0;
}
void handleButton(button_t *but, const char *name, repeat_t rep, bool input)
{
    if (input)
    {
        if (but->_state == 0)
        {
            Serial.write(name);
            Serial.write("=1\n");
            but->_state = DEBOUNCE_DELAY;
            if (rep == repeat)
            {
                tmr_rep = millis() + REPEAT_DELAY;
            }
        }
        if (rep == repeat)
        {
            if (millis() > tmr_rep)
            {
                Serial.write(name);
                Serial.write("=1\n");
                tmr_rep += REPEAT_DELAY;
            }
        }
    }
    else if (but->_state > 0)
    {
        if (--but->_state == 0)
        {
            Serial.write(name);
            Serial.write("=0\n");
        }
    }
}

// Switches
void initSwitch(switch_t *swi)
{
    swi->_state = 0;
}
void handleSwitch(switch_t *swi, const char *name, bool input)
{
    if (input && (swi->_state == 0))
    {
        Serial.write(name);
        Serial.write(".SW.ON\n");
        swi->_state = DEBOUNCE_DELAY;
    }
    if (!input && (swi->_state > 0))
    {
        if (--swi->_state == 0)
        {
            Serial.write(name);
            Serial.write(".SW.OFF\n");
        }
    }
}

// Encoders
void initEncoder(encoder_t *enc)
{
    enc->_count = 0;
    enc->_state = 0;
}
void handleEncoder(encoder_t *enc, const char *up, const char *dn, bool input1, bool input2, uint8_t pulses)
{
    // collect new state
    enc->_state = ((enc->_state & 0x03) << 2) | (input2 << 1) | input1;
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
    if (enc->_count >= pulses)
    {
        Serial.write(up);
        Serial.write("\n");
        enc->_count -= pulses;
    }
    if (enc->_count <= -pulses)
    {
        Serial.write(dn);
        Serial.write("\n");
        enc->_count += pulses;
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
    writeLEDs(0xFFFF);
    delay(500);
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

#if MAX_POTIS > 0
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
#endif

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
#if MAX_POTIS > 0
    for (uint8_t pot = 0; pot < MAX_POTIS; pot++)
    {
        initPoti(&Potis[pot]);
    }
#endif
}

// Main loop routine
void loop()
{
    // scan Multiplexers into virtual inputs
    handleMux();

    // keep alive for RSG connection
    if (millis() >= tmr_next)
    { // timer interval for keepalive
        Serial.write("####RealSimGear#mrusk-G1000XFD1#1#");
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
        // 500ms keep alive cycle
        tmr_next += KEEPALIVE_DELAY;
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

    // first connector for PFD
    // MUX 0
    handleButton(&Buttons[btn++], "BTN_NAV_TOG", single, getMux(Mux, 0, 0));
    handleEncoder(&Encoders[enc++], "ENC_NAV_INNER_UP", "ENC_NAV_INNER_DN", getMux(Mux, 0, 1), getMux(Mux, 0, 2), 4);
    handleEncoder(&Encoders[enc++], "ENC_NAV_OUTER_UP", "ENC_NAV_OUTER_DN", getMux(Mux, 0, 4), getMux(Mux, 0, 3), 4);
    handleButton(&Buttons[btn++], "BTN_COM_TOG", single, getMux(Mux, 0, 5));
    handleEncoder(&Encoders[enc++], "ENC_COM_INNER_UP", "ENC_COM_INNER_DN", getMux(Mux, 0, 6), getMux(Mux, 0, 7), 4);
    handleEncoder(&Encoders[enc++], "ENC_COM_OUTER_UP", "ENC_COM_OUTER_DN", getMux(Mux, 0, 9), getMux(Mux, 0, 8), 4);
    handleButton(&Buttons[btn++], "BTN_CRS_SYNC", single, getMux(Mux, 0, 10));
    handleEncoder(&Encoders[enc++], "ENC_CRS_UP", "ENC_CRS_DN", getMux(Mux, 0, 11), getMux(Mux, 0, 12), 4);
    handleEncoder(&Encoders[enc++], "ENC_BARO_UP", "ENC_BARO_DN", getMux(Mux, 0, 14), getMux(Mux, 0, 13), 4);
    // MUX 1
    handleButton(&Buttons[btn++], "BTN_ALT_SEL", single, getMux(Mux, 1, 0));
    handleEncoder(&Encoders[enc++], "ENC_ALT_INNER_UP", "ENC_ALT_INNER_DN", getMux(Mux, 1, 1), getMux(Mux, 1, 2), 4);
    handleEncoder(&Encoders[enc++], "ENC_ALT_OUTER_UP", "ENC_ALT_OUTER_DN", getMux(Mux, 1, 4), getMux(Mux, 1, 3), 4);
    handleButton(&Buttons[btn++], "BTN_FMS", single, getMux(Mux, 1, 5));
    handleEncoder(&Encoders[enc++], "ENC_FMS_INNER_UP", "ENC_FMS_INNER_DN", getMux(Mux, 1, 6), getMux(Mux, 1, 7), 4);
    handleEncoder(&Encoders[enc++], "ENC_FMS_OUTER_UP", "ENC_FMS_OUTER_DN", getMux(Mux, 1, 9), getMux(Mux, 1, 8), 4);
    handleButton(&Buttons[btn++], "BTN_DIRECT", single, getMux(Mux, 1, 10));
    handleButton(&Buttons[btn++], "BTN_FPL", single, getMux(Mux, 1, 11));
    handleButton(&Buttons[btn++], "BTN_CLR", single, getMux(Mux, 1, 12));
    handleButton(&Buttons[btn++], "BTN_MENU", single, getMux(Mux, 1, 13));
    handleButton(&Buttons[btn++], "BTN_PROC", single, getMux(Mux, 1, 14));
    handleButton(&Buttons[btn++], "BTN_ENT", single, getMux(Mux, 1, 15));
    // MUX 2
#if AP_NXI
    handleButton(&Buttons[btn++], "BTN_AP", single, getMux(Mux, 2, 0));
    handleButton(&Buttons[btn++], "BTN_FD", single, getMux(Mux, 2, 1));
    handleButton(&Buttons[btn++], "BTN_NAV", single, getMux(Mux, 2, 2));
    handleButton(&Buttons[btn++], "BTN_ALT", single, getMux(Mux, 2, 3));
    handleButton(&Buttons[btn++], "BTN_VS", single, getMux(Mux, 2, 4));
    handleButton(&Buttons[btn++], "BTN_FLC", single, getMux(Mux, 2, 5));
    handleButton(&Buttons[btn++], "BTN_YD", single, getMux(Mux, 2, 6));
    handleButton(&Buttons[btn++], "BTN_HDG", single, getMux(Mux, 2, 7));
    handleButton(&Buttons[btn++], "BTN_APR", single, getMux(Mux, 2, 8));
    handleButton(&Buttons[btn++], "BTN_VNAV", single, getMux(Mux, 2, 9));
    handleButton(&Buttons[btn++], "BTN_NOSE_UP", single, getMux(Mux, 2, 10));
    handleButton(&Buttons[btn++], "BTN_NOSE_DN", single, getMux(Mux, 2, 11));
#endif
#if AP_STD
    handleButton(&Buttons[btn++], "BTN_AP", single, getMux(Mux, 2, 0));
    handleButton(&Buttons[btn++], "BTN_HDG", single, getMux(Mux, 2, 1));
    handleButton(&Buttons[btn++], "BTN_NAV", single, getMux(Mux, 2, 2));
    handleButton(&Buttons[btn++], "BTN_APR", single, getMux(Mux, 2, 3));
    handleButton(&Buttons[btn++], "BTN_VS", single, getMux(Mux, 2, 4));
    handleButton(&Buttons[btn++], "BTN_FLC", single, getMux(Mux, 2, 5));
    handleButton(&Buttons[btn++], "BTN_FD", single, getMux(Mux, 2, 6));
    handleButton(&Buttons[btn++], "BTN_ALT", single, getMux(Mux, 2, 7));
    handleButton(&Buttons[btn++], "BTN_VNAV", single, getMux(Mux, 2, 8));
    handleButton(&Buttons[btn++], "BTN_BC", single, getMux(Mux, 2, 9));
    handleButton(&Buttons[btn++], "BTN_NOSE_UP", single, getMux(Mux, 2, 10));
    handleButton(&Buttons[btn++], "BTN_NOSE_DN", single, getMux(Mux, 2, 11));
#endif
    handleEncoder(&Encoders[enc++], "ENC_NAV_VOL_UP", "ENC_NAV_VOL_DN", getMux(Mux, 2, 12), getMux(Mux, 2, 13), 4);
    handleButton(&Buttons[btn++], "BTN_NAV_VOL", single, getMux(Mux, 2, 14));
    handleButton(&Buttons[btn++], "BTN_NAV_FF", single, getMux(Mux, 2, 15));
    // MUX 3
    handleButton(&Buttons[btn++], "BTN_SOFT_1", single, getMux(Mux, 3, 0));
    handleButton(&Buttons[btn++], "BTN_SOFT_2", single, getMux(Mux, 3, 1));
    handleButton(&Buttons[btn++], "BTN_SOFT_3", single, getMux(Mux, 3, 2));
    handleButton(&Buttons[btn++], "BTN_SOFT_4", single, getMux(Mux, 3, 3));
    handleButton(&Buttons[btn++], "BTN_SOFT_5", single, getMux(Mux, 3, 4));
    handleButton(&Buttons[btn++], "BTN_SOFT_6", single, getMux(Mux, 3, 5));
    handleButton(&Buttons[btn++], "BTN_SOFT_7", single, getMux(Mux, 3, 6));
    handleButton(&Buttons[btn++], "BTN_SOFT_8", single, getMux(Mux, 3, 7));
    handleButton(&Buttons[btn++], "BTN_SOFT_9", single, getMux(Mux, 3, 8));
    handleButton(&Buttons[btn++], "BTN_SOFT_10", single, getMux(Mux, 3, 9));
    handleButton(&Buttons[btn++], "BTN_SOFT_11", single, getMux(Mux, 3, 10));
    handleButton(&Buttons[btn++], "BTN_SOFT_12", single, getMux(Mux, 3, 11));
    handleEncoder(&Encoders[enc++], "ENC_COM_VOL_UP", "ENC_COM_VOL_DN", getMux(Mux, 3, 12), getMux(Mux, 3, 13), 4);
    handleButton(&Buttons[btn++], "BTN_COM_VOL", single, getMux(Mux, 3, 14));
    handleButton(&Buttons[btn++], "BTN_COM_FF", single, getMux(Mux, 3, 15));
    // MUX 4
    handleButton(&Buttons[btn++], "BTN_PAN_SYNC", single, getMux(Mux, 4, 0) && !getMux(Mux, 4, 1) && !getMux(Mux, 4, 2) && !getMux(Mux, 4, 3) && !getMux(Mux, 4, 4));
    handleButton(&Buttons[btn++], "BTN_PAN_UP", repeat, getMux(Mux, 4, 0) && getMux(Mux, 4, 1));
    handleButton(&Buttons[btn++], "BTN_PAN_LEFT", repeat, getMux(Mux, 4, 0) && getMux(Mux, 4, 2));
    handleButton(&Buttons[btn++], "BTN_PAN_DN", repeat, getMux(Mux, 4, 0) && getMux(Mux, 4, 3));
    handleButton(&Buttons[btn++], "BTN_PAN_RIGHT", repeat, getMux(Mux, 4, 0) && getMux(Mux, 4, 4));
    handleEncoder(&Encoders[enc++], "ENC_RANGE_UP", "ENC_RANGE_DN", getMux(Mux, 4, 6), getMux(Mux, 4, 5), 2);
    handleEncoder(&Encoders[enc++], "ENC_HDG_UP", "ENC_HDG_DN", getMux(Mux, 4, 12), getMux(Mux, 4, 13), 4);
    handleButton(&Buttons[btn++], "BTN_HDG_SYNC", single, getMux(Mux, 4, 14));

#if LEFT_PANEL
    handleSwitch(&Switches[swi++], "SW_MASTER", getMux(Mux, 2, 0));
    handleSwitch(&Switches[swi++], "SW_AV_MASTER", getMux(Mux, 2, 1));
    handleSwitch(&Switches[swi++], "SW_PITOT", getMux(Mux, 2, 2));
    handleSwitch(&Switches[swi++], "SW_BRAKE", getMux(Mux, 2, 3));
    handleButton(&Buttons[btn++], "BTN_DEICE_MAX", single, getMux(Mux, 2, 5));
    handleSwitch(&Switches[swi++], "SW_DEICE_HIGH", getMux(Mux, 2, 6));
    handleSwitch(&Switches[swi++], "SW_DEICE_OFF", getMux(Mux, 2, 7));
    handleButton(&Buttons[btn++], "BTN_DEICE_WS", single, getMux(Mux, 2, 8));
    handleSwitch(&Switches[swi++], "SW_DEICE_LIGHT", getMux(Mux, 2, 9));
    handleSwitch(&Switches[swi++], "SW_DEICE_ANN", getMux(Mux, 2, 10));
    handleSwitch(&Switches[swi++], "SW_DEICE_BAK", getMux(Mux, 2, 11));
    // MUX 5
    handleSwitch(&Switches[swi++], "SW_ALT_L", getMux(Mux, 5, 0));
    handleButton(&Buttons[btn++], "BTN_START_L", single, getMux(Mux, 5, 1));
    handleButton(&Buttons[btn++], "BTN_START_R", single, getMux(Mux, 5, 2));
    handleSwitch(&Switches[swi++], "SW_ALT_R", getMux(Mux, 5, 3));
    handleSwitch(&Switches[swi++], "SW_PUMP_L", getMux(Mux, 5, 4));
    handleSwitch(&Switches[swi++], "SW_ENGINE_MASTER_L", getMux(Mux, 5, 5));
    handleSwitch(&Switches[swi++], "SW_ENGINE_MASTER_R", getMux(Mux, 5, 6));
    handleSwitch(&Switches[swi++], "SW_PUMP_R", getMux(Mux, 5, 7));
    handleSwitch(&Switches[swi++], "SW_VOTE_L_A", getMux(Mux, 5, 8));
    handleSwitch(&Switches[swi++], "SW_VOTE_L_B", getMux(Mux, 5, 9));
    handleButton(&Buttons[btn++], "BTN_ECU_TEST_L", single, getMux(Mux, 5, 10));
    handleButton(&Buttons[btn++], "BTN_ECU_TEST_R", single, getMux(Mux, 5, 11));
    handleSwitch(&Switches[swi++], "SW_VOTE_R_A", getMux(Mux, 5, 12));
    handleSwitch(&Switches[swi++], "SW_VOTE_R_B", getMux(Mux, 5, 13));
#endif

#if RIGHT_PANEL
    handleButton(&Buttons[btn++], "BTN_TRIM_CENTER", single, getMux(Mux, 4, 10));
    handleEncoder(&Encoders[enc++], "ENC_TRIM_RIGHT", "ENC_TRIM_LEFT", getMux(Mux, 4, 8), getMux(Mux, 4, 9), 4);
    // MUX 5
    handleSwitch(&Switches[swi++], "SW_LIGHT_LDG", getMux(Mux, 5, 0));
    handleSwitch(&Switches[swi++], "SW_LIGHT_TAXI", getMux(Mux, 5, 1));
    handleSwitch(&Switches[swi++], "SW_LIGHT_POS", getMux(Mux, 5, 2));
    handleSwitch(&Switches[swi++], "SW_LIGHT_STRB", getMux(Mux, 5, 3));
    handleSwitch(&Switches[swi++], "SW_FUEL_L_DN", getMux(Mux, 5, 4));
    handleSwitch(&Switches[swi++], "SW_FUEL_L_UP", getMux(Mux, 5, 5));
    handleSwitch(&Switches[swi++], "SW_FUEL_R_DN", getMux(Mux, 5, 6));
    handleSwitch(&Switches[swi++], "SW_FUEL_R_UP", getMux(Mux, 5, 7));
    handleSwitch(&Switches[swi++], "SW_FUEL_AUX_L", getMux(Mux, 5, 8));
    handleSwitch(&Switches[swi++], "SW_FUEL_AUX_R", getMux(Mux, 5, 9));
    handleButton(&Buttons[btn++], "BUT_GEAR_TEST", single, getMux(Mux, 5, 10));
    handleSwitch(&Switches[swi++], "SW_GEAR", getMux(Mux, 5, 11));
    handleSwitch(&Switches[swi++], "SW_FLAP_DN", getMux(Mux, 5, 12));
    handleSwitch(&Switches[swi++], "SW_FLAP_UP", getMux(Mux, 5, 13));
    // analog inputs
    handlePoti(&Potis[pot++], "SW_INSTR", analogRead(ANALOG_BASE));
    handlePoti(&Potis[pot++], "SW_FLOOD", analogRead(ANALOG_BASE + 1));
#endif

#if MFD_PANEL
    // use second connector for MFD (single Arduino setup)
    // MUX 6
    handleButton(&Buttons[btn++], "BTN_NAV_TOG_MFD", single, getMux(Mux, 6, 0));
    handleEncoder(&Encoders[enc++], "ENC_NAV_INNER_UP_MFD", "ENC_NAV_INNER_DN_MFD", getMux(Mux, 6, 1), getMux(Mux, 6, 2), 4);
    handleEncoder(&Encoders[enc++], "ENC_NAV_OUTER_UP_MFD", "ENC_NAV_OUTER_DN_MFD", getMux(Mux, 6, 4), getMux(Mux, 6, 3), 4);
    handleButton(&Buttons[btn++], "BTN_COM_TOG_MFD", single, getMux(Mux, 6, 5));
    handleEncoder(&Encoders[enc++], "ENC_COM_INNER_UP_MFD", "ENC_COM_INNER_DN_MFD", getMux(Mux, 6, 6), getMux(Mux, 6, 7), 4);
    handleEncoder(&Encoders[enc++], "ENC_COM_OUTER_UP_MFD", "ENC_COM_OUTER_DN_MFD", getMux(Mux, 6, 9), getMux(Mux, 6, 8), 4);
    handleButton(&Buttons[btn++], "BTN_CRS_SYNC_MFD", single, getMux(Mux, 6, 10));
    handleEncoder(&Encoders[enc++], "ENC_CRS_UP_MFD", "ENC_CRS_DN_MFD", getMux(Mux, 6, 11), getMux(Mux, 6, 12), 4);
    handleEncoder(&Encoders[enc++], "ENC_BARO_UP_MFD", "ENC_BARO_DN_MFD", getMux(Mux, 6, 14), getMux(Mux, 6, 13), 4);

    // MUX 7
    handleButton(&Buttons[btn++], "BTN_ALT_SEL_MFD", single, getMux(Mux, 7, 0));
    handleEncoder(&Encoders[enc++], "ENC_ALT_INNER_UP_MFD", "ENC_ALT_INNER_DN_MFD", getMux(Mux, 7, 1), getMux(Mux, 7, 2), 4);
    handleEncoder(&Encoders[enc++], "ENC_ALT_OUTER_UP_MFD", "ENC_ALT_OUTER_DN_MFD", getMux(Mux, 7, 4), getMux(Mux, 7, 3), 4);
    handleButton(&Buttons[btn++], "BTN_FMS_MFD", single, getMux(Mux, 7, 5));
    handleEncoder(&Encoders[enc++], "ENC_FMS_INNER_UP_MFD", "ENC_FMS_INNER_DN_MFD", getMux(Mux, 7, 6), getMux(Mux, 7, 7), 4);
    handleEncoder(&Encoders[enc++], "ENC_FMS_OUTER_UP_MFD", "ENC_FMS_OUTER_DN_MFD", getMux(Mux, 7, 9), getMux(Mux, 7, 8), 4);
    handleButton(&Buttons[btn++], "BTN_DIRECT_MFD", single, getMux(Mux, 7, 10));
    handleButton(&Buttons[btn++], "BTN_FPL_MFD", single, getMux(Mux, 7, 11));
    handleButton(&Buttons[btn++], "BTN_CLR_MFD", single, getMux(Mux, 7, 12));
    handleButton(&Buttons[btn++], "BTN_MENU_MFD", single, getMux(Mux, 7, 13));
    handleButton(&Buttons[btn++], "BTN_PROC_MFD", single, getMux(Mux, 7, 14));
    handleButton(&Buttons[btn++], "BTN_ENT_MFD", single, getMux(Mux, 7, 15));
    // MUX 8
#if AP_NXI
    handleButton(&Buttons[btn++], "BTN_AP_MFD", single, getMux(Mux, 8, 0));
    handleButton(&Buttons[btn++], "BTN_FD_MFD", single, getMux(Mux, 8, 1));
    handleButton(&Buttons[btn++], "BTN_NAV_MFD", single, getMux(Mux, 8, 2));
    handleButton(&Buttons[btn++], "BTN_ALT_MFD", single, getMux(Mux, 8, 3));
    handleButton(&Buttons[btn++], "BTN_VS_MFD", single, getMux(Mux, 8, 4));
    handleButton(&Buttons[btn++], "BTN_FLC_MFD", single, getMux(Mux, 8, 5));
    handleButton(&Buttons[btn++], "BTN_YD_MFD", single, getMux(Mux, 8, 6));
    handleButton(&Buttons[btn++], "BTN_HDG_MFD", single, getMux(Mux, 8, 7));
    handleButton(&Buttons[btn++], "BTN_APR_MFD", single, getMux(Mux, 8, 8));
    handleButton(&Buttons[btn++], "BTN_VNAV_MFD", single, getMux(Mux, 8, 9));
    handleButton(&Buttons[btn++], "BTN_NOSE_UP_MFD", single, getMux(Mux, 8, 10));
    handleButton(&Buttons[btn++], "BTN_NOSE_DN_MFD", single, getMux(Mux, 8, 11));
#endif
#if AP_STD
    handleButton(&Buttons[btn++], "BTN_AP_MFD", single, getMux(Mux, 8, 0));
    handleButton(&Buttons[btn++], "BTN_HDG_MFD", single, getMux(Mux, 8, 1));
    handleButton(&Buttons[btn++], "BTN_NAV_MFD", single, getMux(Mux, 8, 2));
    handleButton(&Buttons[btn++], "BTN_APR_MFD", single, getMux(Mux, 8, 3));
    handleButton(&Buttons[btn++], "BTN_VS_MFD", single, getMux(Mux, 8, 4));
    handleButton(&Buttons[btn++], "BTN_FLC_MFD", single, getMux(Mux, 8, 5));
    handleButton(&Buttons[btn++], "BTN_FD_MFD", single, getMux(Mux, 8, 6));
    handleButton(&Buttons[btn++], "BTN_ALT_MFD", single, getMux(Mux, 8, 7));
    handleButton(&Buttons[btn++], "BTN_VNAV_MFD", single, getMux(Mux, 8, 8));
    handleButton(&Buttons[btn++], "BTN_BC_MFD", single, getMux(Mux, 8, 9));
    handleButton(&Buttons[btn++], "BTN_NOSE_UP_MFD", single, getMux(Mux, 8, 10));
    handleButton(&Buttons[btn++], "BTN_NOSE_DN_MFD", single, getMux(Mux, 8, 11));
#endif
    handleEncoder(&Encoders[enc++], "ENC_NAV_VOL_UP_MFD", "ENC_NAV_VOL_DN_MFD", getMux(Mux, 8, 12), getMux(Mux, 8, 13), 4);
    handleButton(&Buttons[btn++], "BTN_NAV_VOL_MFD", single, getMux(Mux, 8, 14));
    handleButton(&Buttons[btn++], "BTN_NAV_FF_MFD", single, getMux(Mux, 8, 15));

    // MUX 9
    handleButton(&Buttons[btn++], "BTN_SOFT_1_MFD", single, getMux(Mux, 9, 0));
    handleButton(&Buttons[btn++], "BTN_SOFT_2_MFD", single, getMux(Mux, 9, 1));
    handleButton(&Buttons[btn++], "BTN_SOFT_3_MFD", single, getMux(Mux, 9, 2));
    handleButton(&Buttons[btn++], "BTN_SOFT_4_MFD", single, getMux(Mux, 9, 3));
    handleButton(&Buttons[btn++], "BTN_SOFT_5_MFD", single, getMux(Mux, 9, 4));
    handleButton(&Buttons[btn++], "BTN_SOFT_6_MFD", single, getMux(Mux, 9, 5));
    handleButton(&Buttons[btn++], "BTN_SOFT_7_MFD", single, getMux(Mux, 9, 6));
    handleButton(&Buttons[btn++], "BTN_SOFT_8_MFD", single, getMux(Mux, 9, 7));
    handleButton(&Buttons[btn++], "BTN_SOFT_9_MFD", single, getMux(Mux, 9, 8));
    handleButton(&Buttons[btn++], "BTN_SOFT_10_MFD", single, getMux(Mux, 9, 9));
    handleButton(&Buttons[btn++], "BTN_SOFT_11_MFD", single, getMux(Mux, 9, 10));
    handleButton(&Buttons[btn++], "BTN_SOFT_12_MFD", single, getMux(Mux, 9, 11));
    handleEncoder(&Encoders[enc++], "ENC_COM_VOL_UP_MFD", "ENC_COM_VOL_DN_MFD", getMux(Mux, 9, 12), getMux(Mux, 9, 13), 4);
    handleButton(&Buttons[btn++], "BTN_COM_VOL_MFD", single, getMux(Mux, 9, 14));
    handleButton(&Buttons[btn++], "BTN_COM_FF_MFD", single, getMux(Mux, 9, 15));

    // MUX 10
    handleButton(&Buttons[btn++], "BTN_PAN_SYNC_MFD", single, getMux(Mux, 10, 0) && !getMux(Mux, 10, 1) && !getMux(Mux, 10, 2) && !getMux(Mux, 10, 3) && !getMux(Mux, 10, 4));
    handleButton(&Buttons[btn++], "BTN_PAN_UP_MFD", repeat, getMux(Mux, 10, 0) && getMux(Mux, 10, 1));
    handleButton(&Buttons[btn++], "BTN_PAN_LEFT_MFD", repeat, getMux(Mux, 10, 0) && getMux(Mux, 10, 2));
    handleButton(&Buttons[btn++], "BTN_PAN_DN_MFD", repeat, getMux(Mux, 10, 0) && getMux(Mux, 10, 3));
    handleButton(&Buttons[btn++], "BTN_PAN_RIGHT_MFD", repeat, getMux(Mux, 10, 0) && getMux(Mux, 10, 4));
    handleEncoder(&Encoders[enc++], "ENC_RANGE_UP_MFD", "ENC_RANGE_DN_MFD", getMux(Mux, 10, 6), getMux(Mux, 10, 5), 2);
    handleEncoder(&Encoders[enc++], "ENC_HDG_UP_MFD", "ENC_HDG_DN_MFD", getMux(Mux, 10, 12), getMux(Mux, 10, 13), 4);
    handleButton(&Buttons[btn++], "BTN_HDG_SYNC_MFD", single, getMux(Mux, 10, 14));
#endif

#if DEBUG
    // show number of used input devices and halt program in case of error since memory is corrupted
    if (!init_mark)
    {
        init_mark = true;
        Serial.print("Buttons:  ");
        Serial.println(btn);
        if (btn > MAX_BUTTONS)
        {
            Serial.println("ERROR: Too many buttons used");
            while (true)
                ;
        }
        Serial.print("Switches: ");
        Serial.println(swi);
        if (swi > MAX_SWITCHES)
        {
            Serial.println("ERROR: Too many Switches used");
            while (true)
                ;
        }
        Serial.print("Encoders: ");
        Serial.println(enc);
        if (enc > MAX_ENCODERS)
        {
            Serial.println("ERROR: Too many encoders used");
            while (true)
                ;
        }
        Serial.print("Potis:    ");
        Serial.println(pot);
        if (pot > MAX_POTIS)
        {
            Serial.println("ERROR: Too many potis used");
            while (true)
                ;
        }
    }
#endif
}