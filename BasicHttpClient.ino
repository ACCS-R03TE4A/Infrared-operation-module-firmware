/**
   BasicHTTPClient.ino

    Created on: 24.05.2015

*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRutils.h>

//WiFi関係
ESP8266WiFiMulti WiFiMulti;
//Remote関係
const uint16_t kIrLed = D1; //赤外線LEDのピンを取得
IRsend irsend(kIrLed);      // 送信オブジェクト
bool success = true;

void setup() {
  //WiFi関係
  Serial.begin(115200);
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("106F3F754BD1", "nermxh0gvk23i");

  //Remote関係
  irsend.begin(); // 赤外線LEDの設定
}

void loop() {
  //WiFi関係
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    WiFiClient client;
    HTTPClient http;
    Serial.print("[HTTP] begin...\n");
    if (http.begin(client, "http://192.168.59.128:5000/operation")) {  // HTTP
      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();

          //json関係
          DynamicJsonDocument doc(1024);
          DeserializationError error = deserializeJson(doc, payload);
          if (error) {
            Serial.println(error.f_str());
          } else {
            const char isExist = doc["status"];
            if(isExist != 0) {
              //Serial.println(pyload);
              //Serial.println(doc["status"]);
              //Serial.println(doc["protocol"]);
              //Serial.println(doc["data"]);
              decode_type_t protocol = (decode_type_t)doc["protocol"];

              String stsize = doc["size"];
              int intsize = stsize.toInt();
              uint16_t size = intsize;

              String stvalue = doc["data"];
              stvalue = stvalue.substring(2);
              Serial.println(stvalue);
              char stvalueCharArray[16];
              stvalue.toCharArray(stvalueCharArray,16);
              unsigned long intvalue = strtoul(stvalueCharArray,NULL,16);
              const uint32_t value = intvalue;
              
              //String stvalue = "0x1234";//doc["data"];
              //int intvalue = stvalue.toInt();
              //const uint32_t value = intvalue;
              //const char* data = doc["status"];
              //Serial.println(data);
              Serial.println("protocol");
              Serial.println(protocol);
              Serial.println("value");
              Serial.println(value);
              Serial.println("size");
              Serial.println(size);
              
              //Remote関係
              success = irsend.send(protocol, value, size); //送信
              if (success) {
                Serial.println("send success");
              }
            } else {
              Serial.println("no data");
            }
          }
        } else {
          Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
      } else {
        Serial.printf("[HTTP} Unable to connect\n");
      }
    }
    delay(10000);
  }
}
