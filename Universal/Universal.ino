/*
**Disclaimer and Limitation of Liability**

This sketch (software) has been developed specifically for the **ESP32 Devkit 1** and the **SN65HVD230** CAN transceiver module. It has only been tested on the author’s personal model railway layout.

**The sketch is provided “AS IS” and “AS AVAILABLE”**, without any warranties or guarantees of any kind. The author explicitly disclaims all warranties, whether express, implied, or statutory, including but not limited to any warranties of merchantability, fitness for a particular purpose, accuracy, reliability, or non-infringement.

The author accepts **no responsibility or liability** for:
- Any malfunction, failure, or unexpected behaviour of the sketch
- Damage to hardware, loss of data, or disruption to your layout
- Incompatibility caused by updates to third-party libraries, Arduino core, JMRI, or other software
- Any direct, indirect, incidental, consequential, or punitive damages arising from the use or inability to use this sketch

This code is offered strictly for **educational and hobbyist purposes** to help railway modellers learn how to use the OpenLCB Single Thread Library. It is not intended for commercial use, safety-critical applications, or any situation where failure could cause damage or injury.

By downloading, using, or modifying this sketch, you acknowledge that you assume **all risk** and full responsibility for any outcomes resulting from its use.

The author reserves the right to modify or remove this sketch at any time without notice.

---
*/

// Universal 1.0.0 Base node
/*This is in beta testing but here to give a chance to have a look
  at the sketch

  2026.01.17 changes: Added second MCP23017, modified CDI

  2025.08.25 changes: Can Transceiver Version Only

Pin allocations
  - Pins 15 RX and 2 TX for the transceiver module
  - Pins 16,17,18,19,14,27,26,25  are used for input or output
  - Pin  21 SDA 
  - Pin  22 SCL

*/
/*
==============================================================
 Built from the AVR 2Servos NIO using ESPcan

 Coprright 2024 David P Harris
 derived from work by Alex Shepherd and David Harris
 Updated 2026 June David P Harris & John holmes
==============================================================
 - 8 Native input/output channels:
 - 32 MCP23017 input/output channels: Note A7 & B7 our Output only
    - type: 0=None, 
             1=Input, 2=Input inverted, 
             3=Input with pull-up, 4=Input with pull-up inverted, 
             5=Input toggle, 6=Toggle with pull-up
             7=Output, 8=Output inverted.
     - for Outputs: 
      - Events are consumed
       - On-duration: how long the output is set, from 10ms - 2550ms, 0 means forever
       - Off-period: the period until a repeat pulse, 0 means no repeat
     - for Inputs:
       - Events are produced
       - On-delay: delay before on-event is sent
       - Off-delay: the period before the off-event is sent
 - 32 servos 2 x PCA9685
     - 2 end-user configured position for Turnout control
     - Consumed event
     - Midpoint frog switch events when going to position 2
     - Midpoint frog switch events when going to position 1
     - indepentent servo speed adjustment
 - PCA servo option selector
     - 2 option for all servo start up 90 degree or midpoint 
     - 2 option for end of movement selector shutdown servo or leave servo active
==============================================================
*/

/* Third party Library needsed for this sketch to work are Available in the library manager.
  - mcp23017 by Bertrand Lemasle tested with version 2.0.0
  - ESP32Servo by Kevin Harrigton tested with version 3.2.1
  - ServoEasing by Armin Joachimsmeyer tested with version 3.6.0
  - ACAN_ESP32 by Mohamed Irfanulla tested on version 3.0.3
*/

#include <Arduino.h>
#include "Config.h"   // Contains configuration, see "Config.h"
#include "mdebugging.h"           // debugging
#include "OpenLCBHeader.h"        // System house-keeping.


