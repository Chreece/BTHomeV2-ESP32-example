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


BTHome bthome;

void setup() {

  Serial.begin(115200);
  bthome.begin();
}

void loop() {
  bthome.resetMeasurement();
  bthome.addMeasurement(ID_ILLUMINANCE, 123.02f);
  bthome.addMeasurement(ID_PRESSURE, 1001.86f);
  bthome.buildPaket(DEVICE_NAME);
  bthome.start();  //start the first adv data cause the limit length(31) of ble 4.0
  sleep(500);

  bthome.resetMeasurement();
  bthome.addMeasurement(ID_TEMPERATURE_PRECISE, 26.00f);
  bthome.addMeasurement(ID_HUMIDITY_PRECISE, 50.00f);
  bthome.buildPaket(DEVICE_NAME);  //change the adv data
  sleep(500);

  bthome.resetMeasurement();
  bthome.addMeasurement_state(STATE_POWER_ON, STATE_ON);
  bthome.addMeasurement(ID_TVOC, (uint64_t)250);
  bthome.addMeasurement(ID_CO2, (uint64_t)800);
  bthome.buildPaket(DEVICE_NAME);  //change the adv data
  sleep(500);

  bthome.stop();
  sleep(10000);
}
