/* ===========================================================================
 
Project:      SmarthomeMyRoom

==============================================================================
 
Filename:     Server_AlexaSinricCupboardLight1.1.ino
 
Authors:      Tim Keicher
 
Version:      1.0 vom 01.12.2020
 
Hardware:     esp8266-01, 3.3V/5V source, TIP31C, LED-String 
 
Software:     Arduino 1.8.10
 
Function:     CupboardLight handles the LED string on the Cupboard. It receive comands from
              Alexa Sinric and the android app and put it into the color to write 
              them on the LED string.
 
Buttonfunction:   - 
 
Moduls:       -
 
=============================================================================*/

//***OTA includes***
#include <ArduinoOTA.h>     //contents the "over the air" (OTA) functions, upload via wifi possible
#include <WiFiUdp.h>
#include <TelnetStream2.h>  //open TelnetStream communication with esp and Putty

#ifdef ESP32                //if use esp32
  #include <WiFi.h>
  #include <ESPmDNS.h>
#else                       //if use esp8266
  #include <ESP8266WiFi.h>
  #include <ESP8266mDNS.h>
#endif

//***Alexa Sinric includes***
#include <Arduino.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h> //https://github.com/kakopappa/sinric/wiki/How-to-add-dependency-libraries
#include <ArduinoJson.h>      //https://github.com/kakopappa/sinric/wiki/How-to-add-dependency-libraries
#include <StreamString.h>

//***esp wifi defines***
#define STASSID ""                             //router name
#define STAPSK  ""                    //router password
#define USER_SERIAL TelnetStream2

//***Alexa Sinric defines***
#define MyApiKey ""  //TODO: Change to your sinric API Key. Your API Key is displayed on sinric.com dashboard
#define HEARTBEAT_INTERVAL 300000                         //send every 5 Minutes to sinric web

//***CupboardLight defines***
#define PORT 80        //port number
//pins
#define RED_PIN 2      //GPIO2
#define BLUE_PIN 3     //TXD
#define GREEN_PIN 0    //GPIO0
#define RELAY 1        //RX (check)

//maximal color output
#define MAX_RED 255   //maximal analog RED_PIN output 255
#define MAX_GREEN 100  //maximal analog GREEN_PIN output 100 // rgb=255 doesn´t look white
#define MAX_BLUE 150  //maximal analog BLUE_PIN output 150

//***global variables***
unsigned char colors[12][3] = {{0, 0, 0},     //content redColorvalue, greenColorValue, blueColorValue. Variable part for android received. 0
                              {255,0,0},      // red 1
                              {255,165,0},    // orange 2
                              {255,255,0},    // yellow 3
                              {0,255,0},      // green 4
                              {173,216,230},  // light blue 5
                              {5,124,158},    // blue 6 //usually 0,0,255 but this one is more beatiful
                              {238,130,238},  // violate 7
                              {160,32,240},   // purple 8
                              {255,192,203},  // pink 9
                              {255,255,255},  //white (ON) 10
                              {0,0,0}};       //black (OFF) 11
                              // extension possible with more colors

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
WiFiClient client;
WiFiServer wifiServer(PORT);

//-------------- setup --------------
void setup() 
{
  Serial.begin(115200);
  Serial.println("Booting");

  setupOTA();
  setupSinric();
  setupCupboardLight();
  
  // put your setup code here, to run once:
}

//-------------- loop --------------
void loop() 
{
  ArduinoOTA.handle();  //OTA loop
  loopSinric();
  loopCupboardLight();
  
  // use "TelnetStream2.println("...");" to communicate with putty
}
