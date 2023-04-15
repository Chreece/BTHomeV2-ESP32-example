#include "Arduino.h"
#include "NimBLEDevice.h"
#include "BTHome.h"

RTC_DATA_ATTR static uint32_t bootcount; // remember number of boots in RTC Memory

BLEAdvertising *pAdvertising;

void BTHome::sendPaket(int sensorNumber, int sensor[], int state[], String device_name, bool encryption) {

  BLEDevice::init("");
  
  bootcount++;
  
  Serial.printf("start ESP32 %d\n", bootcount);

  pAdvertising = BLEDevice::getAdvertising();

  // Create the BLE Device

  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  BLEAdvertisementData oScanResponseData = BLEAdvertisementData();

  oAdvertisementData.setFlags(FLAGS);
  
  std::string strServiceData = "";
  std::string strServiceData2 = "";
  std::string payload = "";
  
  int dn_length = device_name.length() + 1;
  byte len_buf = dn_length;
  char str_buf[dn_length];
  device_name.toCharArray(str_buf, dn_length);

  strServiceData += len_buf;         // Add the length of the Name
  strServiceData += COMPLETE_NAME;   // Complete_Name: Complete local name -- Short_Name: Shortened Name
  strServiceData += str_buf;         // Add the Name to the payload

  payload += SERVICE_DATA;  // Service Data - 16-bit UUID

  payload += UUID1;  // DO NOT CHANGE -- UUID
  payload += UUID2;  // DO NOT CHANGE -- UUID

  
  // The encryption is not yet implemented
  if (encryption){    
    payload += ENCRYPT;
    std::string counter = intToFourBytes(bootcount);
    BLEAddress esp32Mac;  
    esp32Mac = NimBLEDevice::getAddress();
    } else {
      payload += NO_ENCRYPT;
    }

// Add the sensor data (object id + state) to the Service Data
  for( int i = 0; i < sensorNumber; i++){
    
    byte sensname = static_cast<byte>((sensor[i]) & 0xFF);
    
    payload += sensname; 

    if (state[i] != ON && state[i] != OFF){
      int byteNum = getByteNumber(sensor[i]);
      Serial.print(i);
      Serial.printf("\nbyteNum:");
      Serial.print(byteNum);
      payload += intToLittleEndianHexString(state[i], byteNum);
    } else {
      Serial.print(i);
      Serial.printf("\nstate:");
      Serial.print(state[i]);
      byte st = static_cast<byte>((state[i]) & 0xFF);
      payload += st;
    }           
  }
  
  byte payload_buf = payload.length();    // Generate the length of the Service Data
  strServiceData2 += payload_buf;         // Add the length to the Service Data
  strServiceData2 += payload;             // Finalize the packet
  
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
  
  pAdvertising->start();
  Serial.println("Advertizing started...");
  delay(100);
  pAdvertising->stop();
  
}

std::string BTHome::intToLittleEndianHexString(int value, int numBytes) {
  std::string hexString = "";
  for (int i = 0; i < numBytes; i++) {
    byte byteValue = static_cast<byte>((value >> (8 * i)) & 0xFF);
    hexString += byteValue;
  }
  return hexString;
}

std::string BTHome::intToFourBytes(int value) {
  // Create a buffer to hold the 4 bytes
  uint8_t buffer[4];

  // Copy the integer value into the buffer
  memcpy(buffer, &value, sizeof(value));

  // Create a std::string from the buffer
  std::string result(reinterpret_cast<char*>(buffer), sizeof(buffer));

  return result;
}

int BTHome::getByteNumber(int sens) {
  if ( sens == BATTERY
        || sens == COUNT
        || sens == HUMIDITY
        || sens == MOISTURE
        || sens == UV ){
      return 1;
        } else {
            if ( sens == DURATION
                  || sens == ENERGY
                  || sens == GAS
                  || sens == ILLUMINANCE
                  || sens == POWER
                  || sens == PRESSURE ){
                return 3;
                  } else {
                      if ( sens == COUNT4
                            || sens == ENERGY4
                            || sens == GAS4
                            || sens == VOLUME
                            || sens == WATER ){                
                        return 4;
                            } else { return 2; } 
                    }
          }
}