// CDI (Configuration Description Information) in xml, must match MemStruct
// See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.1-ConfigurationDescriptionInformation-2016-02-06.pdf
extern "C" {
    #define N(x) xN(x)     // allow the insertion of the value (x) ..
    #define xN(x) #x       // .. into the CDI string.
const char configDefInfo[] PROGMEM =
// ===== Enter User definitions below =====
  CDIheader R"(
<name>Application Configuration</name>
<hints><visibility hideable='yes' hidden='yes' ></visibility></hints>
<group replication=')" N(NUM_NATIVE_IO) R"('>
  <name>ESP32 Native Input or Output pins</name>
  <hints><visibility hideable='yes' hidden='yes' ></visibility></hints>
  <description>Each pin can be either HIGH or LOW state logic, they can be set as Input, Output, or not used, with 11 end-user options to choose from the dropdown list.</description>
  <repname>Pin D16</repname>
  <repname>Pin D17</repname>
  <repname>Pin D18</repname>
  <repname>Pin D19</repname>
  <repname>Pin D14</repname>
  <repname>Pin D27</repname>
  <repname>Pin D26</repname>
  <repname>Pin D25</repname>
  <string size='24'><name>Description</name></string>
  <int size='1'>
    <name>Channel type</name>
    <map>
      <relation><property>0</property><value>None</value></relation> 
      <relation><property>1</property><value>Input</value></relation> 
      <relation><property>2</property><value>Input Inverted</value></relation> 
      <relation><property>3</property><value>Input with pull-up</value></relation>
      <relation><property>4</property><value>Input with pull-up, Inverted</value></relation>
      <relation><property>5</property><value>Toggle</value></relation>
      <relation><property>6</property><value>Toggle with pull-up</value></relation>
      <relation><property>7</property><value>Output Phase A</value></relation>
      <relation><property>8</property><value>Output Phase A Inverted</value></relation>
      <relation><property>9</property><value>Output Phase B</value></relation>
      <relation><property>10</property><value>Output Phase B Inverted</value></relation>
    </map>
  </int>
  <int size='1'>
    <name>On-Duration/On-delay 1-255 = 100ms-25.5s, 0=steady-state</name>
    <hints><slider tickSpacing='85' immediate='yes' showValue='yes'> </slider></hints>
  </int>
  <int size='1'>
    <name>Off-Period/Off-delay 1-255 = 100ms-25.5s, 0=No repeat</name>
    <hints><slider tickSpacing='85' immediate='yes' showValue='yes'> </slider></hints>
  </int>
  <eventid><name>Pin State HIGH 3.3 volts-Event</name></eventid>
  <eventid><name>Pin State LOW 0 volts-Event</name></eventid>
</group>
<group replication=')" N(NUM_MCP) R"('>
  <name>MCP23017 16 channel expander.</name>
  <hints><visibility hideable='yes' hidden='yes' ></visibility></hints>
  <description>Each pin can be either HIGH or LOW state logic. They can be set as Input(Except for A7 and B7), Output or not used, with 11 end-user options to choose from the dropdown list.</description>
  <repname>MCP: on 0x20 </repname>
  <repname>MCP: on 0x21 </repname>
  <string size='24'><name>Description of the boards location</name></string>
  <string size='8'><name>This MCP is </name></string>
  <group replication=')" N(NUM_MCP_PORTS) R"('>
    <name>Port banks selector care must be taken when using pins A7 or B7 as they are Ouptuts only</name>
    <repname>Port A</repname>
    <repname>Port B</repname>
    <group replication=')" N(NUM_MCP_IO_PER_PORT) R"('>
      <name>Port pins offer the same selections as the native inputs and outputs from the dropdown list</name>
      <repname>0</repname>
      <repname>1</repname>
      <repname>2</repname>
      <repname>3</repname>
      <repname>4</repname>
      <repname>5</repname>
      <repname>6</repname>
      <repname>7(OUT ONLY)</repname>
      <string size='24'><name>Description</name></string>
      <int size='1'>
        <name>Channel type</name>
        <map>
          <relation><property>0</property><value>None</value></relation> 
          <relation><property>1</property><value>Input</value></relation> 
          <relation><property>2</property><value>Input Inverted</value></relation> 
          <relation><property>3</property><value>Input with pull-up</value></relation>
          <relation><property>4</property><value>Input with pull-up, Inverted</value></relation>
          <relation><property>5</property><value>Toggle</value></relation>
          <relation><property>6</property><value>Toggle with pull-up</value></relation>
          <relation><property>7</property><value>Output Phase A</value></relation>
          <relation><property>8</property><value>Output Phase A Inverted</value></relation>
          <relation><property>9</property><value>Output Phase B</value></relation>
          <relation><property>10</property><value>Output Phase B Inverted</value></relation>
        </map>
      </int>
      <int size='1'>
        <name>On-Duration/On-delay 1-255 = 100ms-25.5s, 0=steady-state</name>
        <hints><slider tickSpacing='85' immediate='yes' showValue='yes'> </slider></hints>
      </int>
      <int size='1'>
        <name>Off-Period/Off-delay 1-255 = 100ms-25.5s, 0=No repeat</name>
        <hints><slider tickSpacing='85' immediate='yes' showValue='yes'> </slider></hints>
      </int>
      <eventid><name>Pin State HIGH 3.3 volts-Event</name></eventid>
      <eventid><name>Pin State LOW 0 volts-Event</name></eventid>
    </group>
  </group>
</group>
<group replication=')" N(NUM_PCA) R"('>
  <name>PCA9685 Servos modules</name>
  <hints><visibility hideable='yes' hidden='yes' ></visibility></hints>
  <description>Care must be taken when supplying the 5 volt power connections to the PCA9685, as many clones do not have reverse polarity protection as adversied.</description>
  <repname>PCA: on 0x40 </repname>
  <repname>PCA: on 0x41 </repname>
  <string size='24'><name>Description for this PCA boards location</name></string>
  <string size='8'><name>This PCA is </name></string>
  <group replication=')" N(NUM_PCA_PORTS) R"('>
  <name> Take note of the numbers in brackets are for channels 8 to 15</name>
    <repname>Channels 0-7</repname>
    <repname>Channels (8-15)</repname>
    <group replication=')" N(NUM_PCA_SERVO_PER_PORT) R"('>
      <name>Servo pins start 0 to 15 check marking to ensure the servo is on the channel you think.</name>
      <repname>0(8)</repname>
      <repname>1(9)</repname>
      <repname>2(10)</repname>
      <repname>3(11)</repname>
      <repname>4(12)</repname>
      <repname>5(13)</repname>
      <repname>6(14)</repname>
      <repname>7(15)</repname>
      <string size='24'><name>Description</name></string>
      <int size='1'><name>Speed of movement 5-50</name>
        <min>5</min><max>50</max>
        <hints><slider tickSpacing='20' immediate='yes' showValue='yes'> </slider></hints>
      </int>
      <int size='1'><name>Position 1 (Suggestion Closed). Angles between approximately 0-180 Care must be taken when using the slider in case of damae to turnouts.</name>
        <min>0</min><max>180</max>
        <hints><slider tickSpacing='45' immediate='yes' showValue='yes'> </slider></hints>
      </int>
      <eventid><name>When consumed, move to this angle</name></eventid>
      <int size='1'><name>Position 2 (Suggestion Thrown). Angles between approximately 0-180 Care must be taken when using the slider in case of damae to turnouts.</name>
        <min>0</min><max>180</max>
        <hints><slider tickSpacing='45' immediate='yes' showValue='yes'> </slider></hints>
      </int>
      <eventid><name>When consumed, move to this angle</name></eventid>
      <group>
        <name>MidPoint Events</name>
        <eventid><name>Sends this event when the servo passes the midpoint moving towards position 2</name></eventid>
        <eventid><name>Sends this event when the servo passes the midpoint moving towards position 1</name></eventid>
      </group>
    </group>
  </group>
</group>
<group>
  <name>PCA Servo Options</name>
  <hints><visibility hideable='yes' hidden='yes' ></visibility></hints>
  <int size='1'><name>At start-up, intialize the servos to:</name>
    <map>
      <relation><property>0</property><value>90 degrees</value></relation> 
      <relation><property>1</property><value>Midpoint between the endpoints</value></relation> 
    </map>
  </int>
  <int size='1'><name>At start-up, intialize with this interval between each 10ms increments 0-2550ms</name></int>
  <int size='1'><name>At end of servo movement </name>
    <map>
      <relation><property>0</property><value>Leave servo active</value></relation> 
      <relation><property>1</property><value>Shutdown servo</value></relation> 
    </map>
  </int>
</group>
    )" CDIfooter;
// ===== Enter User definitions above =====
} // end extern

// ===== MemStruct =====
//   Memory structure of EEPROM, must match CDI above
    typedef struct {
          EVENT_SPACE_HEADER eventSpaceHeader; // MUST BE AT THE TOP OF STRUCT - DO NOT REMOVE!!!
          char nodeName[20];  // optional node-name, used by ACDI
          char nodeDesc[24];  // optional node-description, used by ACDI
      // ===== Enter User definitions below =====
          struct {
            char desc[24];
            uint8_t type;
            uint8_t duration;    // 100ms-25.5s, 0=solid
            uint8_t period;      // 100ms-25.5s, 0=no repeat
            EventID onEid;
            EventID offEid;
          } natio[NUM_NATIVE_IO];
          struct {
            char desc[24];
            char status[8];
            struct {
              char desc[24];
              uint8_t type;
              uint8_t duration;    // 100ms-25.5s, 0=solid
              uint8_t period;      // 100ms-25.5s, 0=no repeat
              EventID onEid;
              EventID offEid;
            } io[16]; 
          } mcp[NUM_MCP];         
          struct {
            char desc[24];
            char status[8];
            struct {
              char desc[24];
              uint8_t speed;
              uint8_t angle1;
              EventID eid1;
              uint8_t angle2;
              EventID eid2;
              EventID eidup;
              EventID eiddown;
            } pcaservo[16];
          } pca[NUM_PCA];
          uint8_t pcaStartupPosition;
          uint8_t startupInterval;
          bool doreattach;
      // ===== Enter User definitions above =====
      // items below will be included in the EEPROM, but are not part of the CDI
    } MemStruct;                 // type definition


extern "C" {
    // ===== eventid Table =====
    //#define REG_SERVO_OUTPUT(s) CEID(servos[s].pos[0].eid), CEID(servos[s].pos[1].eid), CEID(servos[s].pos[2].eid)
    //#define REG_IO(i) PCEID(io[i].onEid), PCEID(io[i].offEid)
    //#define REG_NAT(g) REG_IO(g+0), REG_IO(g+1), REG_IO(g+2), REG_IO(g+3), REG_IO(g+4), REG_IO(g+5), REG_IO(g+6), REG_IO(g+7) 
    //#define REG_MCP(g) REG_IO(g+0), REG_IO(g+1), REG_IO(g+2), REG_IO(g+3), REG_IO(g+4), REG_IO(g+5), REG_IO(g+6), REG_IO(g+7) 
    //#define REG_SVO(s) CEID(servo[s].eid1), CEID(servo[s].eid2), PEID(servo[s].eidup), PEID(servo[s].eiddown)  
    //#define REG_PCA(g) REG_SVO(g+0), REG_SVO(g+1), REG_SVO(g+2), REG_SVO(g+3), REG_SVO(g+4), REG_SVO(g+5), REG_SVO(g+6), REG_SVO(g+7)

    //  Array of the offsets to every eventID in MemStruct/EEPROM/mem, and P/C flags
    const EIDTab eidtab[NUM_EVENT] PROGMEM = {
        IOEID(NUM_NATIVE_IO),     // native io
        MCPEID(NUM_MCP_PORTS),    // mcp io
        PCAEID(NUM_PCA_PORTS)     // pca servos
    };
    
    // SNIP Short node description for use by the Simple Node Information Protocol
    // See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.3-SimpleNodeInformation-2016-02-06.pdf
    extern const char SNII_const_data[] PROGMEM = "\001" MANU "\000" MODEL "\000" HWVERSION "\000" SWVERSION " " OlcbCommonVersion ; // last zero in double-quote
} // end extern "C"

