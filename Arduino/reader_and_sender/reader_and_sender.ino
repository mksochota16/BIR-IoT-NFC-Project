#include "WiFiS3.h"
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

#include "arduino_secrets.h"

//please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);
String tagId = "None";
byte payload[20];
String payload_str;
byte nuidPICC[4];

int status = WL_IDLE_STATUS;
IPAddress server_ip(192,168,222,86);  // numeric IP
int server_port = 80; // port 80 is the default for HTTP

WiFiClient client;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("Start setup");
  
  digitalWrite(LED_BUILTIN, HIGH);

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(5000);
  }

  print_wifi_status();
  Serial.println("Wifi initialized");
  nfc.begin();
  Serial.println("NFC initialized");
  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
  read_and_send_NFC();
}

void print_wifi_status() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}


void read_response() {
  if(client.available()){
    while (client.available()) {
      client.read();
      //we don't really care about the response ,everything is handled on the server side, we just need to read it
    }
    Serial.println("Disconnecting from server.");
    client.stop();
  }
}


void post_request() {
  while (!client.connect(server_ip, server_port)) {
    Serial.println("Connection failed, trying again...");
    delay(100);
  }

  Serial.println("connected to server");
  // Make a HTTP request:
  client.print("POST /sensor-reading?reading=");
  client.print(payload_str.substring(14));
  client.println(" HTTP/1.1");
  client.println("Host: 192.168.222.86");//IP once again
  client.println("accept: application/json");
  client.println("Connection: close");
  client.println();
  delay(100);//wait a little at least
}


void read_and_send_NFC() {
 	if (nfc.tagPresent()) // blocking
 	{
 	  NfcTag tag = nfc.read();
      tag.getNdefMessage().getRecord(0).getPayload(payload);
      payload_str = String((char*)payload);
      Serial.println(payload_str);
      post_request();
      //read_response();
      delay(1000);
 	}

}