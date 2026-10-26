# P1-dongle
ESP32-based firmware and hardware to receive, parse, store and forward energy data coming from the P1 port of Fluvius digital meters (Flanders, Belgium). Also reads other Belgian and Dutch DSMR meters. Luxembourg meters encrypt their telegrams and are not supported.

![](https://github.com/plan-d-io/P1-dongle/wiki/images/P1dongles.jpg)

Please see the [wiki](https://github.com/plan-d-io/P1-dongle/wiki) for extended instructions on how to build, configure and use the P1-dongle.

## Firmware and source code
I have moved all my P1 projects, this WiFi dongle as well as the LoRa transmitter and
receiver, into one code base: [plan-d-io/P1-firmware](https://github.com/plan-d-io/P1-firmware).
Since version 3.00 the dongle firmware is developed there, together with its issue
tracker and release notes. This repository keeps the hardware design and serves the
files that dongles download when they update themselves. The source code of the 2.x
firmware stays available in the history of this repository (tag `V2.23`).

## Features
- Professional looking yet easy DIY build
- Cheap (<€20 DIY cost)
- Easy to configure through a web interface or a local HTTP API
- MQTT and MQTT over TLS to your own broker
- Native integration into Home Assistant (MQTT discovery)
- Optional upload to EnergieID
- Privacy first: cloud, or even internet access entirely optional
- Automatic firmware updates (optional), with automatic rollback when a new version
  fails to start
- Source-available firmware and hardware design (PolyForm Noncommercial 1.0.0)

I do not provide any guarantees nor timelines for developing functionality. I am however available for specific functionality or developments at request. 
Contributions are welcome in P1-firmware.

## Basic installation
1. Plug the P1-dongle into the P1 port of your digital meter
2. When the LED turns blue, connect to the `p1dongle` WiFi network
3. Open the setup page, or go to `192.168.4.1`
4. Choose your WiFi network, complete the configuration and save. Reboot the dongle to apply the new network settings; it then joins your network
