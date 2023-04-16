/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by pcbreflux
   Modified to work with NimBLE
   Modified for V2 by Chreece -- Encryption not yet implemented
   BLE advertisement format from https://bthome.io/
*/
#include "esp_sleep.h"
#include "BTHome.h"

#define ENCRYPTION false          // Encryption for the messages, should stay false as encryption is not yet implemented on this code (still in development)
#define DEVICE_NAME "DIY-sensor"  // The name of the sensor
#define SENSOR_NUM 4              // How many sensors to send

#if ENCRYPTION
  #include <AESLib.h>
  #define ENKEY "d4d539a12211baadacac82be74c6f902" // Example Encryption key
#endif

#define GPIO_DEEP_SLEEP_DURATION     10  // sleep x seconds and then wake up

BTHome bthome;

void setup() {

  int object_id[SENSOR_NUM];
  int state[SENSOR_NUM];
  
  Serial.begin(115200);
  
  // Example sensor paket:
  // object_id[x] = <object id> // x = The position of table to save the value starting from 0. 
                                // The < object_id > from the sensor from https://bthome.io/format/ or alternative use the variables from BTHome.h
  // state = <sensors' state>   // The state of the sensor. You can use ON or OFF for the binary sensors or a numerical state with the multiplier from table

  // Example Temperature packet
  object_id[0] = TEMPERATURE; // Temperature Sensor Object Id: 0x02 see BTHome.h defined variables
  state[0] = -2587;     // This will be -25.87°C  since this object id has a multiplier of 0.01
  
  // Example Humidity packet
  object_id[1] = HUMIDITY2; // Humidity Sensor Object Id: 0x03
  state[1] = 5055;     // This will be 50.55%  since this object id has a multiplier of 0.01
  
  // Example Battery packet
  object_id[2] = BATTERY; // Battery Sensor Object Id: 0x01
  state[2] = 100;     // This will be 28%  since this object id has a multiplier of 1
  
  // Example Binary Occupancy packet
  object_id[3] = OCCUPANCY; // Binary Occupancy Sensor Object Id: 0x23
  state[3] = ON;     // This is 0x01 meaning on
  
  bthome.sendPaket(SENSOR_NUM, object_id, state, DEVICE_NAME, ENCRYPTION);

  Serial.printf("enter deep sleep\n");
  esp_deep_sleep(1000000LL * GPIO_DEEP_SLEEP_DURATION);
  Serial.printf("in deep sleep\n");
}

void loop() {
}
