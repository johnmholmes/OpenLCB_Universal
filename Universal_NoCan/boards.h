
// Board definitions
// Add a new variant by copying the appropriate section and changing the pin definitions, etc. 

#if defined(ESP32_DAVIDE) // C6 GPIO0, GPIO1, GPIO2, GPIO3, GPIO16, GPIO17, GPIO18, GPIO19, GPIO20, GPIO21, GPIO22, GPIO23.
  #pragma message ("ESP32_DAVIDE")
  #define BOARD "ESP32"
  //#define NUM_SERVOS 2
  #define SERVOPINS     2,3
  #define NUM_NATIVE_IO  8
  #define IOPINS        16,17,18,19,20,21,22,23
  #define NUM_IO        74 // calc by hand 8+64
  #define CAN_TX_PIN (gpio_num_t) 2 // or 20
  #define CAN_RX_PIN (gpio_num_t) 3 // or 21
  #define ESPcan_TXPIN 2
  #define ESPcan_RXPIN 15
  #ifndef USEGCSERIAL
    //#include "ACAN_ESP32Can.h"
  #endif // USEGCSERIAL
  #include <ESP32Servo.h>
  #define EEPROMSIZE 4096
  #define EEPROMbegin EEPROM.begin(EEPROMSIZE); // this sets the size of the emulated eeprom. 
  #define EEPROMcommit EEPROM.commit();
  #define WIRE_begin Wire.begin(6, 7, 100000) // SDA, SCL  (Adafruit C6 Feather uses 19,18)
//
#elif defined(ESP32_BOARD)
  #pragma message ("ESP32_BOARD")
  #ifndef ARDUINO_ARCH_ESP32
    #error message("USE_ESP32_BOARD was selected, so must use an ESP32 processor")
  #endif
  #define BOARD "ESP32"
  //#define NUM_SERVOS 2
  #define SERVOPINS     32, 33
  #define NUM_NATIVE_IO  8
  #define IOPINS        16,17,18,19,14,27,26,25
  #define NUM_IO        74 // calc by hand 8+64
  #define CAN_TX_PIN (gpio_num_t) 2
  #define CAN_RX_PIN (gpio_num_t) 15
  #define ESPcan_TXPIN 2
  #define ESPcan_RXPIN 15
  #ifndef USEGCSERIAL
    #include "ACAN_ESP32Can.h"
  #endif // USEGCSERIAL
  #include <ESP32Servo.h>
  #define EEPROMSIZE 4096
  #define EEPROMbegin EEPROM.begin(EEPROMSIZE); // this sets the size of the emulated eeprom. 
  #define EEPROMcommit EEPROM.commit();
  #define WIRE_begin Wire.begin(21, 22, 100000)
//
#elif defined(NANO_BOARD)
  #ifndef ARDUINO_ARCH_AVR
    #error Choose the Nano
  #endif
  #pragma message ("NANO_BOARD")
  #define BOARD "Nano"
  //#define NUM_SERVOS 2
  #define SERVOPINS     A4,A5
  #define NUM_NATIVE_IO 11
  #define IOPINS        13,4,5,6,7,8,9,A0,A1,A2,A3
  #define NUM_IO        43 // calc by hand 11+32
  #ifndef USEGCSERIAL
    #define MCP2515_CS  = 10 ; 
    #define MCP2515_INT =  3 ;
    #define CAN_TX_PIN 99
    #define CAN_RX_PIN 99
    #include <ACAN2515.h>
  #endif // USEGCSERIAL
  #include <Servo.h>
  //#define EEPROMbegin EEPROM.begin(1000);
  #define EEPROMbegin 
  #define EEPROMcommit 
  #define WIRE_begin 
