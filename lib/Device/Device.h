#ifndef ESP8266_DEVICE_H
#define ESP8266_DEVICE_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

typedef uint DEVICE_TYPE;
#define TYPE_SWITCH 1 // Switch on/off
#define TYPE_SENSOR 2 // Sensor can get status

class Device {
private:
  String _name;
  DEVICE_TYPE _type;

public:
  Device(const char* name, DEVICE_TYPE type);
  String getName() {return _name;}
  DEVICE_TYPE getType() {return _type;}
};
#endif
