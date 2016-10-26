#ifndef ESP8266_DEVICEMANAGER_H
#define ESP8266_DEVICEMANAGER_H

#include <Arduino.h>
#include <LinkedList.h>
#include "Device.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>


class DeviceManager {
private:
  LinkedList<Device*> _devices;
public:
  DeviceManager();
  void addDevice(Device* device);
  void devices(WiFiClient client);
  void jsonrpc(const char* rawMessage, WiFiClient client);
  uint getDeviceCount();
  Device* getDevice(uint idx);
  void handleDeviceRequest(const char* deviceName, ESP8266WebServer client);

};

#endif
