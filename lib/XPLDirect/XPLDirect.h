/*
  XPLDirect.h - Library for serial interface to Xplane SDK.
  Created by Michael Gerlicher,  September 2020.
  To report problems, download updates and examples, suggest enhancements or get technical support, please visit my patreon page:
     www.patreon.com/curiosityworkshop
  Stripped down to Minimal Version by mrusk, February 2023  
*/

#ifndef XPLDirect_h
#define XPLDirect_h

#define XPLDIRECT_MAXDATAREFS_ARDUINO 20 // This can be changed to suit your needs and capabilities of your board.
#define XPLDIRECT_MAXCOMMANDS_ARDUINO 100  // Same here.
#define XPLDIRECT_RX_TIMEOUT 500 // after detecting a frame header, how long will we wait to receive the rest of the frame.  (default 500)

#define XPLMAX_PACKETSIZE 80 // Probably leave this alone. If you need a few extra bytes of RAM it could be reduced, but it needs to
                              // be as long as the longest dataref name + 10.  If you are using datarefs
                              // that transfer strings it needs to be big enough for those too. (default 200)

#define XPL_USE_PROGMEM 1 // use Flash for strings, requires F() macro for strings in all registration calls

//////////////////////////////////////////////////////////////
// STOP! Dont change any other defines in this header!
//////////////////////////////////////////////////////////////

#if XPL_USE_PROGMEM
  typedef const __FlashStringHelper XPString_t;
#else
  typedef const char XPString_t;
#endif

#define XPLDIRECT_BAUDRATE 115200   // don't mess with this, it needs to match the plugin which won't change
#define XPLDIRECT_PACKETHEADER '<'  // ...or this
#define XPLDIRECT_PACKETTRAILER '>' // ...or this
#define XPLDIRECT_VERSION 2106171   // The plugin will start to verify that a compatible version is being used
#define XPLDIRECT_ID 0              // Used for relabled plugins to identify the company.  0 = normal distribution version

#define XPLERROR 'E'                    // %s         general error
#define XPLRESPONSE_NAME '0'
#define XPLRESPONSE_DATAREF '3'         // %3.3i%s    dataref handle, dataref name
#define XPLRESPONSE_COMMAND '4'         // %3.3i%s    command handle, command name
#define XPLRESPONSE_VERSION 'V'
#define XPLCMD_PRINTDEBUG '1'
#define XPLCMD_RESET '2'
#define XPLCMD_SPEAK 'S'                // speak string
#define XPLCMD_SENDNAME 'a'
#define XPLREQUEST_REGISTERDATAREF 'b'  // %1.1i%2.2i%5.5i%s RWMode, array index (0 for non array datarefs), divider to decrease resolution, dataref name
#define XPLREQUEST_REGISTERCOMMAND 'm'  // just the name of the command to register
#define XPLREQUEST_NOREQUESTS 'c'       // nothing to request
#define XPLREQUEST_REFRESH 'd'          // the plugin will call this once xplane is loaded in order to get fresh updates from arduino handles that write
#define XPLCMD_DUMPREGISTRATIONS 'Z'    // for debug purposes only (disabled)
#define XPLCMD_DATAREFUPDATE 'e'
#define XPLCMD_SENDREQUEST 'f'
#define XPLCMD_DEVICEREADY 'g'
#define XPLCMD_DEVICENOTREADY 'h'
#define XPLCMD_COMMANDSTART 'i'
#define XPLCMD_COMMANDEND 'j'
#define XPLCMD_COMMANDTRIGGER 'k' //  %3.3i%3.3i   command handle, number of triggers
#define XPLCMD_SENDVERSION 'v'    // we will respond with current build version

// TODO: use enums
#define XPL_READ 1
#define XPL_WRITE 2
#define XPL_READWRITE 3

#define XPL_DATATYPE_INT 1
#define XPL_DATATYPE_FLOAT 2
#define XPL_DATATYPE_STRING 3

