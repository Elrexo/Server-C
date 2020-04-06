#include "ESP8266WiFi.h"
#include <string.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

const char* ssid     = ""; //Router name
const char* password = "";  //router password

WiFiServer wifiServer(80);

#define IR_SEND_PIN 2
IRsend irsend(IR_SEND_PIN);


int StringToInt(String req){
  
  String a_string = req.substring(req.length()-4, req.length());
  String b_string = req.substring(0, req.length()-4);
  long a_zahl = a_string.toInt();Serial.println(a_zahl);
  long b_zahl = b_string.toInt();Serial.println(b_zahl);
  long ergebnis = b_zahl*10000 + a_zahl;

  return ergebnis;
}


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
        long frequenz = StringToInt(req);
        Serial.print("IR-Signal send: ");
        Serial.println(frequenz);
        if(frequenz > -1500000000){
          irsend.sendSAMSUNG(frequenz,32); //table on: 16753245; table off: 16769565
          Serial.println("Send Samsung");
        }
        else{
          irsend.sendNEC(frequenz,32);
          Serial.println("Send NEC");
        }
      }
      delay(10);
    }

    client.stop();
    Serial.println("Client disconnected");

  }
}
