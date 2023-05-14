#include "Arduino.h"
#include "NimBLEDevice.h"
#include "BTHome.h"

static BLEAdvertising *pAdvertising;

void BTHome::begin(String dname, bool encryption, uint8_t const* const key) {
  BLEDevice::init("");
  pAdvertising = BLEDevice::getAdvertising();
  setDeviceName(dname);
  if (encryption) {
    this->m_encryptEnable = true;
    this->m_encryptCount = esp_random() % 0x427;
    memcpy(bindKey, key, sizeof(uint8_t) * BIND_KEY_LEN);
    mbedtls_ccm_init(&this->m_encryptCTX);
    mbedtls_ccm_setkey(&this->m_encryptCTX, MBEDTLS_CIPHER_ID_AES, bindKey, BIND_KEY_LEN * 8);
  }
  else this->m_encryptEnable = false;
}

void BTHome::begin(String dname, bool encryption, String key) {
  uint8_t bind_key[BIND_KEY_LEN];
  for (int i = 0; i < BIND_KEY_LEN; i++) {
    bind_key[i] = strtol(key.substring(i * 2, i * 2 + 2).c_str(), NULL, BIND_KEY_LEN);
  }
  begin(dname, encryption, bind_key);
}

void BTHome::setDeviceName(String dname) {
  if (!dname.isEmpty())
    this->dev_name = dname;
}

void BTHome::resetMeasurement() {
  this->m_sensorDataIdx = 0;
}

void BTHome::addMeasurement_state(uint8_t sensor_id, uint8_t state, uint8_t steps) {
  if ((this->m_sensorDataIdx + 2 + (steps > 0 ? 1 : 0)) <= (MEASUREMENT_MAX_LEN - (this->m_encryptEnable ? 8 : 0))) {
    this->m_sensorData[this->m_sensorDataIdx] = static_cast<byte>(sensor_id & 0xff);
    this->m_sensorDataIdx++;
    this->m_sensorData[this->m_sensorDataIdx] = static_cast<byte>(state & 0xff);
    this->m_sensorDataIdx++;
    if (steps > 0) {
      this->m_sensorData[this->m_sensorDataIdx] = static_cast<byte>(steps & 0xff);
      this->m_sensorDataIdx++;
    }
  }
  else {
    sendPacket();
    addMeasurement_state(sensor_id, state, steps);
  }
}

void BTHome::addMeasurement(uint8_t sensor_id, uint64_t value) {
  uint8_t size = getByteNumber(sensor_id);
  uint16_t factor = getFactor(sensor_id);
  if ((this->m_sensorDataIdx + size + 1) <= (MEASUREMENT_MAX_LEN - (this->m_encryptEnable ? 8 : 0))) {
    this->m_sensorData[this->m_sensorDataIdx] = static_cast<byte>(sensor_id & 0xff);
    this->m_sensorDataIdx++;
    for (uint8_t i = 0; i < size; i++)
    {
      this->m_sensorData[this->m_sensorDataIdx] = static_cast<byte>(((value * factor) >> (8 * i)) & 0xff);
      this->m_sensorDataIdx++;
    }
  }
  else {
    sendPacket();
    addMeasurement(sensor_id, value);
  }
}

void BTHome::addMeasurement(uint8_t sensor_id, float value) {
  uint8_t size = getByteNumber(sensor_id);
  uint16_t factor = getFactor(sensor_id);
  if ((this->m_sensorDataIdx + size + 1) <= (MEASUREMENT_MAX_LEN - (this->m_encryptEnable ? 8 : 0))) {
    uint64_t value2 = static_cast<uint64_t>(value * factor);
    this->m_sensorData[this->m_sensorDataIdx] = static_cast<byte>(sensor_id & 0xff);
    this->m_sensorDataIdx++;
    for (uint8_t i = 0; i < size; i++)
    {
      this->m_sensorData[this->m_sensorDataIdx] = static_cast<byte>((value2 >> (8 * i)) & 0xff);
      this->m_sensorDataIdx++;
    }
  }
  else {
    sendPacket();
    addMeasurement(sensor_id, value);
  }
}

