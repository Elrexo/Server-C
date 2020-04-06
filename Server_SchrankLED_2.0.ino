#include "ESP8266WiFi.h"
#include <string.h>

const char* ssid     = ""; //Router name
const char* password = "";  //router password

WiFiServer wifiServer(80);

//LED defines
#define redPin 2 //GPIO0
#define greenPin 1 //GPIO1
#define bluePin 3  //GPIO2
//GPIO3 -> D9

#define max_red 255
#define max_green 60  // rgb=255 doesn´t look white
#define max_blue 100

byte colors[3] = {0, 0, 0};
byte r,g,b;

void HTMLtoRGB(String hexstring)  //HTML to RGB
{
  int number = (int) strtol( &hexstring[1], NULL, 16);

  // Split them up into r, g, b values
  r = number >> 16;
  g = number >> 8 & 0xFF;
  b = number & 0xFF;
}


void setup() {

  //LEDs
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  
  //WiFi connection
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
        String req = client.readStringUntil('X'); // String zerlegen und in char packen
        Serial.print("HTML Code: ");
        Serial.println(req);
        req.remove(0,2);
        HTMLtoRGB(req); //maby give them as a pointer
        Serial.print("R: ");Serial.println(r);
        Serial.print("G: ");Serial.println(g);
        Serial.print("B: ");Serial.println(b);
        //LED Write
        colors[0] = r;
        colors[1] = g;
        colors[2] = b;
        analogWrite(redPin, map(colors[0], 0, 255, 0, max_red));
        analogWrite(greenPin, map(colors[1], 0, 255, 0, max_green));
        analogWrite(bluePin, map(colors[2], 0, 255, 0, max_blue));
 
      }
      delay(10);
    }

    client.stop();
    Serial.println("Client disconnected");

  }
}
