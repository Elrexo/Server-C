#include "ESP8266WiFi.h"
#include <string.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

const char* ssid     = "WLAN-149267"; //Router name
const char* password = "70169424204811969647";  //router password

WiFiServer wifiServer(80);

#define IR_SEND_PIN D2
IRsend irsend(IR_SEND_PIN);


void setup() {
  
  Serial.begin(115200);

  delay(1000);

  WiFi.begin(ssid, password);

  irsend.begin();

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting..");
  }

  Serial.print("Connected to WiFi. IP:");
  Serial.println(WiFi.localIP());

  wifiServer.begin();
}

void loop() {

  WiFiClient client = wifiServer.available();

  if (client) {

    while (client.connected()) {
      while (client.available() > 0) {
        String req = client.readStringUntil('X'); //try \r
        req.remove(0,2);
        long frequenz = req.toInt();
        Serial.print("IR-Signal send: ");
        Serial.println(frequenz);
        irsend.sendNEC(frequenz,32); //table on: 16753245; table off: 16769565
      }
      delay(10);
    }

    client.stop();
    Serial.println("Client disconnected");

  }
}
