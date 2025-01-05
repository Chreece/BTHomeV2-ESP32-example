This repository demonstatrates how you to create your own Bluetooth Low Energy (BLE) on an ESP32 using the Arduino framework.

The [BTHome v2](https://bthome.io/) sensor broadcasting standard is used so that Home Assistant will automatically discovery your device (the BTHome integration is required). BTHome allows you to create your own DIY device and send sensor data to Home Assistant over BLE rather that Wifi. This is a significantly more energy efficient approach, especially if you oplan to run your ESP32 on batteries.

Tested on a TTGO ESP WROVER T8 v1.8, using NimBLE-Arduino v2.1.2

This repository is a fork of the following repository. I have modified it to utilise [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino) v2.x. Thanks to everyone for their contributions. This is awesome.


<hr/>
~# The changes to the [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino/commit/edc66e8b6c992d4992602c091bcee26261aa77d3) library are no longer compatible with the project.~
~# PRs are welcome but I'm no longer maintaining this example~

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
