#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include "WiFiUdp.h"

// Define AP Configuration
#define AP_IP IPAddress(19,11,20,12)
#define AP_GATEWAY IPAddress(19,11,20,12)
#define AP_SUBNET IPAddress(255, 255, 255, 0)
#define WIFI_RECONNECT_INTERVAL 30 * 1000
#define BROKER_HOST "iot.eclipse.org"
#define BROKER_PORT 1883

// MQTT
#define COMMAND_TOPIC "esp_general_command"
#define HEALTH_CHECK_TOPIC "esp_health_check"

int lastWiFiReconnect;
WiFiManager wifiManager;
WiFiClient wifiClient;
PubSubClient client(wifiClient);

void connectToBroker() {
  Serial.println("Attempting MQTT connection...");
  char * clientId = (char*)malloc(32 * sizeof(char));
  sprintf(clientId, "ESP8266Client-%d", ESP.getChipId());
  if (client.connect(clientId)) {
    Serial.println("connected");
    client.subscribe(COMMAND_TOPIC);
    Serial.print("Subscribeb to ");
    Serial.println(COMMAND_TOPIC);
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

void initWifiManager(uint timeout = 180) {
  Serial.println("Starting WiFiManager configuration...");
  wifiManager.setConfigPortalTimeout(timeout);
  wifiManager.setDebugOutput(true);
  wifiManager.setBreakAfterConfig(true);
  wifiManager.setAPStaticIPConfig(AP_IP,AP_GATEWAY, AP_SUBNET);

  int chipId = ESP.getChipId();
  char * apName = (char*)malloc(50 * sizeof(char));
  sprintf(apName, "ESP8266-%d", chipId);
  if(!wifiManager.autoConnect(apName)) {
    Serial.println("Failed to connect and hit timeout");
  } else {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Connected to " + WiFi.SSID());
      initPubSubClient();
    }
  }
  free(apName);
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

void sendMulticast() {
  // send a reply, to the IP address and port that sent us the packet we received
  Serial.println("Sending multicast...");
  WiFiUDP Udp = WiFiUDP();
  Udp.beginMulticast(WiFi.localIP(), IPAddress(239,255,255,250), 1900);
  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  Udp.write("ESP-");
  Udp.print(system_get_chip_id(),HEX);
  Udp.print(":");
  Udp.print(WiFi.localIP());
  Udp.endPacket();
}

void setup() {
  Serial.begin(115200);
  initWifiManager();
}


void loop() {
  checkWiFiConnection();
  //checkMQTTConnection();
  if (client.connected()) {
    client.loop();
  }

  sendMulticast();
}