void BTHome::buildPaket() {

  // Create the BLE Device
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  BLEAdvertisementData oScanResponseData = BLEAdvertisementData();

  std::string payloadData = "";
  std::string serviceData = "";
  int i;

  //head
  payloadData += FLAG1;
  payloadData += FLAG2;
  payloadData += FLAG3;
  //local name: move to the response packet
  /*
    if (!device_name.isEmpty()) {
    int dn_length = device_name.length() + 1;
    if (this->m_encryptEnable) {
      //deal with the device name to make sure the adv length <= 31
      //18=3(FLAG)+1(device name length bit)+1(COMPLETE_NAME)+1(SERVICE_DATA)+2(UUID)+1(ENCRYPT)+4(nonce)+4(mic)+1(serviceData length bit)
      if (dn_length > BLE_ADVERT_MAX_LEN - this->m_sensorDataIdx - 18)
        dn_length = BLE_ADVERT_MAX_LEN - this->m_sensorDataIdx - 18;
    }
    else {
      //10=3(FLAG)+1(device name length bit)+1(COMPLETE_NAME)+1(SERVICE_DATA)+2(UUID)+1(ENCRYPT)+1(serviceData length bit)
      if (dn_length > BLE_ADVERT_MAX_LEN - this->m_sensorDataIdx - 10)
        dn_length = BLE_ADVERT_MAX_LEN - this->m_sensorDataIdx - 10;
    }
    byte len_buf = dn_length;
    char str_buf[dn_length];
    //chop the device name if needed
    device_name.substring(0, dn_length - 1).toCharArray(str_buf, dn_length);
    payloadData += len_buf;         // Add the length of the Name
    payloadData += COMPLETE_NAME;   // Complete_Name: Complete local name -- Short_Name: Shortened Name
    payloadData += str_buf;         // Add the Name to the payload
    }
  */

  serviceData += SERVICE_DATA;  // DO NOT CHANGE -- Service Data - 16-bit UUID
  serviceData += UUID1;  // DO NOT CHANGE -- UUID
  serviceData += UUID2;  // DO NOT CHANGE -- UUID
  // The encryption
  if (this->m_encryptEnable) {
    serviceData += ENCRYPT;

    uint8_t ciphertext[BLE_ADVERT_MAX_LEN];
    uint8_t encryptionTag[MIC_LEN];
    //buildNonce
    uint8_t nonce[NONCE_LEN];
    uint8_t* countPtr  = (uint8_t*)(&this->m_encryptCount);
    esp_read_mac(&nonce[0], ESP_MAC_BT);
    nonce[6] = UUID1;
    nonce[7] = UUID2;
    nonce[8] = ENCRYPT;
    memcpy(&nonce[9], countPtr, 4);
    //encrypt sensorData
    mbedtls_ccm_encrypt_and_tag(&this->m_encryptCTX, this->m_sensorDataIdx, nonce, NONCE_LEN, 0, 0,
                                &this->m_sensorData[0], &ciphertext[0], encryptionTag,
                                MIC_LEN);
    for (i = 0; i < this->m_sensorDataIdx; i++)
    {
      serviceData += ciphertext[i];
    }
    //writeCounter
    serviceData += nonce[9];
    serviceData += nonce[10];
    serviceData += nonce[11];
    serviceData += nonce[12];
    this->m_encryptCount++;
    //writeMIC
    serviceData += encryptionTag[0];
    serviceData += encryptionTag[1];
    serviceData += encryptionTag[2];
    serviceData += encryptionTag[3];
  }
  else {
    serviceData += NO_ENCRYPT;
    for (i = 0; i < this->m_sensorDataIdx; i++)
    {
      serviceData += this->m_sensorData[i]; // Add the sensor data to the Service Data
    }
  }

  byte sd_length = serviceData.length(); // Generate the length of the Service Data
  payloadData += sd_length;         // Add the length of the Service Data
  payloadData += serviceData;             // Finalize the packet

  oAdvertisementData.addData(payloadData);
  pAdvertising->setAdvertisementData(oAdvertisementData);

  //fill the local name into oScanResponseData
  if (!this->dev_name.isEmpty()) {
    int dn_length = this->dev_name.length() + 1;
    if (dn_length > 28) dn_length = 28;//BLE_ADVERT_MAX_LEN - FLAG = 31 - 3
    oScanResponseData.setName(this->dev_name.substring(0, dn_length - 1).c_str());
  }
  pAdvertising->setScanResponseData(oScanResponseData);

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

void BTHome::sendPacket(uint32_t delay_ms)
{
  if (this->m_sensorDataIdx > 0) {
    buildPaket();
    if (!isAdvertising()) start();
    delay(delay_ms);
    resetMeasurement();
  }
}

uint8_t BTHome::getByteNumber(uint8_t sens) {
  switch (sens)
  {
    case ID_BATTERY:
    case ID_COUNT:
    case ID_HUMIDITY:
    case ID_MOISTURE:
    case ID_UV:
      return 1; break;
    case ID_DURATION:
    case ID_ENERGY:
    case ID_GAS:
    case ID_ILLUMINANCE:
    case ID_POWER:
    case ID_PRESSURE:
      return 3; break;
    case ID_COUNT4:
    case ID_ENERGY4:
    case ID_GAS4:
    case ID_VOLUME:
    case ID_WATER:
      return 4; break;
    default:
      return 2;
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
