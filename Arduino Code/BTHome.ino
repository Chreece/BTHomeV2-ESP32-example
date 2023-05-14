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
  bthome.begin(DEVICE_NAME, true, BIND_KEY);
#else
  bthome.begin(DEVICE_NAME);
#endif
}

void loop() {
  //MEASUREMENT_MAX_LEN = 23, ENABLE_ENCRYPT will use extra 8 bytes, so each Measurement should smaller than 15

  // 1st method: just addMeasurement as much as you can, the code will split and send the adv packet automatically
  // each adv packet sending lasts for 1500ms
  bthome.resetMeasurement();
  // bthome.addMeasurement(sensorid, value) you can use the sensorids from the BTHome.h file
  bthome.addMeasurement(ID_ILLUMINANCE, 50.81f);//4 bytes
  bthome.addMeasurement(ID_PRESSURE, 1023.86f);//4
  bthome.addMeasurement(ID_TEMPERATURE_PRECISE, 35.00f);//3
  bthome.addMeasurement(ID_HUMIDITY_PRECISE, 40.00f);//3
  bthome.addMeasurement(ID_TVOC, (uint64_t)350);//3
  bthome.addMeasurement(ID_CO2, (uint64_t)1208);//3
  bthome.addMeasurement_state(STATE_POWER_ON, STATE_ON);//2
  bthome.sendPacket();
  bthome.stop();

  // 2nd method: make sure each measurement data length <=15 and start(stop) manually
  bthome.resetMeasurement();
  bthome.addMeasurement(ID_ILLUMINANCE, 1008.81f);//4 bytes
  bthome.addMeasurement(ID_PRESSURE, 1000.86f);//4
  bthome.addMeasurement(ID_TEMPERATURE_PRECISE, 26.00f);//3
  bthome.addMeasurement(ID_HUMIDITY_PRECISE, 70.00f);//3
  bthome.buildPaket();
  bthome.start();//start the first adv data
  delay(1500);
  bthome.resetMeasurement();
  bthome.addMeasurement(ID_TVOC, (uint64_t)220);//3
  bthome.addMeasurement(ID_CO2, (uint64_t)458);//3
  bthome.addMeasurement_state(STATE_POWER_ON, STATE_OFF);//2
  bthome.buildPaket();//change the adv data
  delay(1500);
  bthome.stop();

  delay(10000);
}