// PIP Protocol Identification Protocol uses a bit-field to indicate which protocols this node supports
// See 3.3.6 and 3.3.7 in http://openlcb.com/wp-content/uploads/2016/02/S-9.7.3-MessageNetwork-2016-02-06.pdf
uint8_t protocolIdentValue[6] = {   //0xD7,0x58,0x00,0,0,0};
        pSimple | pDatagram | pMemConfig | pPCEvents | !pIdent    | pTeach     | !pStream   | !pReservation, // 1st byte
        pACDI   | pSNIP     | pCDI       | !pRemote  | !pDisplay  | !pTraction | !pFunction | !pDCC        , // 2nd byte
        0, 0, 0, 0                                                                                           // remaining 4 bytes
    };

#define OLCB_NO_BLUE_GOLD  // blue/gold not used in this sketch
#ifndef OLCB_NO_BLUE_GOLD
    #define BLUE 40  // built-in blue LED
    #define GOLD 39  // built-in green LED
    ButtonLed blue(BLUE, LOW);
    ButtonLed gold(GOLD, LOW);
    
    uint32_t patterns[8] = { 0x00010001L, 0xFFFEFFFEL }; // two per channel, one per event
    ButtonLed pA(13, LOW);
    ButtonLed pB(14, LOW);
    ButtonLed pC(15, LOW);
    ButtonLed pD(16, LOW);
    ButtonLed* buttons[8] = { &pA,&pA,&pB,&pB,&pC,&pC,&pD,&pD };
#endif // OLCB_NO_BLUE_GOLD

//#include <Servo.h>    //// NANO, Minima etc
//#include <ESP32Servo.h> //// ESP32
Servo servo[2];

uint8_t iopin[NUM_IO] = { IOPINS }; //// ESP32 

enum Type { tNONE=0, tIN, tINI, tINP, tINPI, tTOG, tTOGI, tPA, tPAI, tPB, tPBI };
bool iostate[NUM_IO] = {0};  // state of the iopin
bool logstate[NUM_IO] = {0}; // logic state for toggle
unsigned long next[NUM_IO] = {0};

bool i2cexists(uint8_t addr) {
  Wire.beginTransmission(addr);
  return Wire.endTransmission()==0;
}
bool i2chubexists = 0;
bool mcpexists[NUM_MCP];

uint8_t pcastate[NUM_PCA_SERVO];
bool fakeinput = 0;



// This is called to initialize the EEPROM to Factory Reset
void userInitAll()
{ 
  NODECONFIG.put(EEADDR(nodeName), ESTRING(BOARD));
  NODECONFIG.put(EEADDR(nodeDesc), ESTRING("Universal"));
  dP("\n NUM_NATIVE_IO"); dP(NUM_NATIVE_IO);
  for(uint8_t i = 0; i < NUM_NATIVE_IO; i++) {
    NODECONFIG.put(EEADDR(natio[i].desc), ESTRING(""));
    NODECONFIG.update(EEADDR(natio[i].type), 0);
    NODECONFIG.update(EEADDR(natio[i].duration), 0);
    NODECONFIG.update(EEADDR(natio[i].period), 0);
  }  
  dP("\n NUM_MCP"); dP(NUM_MCP);
  for(uint8_t m=0; m<NUM_MCP; m++) {
    NODECONFIG.put(EEADDR(mcp[m].desc), ESTRING(""));
    NODECONFIG.put(EEADDR(mcp[m].status), ESTRING("??"));
    for(uint8_t i = 0; i < 16; i++) {
      NODECONFIG.put(EEADDR(mcp[m].io[i].desc), ESTRING(""));
      NODECONFIG.update(EEADDR(mcp[m].io[i].type), 0);
      NODECONFIG.update(EEADDR(mcp[m].io[i].duration), 0);
      NODECONFIG.update(EEADDR(mcp[m].io[i].period), 0);
    }    
  }
  for(uint8_t p=0; p<NUM_PCA; p++) {
    NODECONFIG.put(EEADDR(pca[p].desc), ESTRING(""));
    NODECONFIG.put(EEADDR(pca[p].status), ESTRING("??"));
    for(uint8_t i=0; i<16; i++) {
      NODECONFIG.put(EEADDR(pca[p].pcaservo[i].desc), ESTRING(""));
      NODECONFIG.update(EEADDR(pca[p].pcaservo[i].speed), 5);
      NODECONFIG.update(EEADDR(pca[p].pcaservo[i].angle1), 90);
      NODECONFIG.update(EEADDR(pca[p].pcaservo[i].angle2), 90);
    }
  }
  NODECONFIG.update(EEADDR(pcaStartupPosition), 0);
  NODECONFIG.update(EEADDR(startupInterval), 25);  // 250 ms between each intial positioning.
  
  NODECONFIG.update(EEADDR(doreattach), 1);  // shutdown servo between moves

}
bool doreattach;
// determine the state of each eventid
enum evStates { VALID=4, INVALID=5, UNKNOWN=7 };
uint8_t userState(uint16_t index) {
  //dP("\n userState "); dP((uint16_t) index);
    if( index<NUM_IO_EVENT ) {
      int ch = (index)/2;
      uint8_t type = NODECONFIG.read(EEADDR(natio[ch].type));
      if( type==0) return UNKNOWN;
      int evst = index % 2;
      if(type==5 || type==6) { // ie a toggle
        Serial.print("\n toggle state"); PV(type); PV(evst); PV(logstate[ch]);
        if( logstate[ch]!=evst ) return VALID;  // (0 is on, and 1 is off)
        return INVALID;
      } else {
        if( iostate[ch]!=evst ) return VALID;
        return INVALID;
      }
    }
    index -= NUM_IO_EVENT;
    if( index<NUM_PCA_SERVO_EVENT ) {
      uint8_t s = index/2;
      if( pcastate[s]==2 ) return UNKNOWN;
      if( index%1 ) {
        if( pcastate[s]==1 ) return VALID;
        else return INVALID;
      }
    }
    return UNKNOWN;
}
    

  #ifdef DEBUG
    #define PV(x) { dP(" " #x "="); dP(x); }
  #else
    #define PV(x) 
  #endif

