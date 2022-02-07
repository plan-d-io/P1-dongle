# P1-dongle
ESP32-based firmware and hardware to receive, parse, store and forward energy data coming from the P1 port of Fluvius digital meters (Flanders, Belgium). Should also work on Dutch, Luxembourg and Swedish digital meters (untested).

## Features
- Professional looking yet easy DIY build
- Cheap (<€20 DIY cost)
- Easy to configure through web-based interface
- Supports MQTT, MQTT over TLS, HTTP and HTTPS data upload to local or remote servers
- Native integration into Home Assistant (MQTT autodiscovery)
- Open source firmware & hardware
- Privacy first: cloud, or even internet access entirely optional
- Auto-updating firmware (optional)

## Requirements
- Arduino IDE
- ESP32-based development board (M5 Atom recommended)
- P1 level shifter (M5 Atom hardware design available in /hardware)
- Digital meter with P1 port

## Build process
- Download this repo into your Arduino sketch folder
- Replace default WiFiClientSecure library in your Arduino library folder with the version included here
> The default WiFiClientSecure does not support verification of arbitrary TLS certificates,
> which is required to connect to user provided MQTTS or HTTPS servers. 
- (Optionally) Increase MQTT_MAX_PACKET_SIZE to 2048 in the PubSubClient library
- Open and compile the P1-dongle-VX.YY.ino sketch in the Arduino IDE
> The Arduino IDE will throw errors indicating missing libraries, which you can 
> install through the library manager
- (Alternatively, use the precompiled bin file available under /bin with esptool.py)
- Flash the sketch to the ESP32 using the following settings:
  - Flash mode: DIO
  - Partion mode: minimal SPIFFS
- After flashing, use the ESP32 Sketch Data Upload function to transfer the web interface files to the ESP32
