/* ===========================================================================
 
Project:      SmarthomeMyRoom

==============================================================================
 
Filename:     Server_AlexaSinricRoomLight1.1.ino
 
Authors:      Tim Keicher
 
Version:      1.0 vom 03.12.2020
 
Hardware:     esp8266-01, 3.3V/5V source, 230V Relay, 10k Ohm Widerstand, 230V 
              AC to 12V DC transmitter 
 
Software:     Arduino 1.8.5
 
Function:     VorlageOTA_2 is the template of all esp moduls codes in SmarthomeMyRoom project, that
              use AlexaSinric and/or OTA upload.
 
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
#define STASSID "WLAN-149267"                             //router name
#define STAPSK  "70169424204811969647"                    //router password

//***Alexa Sinric defines***
#define MyApiKey "aacadef6-c35c-4ef0-85fc-b00d5a63acc1 "  //TODO: Change to your sinric API Key. Your API Key is displayed on sinric.com dashboard
#define HEARTBEAT_INTERVAL 300000                         //send every 5 Minutes to sinric web

//***RoomLight defines***
#define PORT 80                   //port number

//pins
#define SWITCH 2  //GPIO2
#define RELAY 3   //RXD

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
  setupRoomLight();
  // put your setup code here, to run once:
}

//-------------- loop --------------
void loop() 
{
  ArduinoOTA.handle();  //OTA loop
  loopSinric();
  loopRoomLight();
  
  // use "TelnetStream2.println("...");" to communicate with putty
}