// MCP23017 INIT
void mcpinit() {
  for (int i = 0; i < NUM_MCP; i++) {
    mcpexists[i] = i2cexists(MCP_ADDRESSES[i]);
    Serial.print("\nMCP23017 at ");
    Serial.print(MCP_ADDRESSES[i], HEX);
    if(mcpexists[i]) {
      mcp[i] = new MCP23017(MCP_ADDRESSES[i]);
      mcp[i]->init(); 
      //String text = String("@") + String(MCP_ADDRESSES[i]);
      //NODECONFIG.put( EEADDR( mcp[i].status ), text.c_str() );
      NODECONFIG.put( EEADDR( mcp[i].status ), "OK" );
      Serial.println(" is ready.");
    } else {
      Serial.println(" is missing.");
      NODECONFIG.put( EEADDR( mcp[i].status ), "Missing" );
    }
  }

 #if 0 // mcp0 test
  for(uint8_t i=0; i<16; i++) mcp[0]->pinMode(i, OUTPUT);
  for(uint8_t t=0; t<10; t++) {
    for(uint8_t i=0; i<16; i++) { mcp[0]->digitalWrite(i, 0); delay(100); }
    for(uint8_t i=0; i<16; i++) { mcp[0]->digitalWrite(i, 1); delay(100); }
  }
 #endif
}
void setMode(uint8_t i, uint8_t mode) {
  if(i<NUM_NATIVE_IO) pinMode(iopin[i], mode);
  else {
    uint8_t ch = i-NUM_NATIVE_IO;
    if(!mcpexists[ch/16]) { Serial.print("\nmcp missing1"); return; }
    mcp[ch/16]->pinMode(ch%16, mode);
  }
}
void digOut(uint8_t i, uint8_t value) {
  if(i<NUM_NATIVE_IO) digitalWrite(iopin[i], value);
  else {
    uint8_t ch = i-NUM_NATIVE_IO;
    if(!mcpexists[ch/16]) { Serial.print("\nmcp missing2"); return; }
    mcp[ch/16]->digitalWrite(ch%16, value); 
  }
}
uint8_t digIn(uint8_t c) {
  if(c<NUM_NATIVE_IO) return digitalRead(iopin[c]);
  else {
    uint8_t ch = c-NUM_NATIVE_IO;
    if(!mcpexists[ch/16]) { return fakeinput; }
    return mcp[ch/16]->digitalRead(ch%16);
  }
}


// PCA9685 ROUTINES
bool pcaexists[NUM_PCA_SERVO];
uint8_t target[NUM_PCA_SERVO];
uint16_t pcabase =  NUM_IO_EVENT;

void getAndAttach16ServosToPCA9685Expander(uint8_t base, uint8_t aPCA9685I2CAddress) {
    ServoEasing *tServoEasingObjectPtr;
    Serial.print(F("\n    Get ServoEasing objects and attach servos to PCA9685 expander at address=0x"));
    Serial.print(aPCA9685I2CAddress, HEX);
    uint8_t pcaStartupPosition = NODECONFIG.read( EEADDR(pcaStartupPosition));
    dP("\n    pcaStartupPosition="); dP(pcaStartupPosition);
    //for (uint_fast8_t i = 0; i < PCA9685_MAX_CHANNELS; ++i) {
    for (uint_fast8_t i = 0; i < 16; ++i) {
        tServoEasingObjectPtr = new ServoEasing(aPCA9685I2CAddress); // Get the ServoEasing object
        tServoEasingObjectPtr->setPWM(0, 4096);
        uint8_t angle;
        if(pcaStartupPosition==0) {
          angle = 90;
        } else {
          uint8_t ch = base + i; 
          uint8_t a1 = NODECONFIG.read( EEADDR(pca[ch/16].pcaservo[ch%16].angle1) );
          uint8_t a2 = NODECONFIG.read( EEADDR(pca[ch/16].pcaservo[ch%16].angle2) );
          angle = (a1+a2)/2;
        }
        dP(" angle="); dP(angle);
        if (tServoEasingObjectPtr->attach(i, angle) == INVALID_SERVO) {
            Serial.print(F("Address=0x"));
            Serial.print(aPCA9685I2CAddress, HEX);
            Serial.print(F(" i="));
            Serial.print(i);
            Serial.println(F(" Error attaching servo - maybe MAX_EASING_SERVOS=" STR(MAX_EASING_SERVOS) " is to small to hold all servos"));
        } else {
            uint16_t sdelay = NODECONFIG.read( EEADDR(startupInterval) );
            delay(sdelay * 10);
            dP("\n delay "); dP(sdelay * 10);
            tServoEasingObjectPtr->setPWM(0, 4096);
        }
        while(0) {
          tServoEasingObjectPtr->easeTo(75); delay(1000);
          tServoEasingObjectPtr->easeTo(135); delay(1000);
        }
    }
}

void endOfMove(ServoEasing* servo);
void pcaInit() {
  doreattach = NODECONFIG.read( EEADDR( doreattach ) );
  if( i2cexists(0x40) ) i2chubexists = 0;
  else if( i2cexists(0x70) ) i2chubexists = 1;
  dP("\nPCA Init");
  for(uint8_t i=0; i<NUM_PCA; i++) {
    if( i2cexists(PCA_ADDRESSES[i]) ) pcaexists[i]=true;
    Serial.print("\n  PCA address:"); Serial.print(PCA_ADDRESSES[i],HEX);
    if( !i2cexists(PCA_ADDRESSES[i]) ) {
      Serial.print(F(" PCA9685 expander not connected -> disabled."));
      NODECONFIG.put( EEADDR( pca[i].status), "Missing"); // xxx
      pcaexists[i]=false;
    } else {
      Serial.print(F(" PCA9685 expander connected."));
      NODECONFIG.put( EEADDR( pca[i].status), "OK"); // xxx
      getAndAttach16ServosToPCA9685Expander(i*16, PCA_ADDRESSES[i]);
    }
  }
  setEasingTypeForAllServos(EASE_CUBIC_IN_OUT); //
  for(int ch=0; ch<NUM_PCA_SERVO; ch++) {
    if( pcaexists[ch/16] ) {
        uint8_t speed = NODECONFIG.read( EEADDR( pca[ch/16].pcaservo[ch%16].speed ) ); 
        ServoEasing::ServoEasingArray[ch]->setSpeed(speed);
        ServoEasing::ServoEasingArray[ch]->setTargetPositionReachedHandler(endOfMove); 
    }
    pcastate[ch]=2;
  }
  Serial.print("\n End of PCA Init");
}
void pcawrite(int ch, int angle) {
  //ServoEasing::ServoEasingArray[i]->easeTo(angle);
  dP("\n pcawrite");
  if( pcaexists[ch/16] ) {
    dP(" startEaseTo "); dP(angle); dP("\n");
    //if(doreattach && !ServoEasing::ServoEasingArray[ch]->attached()) {
    //  dP("\n Channel "); dP(ch); dP(" reattached");
    //  ServoEasing::ServoEasingArray[ch]->reattach();
    //}
    dP("\n pcawrite ch="); dP(ch); dP(" easeTo angle="); dP(angle);
    ServoEasing::ServoEasingArray[ch]->startEaseTo(angle);
  }
}
/*
void processPCA() {
  static long last = 0;
  if( (millis()-last) < 100 ) return;
  last = millis();
  //uint8_t s = 0;
  for(int ch=0; ch<NUM_PCA_SERVO; ch++) {
    if( !ServoEasing::ServoEasingArray[ch]->isMoving() ) {
      //dP("\n pca "); dP(i); dP(" is stopped.");
      if( ServoEasing::ServoEasingArray[ch]->getCurrentAngle() == target[ch] ) {
        dP(" at target, so shutdown the PWM.");
        ServoEasing::ServoEasingArray[ch]->setPWM(0, 4096);
        continue;
      }
      uint8_t t1 = NODECONFIG.read( EEADDR(pca[ch/16].pcaservo[ch%16].angle1) );
      uint8_t t2 = NODECONFIG.read( EEADDR(pca[ch/16].pcaservo[ch%16].angle2) );
      uint8_t mdpt = (t1+t2)/2;
      if( ServoEasing::ServoEasingArray[ch]->getCurrentAngle() == mdpt ) {
        if( ServoEasing::ServoEasingArray[ch]->getCurrentAngle() < target[ch] ) {
          dP("\n is going up");
          OpenLcb.produce(pcabase+ch*4+3);
        } else {
          dP("\n is going down");
          OpenLcb.produce(pcabase+ch*4+3);
        }
        pcawrite(ch, target[ch]);
      }
      //dP("\n continue to the target: pcaservo "); dP(ch/16); dP(":"); dP(ch%16); dP(" to "); dP(target[ch]);
    }
  }
}
*/

