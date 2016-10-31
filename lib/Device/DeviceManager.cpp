#include "DeviceManager.h"

DeviceManager::DeviceManager() {
  _devices = LinkedList<Device*>();
}

void DeviceManager::addDevice(Device* device) {
  _devices.add(device);
}


Device* DeviceManager::getDevice(uint idx) {
  return _devices.get(idx);
}

uint DeviceManager::getDeviceCount() {
  return _devices.size();
}

const String parseError = "{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32700, \"message\": \"Parse error\"}, \"id\": null}";
const String deviceNotFound = "{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32600, \"message\": \"Device not found\"}, \"id\": null}";

void writeStatusLine(WiFiClient client, int code, String status) {
  client.print("HTTP/1.1 "  + String(code) + " " + status +"\r\n");
}

void writeHeader(WiFiClient client, String key, String value) {
  client.print(key + ":" + value +"\r\n");
}

void writeDefaultHeaders(WiFiClient client) {
  writeHeader(client, "Connection", "close");
  writeHeader(client, "Content-Type", "application/json");
  writeHeader(client, "Access-Control-Allow-Origin", "*");
}

void returnMessage(WiFiClient client, int code, String status, String content) {
  writeStatusLine(client, code, status);
  writeDefaultHeaders(client);
  writeHeader(client, "Content-Length", String(content.length()));
  client.print("\r\n");
  client.print(content.c_str());
  client.print("\r\n");
}

void DeviceManager::devices(WiFiClient client) {
  StaticJsonBuffer<400> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonArray& deviceArray = root.createNestedArray("devices");
  for (int idx = 0; idx < _devices.size(); ++idx) {
    Device *device = _devices.get(idx);
    JsonObject& deviceRoot = deviceArray.createNestedObject();
    deviceRoot["name"] = device->getName();
    deviceRoot["type"] = device->getType();
  }
  int length = root.measureLength() + 1;
  char * body = new char[length];
  root.printTo(body, length);
  //client.print(body);
  returnMessage(client, 200, "OK", body);
  delete body;
}




void DeviceManager::jsonrpc(const char* rawMessage, WiFiClient client) {
  StaticJsonBuffer<400> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(rawMessage);
  if (strcmp(root["jsonrpc"],"2.0") !=0) {
    returnMessage(client, 400, "Bad Request", parseError);
    return;
  }
  Device* device = NULL;
  const char * deviceName = root["params"]["device"];

  for (int idx = 0; idx < _devices.size(); ++idx) {
    if (strcmp(_devices.get(idx)->getName().c_str(),deviceName) == 0) {
      device = _devices.get(idx);
      break;
    }
  }
  //delete deviceName;
  if (device == NULL) {
    returnMessage(client, 404, "Device not found", deviceNotFound);
    return;
  }

  returnMessage(client, 200, "OK", "{\"jsonrpc\": \"2.0\", \"method\": \"foobar\"}");
}

DeviceManager deviceManager;
