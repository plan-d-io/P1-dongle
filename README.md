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
- ESP32-based development board (M5 Atom Lite recommended)
- P1 level shifter (M5 Atom hardware design available in /hardware)
- Digital meter with P1 port

## Build process
### Using the Arduino IDE
- Download this repo into your Arduino sketch folder
- Replace default WiFiClientSecure library in your Arduino library folder with the version included here
> The default WiFiClientSecure does not support verification of arbitrary TLS certificates,
> which is required to connect to user provided MQTTS or HTTPS servers. 
- Open and compile the P1-dongle-VX.YY.ino sketch in the Arduino IDE
> The Arduino IDE will throw errors indicating missing libraries, which you can 
> install through the library manager
- (Alternatively, use the precompiled bin file available under /bin with esptool.py)
- Flash the sketch to the ESP32 using the following settings:
  - Flash mode: DIO
  - Partion mode: minimal SPIFFS
- After flashing, use the ESP32 Sketch Data Upload function to transfer the web interface files to the ESP32
### Direct flash with esptool
You can download a binary compiled for ESP32-PICO-D4 chips (e.g. M5 Atom Lite) from the /bin folder and flash this directly with the Espressif esptool. Use the following options (change the serial port and binary name)
> python3 -m esptool --chip esp32 --port **/dev/YOURSERIALPORTHERE** --baud 115200 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size detect 0xe000 boot_app0.bin 0x1000 bootloader_dio_80m.bin 0x10000 P1-dongle-V**X.YY**.ino.bin 0x8000 P1-dongle-V**X.YY**.ino.partitions.bin
The binaries in /bin are kept up to date to the latest version, as they are also downloaded by the dongle when there firmware auto-update is enabled.

## Hardware
The firmware is built for and tested on ESP32-PICO-D4 chips, but should work on any ESP32 flavour.

The following hardware is recommended:
- [ATOM Lite ESP32 IoT Development Kit](https://shop.m5stack.com/products/atom-lite-esp32-development-kit?variant=32259605200986)
- [ATOMIC DIY Proto Kit for ATOM](https://shop.m5stack.com/products/atomic-proto-kit)
- RJ12 to 6 Pins Dupont-Jumper Adapter

The P1 port is 5V TTL level, while the ESP32 GPIO are rated 3.3V. A schematic for a level shifter is provided in the /hardware folder. You can solder these components onto the proto PCB provided with the ATOMIC DIY Proto Kit. Alternatively, the /hardware folder also contains the files to build a PCB which fits in the ATOMIC DIY Proto Kit housing.

When using this hardware, the end result should look something like this.
![P1 dongle using the M5 Atom](https://github.com/plan-d-io/P1-dongle/blob/main/hardware/P1-dongle.PNG?raw=true)