// this routine is called when a servo reaches its endpoint
void endOfMove(ServoEasing* servo) {
    int servoIndex = -1;
    // Iterate through the global array to find a pointer match
    for (int ch = 0; ch < NUM_PCA_SERVO; ch++) {
        if (servo == ServoEasing::ServoEasingArray[ch]) {
            servoIndex = ch;  // the matching servo's index
            break;
        }
    }
    dP("\n Found index = "); dP(servoIndex);
    if (servoIndex != -1) { // if found
      uint8_t a1 = NODECONFIG.read( EEADDR(pca[servoIndex/16].pcaservo[servoIndex%16].angle1) );
      uint8_t a2 = NODECONFIG.read( EEADDR(pca[servoIndex/16].pcaservo[servoIndex%16].angle2) );
      uint8_t mp = (a1+a2)/2;  // calulate the midpoint
      if( servo->getCurrentAngle() == mp ) {
        if( mp<target[servoIndex] ) OpenLcb.produce(pcabase+servoIndex*4+2); // if going up, send the up-event
        if( mp>target[servoIndex] ) OpenLcb.produce(pcabase+servoIndex*4+3); // if going down, send the down-event
        ServoEasing::ServoEasingArray[servoIndex]->setEasingType(EASE_CUBIC_OUT);
        pcawrite(servoIndex, target[servoIndex]);  // finish the move
      } else { // we assume its at the endpoint
        dP("\n at end point ->");
        if( doreattach) {
          //ServoEasing::ServoEasingArray[servoIndex]->detach();
          //ServoEasing::ServoEasingArray[servoIndex]->setPWM(0, 4096);
          servo->setPWM(0, 4096);
          dP(" turnoff");
        }
        else dP(" leave active");
      }
    } 
}

// ===== Process Consumer-eventIDs =====
uint8_t getType(uint16_t ch) {
  if(ch<NUM_NATIVE_IO) return NODECONFIG.read( EEADDR( natio[ch].type) );
  ch -= NUM_NATIVE_IO;
  return NODECONFIG.read( EEADDR( mcp[ch/16].io[ch%16].type) );
}
uint8_t getDurn(uint16_t ch) {
  if(ch<NUM_NATIVE_IO) return NODECONFIG.read( EEADDR( natio[ch].duration) );
  ch -= NUM_NATIVE_IO;
  return NODECONFIG.read( EEADDR( mcp[ch/16].io[ch%16].duration) );
}
uint8_t getPeriod(uint16_t ch) {
  if(ch<NUM_NATIVE_IO) return NODECONFIG.read( EEADDR( natio[ch].period) );
  ch -= NUM_NATIVE_IO;
  return NODECONFIG.read( EEADDR( mcp[ch/16].io[ch%16].period) );
}
void pceCallback(uint16_t index) {
// Invoked when an event is consumed; drive pins as needed
// from index of all events.
// Sample code uses inverse of low bit of pattern to drive pin all on or all off.
// The pattern is mostly one way, blinking the other, hence inverse.
//
  dP("\npceCallback, index="); dP((uint16_t)index);
  
  dP("\nless servo index="); dP((uint16_t)index);
  if( index< ( NUM_NAT_IO_EVENT+NUM_MCP_EVENT) ) {
    dP("\nnative and mcp io");
    dP("\n  NUM_NAT_IO_EVENT+NUM_MCP_EVENT="); dP(index);
    int ch = index/2;
    
    uint8_t type = getType(ch);
    if(ch<NUM_NATIVE_IO) { dP("\n Native io "); dP(ch); PV(type); }
    else  { dP("\n MCP io "); dP(ch-NUM_NATIVE_IO); PV(type); }
    if(type>=7) {
      // 7=PA 8=PAI 9=PB 10=PBI
      bool inv = !(type&1);       // inverted
      bool pha = type<9;          // phaseA
      if( index%2 ) {
        dP("\noff"); PV(ch); PV(type); PV(inv);
        digOut(ch, inv);
        next[ch] = 0;
      } else {
        dP("\nON"); PV(ch); PV(pha); PV(inv); 
        digOut(ch, pha ^ inv);
        iostate[ch] = 1;
        uint8_t durn;
        durn = getDurn(ch);
        PV(durn);
        if(durn) next[ch] = millis() + 100*durn; // note duration==0 means forever
        else next[ch]=0;
      }
      PV((uint32_t)millis()); PV((uint32_t)next[ch]);
    }
    return;
  }
  // PCA9685 Servos
  index -= ( NUM_NAT_IO_EVENT+NUM_MCP_EVENT);    // adjust index   
  dP("\nless IO+mcp servos index="); dP((uint16_t)index);      
  if( index<NUM_PCA_SERVO_EVENT) {
    dP("\n NUM_PCA_SERVO_EVENT="); dP(index);
    uint8_t i = index/(16*4);       // which PCA board  0->1
    uint8_t ch = index/4;           // which channel    0->32
    uint8_t e = index%4;            // which consumer event 0->3
    uint8_t speed = NODECONFIG.read( EEADDR(pca[ch/16].pcaservo[ch%16].speed) );
    if(speed==0) speed=1;
    if(speed>100) speed=100;
    ServoEasing::ServoEasingArray[ch]->setSpeed(speed);
    uint8_t t1 = NODECONFIG.read( EEADDR(pca[ch/16].pcaservo[ch%16].angle1) ); // get the angle-positions for this channel
    uint8_t t2 = NODECONFIG.read( EEADDR(pca[ch/16].pcaservo[ch%16].angle2) );
    dP("\n pca board="); dP(i); dP(" channel="); dP(ch); dP(" eid#="); dP(e);
    uint8_t mdpt = (t1+t2)/2;    // call the midpoint
    dP(" posn1="); dP(t1); dP(" posn2="); dP(t2); dP(" current position="); dP(ServoEasing::ServoEasingArray[ch]->getCurrentAngle());
    if(e==0 && ServoEasing::ServoEasingArray[ch]->getCurrentAngle()!=t1) {
      target[ch] = t1;  // eventual target isangle 1, after passing thorugh the midpoint
      dP("\n move pcaservo "); dP(i); dP(":"); dP(ch); dP(" to midpoint at "); dP(mdpt); dP(" then to "); dP(t1); 
      ServoEasing::ServoEasingArray[ch]->setEasingType(EASE_CUBIC_IN);
      pcawrite(ch, mdpt); // go to the midpoint, endOfMove() will be called when it reaches the midpoint
    }
    if(e==1 && ServoEasing::ServoEasingArray[ch]->getCurrentAngle()!=t2) {
      target[ch] = t2;  // eventual target is angle 2, after passing thorugh the midpoint
      dP("\n move pcaservo "); dP(i); dP(":"); dP(ch); dP(" to midpoint at "); dP(mdpt); dP(" then to "); dP(t2); 
      ServoEasing::ServoEasingArray[ch]->setEasingType(EASE_CUBIC_IN);
      pcawrite(ch, mdpt); // go to the midpoint, endOfMove() will be called when it reaches the midpoint
    }
  }
}

