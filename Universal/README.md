ToDO / Ideas

1. Write own easing, that way we can detect the 50% of movement and activate the 'midpoint' events.  
We can just copy the cubic and bounce, and maybe define our own.  
2. Test detach and reattach.
3. Test midpoint startup.




ESP32 2SVO8IO2MCP2PCA Example.

This sketch implements: 
Native channels, using the processors pins:
  * two servos, each with three positions
    Positions cabe set to angles 0-180
    The 0 and 180 end-points of the servos can be configured.
  * N i/o channels, each of which can be an input or an output,
    If an output it may be solid, pulse or flashing, and consumes an on- and 0ff-event. 
    If an input it produces an on- and off-event, which may each be delayed.  The 
     inputs can be with or with pullups, inverted or not, or can be toggle. 
  * N is a variable number of 'native' i/o depending on the processor, see board.h
Two MCP23017 Port Expanders
  * Each with two banks of 8 i/o channels
  * These have the same characteristics as the native i/o.
  * See: e.g. https://www.amazon.ca/MCP23017-MCP23017-Expander-Expansion-3-0V-5-5V/dp/B0FHRRVKSP/
Two PCA9685 Servo Boards
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

This sketch supports: 
 Processors: Nano, R4 Nano, R4 Minima, ESP32, M5NANOC6
 CAN: MCP2515, R4 CAN, ESP32 TWAI

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
    * Use Hardware Reset: Connect the Reset pin (Pin 18) to the microcontroller and actively pull it high, or use 
       a 10k resistor to pull it up to VCC to prevent floating.
    * Add Decoupling Capacitors: Place a 0.1µF ceramic capacitor between VCC and GND as close to the chip as 
       possible to filter power noise, which can cause erratic behavior.
    * Use Pull-up Resistors on I2C: Ensure the SDA and SCL lines have pull-up resistors (typically 4.7kΩ).



