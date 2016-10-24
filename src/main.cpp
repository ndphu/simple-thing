#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <WiFiUDP.h>
#include <Ticker.h>
#include <ESP8266SSDP.h>
#include <ArduinoJson.h>

// Define AP Configuration
#define AP_IP IPAddress(19,11,20,12)
#define AP_GATEWAY IPAddress(19,11,20,12)
#define AP_SUBNET IPAddress(255, 255, 255, 0)
String accessPointName;

#define BROKER_HOST "iot.eclipse.org"
#define BROKER_PORT 1883
String clientId;

// MQTT
#define GENERAL_COMMAND_TOPIC "esp8266/all_devices/command"
#define ONLINE_NOTIFICATION_TOPIC "esp8266/all_devices/notify/online"
String deviceCommandTopic;
// Ticker
#define BLINK_LED 2
Ticker blink;

void blinkLed() {
  int state = digitalRead(BLINK_LED);
  digitalWrite(BLINK_LED, !state);
}
// Health
Ticker healthCheck;
bool publishHealthFlag = false;
String deviceHealthTopic;

WiFiManager wifiManager;
WiFiClient wifiClient;
PubSubClient client(wifiClient);

// SSDP
#define SSDP_HTTP_PORT 8080
ESP8266WebServer SSDP_HTTP(SSDP_HTTP_PORT);

void setPublishHealthFlag() {
  publishHealthFlag = true;
}

void publishHealthMessage() {
  if (client.connected()) {
    StaticJsonBuffer<400> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["chipId"] = String(ESP.getChipId());
    root["uptime"] = millis();
    root["freeHeap"] = String(ESP.getFreeHeap());
    int length = root.measureLength() + 1;
    char buffer[length];
    root.printTo(buffer, length);
    client.publish(deviceHealthTopic.c_str(), buffer);
    publishHealthFlag = false;
  }
}

void publishOnlineMessage() {
  if (client.connected()) {
    StaticJsonBuffer<400> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["chipId"] = String(ESP.getChipId());
    root["uptime"] = millis();
    root["coreVersion"] = ESP.getCoreVersion();
    root["cpuFreqMHz"] = ESP.getCpuFreqMHz();
    root["flashChipId"] = String(ESP.getFlashChipId());
    root["freeHeap"] = String(ESP.getFreeHeap());
    root["sdkVersion"] = ESP.getSdkVersion();
    root["coreVersion"] = ESP.getCoreVersion();
    int length = root.measureLength() + 1;
    char buffer[length];
    root.printTo(buffer, length);
    client.publish(ONLINE_NOTIFICATION_TOPIC, buffer);

  }
}

void connectToBroker() {
  Serial.printf("Connecting to MQTT broker [ %s:%d ]... \n", BROKER_HOST, BROKER_PORT);
  if (client.connect(clientId.c_str())) {
    client.subscribe(GENERAL_COMMAND_TOPIC);
    Serial.printf("Subscribeb to general command topic \"%s\"\n", GENERAL_COMMAND_TOPIC);
    client.subscribe(deviceCommandTopic.c_str());
    Serial.printf("Subscribeb to device command topic \"%s\"\n", deviceCommandTopic.c_str());
    publishOnlineMessage();
    healthCheck.attach(15, setPublishHealthFlag);
  } else {
    Serial.printf("failed, rc=%d\n", client.state());
    healthCheck.detach();
  }
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.printf("Topic: %s\n", topic);
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& msgJson = jsonBuffer.parseObject((char*)payload);
  const char* name = msgJson["name"];
  Serial.printf("%s\n", name);

}

void initPubSubClient() {
  client.setServer(BROKER_HOST, BROKER_PORT);
  client.setCallback(callback);
}

void initSSDPDeviceInfo() {
  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(SSDP_HTTP_PORT);
  SSDP.setName("The Lamp");
  SSDP.setSerialNumber(String(ESP.getChipId()));
  SSDP.setURL("index.html");
  SSDP.setModelName("DYI Smart Thing");
  SSDP.setModelURL("http://19november.ddns.net:8080/model/dyi_smart_thing/1.0");
  SSDP.setModelNumber("1.0");
  SSDP.setManufacturer("Espressif NodeMCU");
  SSDP.setManufacturerURL("http://19november.ddns.net:8080/about");
}

void initSSDP() {
  SSDP_HTTP.on("/devices", HTTP_GET, [](){
    SSDP_HTTP.send(200, "application/json", "Hello World!");
  });
  SSDP_HTTP.on("/description.xml", HTTP_GET, [](){
    SSDP.schema(SSDP_HTTP.client());
  });
  SSDP_HTTP.begin();

  Serial.printf("Starting SSDP...\n");
  SSDP.begin();
}

void initAfterConnected() {
  initPubSubClient();
  initSSDP();
}

void initWifiManager(uint timeout = 180) {
  blink.attach(0.1, blinkLed);
  Serial.println("Starting WiFiManager configuration...");
  wifiManager.setConfigPortalTimeout(timeout);
  wifiManager.setDebugOutput(true);
  wifiManager.setBreakAfterConfig(true);
  wifiManager.setAPStaticIPConfig(AP_IP,AP_GATEWAY, AP_SUBNET);

  int chipId = ESP.getChipId();

  if(!wifiManager.autoConnect(accessPointName.c_str())) {
    Serial.println("Failed to connect and hit timeout");
  } else {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Connected to " + WiFi.SSID());
      blink.detach();
      yield();
      digitalWrite(BLINK_LED, LOW);
      initAfterConnected();
    }
  }
}

void checkWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    initWifiManager();
  }
}

void checkMQTTConnection() {
  if (WiFi.status() == WL_CONNECTED && !client.connected()) {
      connectToBroker();
  }
}

void setup() {
  Serial.begin(115200);
  clientId = accessPointName = "ESP8266-" + String(ESP.getChipId());
  pinMode(BLINK_LED, OUTPUT);
  deviceCommandTopic = "esp_" + String(ESP.getChipId()) + "_command";
  deviceHealthTopic = "esp8266/" + String(ESP.getChipId()) + "/health";
  initSSDPDeviceInfo();
  initWifiManager();
}

void loop() {
  checkWiFiConnection();
  checkMQTTConnection();
  if (WiFi.status() == WL_CONNECTED) {
    SSDP_HTTP.handleClient();
    yield();
  }

  if (client.connected()) {
    if (publishHealthFlag) {
      publishHealthMessage();
    }
    client.loop();
    yield();
  }
}
