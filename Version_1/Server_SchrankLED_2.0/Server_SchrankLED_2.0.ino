#include "ESP8266WiFi.h"
#include <string.h>

const char* ssid     = "WLAN-149267";           //router name
const char* password = "70169424204811969647";  //router password

WiFiServer wifiServer(80);

//defines
#define RED_PIN 2      //GPIO0
#define BLUE_PIN 3     //GPIO2
#define GREEN_PIN 0    //GPIO3
//#define GREEN_PIN 1  //GPIO1
// GPIO zuteilung ist nicht sicher richtig    // Pin 1 ist der TX anschluss -> Probleme bei der ausgabe da er auch fuer die datenuebertragung zustaendig ist

#define MAX_RED 255   //maximal analog RED_PIN output255
#define MAX_GREEN 60  //maximal analog GREEN_PIN output 60 // rgb=255 doesn´t look white
#define MAX_BLUE 100  //maximal analog BLUE_PIN output 100

byte colors[3] = {0, 0, 0}; //content redColorvalue, greenColorValue, blueColorValue
byte redColorValue, greenColorValue, blueColorValue;

void HTMLtoRGB(String hexstring)  //convert HTML to RGB
{
  int number = (int) strtol( &hexstring[1], NULL, 16);
  
  //split hexstring into  decimal red, green, blue values
  redColorValue = number >> 16;
  greenColorValue = number >> 8 & 0xFF;
  blueColorValue = number & 0xFF;
}

void writePins(){ //write pins analog
        analogWrite(RED_PIN, map(colors[0], 0, 255, 0, MAX_RED));  //map(colors[0], 0, 255, 0, MAX_RED)
        Serial.println("Red pin was setting!");
        
        analogWrite(GREEN_PIN, map(colors[1], 0, 255, 0, MAX_GREEN));
        Serial.println("Green pin was setting!");
        
        analogWrite(BLUE_PIN, map(colors[2], 0, 255, 0, MAX_BLUE));
        Serial.println("Blue pin was setting!");
}


void setup() {

  //set output pins
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  
  //WiFi connection
  Serial.begin(115200);

  delay(1000);

  WiFi.begin(ssid, password);   //connecting with wlan

  while (WiFi.status() != WL_CONNECTED) {   //give feedback until wifi is conected
    delay(1000);
    Serial.println("Connecting..");
  }

  Serial.print("Connected to WiFi. IP:");
  Serial.println(WiFi.localIP());     // print esp8266 dedicated ip

  wifiServer.begin();

  writePins();  // set red- green- blue- pin low
}


void loop() {

  WiFiClient client = wifiServer.available();

  if (client) {

    while (client.connected()) {  //if everything connected to esp8266
      while (client.available() > 0) {  //as long as get input
        String req = client.readBytesUntil('\n'); // String zerlegen und in char packen
        Serial.print("raw req: ");Serial.println(req);
        //Serial.print("HTML Code: ");
        req.remove(0,2);                          //remove the first both chars of the string req (always ff)
        Serial.print("remove: ");Serial.println(req);
        long a = req.toInt();
        Serial.print("toInt : ");Serial.println(a);
        /*HTMLtoRGB(req);                           //maby give them as a pointer
        Serial.print("R: ");Serial.println(redColorValue);
        Serial.print("G: ");Serial.println(greenColorValue);
        Serial.print("B: ");Serial.println(blueColorValue);
        //write color value into colors array
        colors[0] = redColorValue;
        colors[1] = greenColorValue;
        colors[2] = blueColorValue;
        writePins();*/
      }
      delay(10);
    }

    client.stop();
    Serial.println("Client disconnected");  //stop recieve

  }
}
