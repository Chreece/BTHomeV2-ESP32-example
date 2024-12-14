# This project is no longer compatible with the changes from [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino/commit/edc66e8b6c992d4992602c091bcee26261aa77d3) library. You need to downgrade it in order to use this example.
# There will be no longer any updates here unless someone makes a PR to support these changes

# BTHomeV2 ESP32 Example
# V2 of the example, simplified for least input
An BTHome v2 example.
Make your DIY sensor, BTHome compatible.

Thanks [@countrysideboy](https://github.com/countrysideboy) for his contributions on Encryption and cleanup code, the most of the code is from his side!

This is an example for a DIY [BTHome v2](https://bthome.io/) sensor.

The original code is from: https://github.com/TheDigital1/ESP32_BTHome

The header file contains human readable variables for the hex Object ids.

Sending entities to Home Assistant is simplified.

1) Download the files from Arduino Code folder.

2) Edit the BTHome.ino in your favourite Arduino IDE 

3) Change the DEVICE_NAME to the one that should be recognised in BTHome integration

4) Change the BIND_KEY with your preference or remove it along with the `#define ENABLE_ENCRYPT` for no encryption

5) Add the object id of the individual entities and the state of it as in the examples

6) Compile and upload to ESP32

DONE!

Read the comments in the code for more info. 
For the object ids consider using the variables from BTHome.h instead.
