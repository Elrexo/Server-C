#include "ESP8266WiFi.h"
#include <string.h>

const char* ssid     = "WLAN-149267";           //Router name
const char* password = "70169424204811969647";  //router password

WiFiServer wifiServer(80);

#define RELAY 5         // RELAY
#define SWITCH 6        // light_SWITCH
int relayState = 0;     // hold RELAY on power

void setup() {
  pinMode(RELAY,OUTPUT);
  pinMode(SWITCH,INPUT_PULLUP);
  digitalWrite(RELAY,LOW);

  Serial.begin(115200);

  delay(1000);

  WiFi.begin(ssid, password);

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
        String req = client.readStringUntil('X');
        req.remove(0,2);   
        long a = req.toInt();
        Serial.print("Receive integer: ");
        Serial.println("toInt: " + a);
        
        /*if(a<0 && relayState==0){
          digitalWrite(RELAY,HIGH);
          relayState = 1;
          Serial.println("RELAY: HIGH\nTCP_Signal\n");
        }
        else if(a<0 && relayState==1){
          digitalWrite(RELAY,LOW);
          relayState = 0;
          Serial.println("RELAY: LOW\nTCP_Signal\n");
        }
        else
          Serial.println("Error");*/
      }
      delay(10);
    }
    client.stop();
    Serial.println("Client disconnected");
  }

  /*else if(digitalRead(SWITCH)==HIGH && relayState==0){  //activate RELAY by button
    digitalWrite(RELAY,HIGH);       //light on
    relayState = 1;
    Serial.println("RELAY: HIGH\nButton pressed!\n");
    delay(700);
  }
  else if(digitalRead(SWITCH)==HIGH && relayState==1){  //disactivate RELAY by button
    digitalWrite(RELAY,LOW);        //light off
    relayState = 0;
    Serial.println("RELAY: LOW\nButton pressed!\n");
    delay(700);
  }*/
}