// === Process servos ===
// This is called from loop to service the servos
bool posdirty = false;


// ==== Process Inputs ====
void produceFromInputs() {
    // called from loop(), this looks at changes in input pins and
    // and decides which events to fire
    // with pce.produce(i);
    const uint8_t base =  0;
    static uint8_t c = 0;
    static unsigned long last = 0;
    if((millis()-last)<(50/NUM_IO)) return;
    last = millis();
    //uint8_t type = NODECONFIG.read(EEADDR(io[c].type));
    uint8_t type = getType(c);
    uint8_t d;
    if(type==5 || type==6) {
      bool inv = type==6;
      bool s = inv^digIn(c);
      //dP("\n"); PV(c); PV(type); PV(s); PV(iostate[c]);
      if(s != iostate[c]) {  // !s ??
        iostate[c] = s;
        if(s) {
          logstate[c] ^= 1;
          if(logstate[c]) d = getDurn(c); //NODECONFIG.read(EEADDR(io[c].duration));
          else            d = getPeriod(c); //NODECONFIG.read(EEADDR(io[c].period));
          dP("\ntoggle "); PV(c); PV(type); PV(s); PV(logstate[c]); PV(d);
          if(d==0) OpenLcb.produce( base+c*2 + !logstate[c] ); // if no delay send the event
          else next[c] = millis() + (uint16_t)d*100;          // else register the delay
        }
      }
    }
    if(type>0 && type<5) {
      bool inv = !(type&1);
      bool s = inv ^ digIn(c);
      if(s != iostate[c]) {
        dP("\n type1-4"); PV(c); PV(type); PV(s);
        iostate[c] = s;
        //d = s ? NODECONFIG.read(EEADDR(io[c].duration)) : NODECONFIG.read(EEADDR(io[c].period));
        d = s ? getDurn(c) : getPeriod(c);
        dP("\ninput delay?"); PV(type); PV(s); PV(d);
        if(d==0) OpenLcb.produce( base+c*2 + !s ); // if no delay send event immediately
        else {
          next[c] = millis() + (uint16_t)d*100;                   // else register the delay
          //PV((uint32_t)millis()); PV((uint32_t)next[c]);
        }
      }
    }
    if(++c>=NUM_IO) c = 0;
}

// Process pending producer events
// Called from loop to service any pending event waiting on a delay
void processProducer() {
  const uint8_t base =  0;
  static unsigned long last = 0;
  unsigned long now = millis();
  if( (now-last) < 50 ) return;
  for(int c=0; c<NUM_IO; c++) {
    if(next[c]==0) continue;
    if(now<next[c]) continue; 
    //uint8_t type = NODECONFIG.read(EEADDR(io[c].type));
    uint8_t type = getType(c);
    if(type>6) return; // do not process outputs
    uint8_t s = iostate[c];
    //dP("\nproducer"); PV(type); PV(s); PV((!s^(type&1)));
//    if(type<5)  OpenLcb.produce( base+c*2 + !(s^(type&1)) ); // reg inputs
//    else OpenLcb.produce( base+c*2 + logstate[c] );          // toggle inputs
    if(type<5)  OpenLcb.produce( base+c*2 + !s ); // reg inputs
    else OpenLcb.produce( base+c*2 + !logstate[c] );          // toggle inputs
    next[c] = 0;
    dP("\nend of processProducer");
  }
}

void userSoftReset() {}
void userHardReset() {}

NodeID nodeid(NODE_ADDRESS);  // this node's nodeid, do not move
#include "OpenLCBMid.h"    // Essential, do not move or delete

void pcaServoSet(uint32_t address) {
  for(int s=0; s<NUM_PCA_SERVO; s++) {
    if(address==EEADDR(pca[s/16].pcaservo[s%16].angle1)) { 
      //if(doreattach) ServoEasing::ServoEasingArray[s]->reattach();
      ServoEasing::ServoEasingArray[s]->setEasingType(EASE_CUBIC_OUT);
      pcawrite(s, NODECONFIG.read( EEADDR(pca[s/16].pcaservo[s%16].angle1) ) );  
      //if(doreattach) ServoEasing::ServoEasingArray[s]->detach();
      return;
    }
    if(address==EEADDR(pca[s/16].pcaservo[s%16].angle2)) { 
      //if(doreattach) ServoEasing::ServoEasingArray[s]->reattach();
      ServoEasing::ServoEasingArray[s]->setEasingType(EASE_CUBIC_OUT);
      pcawrite(s, NODECONFIG.read( EEADDR(pca[s/16].pcaservo[s%16].angle2) ) );  
      //if(doreattach) ServoEasing::ServoEasingArray[s]->detach();
      return;
    }
  }
}
// Callback from a Configuration write
// Use this to detect changes in the node's configuration
// This may be useful to take immediate action on a change.
void userConfigWritten(uint32_t address, uint16_t length, uint16_t func)
{
  dPS("\nuserConfigWritten: Addr: ", (uint32_t)address);
  dPS(" Len: ", (uint16_t)length);
  dPS(" Func: ", (uint8_t)func);
  setupIOPins();
  pcaServoSet(address);
  EEPROMcommit;
}


// Setup the io pins
// called by setup() and after a configuration change
void setupIOPins() {
  dP("\nPins: ");
  for(uint8_t i=0; i<NUM_IO; i++) {
    //uint8_t type = NODECONFIG.read( EEADDR(io[i].type));
    uint8_t type = getType(i);
    dP("\n "); dP(i); dP(" type="); dP(type);
    switch (type) {  // No PULLUP
      case 1: case 2: case 5:
        if(type==1) dP(" IN");
        if(type==2) dP(" INI");
        if(type==5) dP(" TOG");
        setMode(i, INPUT);
        iostate[i] = type&1;
        if(type==5) iostate[i] = 0;
        break;
      case 3: case 4: case 6: // PULLUPS
        if(type==3) dP(" INP");
        if(type==4) dP(" INPI");
        if(type==6) dP(" TOGP");
        setMode(i, INPUT_PULLUP);
        iostate[i] = type&1;
        break;
      case 7: case 8: case 9: case 10:
        if(type==7) dP(" PA");
        if(type==8) dP(" PAI");
        if(type==9) dP(" PB");
        if(type==10) dP(" PBI");
        setMode(i, OUTPUT);
        bool inv = !(type&1);
        iostate[i] = inv;
        digOut(i, inv);
        break;
    }
    if(i<NUM_NATIVE_IO) { dP("\n native "); dP(iopin[i]); dP(":"); dP(type); }
    else { dP("\n mcp "); dP(i-NUM_NATIVE_IO); dP(":"); dP(type); }
  }
}

