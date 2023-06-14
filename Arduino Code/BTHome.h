#include "mbedtls/ccm.h"
#if defined(ESP32)
#include "esp_random.h"
#endif

#define BLE_ADVERT_MAX_LEN 31
#define MEASUREMENT_MAX_LEN 23 //23=31(BLE_ADVERT_MAX_LEN)-3(FLAG)-1(SERVICE_DATA)-2(UUID)-1(ENCRYPT)-1(serviceData length bit)
#define BIND_KEY_LEN 16
#define NONCE_LEN 13
#define MIC_LEN 4

#define FLAG 0x020106
#define FLAG1 0x02
#define FLAG2 0x01
#define FLAG3 0x06
#define UUID 0xD2FC
#define UUID1 0xD2
#define UUID2 0xFC
#define SERVICE_DATA 0x16

#define NO_ENCRYPT 0x40
#define NO_ENCRYPT_TRIGGER_BASE 0x44
#define ENCRYPT 0x41
#define ENCRYPT_TRIGGER_BASE 0x45

#define SHORT_NAME 0x08
#define COMPLETE_NAME 0x09

#define ID_PACKET 0x00
#define ID_BATTERY 0x01
#define ID_CO2 0x12
#define ID_COUNT 0x09
#define ID_COUNT2 0x3D
#define ID_COUNT4 0x3E
#define ID_CURRENT 0x43
#define ID_DEWPOINT 0x08
#define ID_DISTANCE 0x40
#define ID_DISTANCEM 0x41
#define ID_DURATION 0x42
#define ID_ENERGY 0x0A
#define ID_ENERGY4 0x4D
#define ID_GAS 0x4B
#define ID_GAS4 0x4C
#define ID_HUMIDITY 0x2E
#define ID_HUMIDITY_PRECISE 0x03
#define ID_ILLUMINANCE 0x05
#define ID_MASS 0x06
#define ID_MASSLB 0x07
#define ID_MOISTURE 0x2F
#define ID_MOISTURE_PRECISE 0x14
#define ID_PM25 0x0D
#define ID_PM10 0x0E
#define ID_POWER 0x0B
#define ID_PRESSURE 0x04
#define ID_ROTATION 0x3F
#define ID_SPD 0x44
#define ID_TEMPERATURE_PRECISE 0x02
#define ID_TEMPERATURE 0x45
#define ID_TVOC 0x13
#define ID_VOLTAGE 0x0C
#define ID_VOLTAGE1 0x4A
#define ID_VOLUME 0x4E
#define ID_VOLUME1 0x47
#define ID_VOLUME2 0x48
#define ID_VOLUMEFR 0x49
#define ID_UV 0x46
#define ID_WATER 0x4F

#define STATE_BATTERY_LOW 0x15
#define STATE_BATTERY_CHARHING 0x16
#define STATE_CO 0x17
#define STATE_COLD 0x18
#define STATE_CONNECTIVITY 0x19
#define STATE_DOOR 0x1A
#define STATE_GARAGE_DOOR 0x1B
#define STATE_GAS_DETECTED 0x1C
#define STATE_GENERIC_BOOLEAN 0x0F
#define STATE_HEAT 0x1D
#define STATE_LIGHT 0x1E
#define STATE_LOCK 0x1F
#define STATE_MOISTURE 0x20
#define STATE_MOTION 0x21
#define STATE_MOVING 0x22
#define STATE_OCCUPANCY 0x23
#define STATE_OPENING 0x11
#define STATE_PLUG 0x24
#define STATE_POWER_ON 0x10
#define STATE_PRESENCE 0x25
#define STATE_PROBLEM 0x26
#define STATE_RUNNING 0x27
#define STATE_SAFETY 0x28
#define STATE_SMOKE 0x29
#define STATE_SOUND 0x2A
#define STATE_TAMPER 0x2B
#define STATE_VIBRATION 0x2C
#define STATE_WINDOW 0x2D

#define STATE_ON 0x01
#define STATE_OFF 0x00

#define EVENT_BUTTON 0x3A
#define EVENT_BUTTON_NONE 0x00
#define EVENT_BUTTON_PRESS 0x01
#define EVENT_BUTTON_DOUBLE_PRESS 0x02
#define EVENT_BUTTON_TRIPLE_PRESS 0x03
#define EVENT_BUTTON_LONG_PRESS 0x04
#define EVENT_BUTTON_LONG_DOUBLE_PRESS 0x05
#define EVENT_BUTTTON_LONG_TRIPLE_PRESS 0x06

#define EVENT_DIMMER 0x3C
#define EVENT_DIMMER_NONE 0x00
#define EVENT_DIMMER_LEFT 0x01
#define EVENT_DIMMER_RIGHT 0x02

class BTHome {
  public:
    void begin(String dname = "DIY-sensor", bool encryption = false, uint8_t const* const key = NULL, bool trigger_based_device = false);
    void begin(String dname = "DIY-sensor", bool encryption = false, String key = "", bool trigger_based_device = false);
    void setDeviceName(String dname = "");
    void buildPaket();
    void start(uint32_t duration = 0);
    void stop();
    bool isAdvertising();
    void resetMeasurement();
    void sendPacket(uint32_t delay_ms = 1500);
    void addMeasurement_state(uint8_t sensor_id, uint8_t state, uint8_t steps = 0);
    void addMeasurement(uint8_t sensor_id, uint64_t value);
    void addMeasurement(uint8_t sensor_id, float value);

  private:
    uint8_t getByteNumber(uint8_t sens);
    uint16_t getFactor(uint8_t sens);
    uint8_t m_sensorDataIdx;
    byte m_sensorData[MEASUREMENT_MAX_LEN] = {0};
    void sortSensorData();
    String dev_name;
    bool m_encryptEnable;
    bool m_triggerdevice;
    uint32_t m_encryptCount;
    mbedtls_ccm_context m_encryptCTX;
    uint8_t bindKey[BIND_KEY_LEN];
    bool m_sortEnable;
    byte last_object_id;
};
