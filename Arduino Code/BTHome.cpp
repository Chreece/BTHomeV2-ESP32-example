#include "Arduino.h"
#include "NimBLEDevice.h"
#include "BTHome.h"

static BLEAdvertising *pAdvertising;

void BTHome::begin(String dname, bool encryption, uint8_t const* const key, bool trigger_based_device) {
  BLEDevice::init("");
  pAdvertising = BLEDevice::getAdvertising();
  setDeviceName(dname);
  if (encryption) {
    this->m_encryptEnable = true;
#if defined(ESP32)
    this->m_encryptCount = esp_random() % 0x427;
#else
    this->m_encryptCount = random(0, UINT32_MAX) % 0x427;
#endif
    memcpy(bindKey, key, sizeof(uint8_t) * BIND_KEY_LEN);
    mbedtls_ccm_init(&this->m_encryptCTX);
    mbedtls_ccm_setkey(&this->m_encryptCTX, MBEDTLS_CIPHER_ID_AES, bindKey, BIND_KEY_LEN * 8);
  }
  else this->m_encryptEnable = false;
  this->m_triggerdevice = trigger_based_device;
  resetMeasurement();
}

void BTHome::begin(String dname, bool encryption, String key, bool trigger_based_device) {
  uint8_t bind_key[BIND_KEY_LEN];
  for (uint8_t i = 0; i < BIND_KEY_LEN; i++) {
    bind_key[i] = strtol(key.substring(i * 2, i * 2 + 2).c_str(), NULL, BIND_KEY_LEN);
  }
  begin(dname, encryption, bind_key, trigger_based_device);
}

void BTHome::setDeviceName(String dname) {
  if (!dname.isEmpty())
    this->dev_name = dname;
}

void BTHome::resetMeasurement() {
  this->m_sensorDataIdx = 0;
  this->last_object_id = 0;
  this->m_sortEnable = false;
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
    if (!this->m_sortEnable) {
      if (sensor_id < this->last_object_id) this->m_sortEnable = true;
    }
    last_object_id = sensor_id;
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
    if (!this->m_sortEnable) {
      if (sensor_id < this->last_object_id) this->m_sortEnable = true;
    }
    last_object_id = sensor_id;
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
    if (!this->m_sortEnable) {
      if (sensor_id < this->last_object_id) this->m_sortEnable = true;
    }
    last_object_id = sensor_id;
  }
  else {
    sendPacket();
    addMeasurement(sensor_id, value);
  }
}

void BTHome::sortSensorData() {
  uint8_t i, j, k, data_block_num;

  struct DATA_BLOCK
  {
    byte object_id;
    byte data[4];
    uint8_t data_len;
  };
  struct DATA_BLOCK data_block[MEASUREMENT_MAX_LEN / 2 + 1];
  struct DATA_BLOCK temp_data_block;

  for (i = 0, j = 0, data_block_num = 0; j < this->m_sensorDataIdx; i++) {
    //copy the object id
    data_block[i].object_id = this->m_sensorData[j];
    data_block_num++;
    //copy the data length
    if (this->m_sensorData[j] == EVENT_DIMMER) {
      if (this->m_sensorData[j + 1] == EVENT_DIMMER_NONE)
        data_block[i].data_len = 1;
      else
        data_block[i].data_len = 2;
    }
    else {
      data_block[i].data_len = getByteNumber(this->m_sensorData[j]);
    }
    //copy the data
    for (k = 0; k < data_block[i].data_len; k++) {
      data_block[i].data[k] = this->m_sensorData[j + 1 + k];
    }
    //move to the next object id location
    j = j + data_block[i].data_len + 1;
  }

  if (data_block_num > 1) {
    //bubble sort
    for (i = 0; i < data_block_num - 1; i++) {
      for (j = 0; j < data_block_num - 1 - i; j++) {
        if (data_block[j].object_id > data_block[j + 1].object_id) {
          memcpy(&temp_data_block, &data_block[j], sizeof(struct DATA_BLOCK));
          memcpy(&data_block[j], &data_block[j + 1], sizeof(struct DATA_BLOCK));
          memcpy(&data_block[j + 1], &temp_data_block, sizeof(struct DATA_BLOCK));
        }
      }
    }
    //copy the new order to m_sensorData array
    for (i = 0, j = 0; i < data_block_num && j < this->m_sensorDataIdx; i++) {
      this->m_sensorData[j] = data_block[i].object_id;
      for (k = 0; k < data_block[i].data_len; k++) {
        this->m_sensorData[j + 1 + k] = data_block[i].data[k];
      }
      j = j + data_block[i].data_len + 1;
    }
  }
}

void BTHome::buildPaket() {
  //the Object ids have to be applied in numerical order (from low to high)
  if (this->m_sortEnable) sortSensorData();

  // Create the BLE Device
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  BLEAdvertisementData oScanResponseData = BLEAdvertisementData();

  std::string payloadData = "";
  std::string serviceData = "";
  uint8_t i;

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
    if (this->m_triggerdevice)
      serviceData += ENCRYPT_TRIGGER_BASE;
    else
      serviceData += ENCRYPT;

    uint8_t ciphertext[BLE_ADVERT_MAX_LEN];
    uint8_t encryptionTag[MIC_LEN];
    //buildNonce
    uint8_t nonce[NONCE_LEN];
    uint8_t* countPtr  = (uint8_t*)(&this->m_encryptCount);
    const uint8_t *addrs = BLEDevice::getAddress().getNative();
    nonce[0] = addrs[5];
    nonce[1] = addrs[4];
    nonce[2] = addrs[3];
    nonce[3] = addrs[2];
    nonce[4] = addrs[1];
    nonce[5] = addrs[0];
    //esp_read_mac(&nonce[0], ESP_MAC_BT);
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
    if (this->m_triggerdevice)
      serviceData += NO_ENCRYPT_TRIGGER_BASE;
    else
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
    case ID_PACKET:
    case ID_BATTERY:
    case ID_COUNT:
    case ID_HUMIDITY:
    case ID_MOISTURE:
    case ID_UV:
    case STATE_BATTERY_LOW:
    case STATE_BATTERY_CHARHING:
    case STATE_CO:
    case STATE_COLD:
    case STATE_CONNECTIVITY:
    case STATE_DOOR:
    case STATE_GARAGE_DOOR:
    case STATE_GAS_DETECTED:
    case STATE_GENERIC_BOOLEAN:
    case STATE_HEAT:
    case STATE_LIGHT:
    case STATE_LOCK:
    case STATE_MOISTURE:
    case STATE_MOTION:
    case STATE_MOVING:
    case STATE_OCCUPANCY:
    case STATE_OPENING:
    case STATE_PLUG:
    case STATE_POWER_ON:
    case STATE_PRESENCE:
    case STATE_PROBLEM:
    case STATE_RUNNING:
    case STATE_SAFETY:
    case STATE_SMOKE:
    case STATE_SOUND:
    case STATE_TAMPER:
    case STATE_VIBRATION:
    case STATE_WINDOW:
    case EVENT_BUTTON:
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
