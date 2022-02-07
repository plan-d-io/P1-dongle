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

## To-do list
These features are partially implemented or planned to be implemented.
- P1-port passthrough (to daisy chain another P1 dongle)
- Connection for up to 2 pulse inputs (e.g. gas or water meter)
- Multiple HTTP(S) endpoints
- Configurable data payload
- Temperature sensor input

As this is a non-commercial development, we do not provide any timelines nor guarantees for this functionality.

## Requirements for DIY build
- Arduino IDE
- ESP32-based development board (M5 Atom Lite recommended)
- P1 level shifter (M5 Atom hardware design available in /hardware)
- Digital meter with P1 port

>Currently this is a DIY project. If we detect sufficient interest in this project, we might start offering more ready-to-use versions of the dongle for reasonable prices.

## Build process
### Using the Arduino IDE
- Download this repo into your Arduino sketch folder
- Replace default WiFiClientSecure library in your Arduino library folder with the version included here
> The default WiFiClientSecure does not support verification of arbitrary TLS certificates,
> which is required to connect to user provided MQTTS or HTTPS servers. 
- Open and compile the P1-dongle-V**X.YY**.ino sketch in the Arduino IDE (with **X.YY** the current release number)
> The Arduino IDE will throw errors indicating missing libraries, which you can 
> install through the library manager
- Flash the sketch to the ESP32 using the following settings:
  - Flash mode: DIO
  - Partion mode: minimal SPIFFS
- After flashing, use the ESP32 Sketch Data Upload function to transfer the web interface files to the ESP32
### Direct flash with esptool
You can download a binary compiled for ESP32-PICO-D4 chips (e.g. M5 Atom Lite) from the /bin folder and flash this directly with the Espressif esptool. Use the following options (change the serial port and binary name)
> python3 -m esptool --chip esp32 --port **/dev/YOURSERIALPORTHERE** --baud 115200 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size detect 0xe000 boot_app0.bin 0x1000 bootloader_dio_80m.bin 0x10000 P1-dongle-V**X.YY**.ino.bin 0x8000 P1-dongle-V**X.YY**.ino.partitions.bin

The binaries in /bin are kept up to date to the latest version, as they are also downloaded by the dongle when the firmware auto-update is enabled.

## Hardware
The firmware is built for and tested on ESP32-PICO-D4 chips, but should work on any ESP32 flavour.

