#ifndef ESP8266_UTILS_H
#define ESP8266_UTILS_H

#include <Arduino.h>
#include <ESP8266WiFi.h>

#define UserAgent "Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/54.0.2840.71 Safari/537.36"

String getSID() {
  WiFiClient client;
  String host = "tinhte.vn";
  if (!client.connect(host.c_str(), 80)) {
    Serial.println("connection failed");
    return "";
  }
  String url = "/";
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: Chrome\r\n" +
               "Connection: close\r\n\r\n");

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    } else if (line.indexOf("name=\"sid\"") >= 0) {
      int beginIdx = line.indexOf("value") + 7;
      int endIdx = beginIdx+ 12;
      return line.substring(beginIdx, endIdx);
    }
  }
  return "";
}

void login(String sid, String passcode) {
  WiFiClientSecure client;
  String host = "login.globalsuite.net";
  if (!client.connect(host.c_str(), 443)) {
    Serial.println("Connection failed");
    return;
  }
  String url = "/rwd/PasscodeVerificationServlet";
  String formData = String("passcode=") + passcode + "&sid=" + sid;
  Serial.println("Post data: " + formData);
  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Content-Type: application/x-www-form-urlencoded\r\n" +
               "User-Agent: " + UserAgent + "\r\n" +
               "Content-Length: " + formData.length() + "\r\n" +
               "Connection: close\r\n\r\n");
  client.print(formData.c_str());
  while (client.connected()) {
     String line = client.readStringUntil('\n');
     Serial.println(line);
   }
}
#endif
