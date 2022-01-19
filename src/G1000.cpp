// G1000 handler
// mrusk
#include "Arduino.h"

// virtual inputs
bool mux[24][16];

// setup multiplexer pins as outputs
void muxSetup()
{
    pinMode(22, OUTPUT);
    pinMode(23, OUTPUT);
    pinMode(24, OUTPUT);
    pinMode(25, OUTPUT);
}
// scan all multiplexers simultaneously into virtual inputs
void muxScan()
{
    for (byte channel = 0; channel < 16; channel++)
    {
        digitalWrite(22, channel & 1);
        digitalWrite(23, channel & 2);
        digitalWrite(24, channel & 4);
        digitalWrite(25, channel & 8);
        for (byte input = 0; input < 24; input++)
        {
            mux[input][channel] = !digitalRead(input + 26); // invert signal here b/c switches are pull down
        }
    }
}

class Button
{
public:
    Button(const char *, bool);
    Button(const char *, bool, bool *);
    void handle(bool);
    void handle();
private:
    const char *_name;
    bool _repeat;
    bool* _input;
    bool _state;
};
Button::Button(const char *name, bool repeat)
{
    _name = name;
    _repeat = repeat;
    _input = NULL;
}
Button::Button(const char *name, bool repeat, bool *input)
{
    _name = name;
    _repeat = repeat;
    _input = input;
}
void Button::handle(bool input)
{
    if (!input && _state)
    {
        Serial.write(_name);
        Serial.write("=0\n");
    }
    if (_repeat)
    {
        if (input)
        {
            Serial.write(_name);
            Serial.write("=1\n");
            delay(100);
        }
    }
    else
    {
        if (input && !_state)
        {
            Serial.write(_name);
            Serial.write("=1\n");
        }
    }
    _state = input;
}
void Button::handle()
{
    this->handle(*_input);
}

class Encoder
{
public:
    Encoder(const char *, const char *);
    Encoder(const char *, const char *, bool *, bool *);
    void handle(bool, bool);
    void handle();

private:
    const char *_up;
    const char *_dn;
    bool _old1, _old2;
    long _count, _mark;
    bool *_input1, *_input2;
};
Encoder::Encoder(const char* up, const char* dn) {
    _up = up;
    _dn = dn;
    _count = 0;
    _mark = 0;
    _old1 = false;
    _old2 = false;
    _input1 = NULL;
    _input2 = NULL;
}
Encoder::Encoder(const char* up, const char* dn, bool *input1, bool *input2) {
    _up = up;
    _dn = dn;
    _count = 0;
    _mark = 0;
    _old1 = false;
    _old2 = false;
    _input1 = input1;
    _input2 = input2;
}
void Encoder::handle(bool input1, bool input2) {
    uint8_t s = 0;
    if (_old1) s |= 4;
    if (_old2) s |= 8;
    if ((_old1=input1)) s |= 1;
    if ((_old2=input2)) s |= 2;
	switch (s) {
		case 0: case 5: case 10: case 15:
			break;
		case 1: case 7: case 8: case 14:
			_count++; break;
		case 2: case 4: case 11: case 13:
			_count--; break;
		case 3: case 12:
			_count += 2; break;
		default:
			_count -= 2; break;
	}
    if (_count > _mark + 3) {
        Serial.write(_up);
        Serial.write("\n");
        _mark = _count;
    }
    if (_count < _mark - 3) {
        Serial.write(_dn);
        Serial.write("\n");
        _mark = _count;
    }
}
void Encoder::handle() {
    this->handle(*_input1, *_input2);
}

#define MAX_BUTTONS 100
Button *Buttons[MAX_BUTTONS];
uint8_t numBtn = 0;

#define MAX_ENCODERS 30
Encoder *Encoders[MAX_ENCODERS];
uint8_t numEnc = 0;

