
#ifndef CONFIG_H
#define CONFIG_H

// To set a new nodeid edit the next line
#define NODE_ADDRESS  0x05,0x01,0x01,0x01,0x8E,0x01  // must be unique address owned by you for DIY

// To Force Reset EEPROM to Factory Defaults set this value to 1, else 0.
// Need to do this at least once.
#define RESET_TO_FACTORY_DEFAULTS 1

/*

  ===================================================================================================
  End of basic users configerations, anything below this is for advanced user to learn how the sketch 
  is setup. Change any of these values may break the Node
  ===================================================================================================
*/

// Choose a board, uncomment one line, see boards.h

#define ESP32_BOARD // ESP32_BOARD ESP32 Dev 
//#define ATOM_BOARD

// Debugging -- uncomment to activate debugging statements:

//#define DEBUG Serial

// Allow direct to JMRI via USB, without CAN controller, comment out for CAN

//#define USEGCSERIAL
#define NOCAN  // prevent the built in OpenLCB CAN drivers

#include "boards.h"

//#define NUM_NATIVE_IO 8 // defined in boards.h
#define NUM_NAT_IO_EVENT (NUM_NATIVE_IO*2)

const uint8_t MCP_ADDRESSES[] = {  0x20, 0x21 };
#define NUM_MCP 2
#define NUM_MCP_PORTS 4   //// calc by hand - two each on two mcp23017s
#define NUM_MCP_IO_PER_PORT 8
#define NUM_MCP_IO (NUM_MCP_PORTS * NUM_MCP_IO_PER_PORT)
#define NUM_MCP_EVENT (NUM_MCP_IO*2) 

#define NUM_IO (NUM_NATIVE_IO+NUM_MCP_IO)
#define NUM_IO_EVENT (NUM_IO*2)

//#define PCA_ADDRESS1 0x40  //// Choose address for first pca9685 board
//#define PCA_ADDRESS2 0x41  //// Choose address for second pca9685 board
const uint8_t PCA_ADDRESSES[] = { 0x40, 0x41, 0x42, 0x43 };
#define NUM_PCA 2
#define NUM_PCA_PORTS 4 // MUST BE CALCULATED BY HAND
#define NUM_PCA_SERVO_PER_PORT 8
#define NUM_PCA_SERVO (NUM_PCA_PORTS * NUM_PCA_SERVO_PER_PORT)
#define NUM_PCA_SERVO_EVENT (NUM_PCA_SERVO*4)
#define NUM_EVENT (  ( (NUM_NATIVE_IO + NUM_MCP_IO) *2 ) + ((NUM_PCA_SERVO*4)) )
//#define DISABLE_MICROS_AS_DEGREE_PARAMETER // may save space
#define DISABLE_PAUSE_RESUME // saves some memory

///////#define PCA_INIT_TO_90  // inti servo to 90 degrees, comment out if you want the servos to initialize to the midpoint. 

// Global defs
const bool USE_90_ON_STARTUP = true;  // move 

// Board definitions
#define MANU "OpenLCB"           // The manufacturer of node
#define MODEL BOARD "40IO32SVO" // The model of the board
#define HWVERSION "Universal"          // Hardware version
#define SWVERSION "1.0.0"          // Software version

  #ifdef DEBUG
    #define PV(x) { dP(" " #x "="); dP(x); }
  #else
    #define PV(x) 
  #endif

#ifdef USEGCSERIAL
  #include "GCSerial.h"
  //#undef DEBUG           // Cannot use DEBUG when using GCSerial
#endif

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

// Definitions for EIDTab
// These allow automatic regostering of eventids
// If NUM_NAT_SERVO>8 or NUM_IO>40 these will need extending
// Expands depending on NUM_NAT_SERVO and NUM_IO
#define REG_SERVO_OUTPUT(s) CEID(servos[s].pos[0].eid), CEID(servos[s].pos[1].eid), CEID(servos[s].pos[2].eid)
#define _SERVOEID_0 
#define _SERVOEID_1 REG_SERVO_OUTPUT(0)
#define _SERVOEID_2 _SERVOEID_1, REG_SERVO_OUTPUT(1)
#define _SERVOEID_3 _SERVOEID_2, REG_SERVO_OUTPUT(2)
#define _SERVOEID_4 _SERVOEID_3, REG_SERVO_OUTPUT(3)
#define _SERVOEID_5 _SERVOEID_4, REG_SERVO_OUTPUT(4)
#define _SERVOEID_6 _SERVOEID_5, REG_SERVO_OUTPUT(5)
#define _SERVOEID_7 _SERVOEID_6, REG_SERVO_OUTPUT(6)
#define _SERVOEID_8 _SERVOEID_7, REG_SERVO_OUTPUT(7)
#define _SERVOEID(n) _SERVOEID_##n
#define SERVOEID(n) _SERVOEID(n)