// Process IO pins
// called by loop to implement flashing on io pins
void appProcess() {
  uint8_t base =  0;
  unsigned long now = millis();
  for(int i=0; i<NUM_IO; i++) {
    //uint8_t type = NODECONFIG.read(EEADDR(io[i].type));
    uint8_t type = getType(i);
    if(type >= 7) {
      if( next[i] && now>next[i] ) {
        //dP("\nappProcess "); PV((uint32_t)now);
        bool inv = !(type&1);
        bool phb = type>8;
        if(iostate[i]) {
          // phaseB
          dP("\nphaseB"); PV(i); PV(phb); PV(inv); PV(phb ^ inv);
          digOut(i, phb ^ inv);
          iostate[i] = 0;
          //if( NODECONFIG.read(EEADDR(io[i].period)) > 0 ) 
          if( getPeriod(i) > 0 ) 
          //next[i] = now + 100*NODECONFIG.read(EEADDR(io[i].period));
          next[i] = now + 100 * getPeriod(i);
          else next[i] = 0;
            PV((uint32_t)next[i]);
        } else {
          // phaseA
          dP("\nphaseA"); PV(i); PV(phb); PV(inv); PV(!phb ^ inv);
          digOut(i, !phb ^ inv);
          iostate[i] = 1;
          //if( NODECONFIG.read(EEADDR(io[i].duration)) > 0 )
          if( getDurn(i) > 0 )
            //next[i] = now + 100*NODECONFIG.read(EEADDR(io[i].duration));
            next[i] = now + 100 * getDurn(i);
          else next[i] = 0;
            //PV((uint32_t)next[i]);
        }
      }
    }
  }
}

// ==== Setup does initial configuration ======================
void setup()
{
  #ifdef DEBUG
    #define dP(...) Serial.print(__VA_ARGS__)
    Serial.begin(115200); while(!Serial) delay(100); delay(2000);
    dP("\n 2Servo8IO2MCP2PCA");
    dP("\n num native io = "); dP(NUM_NATIVE_IO);
    dP("\n num mcp groups = "); dP(NUM_MCP_PORTS);
    dP("\n num io in each mcp group = "); dP(NUM_MCP_IO_PER_PORT);
    dP("\n num mcp io = "); dP(NUM_MCP_IO);
    //dP("\n mcp23017-1 address = "); dPH(MCP_ADDRESS1);
    //dP("\n mcp23017-2 address = "); dPH(MCP_ADDRESS2);
    for(int i=0; i<NUM_MCP; i++) { dP("\n mcp23017("); dP(i); dP(") address = "); dPH(MCP_ADDRESSES[i]); }
    //dP("\n pca9685-1 address = "); dPH(PCA_ADDRESS1);
    //dP("\n pca9685-2 address = "); dPH(PCA_ADDRESS2);
    for(int i=0; i<NUM_PCA; i++) { dP("\n pca9685("); dP(i); dP(") address = "); dPH(PCA_ADDRESSES[i]); }
    dP("\n total num io = "); dP(NUM_IO);
    dP("\n num events = "); dP(NUM_EVENT);
    dP("\n num native io events = "); dP( NUM_NAT_IO_EVENT);
    dP("\n num mcp events = "); dP(NUM_MCP_EVENT);
    dP("\n num pca servo events = "); dP(NUM_PCA_SERVO_EVENT);
    dP("\n size of MemStruct = "); dP(sizeof(MemStruct));
  #endif

  WIRE_begin;                        // defined in boards.h
  EEPROMbegin;

  NodeID nodeid(NODE_ADDRESS);       // this node's nodeid
  Olcb_init(nodeid, RESET_TO_FACTORY_DEFAULTS);
  
  mcpinit();
  setupIOPins();
  pcaInit();
  dP("\n NUM_EVENT="); dP(NUM_EVENT);
  

  #if 0
      dP("\n Configuration:");
      dP("\n"); dP( 8+EEADDR( nodeName ),HEX );      dP(" nodeName=");      dP( NODECONFIG.read( EEADDR(nodeName) ) );
      dP("\n"); dP( 8+EEADDR( nodeDesc ),HEX );      dP(" nodeDesc=");      dP( NODECONFIG.read( EEADDR(nodeDesc) ) );
      dP("\nNative and MCP IO:");
      for(int i=0; i<NUM_IO; i++) {
        dP("\n"); dP(i); 
        dP("\n   "); dP( 8+EEADDR( io[i].type ),HEX );     dP(" io[i].type=");     dP( NODECONFIG.read( EEADDR( io[i].type ) ), HEX );
        dP("\n   "); dP( 8+EEADDR( io[i].duration ),HEX ); dP(" io[i].duration="); dP( NODECONFIG.read( EEADDR( io[i].duration ) ), HEX );
        dP("\n   "); dP( 8+EEADDR( io[i].period ),HEX );   dP(" io[i].period=");   dP( NODECONFIG.read( EEADDR( io[i].period ) ), HEX );
      }
      dP("\nPDA Servos:");
      for(int i=0; i<NUM_PCA_SERVO; i++) {
        dP("\n"); dP(i); 
        dP("\n   "); dP( 8+EEADDR( pca[i/16].pcaservo[i%16].speed ),HEX );  dP(" speed=");   dP( NODECONFIG.read( EEADDR( pca[i/16].pcaservo[i%16].speed ) ), HEX );
        dP("\n   "); dP( 8+EEADDR( pca[i/16].pcaservo[i%16].angle1 ),HEX ); dP(" angle1 ="); dP( NODECONFIG.read( EEADDR( pca[i/16].pcaservo[i%16].angle1 ) ), HEX );
        dP("\n   "); dP( 8+EEADDR( pca[i/16].pcaservo[i%16].angle2 ),HEX ); dP(" angle2 ="); dP( NODECONFIG.read( EEADDR( pca[i/16].pcaservo[i%16].angle2 ) ), HEX );
      }
      dP("\n"); dP( EEADDR( pcaStartupPosition ) ); dP(" selected pcaStartupPosition="); dP( NODECONFIG.read( EEADDR( pcaStartupPosition ) ) );
  #endif


 }

// ==== Loop ==========================
void loop() {
  bool activity = Olcb_process();
  #ifndef OLCB_NO_BLUE_GOLD
    if (activity) {
      blue.blink(0x1); // blink blue to show that the frame was received
    }
    if (olcbcanTx.active) {
      gold.blink(0x1); // blink gold when a frame sent
      olcbcanTx.active = false;
    }
    // handle the status lights
    gold.process();
    blue.process();
  #endif // OLCB_NO_BLUE_GOLD
  produceFromInputs();  // scans inputs and generates events on change
  appProcess();         // processes io pins, eg flashing
  processProducer();    // processes delayed producer events from inputs
  //processPCA();
 #if 1
  #if defined(DEBUG) && !defined(USEGCSERIAL) 
    if(Serial.available()) {
      char c = Serial.read();
      userUI(c);
    }
  #endif
 #endif


}

