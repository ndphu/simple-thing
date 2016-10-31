#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <WiFiUDP.h>
#include <Ticker.h>
#include <ESP8266SSDP.h>
#include <ArduinoJson.h>
#include "Device.h"
#include "DeviceManager.h"
#include <Wire.h>
#include "SSD1306.h"
#include "OLEDDisplayUi.h"
#include "images.h"
#include "fonts.h"
#include "utils.h"

ADC_MODE(ADC_VCC);
// Define AP Configuration
#define AP_IP IPAddress(19,11,20,12)
#define AP_GATEWAY IPAddress(19,11,20,12)
#define AP_SUBNET IPAddress(255, 255, 255, 0)
String accessPointName;

//#define BROKER_HOST "iot.eclipse.org"
#define BROKER_HOST "broker.hivemq.com"
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

// display
SSD1306  display(0x3c, D6, D5);
OLEDDisplayUi ui     ( &display );

WiFiManager wifiManager;
WiFiClient wifiClient;
PubSubClient client(wifiClient);

// SSDP
#define SSDP_HTTP_PORT 8080
ESP8266WebServer SSDP_HTTP(SSDP_HTTP_PORT);

void setPublishHealthFlag() {
  publishHealthFlag = true;
}

void msOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  int sec = millis() / 1000;
  int min = sec / 60;
  int hour = min / 60;
  char* timestr = new char[10];
  sprintf(timestr, "%02d:%02d:%02d", hour, min % 60, sec % 60);
  display->drawString(0, 54, timestr);
  delete timestr;
}

OverlayCallback overlays[] = { msOverlay,  };
int overlaysCount = 1;

void drawInfoFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  //display->drawXbm(x, y + 12, APP_LOGO_WIDTH, APP_LOGO_HEIGHT, App_Logo_Image_bits);
  display->setFont(Droid_Sans_Mono_10);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0, 0, "Chip ID: " + String(ESP.getChipId()));
  display->drawString(0, 10, "Free heap: " + String(ESP.getFreeHeap()/(float)1000) + " KB");

  display->drawString(0, 20, "AP: " + WiFi.SSID());
  display->drawString(0, 30, "IP: " + WiFi.localIP().toString());
  display->drawString(0, 40, String("Broker: ") + (client.connected() ? "Connected" : "Disconnected"));
}

void drawClockFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {

}

FrameCallback initFrames[] = { drawInfoFrame, drawClockFrame };
int initFrameCount = 2;

void initDisplayUI() {
  //display.flipScreenVertically();
  ui.setTargetFPS(60);
  // ui.setActiveSymbol(activeSymbol);
  // ui.setInactiveSymbol(inactiveSymbol);
  // ui.setIndicatorPosition(BOTTOM);
  //

  //ui.setIndicatorDirection(LEFT_RIGHT);
  ui.disableIndicator();
  ui.setFrameAnimation(SLIDE_LEFT);
  ui.setFrames(initFrames, initFrameCount);
  ui.setOverlays(overlays, overlaysCount);
  //ui.disableAutoTransition();
  ui.setTimePerFrame(2000);
  ui.setTimePerTransition(500);
  ui.init();

  ui.update();

}

Device d1("switch1", TYPE_SWITCH);
Device d2("sensor1", TYPE_SENSOR);
Device d3("switch2", TYPE_SWITCH);

void publishJsonObject(const char * topic, JsonObject &obj) {
  int length = obj.measureLength() + 1;
  char buffer[length];
  obj.printTo(buffer, length);
  client.publish(topic, buffer);
}

void publishHealthMessage() {
  if (client.connected()) {
    StaticJsonBuffer<400> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["chipId"] = String(ESP.getChipId());
    root["uptime"] = millis();
    root["freeHeap"] = String(ESP.getFreeHeap());
    publishJsonObject(deviceHealthTopic.c_str(), root);
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
    publishJsonObject(ONLINE_NOTIFICATION_TOPIC, root);
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
  SSDP.setName(String(ESP.getChipId()));
  SSDP.setSerialNumber(String(ESP.getChipId()));
  SSDP.setURL("index.html");
  SSDP.setModelName("DYI Smart Thing");
  SSDP.setModelURL("http://19november.ddns.net:8080/model/dyi_smart_thing/1.0");
  SSDP.setModelNumber("1.0");
  SSDP.setManufacturer("Espressif NodeMCU");
  SSDP.setManufacturerURL("http://19november.ddns.net:8080/about");
}

void initDeviceManager() {
  SSDP_HTTP.on("/devices", HTTP_GET, []() {
    deviceManager.devices(SSDP_HTTP.client());
  });

  SSDP_HTTP.on("/jsonrpc", HTTP_POST, [](){
    deviceManager.jsonrpc(SSDP_HTTP.arg("plain").c_str(), SSDP_HTTP.client());
  });
}

void initSSDP() {
  SSDP_HTTP.on("/description.xml", HTTP_GET, [](){
    SSDP.schema(SSDP_HTTP.client());
  });

  SSDP_HTTP.begin();

  Serial.printf("Starting SSDP...\n");
  SSDP.begin();
}

void initAfterConnected() {
  initPubSubClient();
  initDeviceManager();
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
      digitalWrite(BLINK_LED, HIGH);
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
  initDisplayUI();
  clientId = accessPointName = "ESP8266-" + String(ESP.getChipId());
  pinMode(BLINK_LED, OUTPUT);
  deviceCommandTopic = "esp_" + String(ESP.getChipId()) + "_command";
  deviceHealthTopic = "esp8266/" + String(ESP.getChipId()) + "/health";
  initSSDPDeviceInfo();
  deviceManager.addDevice(&d1);
  deviceManager.addDevice(&d2);
  deviceManager.addDevice(&d3);
  initWifiManager();
}

void loop() {
  int remainingTimeBudget = ui.update();
  if (remainingTimeBudget > 0) {
    checkWiFiConnection();
    checkMQTTConnection();
    if (WiFi.status() == WL_CONNECTED) {
      SSDP_HTTP.handleClient();
    }

    if (WiFi.status() == WL_CONNECTED && !client.connected()) {
      String sid = getSID();
      login(sid, "DN3L");
    }

    if (client.connected()) {
      if (publishHealthFlag) {
        publishHealthMessage();
      }
      client.loop();
    }
    delay(remainingTimeBudget);
  }

}
