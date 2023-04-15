#define FLAGS 0x020106
#define UUID1 0xD2
#define UUID2 0xFC
#define SERVICE_DATA 0x16

#define NO_ENCRYPT 0x40
#define ENCRYPT 0x41

#define SHORT_NAME 0x08
#define COMPLETE_NAME 0x09

#define BATTERY 0x01
#define CO2 0x12
#define COUNT 0x09
#define COUNT2 0x3D
#define COUNT4 0x3E
#define CURRENT 0x43
#define DEWPOINT 0x08
#define DISTANCE 0x40
#define DISTANCEM 0x41
#define DURATION 0x42
#define ENERGY 0x0A
#define ENERGY4 0x4D
#define GAS 0x4B
#define GAS4 0x4C
#define HUMIDITY 0x2E
#define HUMIDITY2 0x03
#define ILLUMINANCE 0x05
#define MASS 0x06
#define MASSLB 0x07
#define MOISTURE 0x2F
#define MOISTURE2 0x14
#define PM25 0x0D
#define PM10 0x0E
#define POWER 0x0B
#define PRESSURE 0x04
#define ROTATION 0x3F
#define SPD 0x44
#define TEMPERATURE 0x02
#define TEMPERATURE1 0x45
#define TVOC 0x13
#define VOLTAGE 0x0C
#define VOLTAGE1 0x4A
#define VOLUME 0x4E
#define VOLUME1 0x47
#define VOLUME2 0x48
#define VOLUMEFR 0x49
#define UV 0x46
#define WATER 0x4F

#define BATTERY_LOW 0x15
#define BATTERY_CHARHING 0x16
#define CO 0x17
#define COLD 0x18
#define CONNECTIVITY 0x19
#define DOOR 0x1A
#define GARAGE_DOOR 0x1B
#define GAS_DETECTED 0x1C
#define GENERIC_BOOLEAN 0x0F
#define HEAT 0x1D
#define LIGHT 0x1E
#define LOCK 0x1F
#define MOISTURE 0x20
#define MOTION 0x21
#define MOVING 0x22
#define OCCUPANCY 0x23
#define OPENING 0x11
#define PLUG 0x24
#define POWER_ON 0x10
#define PRESENCE 0x25
#define PROBLEM 0x26
#define RUNNING 0x27
#define SAFETY 0x28
#define SMOKE 0x29
#define SOUND 0x2A
#define TAMPER 0x2B
#define VIBRATION 0x2C
#define WINDOW 0x2D

#define ON 0x01
#define OFF 0x00

#define BUTTON 0x3A00
#define BUTTON_PRESS 0x3A01
#define BUTTON_DOUBLE_PRESS 0x3A02
#define BUTTON_TRIPLE_PRESS 0x3A03
#define BUTTON_LONG_PRESS 0x3A04
#define BUTTON_LONG_DOUBLE_PRESS 0x3A05
#define BUTTTON_LONG_TRIPLE_PRESS 0x3A05

#define DIMMER 0x3C00
#define DIMMER_LEFT 0x3C01
#define DIMMER_RIGHT 0x3C01

class BTHome{
  private:
    std::string intToLittleEndianHexString(int value, int numBytes);
    int getByteNumber(int sens);
    std::string intToFourBytes(int value);
  
  public:
    void sendPaket(int sensorNumber, int sensor[], int state[], String device_name, bool encryption);
};