#if defined(DEBUG)
void userUI(char c) {
  if(c=='\n') return;
  if(c=='\r') return;
  dP("\n User UI: char="); dP(c);
if(c=='I') {  // io
      char cc = Serial.read();
      switch (cc) {
        case 's': {
            uint8_t i = Serial.parseInt(); // io#
            if(i>=NUM_IO) break;
            uint8_t t = Serial.parseInt();
            uint8_t d = Serial.parseInt();
            uint8_t p = Serial.parseInt();
            NODECONFIG.write( EEADDR( natio[i].type), t );
            NODECONFIG.write( EEADDR( natio[i].duration), d );
            NODECONFIG.write( EEADDR( natio[i].period), p );
            EEPROM.commit();
            dP("\n ==> IO "); dP(i); dP(" type="); dP(t); dP(" durn="); dP(d); dP(" period="); dP(p);
          }
          break;
        case 'o': {
            uint8_t i = Serial.parseInt(); // io#
            if(i>=NUM_IO) break;
            dP("\n ==> Turn IO "); dP(i); dP(" ON");
            pceCallback( i*2 );
          }
          break;
        case 'f': {
            uint8_t i = Serial.parseInt(); // io#
            if(i>=NUM_IO) break;
            pceCallback( i*2+1 );
          }
          break;
        case 'l': {
            for(uint8_t i=0; i<NUM_IO; i++) {
              uint8_t t = NODECONFIG.read( EEADDR( natio[i].type));
              uint8_t d = NODECONFIG.read( EEADDR( natio[i].duration));
              uint8_t p = NODECONFIG.read( EEADDR( natio[i].period));
              dP("\n"); dP(i); dP(" type="); dP(t); dP(" durn="); dP(d); dP(" period="); dP(p);
            }
          }
          break;
      }

  } else if(c=='M') {  // MCP
      char cc = Serial.read();
      switch (cc) {
        case 's': {
            uint8_t mi = Serial.parseInt(); // io#
            if(mi>=NUM_MCP_IO) break;
            uint8_t i = mi + NUM_NATIVE_IO; // skip native io
            uint8_t t = Serial.parseInt();
            uint8_t d = Serial.parseInt();
            uint8_t p = Serial.parseInt();
            NODECONFIG.write( EEADDR( mcp[i/16].io[i%16].type), t );
            NODECONFIG.write( EEADDR( mcp[i/16].io[i%16].duration), d );
            NODECONFIG.write( EEADDR( mcp[i/16].io[i%16].period), p );
            EEPROM.commit();
            dP("\n ==> MCP IO "); dP(mi); dP(" io#="); dP(i); dP(" type="); dP(t); dP(" durn="); dP(d); dP(" period="); dP(p);
          }
          break;
        case 'o': {
            uint8_t mi = Serial.parseInt(); // io#
            if(mi>=NUM_MCP_IO) break;
            uint8_t type = NODECONFIG.read( EEADDR(mcp[mi/16].io[mi%16].type) );
            dP(" type="); dP(type);
            if(type<7) fakeinput = 1;
            else {
              dP("\n ==> Turn Mcp IO "); dP(mi); dP(" ON");
              pceCallback( NUM_NAT_IO_EVENT + mi*2 );
            }
          }
          break;
        case 'f': {
            uint8_t mi = Serial.parseInt(); // io#
            if(mi>=NUM_MCP_IO) break;
            uint8_t type = NODECONFIG.read( EEADDR(mcp[mi/16].io[mi%16].type) );
            dP(" type="); dP(type);
            if(type<7) fakeinput = 0;
            else {
              dP("\n ==> Turn Mcp IO "); dP(mi); dP(" OFF");
              pceCallback( NUM_NAT_IO_EVENT + mi*2+1 );
            }
          }
          break;
        case 'l': {
            for(uint8_t mi=0; mi<NUM_MCP_IO; mi++) {
              uint8_t i= mi+NUM_NATIVE_IO;
              uint8_t t = NODECONFIG.read( EEADDR( mcp[i/16].io[i%16].type));
              uint8_t d = NODECONFIG.read( EEADDR( mcp[i/16].io[i%16].duration));
              uint8_t p = NODECONFIG.read( EEADDR( mcp[i/16].io[i%16].period));
              dP("\n"); dP(mi); dP(" io#="); dP(i); dP(" type="); dP(t); dP(" durn="); dP(d); dP(" period="); dP(p);
            }
        }
        break;
      }

  } else if(c=='P') {  // PCA
      char cc = Serial.read();
      switch (cc) {
        case 'a': { // set angles
            uint8_t s = Serial.parseInt();
            if(s>=NUM_PCA_SERVO) break;
            uint8_t angle1 = Serial.parseInt();
            uint8_t angle2 = Serial.parseInt();
            NODECONFIG.write( EEADDR( pca[s/16].pcaservo[s%16].angle1), angle1 );
            NODECONFIG.write( EEADDR( pca[s/16].pcaservo[s%16].angle2), angle2 );
            EEPROM.commit();
            dP("\n ==> Servo "); dP(s); dP(" angle1="); dP(angle1); dP(" angle2="); dP(angle2);
          }
          break; 
        case 'm': { // move servo position
            uint8_t s = Serial.parseInt();
            if(s>=NUM_PCA_SERVO) break;
            uint8_t p = Serial.parseInt();
            if(p>1) break;
            dP("\n ==> set servo "); dP(s); dP(" to position "); dP(p);
            if(p<=0) pceCallback( NUM_IO_EVENT + s*4 );
            if(p>=1) pceCallback( NUM_IO_EVENT + s*4+1 );
          }
          break;
        case 'v': { // speed
            uint8_t s = Serial.parseInt();
            if(s>=NUM_PCA_SERVO) break;
            uint8_t v = Serial.parseInt();
            NODECONFIG.write( EEADDR( pca[s/16].pcaservo[s%16].speed), v);
            EEPROM.commit();
            dP("\n ==> Servo "); dP(s); dP(" speed set to "); dP(v);
          }
          break;
        case 'l': // list
          dP("\n\nPCA Servos");
          for(int s=0; s<NUM_PCA_SERVO; s++) {
            if( !pcaexists[s/16] ) continue;
            dP("\n"); dP(s); 
            dP(" angle1="); dP(NODECONFIG.read( EEADDR( pca[s/16].pcaservo[s%16].angle1))); 
            dP(" angle2="); dP(NODECONFIG.read( EEADDR( pca[s/16].pcaservo[s%16].angle2))); 
          }
          break;
        case 'c': { // set init config
            uint8_t o = Serial.parseInt();
            if(o==0) {
              NODECONFIG.write( EEADDR( pcaStartupPosition), 0);
              EEPROM.commit();
              dP("\n ==> PCA Startup position is 90 degree");
            }
            if(o==1) {
              NODECONFIG.write( EEADDR( pcaStartupPosition), 1);
              EEPROM.commit();
              dP("\n ==> PCA Startup position is midpoint");
            }
          }
          break;
      }
  } else {
      dP("\nHelp: ");
      dP("\n To send an event: :X195B4123N050101018E03004C;");
      dP("\nNative Servos:");
      dP("\n  Ss: set native servo speed: Ss speed(4-50)");
      dP("\n  Sa: set native servo angles: Sa servo#(0-1) angle1 angle2 angle3 (0-180)");
      dP("\n  Sm: move native servo: Sm servo#(0-1) position(0-2)");
      dP("\n  Sl: list the servos and their positions");
      dP("\nNative:");
      dP("\n  Is: set IO parameters: Is IO# type durn period");
      dP("\n  Io: set IO ON:  Io IO#");
      dP("\n  If: set IO OFF: If IO#");
      dP("\nMCP23017 IO:");
      dP("\n  Ms: set IO parameters: Is IO# type durn period");
      dP("\n  Mo: set IO ON:  Io IO#");
      dP("\n  Mf: set IO OFF: If IO#");
      dP("\n  Ml: list mcp io");
      dP("\nPCA9685 Servos:");
      dP("\n  Pa: set angle: a servo# pos#(1-2) angle(0-180)");
      dP("\n  Pm: move servo: s servo# posn#(1-2)");
      dP("\n  Pv: set servo speed: v servo# speed(1-50)");
      dP("\n  Pi: intiitailize position: i 0 = 90 degress, i 1 = midpoint");
      dP("\n  Pc: configuration: 0=90, 1=midpoint");
  }
}
#endif