//
//
//
//
//  For David H
#elif defined(ATOM_BOARD)
  #pragma message ("ATOM_BOARD")
  #ifndef ARDUINO_M5STACK_ATOM
    #error message("USE_ATOM was selected, so must use a M5Stack Atom processor.")
  #endif
  #define BOARD "Atom"
  //#define NUM_SERVOS 2
  #define SERVOPINS     23,33    
  #define NUM_NATIVE_IO 3
  #define IOPINS        39,22,19  // 39 is the top button, the led is a smart RGB
  #define NUM_IO        67 // calc by hand 2+64
  #define CAN_TX_PIN    (gpio_num_t)26
  #define CAN_RX_PIN    (gpio_num_t)32
  #ifndef USEGCSERIAL
    #include "ACAN_ESP32Can.h"
  #endif // USEGCSERIAL
  #include <ESP32Servo.h>
  #define EEPROMSIZE 4096
  #define EEPROMbegin EEPROM.begin(EEPROMSIZE);
  #define EEPROMcommit { EEPROM.commit(); dP("EEPROM COMMIT"); }
  #define WIRE_begin Wire.begin(26, 32, 100000)  // choose pins for I2C // SDA, SCL
//
#elif defined(ATOM_BOARD_WITH_HAT)
  #pragma message ("ATOM_BOARD_WITH_HAT")
  #ifndef ARDUINO_M5STACK_ATOM
    #error message("ATOM_BOARD_WITH_HAT was selected, so must use a M5Stack Atom processor WITH A HAT.")
  #endif
  #define BOARD "Atom"
  //#define NUM_SERVOS 2
  #define SERVOPINS     23,33    
  #define NUM_NATIVE_IO 1
  #define IOPINS        39  // 39 is the top button, the led is a smart RGB
  #define NUM_IO        65 // calc by hand 2+64
  #define CAN_TX_PIN    (gpio_num_t)19
  #define CAN_RX_PIN    (gpio_num_t)22
  #ifndef USEGCSERIAL
    #include "ACAN_ESP32Can.h"
    #define NOCAN
  #endif // USEGCSERIAL
  #include <ESP32Servo.h>
  #define EEPROMSIZE 4096
  #define EEPROMbegin EEPROM.begin(EEPROMSIZE);
  #define EEPROMcommit { EEPROM.commit(); dP("EEPROM COMMIT"); }
  #define WIRE_begin Wire.begin(26, 32, 100000)  // choose pins for I2C // SDA, SCL
//
#elif defined(M5NANOC6_BOARD)
  #pragma message ("M5NANOC6_BOARD")
  #ifndef ARDUINO_M5STACK_NANOC6
    #error message("M5NANOC6_BOARD was selected, so must use a M5NanoC6 Board.")
  #endif
  #define BOARD "M5NANOC6_BOARD" // ARDUINO_M5STACK_NANOC6
  //#define NUM_SERVOS 2
  #define SERVOPINS     10,11
  #define NUM_NATIVE_IO 1
  #define IOPINS        10
  #define NUM_IO        33 // calc by hand 2+32
  #define CAN_TX_PIN    (gpio_num_t)2
  #define CAN_RX_PIN    (gpio_num_t)1
  #ifndef USEGCSERIAL
    #include "ACAN_ESP32Can.h"
  #endif // USEGCSERIAL
  #include <ESP32Servo.h>
  #define EEPROMbegin EEPROM.begin(4096);
  #define EEPROMcommit { EEPROM.commit(); dP("EEPROM COMMIT"); }
  #define WIRE_begin Wire.begin(2, 1, 100000)  // choose pins for I2C
//
#elif defined(MINIMA_BOARD)
  #pragma message ("MINIMA_BOARD")
  #ifndef ARDUINO_UNOR4_MINIMA
    #error message("MINIMA_BOARD was select, so must use a MINIMA processor")
  #endif
  #define BOARD "Minima" // 
  //#define NUM_SERVOS 2
  #define SERVOPINS     2,3
  #define NUM_NATIVE_IO 8
  #define IOPINS        6,7,8,9,10,11,12,13
  #define NUM_IO        40 // calc by hand 8+32
  #define CAN_TX_PIN 4
  #define CAN_RX_PIN 5
  #ifndef USEGCSERIAL
    #include "R4.h"
  #endif // USEGCSERIAL
  #include <Servo.h>
  #define EEPROMbegin 
  #define EEPROMcommit 
  #define WIRE_begin 
//
#else
  #error "No board selected"
#endif

