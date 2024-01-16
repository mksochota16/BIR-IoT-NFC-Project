// for I2C Communication
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);

String tagId = "None";
byte payload[20];
String payload_str;
byte nuidPICC[4];

void setup(void) {
  Serial.begin(115200);
  Serial.println("Start setup");

 	nfc.begin();
  Serial.println("System initialized");
}

void loop() {
 	readNFC();
}

void readNFC() {
 	if (nfc.tagPresent()) // blocking
 	{
 	  NfcTag tag = nfc.read();
      tag.getNdefMessage().getRecord(0).getPayload(payload);
      payload_str = String((char*)payload);
      Serial.println(payload_str);

      delay(1000);
 	}
 	
}

