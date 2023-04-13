/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by pcbreflux
   Modified to work with NimBLE
   Modified for V2 by Chreece -- Encryption not yet implemented
   BLE advertisement format from https://bthome.io/
*/

#include "NimBLEDevice.h"
#include "esp_sleep.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "BTHome.h"

#define GPIO_DEEP_SLEEP_DURATION     10  // sleep x seconds and then wake up
RTC_DATA_ATTR static uint32_t bootcount; // remember number of boots in RTC Memory

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

BLEAdvertising *pAdvertising;

void setBeacon() {

  // Create the BLE Device

  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  BLEAdvertisementData oScanResponseData = BLEAdvertisementData();

  oAdvertisementData.setFlags(Flags);
  std::string strServiceData = "";
  std::string strServiceData2 = "";
  std::string payload = "";
  
  String device_name = "DIY-sensor";      // The device name
  bool encryption = false;               // Encryption for the messages, should stay false as encryption is not yet implemented on this code (still in develepment)
  
  int dn_length = device_name.length() + 1;
  byte len_buf = dn_length;
  char str_buf[dn_length];
  device_name.toCharArray(str_buf, dn_length);

  strServiceData += len_buf;
  strServiceData += Complete_Name;   // Complete_Name: Complete local name -- Short_Name: Shortened Name
  strServiceData += str_buf;

  payload += Service_Data;  // DO NOT CHANGE -- Service Data - 16-bit UUID

  payload += UUID1;  // DO NOT CHANGE -- UUID
  payload += UUID2;  // DO NOT CHANGE -- UUID

  if (encryption){
    payload += Encrypt;
    std::string enkey = "d4d539a12211baadacac82be74c6f902";    // The encryption Key    
    std::string counter = intToBytes(bootcount);    
    const uint8_t* mac = esp_bt_dev_get_address();
    } else {
      payload += No_Encrypt;
    }  

  //  This section is for the sensors' entities and values see https://bthome.io/format/ you can use the variables defined in BThome.h
  // Example Presence packet
  payload += presence; // This is the presence binary sensor object id: 0x25
  payload += yes; // yes: on (0x01) -- no: off (0x00)
  
  // Example Temperature packet
  payload += temperature; // Temperature Sensor paket
  int state = 2587;       // The temperature that should be parsed this is 25,87°C
  int byteNum = 2;        // The number of bytes from https://bthome.io/format/ for the device id
  payload += intToLittleEndianHexString(state, byteNum);
  

  byte payload_buf = payload.length() + 1; // Generate the length of the Service paket
  strServiceData2 += payload_buf;
  strServiceData2 += payload;
  
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
  
  bootcount++;;
  
  Serial.printf("start ESP32 %d\n", bootcount);

  pAdvertising = BLEDevice::getAdvertising();
  
  setBeacon();

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