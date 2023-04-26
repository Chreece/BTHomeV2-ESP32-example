#include "Arduino.h"
#include "NimBLEDevice.h"
#include "BTHome.h"

#include "esp_random.h"
//#include "mbedtls/ccm.h"

static BLEAdvertising *pAdvertising;

void BTHome::begin(bool encryption, String enkey) {
  BLEDevice::init("");
  pAdvertising = BLEDevice::getAdvertising();
  if (encryption) {
    this->m_encryptEnable = true;
    this->m_encryptCount = esp_random() % 0x427;
  }
  else this->m_encryptEnable = false;
}

void BTHome::resetMeasurement() {
  this->m_payloadIdx = 0;
}

bool BTHome::addMeasurement_state(uint8_t sensor_id, uint8_t state) {
  if ((this->m_payloadIdx + 2) <= MEASUREMENT_MAX_LEN) {
    this->m_payload[this->m_payloadIdx] = static_cast<byte>(sensor_id & 0xff);
    this->m_payloadIdx++;
    this->m_payload[this->m_payloadIdx] = static_cast<byte>(state & 0xff);
    this->m_payloadIdx++;
    return true;
  }
  return false;
}

bool BTHome::addMeasurement(uint8_t sensor_id, uint64_t value) {
  uint8_t size = getByteNumber(sensor_id);
  uint16_t factor = getFactor(sensor_id);
  if ((this->m_payloadIdx + size + 1) <= MEASUREMENT_MAX_LEN) {
    this->m_payload[this->m_payloadIdx] = static_cast<byte>(sensor_id & 0xff);
    this->m_payloadIdx++;
    for (uint8_t i = 0; i < size; i++)
    {
      this->m_payload[this->m_payloadIdx] = static_cast<byte>(((value * factor) >> (8 * i)) & 0xff);
      this->m_payloadIdx++;
    }
    return true;
  }
  return false;
}

bool BTHome::addMeasurement(uint8_t sensor_id, float value) {
  uint8_t size = getByteNumber(sensor_id);
  uint16_t factor = getFactor(sensor_id);
  if ((this->m_payloadIdx + size + 1) <= MEASUREMENT_MAX_LEN) {
    uint64_t value2 = static_cast<uint64_t>(value * factor);
    this->m_payload[this->m_payloadIdx] = static_cast<byte>(sensor_id & 0xff);
    this->m_payloadIdx++;
    for (uint8_t i = 0; i < size; i++)
    {
      this->m_payload[this->m_payloadIdx] = static_cast<byte>((value2 >> (8 * i)) & 0xff);
      this->m_payloadIdx++;
    }
    return true;
  }
  return false;
}

void BTHome::buildPaket(String device_name) {

  // Create the BLE Device

  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  //BLEAdvertisementData oScanResponseData = BLEAdvertisementData();

  oAdvertisementData.setFlags(FLAGS);

  std::string strServiceData = "";
  std::string strServiceData2 = "";
  std::string serviceData = "";
  std::string bthomedata = "";

  int dn_length = device_name.length() + 1;
  byte len_buf = dn_length;
  char str_buf[dn_length];
  device_name.toCharArray(str_buf, dn_length);

  strServiceData += len_buf;         // Add the length of the Name
  strServiceData += COMPLETE_NAME;   // Complete_Name: Complete local name -- Short_Name: Shortened Name
  strServiceData += str_buf;         // Add the Name to the payload

  serviceData += SERVICE_DATA;  // DO NOT CHANGE -- Service Data - 16-bit UUID

  serviceData += UUID1;  // DO NOT CHANGE -- UUID
  serviceData += UUID2;  // DO NOT CHANGE -- UUID

  // Build the BTHome Data
  for (int i = 0; i < this->m_payloadIdx; i++)
  {
    bthomedata += this->m_payload[i]; // Add the sensor data to the Service Data
  }

  // The encryption
  if (this->m_encryptEnable) {
    serviceData += ENCRYPT;
    std::string counter = intToFourBytes(this->m_encryptCount);
    BLEAddress esp32Mac;
    esp32Mac = NimBLEDevice::getAddress();
    this->m_encryptCount++;
  } else {
    serviceData += NO_ENCRYPT;
    serviceData += bthomedata;
  }

  byte payload_buf = serviceData.length(); // Generate the length of the Service Data
  strServiceData2 += payload_buf;         // Add the length to the Service Data
  strServiceData2 += serviceData;             // Finalize the packet

  oAdvertisementData.addData(strServiceData);
  oAdvertisementData.addData(strServiceData2);
  pAdvertising->setAdvertisementData(oAdvertisementData);
  //pAdvertising->setScanResponseData(oScanResponseData);
  pAdvertising->setScanResponse(false);
  /**  pAdvertising->setAdvertisementType(ADV_TYPE_NONCONN_IND);
       Advertising mode. Can be one of following constants:
     - BLE_GAP_CONN_MODE_NON (non-connectable; 3.C.9.3.2).
     - BLE_GAP_CONN_MODE_DIR (directed-connectable; 3.C.9.3.3).
     - BLE_GAP_CONN_MODE_UND (undirected-connectable; 3.C.9.3.4).
  */
  pAdvertising->setAdvertisementType(BLE_GAP_CONN_MODE_NON);
}

void BTHome::stop() {
  pAdvertising->stop();
}

void BTHome::start(uint32_t duration) {
  pAdvertising->start(duration);
}

bool BTHome::isAdvertising() {
  return pAdvertising->isAdvertising();
}


std::string BTHome::intToFourBytes(uint32_t value) {
  // Create a buffer to hold the 4 bytes
  uint8_t buffer[4];

  // Copy the integer value into the buffer
  memcpy(buffer, &value, sizeof(value));

  // Create a std::string from the buffer
  std::string result(reinterpret_cast<char*>(buffer), sizeof(buffer));

  return result;
}

uint8_t BTHome::getByteNumber(uint8_t sens) {
  if ( sens == ID_BATTERY
       || sens == ID_COUNT
       || sens == ID_HUMIDITY
       || sens == ID_MOISTURE
       || sens == ID_UV ) {
    return 1;
  } else {
    if ( sens == ID_DURATION
         || sens == ID_ENERGY
         || sens == ID_GAS
         || sens == ID_ILLUMINANCE
         || sens == ID_POWER
         || sens == ID_PRESSURE ) {
      return 3;
    } else {
      if ( sens == ID_COUNT4
           || sens == ID_ENERGY4
           || sens == ID_GAS4
           || sens == ID_VOLUME
           || sens == ID_WATER ) {
        return 4;
      } else {
        return 2;
      }
    }
  }
}

uint16_t BTHome::getFactor(uint8_t sens) {
  switch (sens)
  {
    case ID_DISTANCEM:
    case ID_ROTATION:
    case ID_TEMPERATURE:
    case ID_VOLTAGE1:
    case ID_VOLUME1:
    case ID_UV:
      return 10; break;
    case ID_DEWPOINT:
    case ID_HUMIDITY_PRECISE:
    case ID_ILLUMINANCE:
    case ID_MASS:
    case ID_MASSLB:
    case ID_MOISTURE_PRECISE:
    case ID_POWER:
    case ID_PRESSURE:
    case ID_SPD:
    case ID_TEMPERATURE_PRECISE:
      return 100; break;
    case ID_CURRENT:
    case ID_DURATION:
    case ID_ENERGY:
    case ID_ENERGY4:
    case ID_GAS:
    case ID_GAS4:
    case ID_VOLTAGE:
    case ID_VOLUME:
    case ID_VOLUMEFR:
    case ID_WATER:
      return 1000; break;
    default:
      return 1;
  }
}