#define REG_IO(i) PCEID(natio[i].onEid), PCEID(natio[i].offEid)
#define _IOEID_0 
#define _IOEID_1 REG_IO(0)
#define _IOEID_2 _IOEID_1, REG_IO(1)
#define _IOEID_3 _IOEID_2, REG_IO(2)
#define _IOEID_4 _IOEID_3, REG_IO(3)
#define _IOEID_5 _IOEID_4, REG_IO(4)
#define _IOEID_6 _IOEID_5, REG_IO(5)
#define _IOEID_7 _IOEID_6, REG_IO(6)
#define _IOEID_8 _IOEID_7, REG_IO(7)
#define _IOEID_9 _IOEID_8, REG_IO(8)
#define _IOEID_10 _IOEID_9, REG_IO(9)
#define _IOEID_11 _IOEID_10, REG_IO(10)
#define _IOEID_12 _IOEID_11, REG_IO(11)
#define _IOEID_13 _IOEID_12, REG_IO(12)
#define _IOEID_14 _IOEID_13, REG_IO(13)
#define _IOEID_15 _IOEID_14, REG_IO(14)
#define _IOEID_16 _IOEID_15, REG_IO(15)
#define _IOEID_17 _IOEID_16, REG_IO(16)
#define _IOEID(n) _IOEID_##n
#define IOEID(n) _IOEID(n)

#define REG_MCPIO(i) PCEID(mcp[(i)/16].io[(i)%16].onEid), PCEID(mcp[(i)/16].io[(i)%16].offEid)
#define REG_MCP(g) REG_MCPIO(g), REG_MCPIO(g+1), REG_MCPIO(g+2), REG_MCPIO(g+3), REG_MCPIO(g+4), REG_MCPIO(g+5), REG_MCPIO(g+6), REG_MCPIO(g+7)
#define _MCPEID_0 
#define _MCPEID_1 REG_MCP(0)
#define _MCPEID_2 _MCPEID_1, REG_MCP(8)
#define _MCPEID_3 _MCPEID_2, REG_MCP(16)
#define _MCPEID_4 _MCPEID_3, REG_MCP(24)
#define _MCPEID_5 _MCPEID_4, REG_MCP(32)
#define _MCPEID_6 _MCPEID_5, REG_MCP(40)
#define _MCPEID_7 _MCPEID_6, REG_MCP(48)
#define _MCPEID_8 _MCPEID_7, REG_MCP(56)
#define _MCPEID(n) _MCPEID_##n
#define MCPEID(n) _MCPEID(n)

#define REG_SVO(s) CEID(pca[(s)/16].pcaservo[(s)%16].eid1), CEID(pca[(s)/16].pcaservo[(s)%16].eid2), PEID(pca[(s)/16].pcaservo[(s)%16].eidup), PEID(pca[(s)/16].pcaservo[(s)%16].eiddown) 
#define REG_PCA(g) REG_SVO(g+0), REG_SVO(g+1), REG_SVO(g+2), REG_SVO(g+3), REG_SVO(g+4), REG_SVO(g+5), REG_SVO(g+6), REG_SVO(g+7)
#define _PCAEID_0 
#define _PCAEID_1 REG_PCA(0)
#define _PCAEID_2 _PCAEID_1, REG_PCA(8)
#define _PCAEID_3 _PCAEID_2, REG_PCA(16)
#define _PCAEID_4 _PCAEID_3, REG_PCA(24)
#define _PCAEID_5 _PCAEID_4, REG_PCA(32)
#define _PCAEID_6 _PCAEID_5, REG_PCA(40)
#define _PCAEID_7 _PCAEID_6, REG_PCA(48)
#define _PCAEID_8 _PCAEID_7, REG_PCA(56)
#define _PCAEID(n) _PCAEID_##n
#define PCAEID(n) _PCAEID(n)


// add MCP23017
#include <MCP23017.h>
MCP23017* mcp[NUM_MCP];

#define USE_PCA9685_SERVO_EXPANDER    // Activating this enables the use of the PCA9685 I2C expander chip/board.
#define MAX_EASING_SERVOS NUM_PCA_SERVO
#include "ServoEasing.hpp"
#define NUMBER_OF_SERVOS  MAX_EASING_SERVOS
#define ENABLE_EASE_CUBIC


#endif // CONFIG_H

