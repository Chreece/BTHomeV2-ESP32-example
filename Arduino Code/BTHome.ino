/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by pcbreflux
   Modified to work with NimBLE
   Modified for V2 by Chreece -- Encryption not yet implemented
   BLE advertisement format from https://bthome.io/
*/
#include "NimBLEDevice.h"
#include "esp_sleep.h"
#include "BTHome.h"

#define ENCRYPTION false          // Encryption for the messages, should stay false as encryption is not yet implemented on this code (still in development)
#define DEVICE_NAME "DIY-Sensor"  // The name of the sensor

#if ENCRYPTION
  #include <AESLib.h>
#endif

#define GPIO_DEEP_SLEEP_DURATION     10  // sleep x seconds and then wake up
RTC_DATA_ATTR static uint32_t bootcount; // remember number of boots in RTC Memory

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif



BLEAdvertising *pAdvertising;

void setBeacon(std::string sensor_data) {

  // Create the BLE Device

  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  BLEAdvertisementData oScanResponseData = BLEAdvertisementData();

  oAdvertisementData.setFlags(Flags);
  std::string strServiceData = "";
  std::string strServiceData2 = "";
  std::string payload = "";
  String device_name = DEVICE_NAME;
  int dn_length = device_name.length() + 1;
  byte len_buf = dn_length;
  char str_buf[dn_length];
  device_name.toCharArray(str_buf, dn_length);

  strServiceData += len_buf;         // Add the length of the Name
  strServiceData += Complete_Name;   // Complete_Name: Complete local name -- Short_Name: Shortened Name
  strServiceData += str_buf;         // Add the Name to the payload

  payload += Service_Data;  // DO NOT CHANGE -- Service Data - 16-bit UUID

  payload += UUID1;  // DO NOT CHANGE -- UUID
  payload += UUID2;  // DO NOT CHANGE -- UUID

  
  // The encryption is not yet implemented
  if (ENCRYPTION){        
    payload += Encrypt;
    std::string enkey = "d4d539a12211baadacac82be74c6f902";    // The encryption Key    
    std::string counter = intToBytes(bootcount);
    BLEAddress esp32Mac;  
    esp32Mac = NimBLEDevice::getAddress();
    } else {
      payload += No_Encrypt;
    }

  payload += sensor_data;                  // Add the sensor entities and states to the payload
  byte payload_buf = payload.length() + 1; // Generate the length of the Service Data
  strServiceData2 += payload_buf;         // Add the length to the Service Data
  strServiceData2 += payload;             // Add the sensor data to the Service Data
  
  oAdvertisementData.addData(strServiceData);
  oAdvertisementData.addData(strServiceData2);
  pAdvertising->setAdvertisementData(oAdvertisementData);
  pAdvertising->setScanResponseData(oScanResponseData);
  /**  pAdvertising->setAdvertisementType(ADV_TYPE_NONCONN_IND);
       Advertising mode. Can be one of following constants:
     - BLE_GAP_CONN_MODE_NON (non-connectable; 3.C.9.3.2).
     - BLE_GAP_CONN_MODE_DIR (directed-connectable; 3.C.9.3.3).
     - BLE_GAP_CONN_MODE_UND (undirected-connectable; 3.C.9.3.4).
  */
  pAdvertising->setAdvertisementType(BLE_GAP_CONN_MODE_NON);

}

std::string intToLittleEndianHexString(int value, int numBytes) {
  std::string hexString = "";
  for (int i = 0; i < numBytes; i++) {
    byte byteValue = static_cast<byte>((value >> (8 * i)) & 0xFF);
    hexString += byteValue;
  }
  return hexString;
}

std::string intToBytes(int value) {
  // Create a buffer to hold the 4 bytes
  uint8_t buffer[4];

  // Copy the integer value into the buffer
  memcpy(buffer, &value, sizeof(value));

  // Create a std::string from the buffer
  std::string result(reinterpret_cast<char*>(buffer), sizeof(buffer));

  return result;
}

void setup() {

  Serial.begin(115200);

  BLEDevice::init("");
  
  bootcount++;
  
  Serial.printf("start ESP32 %d\n", bootcount);

  pAdvertising = BLEDevice::getAdvertising();
  
  std::string pload = "";

  //  This section is for the sensors' entities and states see https://bthome.io/format/ you can use the variables defined in BThome.h
  // Example Presence packet
  pload += presence;  // This is the presence binary sensor, object id: 0x25
  pload += yes;       // yes: on (0x01) -- no: off (0x00)
  
  // Example Temperature packet
  pload += temperature; // Temperature Sensor Object Id: 0x02 
  int state = 2587;     // This will be 25.87°C since this object id has a multiplier of 0.01
  int byteNum = 2;      // 2 bytes
  pload += intToLittleEndianHexString(state, byteNum); // Convert it to little endian hex
  
  // Example Battery packet
  pload += battery;     // Battery Sensor Object Id: 0x01
  state = 37;           // This will be 37% (multiplier 1)
  byteNum = 1;          // 0x01 object id is one byte long
  pload += intToLittleEndianHexString(state, byteNum); // Convert it to little endian hex

  setBeacon(pload);

  // Start advertising
  pAdvertising->start();
  Serial.println("Advertizing started...");
  delay(5000);
  pAdvertising->stop();
  Serial.printf("enter deep sleep\n");
  esp_deep_sleep(1000000LL * GPIO_DEEP_SLEEP_DURATION);
  Serial.printf("in deep sleep\n");
}

void loop() {
}