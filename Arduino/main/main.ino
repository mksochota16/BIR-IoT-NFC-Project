#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>



//wifi
ESP8266WiFiMulti WiFiMulti;
String mainURL ="http://192.168.52.86/";

void setup() {

  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println("SETUP START!");

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("WIFI_NAME", "WIFI_PASSWORD");
}


void loop() {
  //wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;

    HTTPClient http;

    Serial.print("[HTTP] begin...");
    if (http.begin(client, mainURL)) {  // HTTP


      Serial.print("[HTTP] GET...");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect");
    }
  }
  delay(10000);
}