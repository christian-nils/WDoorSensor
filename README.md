# WDoorSensor
Replaces original Tuya firmware on door sensors with battery and ESP8266 wifi module. The firmware is tested with following device:
* TYMC-1 Earykong (oval shaped door sensor)

The sensor can be purchased on AliExpress for example

![Flashing connection](https://github.com/klausahrenberg/WDoorSensor/blob/master/docs/images/doorsensor.jpg)

## Features
* No Tuya cloud connection anymore
* Enables sensor to send door and battery state via MQTT 
* Configuration of connection and device parameters via web interface
## Installation
To install the firmware, follow instructions here:  
https://github.com/klausahrenberg/WDoorSensor/blob/master/Flashing.md
## Initial configuration
After installation/flashing of firmware, disconnect all connections from programmer and insert the batteries:
* Hold reset button for 5 sec until red LED starts Flashing
* Now you have 60 seconds to configure... If you need more time, move the magnet during configuration.
* Look for wifi AP 'DoorSensor...'
* Configure Wifi-Settings
## Json structure
```json
{
  "idx":"doorsensor",
  "ip":"192.168.x.x",
  "firmware":"x.xx",
  "open":true|false,
  "battery":"low|medium|high"
}
```
### Build this firmware from source
For build from sources you can use the Arduino-IDE, Atom IDE or other. All sources needed are inside the folder 'WDoorSensor' and my other library: https://github.com/klausahrenberg/WAdapter. Additionally you will need some other libraries: DNSServer, EEPROM (for esp8266), ESP8266HTTPClient, ESP8266mDNS, ESP8266WebServer, ESP8266WiFi, Hash, NTPClient, Time - It's all available via board and library manager inside of ArduinoIDE