The following hardware is recommended:
- [ATOM Lite ESP32 IoT Development Kit](https://shop.m5stack.com/products/atom-lite-esp32-development-kit?variant=32259605200986)
- [ATOMIC DIY Proto Kit for ATOM](https://shop.m5stack.com/products/atomic-proto-kit)
- RJ12 to 6 Pins Dupont-Jumper Adapter

The P1 port has a 5V TTL signal level, while the ESP32 GPIO are rated 3.3V. A schematic for a level shifter is provided in the /hardware folder. You can solder these components onto the proto PCB provided with the ATOMIC DIY Proto Kit. When using this schematic, the dongle is powered by the P1 port.

Alternatively, the /hardware folder also contains the files to build a PCB which fits in the ATOMIC DIY Proto Kit housing.

When using this hardware, the end result should look something like this.

![P1 dongle using the M5 Atom](https://github.com/plan-d-io/P1-dongle/blob/main/hardware/P1-dongle.PNG?raw=true)

## Configuration

Installing a P1 dongle consists of a few steps:
1. Opening the P1-port on your digital meter
2. Attaching the P1-dongle to the P1-port
3. Connecting the dongle to your COFY-box

### 1. Opening the P1-port on your digital meter.
The P1-port is not enabled by default. You might have to unlock it on your DSO's portal. It might take some time for the activation of the P1-port to take effect.

> We only document the procedure for Flemish digital meters, please check with your DSO for their specific procedure.

#### Fluvius (Flanders, BE)
1. Login to your Fluvius portal via mijn.fluvius.be.
2. Go to port management `(poortbeheer)`.
3. Click `Poortbeheer aanvragen`.
4. Select the electricity meter that you wish to unlock. If your electricity meter is not yet in the list, you can add it here.
5. Click `Ga verder`.
6. Confirm your request and click `Aanvraag bevestigen`.

>This may trigger a verification step by Fluvius. If your e-mail address is known by your energy supplier (and shared with Fluvius), you will receive a verification e-mail. If no e-mail address is know, you will receive a physical letter in the mail.

7. Check your mailbox (e-mail or physical), and follow the instructions.
8. Return to `Poortbeheer`. You should now see a toggle that allows you to open or close the P1-port. Click the toggle, and change the status to `Open`.
9. Wait until the text changes from `Poort openen...` to `Poort open`. This may take some time.

Once the P1-port has been opened, which can take up to a day or more, the P1-dongle can be connected.

### 2. Attaching the P1-dongle to the P1-port
>The following assumes you are using the recommended hardware

Insert the black end of the RJ-12 to DuPont adapter cable into the dongle, **ensuring the blue cable is at the right side of the dongle** when viewed from above.

![p1cookie5.png](https://docs.cofybox.io/p1cookie5.png)
*Incorrectly connecting the cable does not damage the dongle, but prevents it from functioning correctly.
The USB-C port of the dongle is only used for debugging. **Never plug in this USB port while the dongle is connected to the digital meter!***

>The following assumes you have a Fluvius (Flemish) digital meter
Open the yellow cover in the lower left corner of the digital meter. Plug the other end of the cable into the P1 port on the **right**.

![gebruikerspoorten-digitale-meter.jpg](https://docs.cofybox.io/gebruikerspoorten-digitale-meter.jpg)

*The left port is the S1 port and is not used by the dongle.*

The dongle is now connected to the meter and is starting up. The LED should go from red to green.

### 3. Connecting the dongle to your wifi
The dongle must now be connected to your home wifi. Use a smartphone, tablet or laptop and scan for wifi networks. You should see a network named `COFY`, followed by a bunch of letters and number, e.g. `COFY33DC`. Connect to this network. 

>You might receive an error that this network has no internect connectivity, make sure your device still connects to this network.

After connecting, a webpage should open automatically. If this is not the case, open a browser and surf to `http://192.168.4.1`. This webpage allows you to view the real-time data from your Fluvius digital meter, as well as configure the settings of the dongle.

![p1cookie1.png](https://docs.cofybox.io/p1cookie1.png)

 Click on `Configure`. On the next page, click on `Configure wifi`.
 
 ![p1cookie2.png](https://docs.cofybox.io/p1cookie2.png)

Select your home wifi from the `SSID` dropdown list, fill in the wifi password at `Password` and click `Submit query`. A green text should appear indicating the settings have been saved. Return to the previous menu by clicking on `Configuration`.
 
OPTIONAL: You can configure data endpoints at `Configure cloud`.

 ![p1cookie3.png](https://docs.cofybox.io/p1cookie3.png)
 
You can select a local or remote MQTT broker and optionally use TLS encryption. You can also enable native integration into [Home Assistant](https://www.home-assistant.io/) through its MQTT discovery protocol.

Click on `Submit query` if you changed anything, and then on `Configuration` to return to the previous menu.

Click on `Main menu`. You have now returned to the start page. 

The dongle needs to be rebooted for the changes to take effect. Click on the red `Restart` button and confirm. The dongle will now reboot. You can close the browser and connect back to your normal wifi network.

If the dongle has succesfully connected to the COFY-box wifi, the LED should become blue. If it is green, an error has occured (e.g. you mistyped the wifi password). Repeat the procedure in this paragraph.

## Resetting the dongle
To reset the dongle, press and hold the LED button for around 2 seconds. After release, the dongle will reboot and function as access point again.
