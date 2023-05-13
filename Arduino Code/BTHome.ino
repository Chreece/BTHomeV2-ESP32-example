/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by pcbreflux
   Modified to work with NimBLE
   Modified for V2 by Chreece -- Encryption not yet implemented
   BLE advertisement format from https://bthome.io/

   Modify by countrysideboy: add the ability to send more sensor data
*/

#include "BTHome.h"

#define DEVICE_NAME "DIY-sensor"  // The name of the sensor
#define ENABLE_ENCRYPT
uint8_t BIND_KEY[] = {0x23, 0x1d, 0x39, 0xc1, 0xd7, 0xcc, 0x1a, 0xb1, 0xae, 0xe2, 0x24, 0xcd, 0x09, 0x6d, 0xb9, 0x32};


BTHome bthome;

void setup() {
  Serial.begin(115200);
#ifdef ENABLE_ENCRYPT
  bthome.begin(true, BIND_KEY);
#else
  bthome.begin();
#endif
}

void loop() {
  //MEASUREMENT_MAX_LEN = 23, ENABLE_ENCRYPT will use extra 8 bytes, so each Measurement should smaller than 15
  bthome.resetMeasurement();
  bthome.addMeasurement(ID_ILLUMINANCE, 123.02f);//4 bytes
  bthome.addMeasurement(ID_PRESSURE, 1001.86f);//4
  bthome.addMeasurement(ID_TEMPERATURE_PRECISE, 26.00f);//3
  bthome.addMeasurement(ID_HUMIDITY_PRECISE, 50.00f);//3
  bthome.buildPaket(DEVICE_NAME);
  bthome.start();  //start the first adv data
  delay(1500);

  bthome.resetMeasurement();
  bthome.addMeasurement_state(STATE_POWER_ON, STATE_ON);//2 bytes
  bthome.addMeasurement(ID_TVOC, (uint64_t)250);//3
  bthome.addMeasurement(ID_CO2, (uint64_t)800);//3
  bthome.buildPaket(DEVICE_NAME);  //change the adv data
  delay(1500);

  bthome.stop();
  delay(10000);
}
