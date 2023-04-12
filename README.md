# BTHomeV2-ESP32-example
An example BTHome v2 example with a presence and a count sensor

This is an example for a DIY BTHome v2 sensor.

The original code is from: https://github.com/TheDigital1/ESP32_BTHome

The header file contains human readable variables for the hex Object ids.

Note: yes = true, no = false for the binary sensors.

The numerical values will be converted with the intToLittleEndian( var, bytes ) method where: 
var the int value of the sensor 
and bytes the number of hex bytes, that you can find on the Sensor data types on https://bthome.io/format/ (i.e. temperature 0x02 has 2 bytes value).

The length for the messages is automatically generated.

Read the comments in the code for more info on what can be changed and consider using the variables from BTHome.h instead the hex object ids for easier coding.

Unfortunately the encryption documentation is not clear to me and can't yet implement it.
