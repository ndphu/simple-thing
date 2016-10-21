#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <WiFiUDP.h>
#include <Ticker.h>
#include <ESP8266SSDP.h>

// Define AP Configuration
#define AP_IP IPAddress(19,11,20,12)
#define AP_GATEWAY IPAddress(19,11,20,12)
#define AP_SUBNET IPAddress(255, 255, 255, 0)
String apName;


#define BROKER_HOST "iot.eclipse.org"
#define BROKER_PORT 1883

// MQTT
#define GENERAL_COMMAND_TOPIC "esp_general_command"
#define HEALTH_CHECK_TOPIC "esp_health_check"
String deviceCommandTopic;
// Ticker
#define BLINK_LED 2
Ticker blink;

void blinkLed() {
  int state = digitalRead(BLINK_LED);
  digitalWrite(BLINK_LED, !state);
}

WiFiManager wifiManager;
WiFiClient wifiClient;
PubSubClient client(wifiClient);

// SSDP
#define SSDP_HTTP_PORT 8080
ESP8266WebServer SSDP_HTTP(SSDP_HTTP_PORT);

void connectToBroker() {
  Serial.println("Attempting MQTT connection...");
  char * clientId = (char*)malloc(32 * sizeof(char));
  sprintf(clientId, "ESP8266Client-%d", ESP.getChipId());
  if (client.connect(clientId)) {
    Serial.println("connected");
    client.subscribe(GENERAL_COMMAND_TOPIC);
    Serial.print("Subscribeb to general command topic ");
    Serial.printf("\"%s\"\n", GENERAL_COMMAND_TOPIC);

    client.subscribe(deviceCommandTopic.c_str());
    Serial.print("Subscribeb to device command topic ");
    Serial.printf("\"%s\"\n", deviceCommandTopic.c_str());
  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println("");
  }
  free(clientId);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("[");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
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
  SSDP_HTTP.on("/description.json", HTTP_GET, [](){
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

  if(!wifiManager.autoConnect(apName.c_str())) {
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
  apName = "ESP8266-" + String(ESP.getChipId());
  pinMode(BLINK_LED, OUTPUT);
  deviceCommandTopic = "esp_" + String(ESP.getChipId()) + "_command";
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
  if (WiFi.status() == WL_CONNECTED) {

  }
  if (client.connected()) {
    client.loop();
    yield();
  }
}
