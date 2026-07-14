# OpenLCB_Universal
8 Native I/O 32 MCP I/O 32 Servo

## The sketch has two versions A Can bus Node, and a NoCan standalone version so no can bus commincation between other nodes.

## Please be advised even though the sketch has other MCU options. It may not compile and work for other than for the ESP32 Devkit 1 version.

## Please take a look at the issues section as this is used to raise an issue and the fix being put in place in the next version.

This sketch implements: 
Native channels, using the processors pins:
  * 8 i/o channels, each of which can be an input or an output,
    If an output it may be solid, pulse or flashing, and consumes an on- and 0ff-event. 
    If an input it produces an on- and off-event, which may each be delayed.  The 
     inputs can be with or without pullups, inverted or not, or can be toggle state change.
    
Two MCP23017 Port Expanders Address 0x20 for the first module and 0x21 for the second module.
  * Each with two banks of 8 i/o channels
  * These have the same characteristics as the native i/o.
  * See: e.g. https://www.amazon.ca/MCP23017-MCP23017-Expander-Expansion-3-0V-5-5V/dp/B0FHRRVKSP/
    
Two PCA9685 Servo Expander Adress 0x40 for the first module and 0x41 for the second module.
  * Each has two banks of 8 servos
  * Each servo has two endpoint positions, controlled by to consumed events.
  * Each servo has two events, produced as the servo passes the midpoint between the two midpoints.  
  * See: e.g. https://www.adafruit.com/product/815

It demonstrates: 
* CDI
* memstruct of EEPROM reflecting the CDI structure
* setting flags to refect whether eveentids are used as consumers, producers, or both, see: **const EIDTab eidtab[NUM_EVENT] PROGMEM**
* Initialization routine to initialize the EEPROM, see: **userInitAll()**
* Eventid processing to set a servo's position, see: **pceCallback(unsigned int index)**
* Sampling of inputs and producing events.
* MCP23017 via I2C.  Note channel 7 in each port can only be outputs, otherwise the I2C locks up. 
* PCA9685 via I2C.  

Configuration:
  config.h allows one to:
    * Choose a nodeID
    * Choose to initialize the EEPROM to factory defaults
    * Choose processor
    * Choose CAN or GCSerial
    * CHoose Debugging output
    * set paramters for the sketch -- changes to these may need changes tot he sketch.  

Notes:
  * if RESET_TO_FACTORY_DEFAULTS is 1, then the EEPROM is intialized, 
    it must be changed to 0 and the sketch reloaded to allow JMRI to change items permanently.
  * The ACAN_ESP32Can.h library has been changed by David Harris.
 
MCP23017:
  * Avoid Pin 7 as Input: Move any input devices (switches, sensors) on GPA7/GPB7 to other available pins (0–6).
    
Use Hardware Reset:  
  * Connect the Reset pin (Pin 18) to the microcontroller and actively pull it high, or use 
       a 10k resistor to pull it up to VCC to prevent floating.
       
Add Decoupling Capacitors: 
  * Place a 0.1µF ceramic capacitor between VCC and GND as close to the chip as 
       possible to filter power noise, which can cause erratic behavior.
       
Use Pull-up Resistors on I2C:
  * Ensure the SDA and SCL lines have pull-up resistors (typically 4.7kΩ).

---

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


