# P1-dongle
ESP32-based firmware and hardware to receive, parse, store and forward energy data coming from the P1 port of Fluvius digital meters (Flanders, Belgium). Should also work on Dutch, Walloon, Luxembourg and Swedish digital meters (untested).

![](https://github.com/plan-d-io/P1-dongle/wiki/images/P1dongles.jpg)

Please see the [wiki](https://github.com/plan-d-io/P1-dongle/wiki) for extended instructions on how to build, configure and use the P1-dongle.

## Features
- Professional looking yet easy DIY build
- Cheap (<€20 DIY cost)
- Easy to configure through web-based interface or RESTful API
- Supports MQTT, MQTT over TLS, HTTP and HTTPS data upload to local or remote servers
- Native integration into Home Assistant (MQTT autodiscovery)
- Open source firmware & hardware
- Privacy first: cloud, or even internet access entirely optional
- Auto-updating firmware (optional)
- Easy to use as a basis for other (professional) projects

I do not provide any guarantees nor timelines for developing functionality. I am however available for specific functionality or developments at request. 
And, as this is an open-source project, you are free to contribute yourself!

## Basic installation
1. Plug the P1-dongle into the P1 port of your digital meter
2. When the LED turns greens, connect to the `p1-dongle` access point
3. When connected, open the login page or go to `192.168.4.1`
4. Complete the configuration, and click the reboot button at the bottom of the webpage
