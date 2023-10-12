# P1-dongle
ESP32-based firmware and hardware to receive, parse, store and forward energy data coming from the P1 port of Fluvius digital meters (Flanders, Belgium). Should also work on Dutch, Walloon, Luxembourg and Swedish digital meters (untested).

![](https://github.com/plan-d-io/P1-dongle/wiki/images/P1dongles.jpg)

Please see the [wiki](https://github.com/plan-d-io/P1-dongle/wiki) for extended instructions on how to build, configure and use the P1-dongle.

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
- ~~P1-port passthrough (to daisy chain another P1 dongle)~~
- Connection for up to 2 pulse inputs (e.g. gas or water meter)
- Multiple HTTP(S) endpoints
- Configurable data payload
- Temperature sensor input

As this is a non-commercial development, we do not provide any timelines nor guarantees for this functionality. You are however free to contribute yourself!
