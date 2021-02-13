/* ===========================================================================
 
Project:      SmarthomeMyRoom

==============================================================================
 
Filename:     templateEspMyRoom.ino
 
Authors:      Tim Keicher
 
Version:      1.1 vom 25.01.2021
 
Hardware:     esp8266-01 with 3.3V source, ir-transmitter, button, 10 kOhm resistance
 
Software:     Arduino 1.8.10
 
Function:     This programm check the push button and send a signal on dimmingESP if pushed. It
              controls also the party light with a ir-transmiiter. It check the temperature and 
              humidity and give a callback on a wifi request.
 
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

//***handleFunctions includes***
#include <SimpleDHT.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <WebSocketsServer.h>

//***esp wifi defines***
#define STASSID ""                             //router name
#define STAPSK  ""                    //router password
#define USER_SERIAL TelnetStream2

//***Alexa Sinric defines***
#define MyApiKey ""  //TODO: Change to your sinric API Key. Your API Key is displayed on sinric.com dashboard
#define HEARTBEAT_INTERVAL 300000                         //send every 5 Minutes to sinric web

//***handleFunctions defines***
#define PORT 80 //port number
String ROOM_LIGHT_IP = "192.168.2.174";

//pins
#define BUTTON 0      //GPIO0
#define SENSOR 1      //RX  (3 is TX) thats correct I checkt it sure
#define IR_SEND_PIN 2 //GPIO2 (check)(kann not restart if in use or with ir)

//***global variables***
long onOffFrequency = 16753245;          //noch durch richtigen wert ersetzten REEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
boolean partyState = false;

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
WiFiClient client;
WiFiServer wifiServer(PORT);    //Wifi port
IRsend irsend(IR_SEND_PIN);     //in library IRsend.h, send about IR_SEND_PIN infrared

//-------------- setup --------------
void setup() 
{
  Serial.begin(115200);
  Serial.println("Booting");

  setupOTA();
  setupSinric();
  functionsSetup();
}

//-------------- loop --------------
void loop() 
{
  ArduinoOTA.handle();  //OTA loop
  loopSinric();
  functionsLoop();
  
  // use "TelnetStream2.println("...");" to communicate with putty
}
