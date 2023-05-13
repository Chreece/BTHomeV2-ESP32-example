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
#define ENABLE_ENCRYPT // Remove this line for no encryption
String BIND_KEY = "231d39c1d7cc1ab1aee224cd096db932"; // Change this key with a string containing 32 of: a-f and 0-9 characters (hex) this will be asked in HA

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
  bthome.addMeasurement(ID_ILLUMINANCE, 1000.02f);//4 bytes
  bthome.addMeasurement(ID_PRESSURE, 101.86f);//4
  bthome.addMeasurement(ID_TEMPERATURE_PRECISE, 54.00f);//3
  bthome.addMeasurement(ID_HUMIDITY_PRECISE, 67.00f);//3
  bthome.buildPaket(DEVICE_NAME);
  bthome.start();  //start the first adv data
  delay(1500);

  bthome.resetMeasurement();
  bthome.addMeasurement_state(STATE_POWER_ON, STATE_OFF);//2 bytes
  bthome.addMeasurement(ID_TVOC, (uint64_t)213);//3
  bthome.addMeasurement(ID_CO2, (uint64_t)456);//3
  bthome.buildPaket(DEVICE_NAME);  //change the adv data
  delay(1500);

  bthome.stop();
  delay(10000);
}