void setup() {
    Serial.begin(115200);
    muxSetup();
    
    // MUX 6
    Buttons[numBtn++] = new Button {"BTN_NAV_TOG", false, &mux[6][0]};
    Encoders[numEnc++] = new Encoder {"ENC_NAV_INNER_UP", "ENC_NAV_INNER_DN", &mux[6][1], &mux[6][2]};
    Encoders[numEnc++] = new Encoder {"ENC_NAV_OUTER_UP", "ENC_NAV_OUTER_DN", &mux[6][4], &mux[6][3]};
    Buttons[numBtn++] = new Button {"BTN_COM_TOG", false, &mux[6][5]};
    Encoders[numEnc++] = new Encoder {"ENC_COM_INNER_UP", "ENC_COM_INNER_DN", &mux[6][6], &mux[6][7]};
    Encoders[numEnc++] = new Encoder {"ENC_COM_OUTER_UP", "ENC_COM_OUTER_DN", &mux[6][9], &mux[6][8]};
    Buttons[numBtn++] = new Button {"BTN_CRS_SYNC", false, &mux[6][10]};
    Encoders[numEnc++] = new Encoder {"ENC_CRS_UP", "ENC_CRS_DN", &mux[6][11], &mux[6][12]};
    Encoders[numEnc++] = new Encoder {"ENC_BARO_UP", "ENC_BARO_DN", &mux[6][14], &mux[6][13]};
    // MUX 7
    Buttons[numBtn++] = new Button {"BTN_ALT_SEL", false, &mux[7][0]};
    Encoders[numEnc++] = new Encoder {"ENC_ALT_INNER_UP", "ENC_ALT_INNER_DN", &mux[7][1], &mux[7][2]};
    Encoders[numEnc++] = new Encoder {"ENC_ALT_OUTER_UP", "ENC_ALT_OUTER_DN", &mux[7][4], &mux[7][3]};
    Buttons[numBtn++] = new Button {"BTN_FMS", false, &mux[7][5]};
    Encoders[numEnc++] = new Encoder {"ENC_FMS_INNER_UP", "ENC_FMS_INNER_DN", &mux[7][6], &mux[7][7]};
    Encoders[numEnc++] = new Encoder {"ENC_FMS_OUTER_UP", "ENC_FMS_OUTER_DN", &mux[7][9], &mux[7][8]};
    Buttons[numBtn++] = new Button {"BTN_DIRECT", false, &mux[7][10]};
    Buttons[numBtn++] = new Button {"BTN_FPL", false, &mux[7][11]};
    Buttons[numBtn++] = new Button {"BTN_CLR", false, &mux[7][12]};
    Buttons[numBtn++] = new Button {"BTN_MENU", false, &mux[7][13]};
    Buttons[numBtn++] = new Button {"BTN_PROC", false, &mux[7][14]};
    Buttons[numBtn++] = new Button {"BTN_ENT", false, &mux[7][15]};
    // MUX 8
    Buttons[numBtn++] = new Button {"BTN_AP", false, &mux[8][0]};
    Buttons[numBtn++] = new Button {"BTN_HDG", false, &mux[8][1]};
    Buttons[numBtn++] = new Button {"BTN_NAV", false, &mux[8][2]};
    Buttons[numBtn++] = new Button {"BTN_APR", false, &mux[8][3]};
    Buttons[numBtn++] = new Button {"BTN_VS", false, &mux[8][4]};
    Buttons[numBtn++] = new Button {"BTN_FLC", false, &mux[8][5]};
    Buttons[numBtn++] = new Button {"BTN_FD", false, &mux[8][6]};
    Buttons[numBtn++] = new Button {"BTN_ALT", false, &mux[8][7]};
    Buttons[numBtn++] = new Button {"BTN_VNAV", false, &mux[8][8]};
    Buttons[numBtn++] = new Button {"BTN_BC", false, &mux[8][9]};
    Buttons[numBtn++] = new Button {"BTN_NOSE_UP", false, &mux[8][10]};
    Buttons[numBtn++] = new Button {"BTN_NOSE_DN", false, &mux[8][11]};
    Encoders[numEnc++] = new Encoder {"ENC_NAV_VOL_UP", "ENC_NAV_VOL_DN", &mux[8][12], &mux[8][13]};
    Buttons[numBtn++] = new Button {"BTN_NAV_VOL", false, &mux[8][14]};
    Buttons[numBtn++] = new Button {"BTN_NAV_FF", false, &mux[8][15]};
    // MUX 9
    Buttons[numBtn++] = new Button {"BTN_SOFT_1", false, &mux[9][0]};
    Buttons[numBtn++] = new Button {"BTN_SOFT_2", false, &mux[9][1]};
    Buttons[numBtn++] = new Button {"BTN_SOFT_3", false, &mux[9][2]};
    Buttons[numBtn++] = new Button {"BTN_SOFT_4", false, &mux[9][3]};
    Buttons[numBtn++] = new Button {"BTN_SOFT_5", false, &mux[9][4]};
    Buttons[numBtn++] = new Button {"BTN_SOFT_6", false, &mux[9][5]};
    Buttons[numBtn++] = new Button {"BTN_SOFT_7", false, &mux[9][6]};
    Buttons[numBtn++] = new Button {"BTN_SOFT_8", false, &mux[9][7]};
    Buttons[numBtn++] = new Button {"BTN_SOFT_9", false, &mux[9][8]};
    Buttons[numBtn++] = new Button {"BTN_SOFT_10", false, &mux[9][9]};
    Buttons[numBtn++] = new Button {"BTN_SOFT_11", false, &mux[9][10]};
    Buttons[numBtn++] = new Button {"BTN_SOFT_12", false, &mux[9][11]};
    Encoders[numEnc++] = new Encoder {"ENC_COM_VOL_UP", "ENC_COM_VOL_DN", &mux[9][12], &mux[9][13]};
    Buttons[numBtn++] = new Button {"BTN_COM_VOL", false, &mux[9][14]};
    Buttons[numBtn++] = new Button {"BTN_COM_FF", false, &mux[9][15]};
    // MUX 10
    Buttons[numBtn++] = new Button {"BTN_PAN_SYNC", false, &mux[10][0]};
    Buttons[numBtn++] = new Button {"BTN_PAN_UP", true, &mux[10][1]};
    Buttons[numBtn++] = new Button {"BTN_PAN_LEFT", true, &mux[10][2]};
    Buttons[numBtn++] = new Button {"BTN_PAN_DN", true, &mux[10][3]};
    Buttons[numBtn++] = new Button {"BTN_PAN_RIGHT", true, &mux[10][4]};
    Encoders[numEnc++] = new Encoder {"ENC_RANGE_UP", "ENC_RANGE_DN", &mux[10][6], &mux[10][5]};
    Encoders[numEnc++] = new Encoder {"ENC_HDG_UP", "ENC_HDG_DN", &mux[10][12], &mux[10][13]};
    Buttons[numBtn++] = new Button {"BTN_HDG_SYNC", false, &mux[10][14]};

    Serial.println("Buttons:" + String(numBtn)); 
    Serial.println("Encoders:" + String(numEnc)); 
}

uint16_t count = 0;
unsigned long next = 0;

void loop() {  
    // keep alive for RSG connection
    if(millis() >= next) { //timer interval for keepalive       
        Serial.write("\\####RealSimGear#mrusk-G1000XFD1#1#1.0.1#0001\n"); 
        // Serial.println(count);
        count = 0;
        next = millis()+500;
    }
    count++;

    // scan Multiplexers into virtual inputs
    muxScan();

    for (uint8_t i=0; i<numBtn; i++)
    {
        Buttons[i]->handle();
    }
    for (uint8_t i=0; i<numEnc; i++)
    {
        Encoders[i]->handle();
    }
}