class XPLDirect
{
public:
  XPLDirect(Stream *);
  void begin(const char *devicename); // parameter is name of your device for reference
  int connectionStatus(void);
  int commandTrigger(int commandHandle);                    // triggers specified command 1 time;
  int commandTrigger(int commandHandle, int triggerCount);  // triggers specified command triggerCount times.  
  int commandStart(int commandHandle);
  int commandEnd(int commandHandle);
  int datarefsUpdated();      // returns true if xplane has updated any datarefs since last call to datarefsUpdated()
  int hasUpdated(int handle); // returns true if xplane has updated this dataref since last call to hasUpdated()
  int registerDataRef(XPString_t *datarefName, int rwmode, unsigned int rate, float divider, long int *value);
  int registerDataRef(XPString_t *datarefName, int rwmode, unsigned int rate, float divider, long int *value, int index);
  int registerDataRef(XPString_t *datarefName, int rwmode, unsigned int rate, float divider, float *value);
  int registerDataRef(XPString_t *datarefName, int rwmode, unsigned int rate, float divider, float *value, int index);
  int registerDataRef(XPString_t *datarefName, int rwmode, unsigned int rate, char* value);
  int registerCommand(XPString_t *commandName); 
  int sendDebugMessage(const char *msg);
  int sendSpeakMessage(const char* msg);
  int allDataRefsRegistered(void);
  void sendResetRequest(void);
  int xloop(void); // where the magic happens!
private:
  void _processSerial();
  void _processPacket();
  void _sendPacketInt(int command, int handle, long int value); // for ints
  void _sendPacketFloat(int command, int handle, float value);  // for floats
  void _sendPacketVoid(int command, int handle);                // just a command with a handle
  void _sendPacketString(int command, char *str);               // for a string
  void _transmitPacket();
  void _sendname();
  void _sendVersion();
  int _getHandleFromFrame();
  int _getPayloadFromFrame(long int *);
  int _getPayloadFromFrame(float *);
  int _getPayloadFromFrame(char *);

  Stream *streamPtr;
  char *_deviceName;
  char _receiveBuffer[XPLMAX_PACKETSIZE];
  int _receiveBufferBytesReceived;
  char _sendBuffer[XPLMAX_PACKETSIZE];
  int _connectionStatus;
  int _dataRefsCount;
  struct _dataRefStructure
  {
    int dataRefHandle;
    byte dataRefRWType;       // XPL_READ, XPL_WRITE, XPL_READWRITE
    byte dataRefVARType;      // XPL_DATATYPE_INT 1, XPL_DATATYPE_FLOAT  2   XPL_DATATYPE_STRING 3
    float divider;            // tell the host to reduce resolution by dividing then remultiplying by this number to reduce traffic.   (ie .02, .1, 1, 5, 10, 100, 1000 etc)
    byte forceUpdate;         // in case xplane plugin asks for a refresh
    unsigned long updateRate; // maximum update rate in milliseconds, 0 = every change
    unsigned long lastUpdateTime;
    XPString_t *dataRefName;
    void *latestValue;
    union {
      long int lastSentIntValue;
      float lastSentFloatValue;
    };
    byte updatedFlag; //  True if xplane has updated this dataref.  Gets reset when we call hasUpdated method.
    byte arrayIndex;  // for datarefs that speak in arrays
  } *_dataRefs[XPLDIRECT_MAXDATAREFS_ARDUINO];
  int _commandsCount;
  struct _commandStructure
  {
    int commandHandle;
    XPString_t *commandName;
  } *_commands[XPLDIRECT_MAXCOMMANDS_ARDUINO];
  byte _allDataRefsRegistered; // becomes true if all datarefs have been registered
  byte _datarefsUpdatedFlag;   // becomes true if any datarefs have been updated from xplane since last call to datarefsUpdated()
};

extern XPLDirect XP;

#endif
