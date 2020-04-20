#include "ESP8266WiFi.h"
#include <string.h>

const char* ssid     = ""; //Router name
const char* password = "";  //router password

WiFiServer wifiServer(80);

#define relay_1 5         // relay
#define switch_1 6        // light_switch
int temp = 0;             // hold relay on power

void setup() {
  pinMode(relay_1,OUTPUT);
  pinMode(switch_1,INPUT_PULLUP);

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
        Serial.println(a);
        
        if(a<0 && temp==0){
          digitalWrite(relay_1,HIGH);
          temp = 1;
          Serial.println("Relay: HIGH\nTCP_Signal\n");
        }
        else if(a<0 && temp==1){
          digitalWrite(relay_1,LOW);
          temp = 0;
          Serial.println("Relay: LOW\nTCP_Signal\n");
        }
        else
          Serial.println("Error");
      }
      delay(10);
    }
    client.stop();
    Serial.println("Client disconnected");
  }

  else if(digitalRead(switch_1)==HIGH && temp==0){  //activate Relay by button
    digitalWrite(relay_1,HIGH);       //light on
    temp = 1;
    Serial.println("Relay: HIGH\nButton pressed!\n");
    delay(700);
  }
  else if(digitalRead(switch_1)==HIGH && temp==1){  //disactivate Relay by button
    digitalWrite(relay_1,LOW);        //light off
    temp = 0;
    Serial.println("Relay: LOW\nButton pressed!\n");
    delay(700);
  }
}
