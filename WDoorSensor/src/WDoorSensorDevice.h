#ifndef DOOR_SENSOR_MCU_H
#define	DOOR_SENSOR_MCU_H

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include "WTuyaDevice.h"

class WDoorSensorDevice: public WTuyaDevice {
public:

  WDoorSensorDevice(WNetwork* network)
  	: WTuyaDevice(network, "sensor", "sensor", DEVICE_TYPE_DOOR_SENSOR) {
		this->open = new WProperty("open", "Open", BOOLEAN, TYPE_OPEN_PROPERTY);
    this->open->setReadOnly(true);
	  this->addProperty(open);
    this->tampered_open = new WProperty("tampered_open", "Tampered Sensor Open", BOOLEAN, TYPE_OPEN_PROPERTY);
    this->tampered_open->setReadOnly(true);
	  this->addProperty(tampered_open);
    this->battery = new WProperty("battery", "Battery", INTEGER, "");
    this->battery->setReadOnly(true);
    this->addProperty(battery);
    this->initializationStep = 0;
    this->configButtonPressed = false;
    this->notifyAllMcuCommands->setBoolean(false);
  }

  bool isDeviceStateComplete() {
    return ((!this->open->isNull()) && (!this->tampered_open->isNull()) && (!this->battery->isNull()));
  }

  virtual void cancelConfiguration() {
    //send confirmation to put ESP in deep sleep again
    //55 AA 00 05 00 01 00 05
    commandTuyaToSerial(0x05, 0);
    delay(250);
    commandTuyaToSerial(0x05, 0);
    delay(250);
  }

  void loop(unsigned long now) {
    if ((!this->configButtonPressed) && (this->isDeviceStateComplete())) {
      //set the ESP in sleep mode
      cancelConfiguration();
    }
    WTuyaDevice::loop(now);
  }

  void commandTuyaToSerial(byte commandByte) {
    commandTuyaToSerial(commandByte, 0xFF);
  }

  void commandTuyaToSerial(byte commandByte, byte value) {
    unsigned char tuyaCommand[] = { 0x55, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00 };
    tuyaCommand[3] = commandByte;
    tuyaCommand[5] = (value != 0xFF ? 0x01 : 0x00);
    tuyaCommand[6] = (value != 0xFF ? value : 0x00);
    commandCharsToSerial(6 +  (value != 0xFF ? 1 : 0), tuyaCommand);
  }

  void commandTuyaToSerial(byte commandByte, byte value0, byte value1) {
    unsigned char tuyaCommand[] = { 0x55, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    tuyaCommand[3] = commandByte;
    tuyaCommand[5] = 0x02;
    tuyaCommand[6] = (value0 != 0xFF ? value0 : 0x00);
    tuyaCommand[7] = (value0 != 0xFF ? value0 : 0x00);
    commandCharsToSerial(8, tuyaCommand);
  }

  void queryDeviceState() {
    if (!this->configButtonPressed) {
      network->debug(F("Query state of MCU..."));
      commandTuyaToSerial(0x01);
    }
  }

 protected:

  virtual bool processCommand(byte commandByte, byte length) {
    bool knownCommand = false;
    // Initialization sequence:
        // MCU: 55 AA 00 01 00 25 7B 22 70 22 3A 22 64 7A 70 68 67 6D 72 6D 30 6A 30 68 6C 78 66 70 22 2C 22 76 22 3A 22 31 2E 30 2E 31 37 22 7D 2D 
        // ESP: 55 AA 00 02 00 01 02 04 (0)
        // MCU: 55 AA 00 02 00 00 01 
        // ESP: 55 AA 00 02 00 01 02 04 (1)
        // MCU: 55 AA 00 02 00 00 01 
        // ESP: 55 AA 00 02 00 01 03 05 (2)
        // MCU: 55 AA 00 02 00 00 01 
        // ESP: 55 AA 00 02 00 01 04 06 (3)
        // MCU: 55 AA 00 02 00 00 01 
        // ESP: 55 AA 00 05 00 01 00 05 (4) if device initialized otherwise 
                
    switch (commandByte) {
      // network->debug(F("commandByte received: '%a'"), commandByte);
      case 0x01: {
        //Response to initialization request commandTuyaToSerial(0x01);
        //55 aa 00 01 00 24 7b 22 70 22 3a 22 68 78 35 7a 74 6c 7a 74 69 6a 34 79 78 78 76 67 22 2c 22 76 22 3a 22 31 2e 30 2e 30 22 7d

        //CN: 55 AA 00 01 00 25 7B 22 70 22 3A 22 64 7A 70 68 67 6D 72 6D 30 6A 30 68 6C 78 66 70 22 2C 22 76 22 3A 22 31 2E 30 2E 31 37 22 7D 2D
        this->initializationStep = 1; 
        commandTuyaToSerial(0x02, 2);
        knownCommand = true;
        break;
      }
      case 0x02: {

        if (length == 0) {

          switch(this->initializationStep){
            case 1:
              commandTuyaToSerial(0x02, 2);
              this->initializationStep = 2; 
              break;
            
            case 2:
              commandTuyaToSerial(0x02, 3);
              this->initializationStep = 3; 
              break;
            
            case 3:
              commandTuyaToSerial(0x02, 4);
              this->initializationStep = 4; 
              break;          
            case 4:
              this->initializationStep = 0; 
              break;
            default:
              commandTuyaToSerial(0x02, 4);
          }          
          knownCommand = true;

        }
        break;
      }
      case 0x03: {
        //Button was pressed > 5 sec - red blinking led
        //55 aa 00 03 00 00
        this->configButtonPressed = true;
        network->debug(F("Config button pressed..."));
        knownCommand = true;
        break;
      }
      case 0x04: {
        //Setup initialization request
        //55 aa 00 04 00 01 00
        network->startWebServer();
        knownCommand = true;
        break;
      }
      case 0x05: {
        // 55 aa 00 05 00 12 65 01 00 01 01 66 01 00 01 01 67 02 00 04 00 00 00 64
        
        // Cleverio Door Sensor:
            // - 65 01 00 01 01: door switch open | 65 01 00 01 00: door switch closed
            // - 66 01 00 01 01: tampered switch open | 66 01 00 01 00: tampered switch closed
            // - 67 02 00 04 00 00 00: battery level 
        if (length == 0x12) {
          //door state
          this->open->setBoolean(receivedCommand[10] == 0x01);
          this->tampered_open->setBoolean(receivedCommand[15] == 0x01);
          this->battery->setInteger((int)receivedCommand[23]);
          commandTuyaToSerial(0x05, 0);
          knownCommand = true;
          
          //battery state
          // battery->setString(battery->getEnumString(receivedCommand[10]));
        }
        break;
      }
    }
    return knownCommand;
  }

  virtual bool processStatusCommand(byte statusCommandByte, byte length) {
    bool knownCommand = false;
    if (length == 0) {
      commandTuyaToSerial(0x07, 0, 0); 
      knownCommand = true;
    }
    return knownCommand;
  }

private:
  bool configButtonPressed;
	WProperty* open; // door sensor
  WProperty* tampered_open; // tampered sensor
  WProperty* battery; // battery level
  int initializationStep;
};


#endif
