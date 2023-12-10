#include "WiFiS3.h"

#include "arduino_secrets.h" 

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID; 
char pass[] = SECRET_PASS;

int status = WL_IDLE_STATUS;
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
IPAddress server(192,168,214,87);  // numeric IP
//char server[] = "www.google.com";    // name address (using DNS)

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiClient client;

/* -------------------------------------------------------------------------- */
void setup() {
/* -------------------------------------------------------------------------- */  
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  while (!Serial) { // this is only for debugging, should be deleted in the final version
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
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
    delay(10000);
  }
  
  printWifiStatus();
}

/* -------------------------------------------------------------------------- */
void loop() {
/* -------------------------------------------------------------------------- */  
  post_request();
  read_response();
  delay(5000);
}

/* -------------------------------------------------------------------------- */
void printWifiStatus() {
/* -------------------------------------------------------------------------- */  
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

/* -------------------------------------------------------------------------- */
void read_response() {
/* -------------------------------------------------------------------------- */  
  uint32_t received_data_num = 0;
  if(client.available()){
    while (client.available()) {
      /* actual data reception */
      char c = client.read();

      Serial.print(c); //we don't really care about the response as long as it is 2XX
      /* wrap data to 80 columns - pretty print*/
      received_data_num++;
      if(received_data_num % 80 == 0) { 
        Serial.println();
      }
    }

    Serial.println();
    Serial.println("Disconnecting from server.");
    client.stop();  
  }
}


/* -------------------------------------------------------------------------- */
void post_request() {
/* -------------------------------------------------------------------------- */  
  while (!client.connect(server, 80)) {
    Serial.println("Connection failed, trying again...");
    delay(100);
  }
  
  Serial.println("connected to server");
  // Make a HTTP request:
  client.println("GET / HTTP/1.1");
  client.println("Host: 192.168.214.87");
  client.println("Connection: close");
  client.println(); 
  delay(100);//wait a little at least
}
