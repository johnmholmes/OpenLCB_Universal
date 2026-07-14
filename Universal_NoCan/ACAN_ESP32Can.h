#if 1
#pragma message("ACAN_ESP32Can.h")
#include <Arduino.h>
#include "driver/twai.h"  // Native ESP32 TWAI/CAN header
#include "OlcbCan.h"
#include "debugging.h"

// Define CAN Hardware Pins if not passed globally by build system
#ifndef CAN_TX_PIN
  #define CAN_TX_PIN  22
#endif
#ifndef CAN_RX_PIN
  #define CAN_RX_PIN  21
#endif

#define P(...) Serial.print(__VA_ARGS__)
#define PV(x) { P(" " #x "="); P(x); }
#define PVH(x) { P(" " #x "="); P(x, HEX); }

class OlcbCanClass : public OlcbCan {
  public:
    virtual void init() {
        // 1. Set general pin layouts and configuration mode
        twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
            (gpio_num_t)CAN_TX_PIN, 
            (gpio_num_t)CAN_RX_PIN, 
            TWAI_MODE_NORMAL
        );
        P("\n Tx="); P((uint8_t)CAN_TX_PIN);
        P("\n Rx="); P((uint8_t)CAN_RX_PIN);

        // Give the M5Atom deep queues to buffer heavy OpenLCB frames safely
        g_config.tx_queue_len = 32;
        g_config.rx_queue_len = 32;

        // 2. Safely configure dynamic 125kbps timings across SDK versions
        twai_timing_config_t t_config = TWAI_TIMING_CONFIG_125KBITS();
        P("\n CAN init to TWAI_TIMING_CONFIG_125KBITS()");

        // 3. Accept all filter tags
        twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

        // 4. Clean resource driver install and startup
        if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
            Serial.println("Error: Failed to install Native TWAI driver.");
            return;
        }
        if (twai_start() != ESP_OK) {
            Serial.println("Error: Failed to spin up Native TWAI Bus active state.");
        } else P("\n TWAI A-OK");
        while(0) {
            delay(1000);
            id = 0x195B4123;
            length=8;
            for(int i=0; i<8; i++) data[i] = (i+1)*16+(i+2);
            write();
        }
    }

    // Checks how many messages are sitting unread in the hardware buffer ring
    virtual uint8_t avail() { 
        twai_status_info_t status_info;
        if (twai_get_status_info(&status_info) == ESP_OK) {
            return (status_info.msgs_to_rx > 0) ? 1 : 0;
        }
        return 0; 
    }

    virtual uint8_t read() {
        twai_message_t frame;
        
      twai_status_info_t status;
      if (twai_get_status_info(&status) == ESP_OK) {
          if (status.tx_error_counter > 0 || status.rx_error_counter > 0) {
              P("\n Bus Errors! TX Err: "); P(status.tx_error_counter);
              P(" RX Err: "); P(status.rx_error_counter);
          }
      }


        // Execute immediate poll (Timeout = 0)
        if (twai_receive(&frame, pdMS_TO_TICKS(0)) != ESP_OK) {
            return 0;
        }
        
        // Match filtering logic from your original library application
        if (frame.rtr) return 0;       // Reject remote transmission requests
        if (!frame.extd) return 0;     // Reject standard 11-bit IDs (OpenLCB uses 29-bit Extended IDs)

        id = frame.identifier;
        length = frame.data_length_code;
        memcpy(data, frame.data, length);
        
        return 1;
    }

    // Check if space exists inside the hardware buffer pipeline for transmission
    virtual uint8_t txReady() { 
        twai_status_info_t status_info;
        if (twai_get_status_info(&status_info) == ESP_OK) {
            // Returns true if the hardware TX queue isn't full
            return (status_info.msgs_to_tx < 32) ? 1 : 0; 
        }
        return 0;
    }

    virtual uint8_t write(long timeout) {
        P("\n CAN out: "); P(id,HEX); for(int i=0;i<length;i++) { P(' '); P(data[i],HEX); }
        //twai_message_t frame;
        twai_message_t frame = { 0 }; 
        frame.identifier = id;
        frame.extd = 1;               // Force Extended ID layout (29-bit)
        frame.rtr = 0;                // Normal Data frame
        //frame.self = 0;
        frame.data_length_code = length > 8 ? 8 : length;
        memcpy(frame.data, data, frame.data_length_code);

        // Convert incoming parameter context to tick configurations safely
        uint32_t wait_ticks = (timeout <= 0) ? pdMS_TO_TICKS(10) : pdMS_TO_TICKS(timeout);

        // Transmit frame without locking execution loops indefinitely
        //esp_err_t res = twai_transmit(&frame, wait_ticks);
        esp_err_t res = twai_transmit(&frame, wait_ticks);
        if (res == ESP_OK) {
            delay(5); // Respect back-off timeline requested in legacy source code block
            P("  OK");
            return 1;
        }
        P(" CAN Transmit error:"); P(res);
        return 0; // Transfer dropped or timed out
    }

    virtual uint8_t write() {
        return write(0L);
    }

    bool active;
};

#else



//https://github.com/pierremolinaro/acan-esp32
#ifndef ARDUINO_ARCH_ESP32
  #error "Select an ESP32 board"
#endif

#ifndef NOCAN
#define NOCAN

#pragma message "Compiling ACAN_ESP32Can.h"

#include "OlcbCan.h"
#include "debugging.h"

#include <ACAN_ESP32.h>
#include <esp_chip_info.h>
#include <esp_flash.h>
#include <core_version.h> // For ARDUINO_ESP32_RELEASE

#define P(...) Serial.print(__VA_ARGS__)
#define PV(x) { P(" " #x "="); P(x); }
#define PVH(x) { P(" " #x "="); P(x, HEX); }

class OlcbCanClass : public OlcbCan  {
  public:
    virtual void init() {                    // initialization
      //P("\n ACAN_ESP32 init");
      ACAN_ESP32_Settings settings(125000L);
      settings.mRxPin = CAN_RX_PIN;
      settings.mTxPin = CAN_TX_PIN;
      settings.mRequestedCANMode = ACAN_ESP32_Settings::NormalMode;
      if( !ACAN_ESP32::can.begin(settings) ) return;
      //P("\n ACAN_ESP32 init'd");
    }
    virtual uint8_t avail() { return 1; }                // read rxbuffer available
    virtual uint8_t read() {                 // read a buffer
      CANMessage frame;
      if( !ACAN_ESP32::can.receive(frame) ) return 0;
      if(frame.rtr) return 0;
      if(!frame.ext) return 0;
      id = frame.id;
      length = frame.len;
      memcpy(data, frame.data, length);
      //P("\n ACAN_ESP32 read "); PVH(id); PV(length); PVH(data[0]);
      return 1;
    }
    virtual uint8_t txReady() { return 1; }             // write txbuffer available
    virtual uint8_t write(long timeout) {    // write, 0= immediately or fail; 0< if timeout occurs fail
      CANMessage frame;
      //P("\n ACAN_ESP32 write >>> "); PVH(id); PV(length); PVH(data[0]);
      frame.id = id;
      frame.ext = true;
      frame.rtr = false;
      frame.len = length;
      memcpy(frame.data, data, length);
      while(!ACAN_ESP32::can.tryToSend(frame));
      //P(" OK");
      delay(5);
      return 1;
    }
    virtual uint8_t write() {               // write(0), ie write immediately
        return write((long) 0);
    };
    bool active;                          // flag net activity
};

#endif

#endif // 0/1