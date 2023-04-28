# BTHomeV2 ESP32 Example
# V2 of the example, simplified for least input
An example BTHome v2 example with a presence and a count sensor

# Notice it's currently under development

This is an example for a DIY [BTHome v2](https://bthome.io/) sensor.

The original code is from: https://github.com/TheDigital1/ESP32_BTHome

The header file contains human readable variables for the hex Object ids.

Sending entities to Home Assistant is simplified.

1) Download the files from Arduino Code folder.

2) Edit the BTHome.ino in your favourite Arduino IDE 

3) Change the DEVICE_NAME to the one that should be recognised in BTHome integration

4) Add the object id of the individual entities and the state of it as in the examples

5) Compile and upload to ESP32

DONE!

Read the comments in the code for more info. 
For the object ids consider using the variables from BTHome.h instead.

Unfortunately the encryption documentation is not clear to me and can't yet implement it.
