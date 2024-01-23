#define CA_CERT "-----BEGIN CERTIFICATE-----\n" \
"MIIEADCCAuigAwIBAgIID+rOSdTGfGcwDQYJKoZIhvcNAQELBQAwgYsxCzAJBgNV\n"  \
"BAYTAlVTMRkwFwYDVQQKExBDbG91ZEZsYXJlLCBJbmMuMTQwMgYDVQQLEytDbG91\n"  \
"ZEZsYXJlIE9yaWdpbiBTU0wgQ2VydGlmaWNhdGUgQXV0aG9yaXR5MRYwFAYDVQQH\n"  \
"Ew1TYW4gRnJhbmNpc2NvMRMwEQYDVQQIEwpDYWxpZm9ybmlhMB4XDTE5MDgyMzIx\n"  \
"MDgwMFoXDTI5MDgxNTE3MDAwMFowgYsxCzAJBgNVBAYTAlVTMRkwFwYDVQQKExBD\n"  \
"bG91ZEZsYXJlLCBJbmMuMTQwMgYDVQQLEytDbG91ZEZsYXJlIE9yaWdpbiBTU0wg\n"  \
"Q2VydGlmaWNhdGUgQXV0aG9yaXR5MRYwFAYDVQQHEw1TYW4gRnJhbmNpc2NvMRMw\n"  \
"EQYDVQQIEwpDYWxpZm9ybmlhMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKC\n"  \
"AQEAwEiVZ/UoQpHmFsHvk5isBxRehukP8DG9JhFev3WZtG76WoTthvLJFRKFCHXm\n"  \
"V6Z5/66Z4S09mgsUuFwvJzMnE6Ej6yIsYNCb9r9QORa8BdhrkNn6kdTly3mdnykb\n"  \
"OomnwbUfLlExVgNdlP0XoRoeMwbQ4598foiHblO2B/LKuNfJzAMfS7oZe34b+vLB\n"  \
"yrP/1bgCSLdc1AxQc1AC0EsQQhgcyTJNgnG4va1c7ogPlwKyhbDyZ4e59N5lbYPJ\n"  \
"SmXI/cAe3jXj1FBLJZkwnoDKe0v13xeF+nF32smSH0qB7aJX2tBMW4TWtFPmzs5I\n"  \
"lwrFSySWAdwYdgxw180yKU0dvwIDAQABo2YwZDAOBgNVHQ8BAf8EBAMCAQYwEgYD\n"  \
"VR0TAQH/BAgwBgEB/wIBAjAdBgNVHQ4EFgQUJOhTV118NECHqeuU27rhFnj8KaQw\n"  \
"HwYDVR0jBBgwFoAUJOhTV118NECHqeuU27rhFnj8KaQwDQYJKoZIhvcNAQELBQAD\n"  \
"ggEBAHwOf9Ur1l0Ar5vFE6PNrZWrDfQIMyEfdgSKofCdTckbqXNTiXdgbHs+TWoQ\n"  \
"wAB0pfJDAHJDXOTCWRyTeXOseeOi5Btj5CnEuw3P0oXqdqevM1/+uWp0CM35zgZ8\n"  \
"VD4aITxity0djzE6Qnx3Syzz+ZkoBgTnNum7d9A66/V636x4vTeqbZFBr9erJzgz\n"  \
"hhurjcoacvRNhnjtDRM0dPeiCJ50CP3wEYuvUzDHUaowOsnLCjQIkWbR7Ni6KEIk\n"  \
"MOz2U0OBSif3FTkhCgZWQKOOLo1P42jHC3ssUZAtVNXrCk3fw9/E15k8NPkBazZ6\n"  \
"0iykLhH1trywrKRMVw67F44IE8Y=\n"\
"-----END CERTIFICATE-----\n"
//ca of cloudflare

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
//IPAddress server_ip(89,76,210,123);  // numeric IP
char server[] = "host";
int server_port = 443; // port 80 is the default for HTTP

WiFiSSLClient client;

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
  while (!client.connect(server, server_port)) {
    Serial.println("Connection failed, trying again...");
    delay(100);
  }

  Serial.println("connected to server");
  // Make a HTTP request:
  client.print("POST /sensor-reading?reading=");
  client.print(payload_str.substring(14));
  client.println(" HTTP/1.1");
  client.println("Host: host");//IP once again
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