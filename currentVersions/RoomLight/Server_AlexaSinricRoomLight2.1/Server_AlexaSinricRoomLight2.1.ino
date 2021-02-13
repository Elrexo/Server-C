/* ===========================================================================
 
Project:      SmarthomeMyRoom

==============================================================================
 
Filename:     Server_AlexaSinricRoomLight1.1.ino
 
Authors:      Tim Keicher
 
Version:      1.1 vom 03.12.2020

Last update:  03.02.2021
 
Hardware:     esp8266-01, 3.3V/5V source, 230V AC to 12V DC transmitter,
 
Software:     Arduino 1.8.10
 
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

//***RomLight includes***
#include <NTPClient.h>
#include <RBDdimmer.h>

//***esp wifi defines***
#define STASSID ""                             //router name
#define STAPSK  ""                    //router password
#define USER_SERIAL TelnetStream2

//***Alexa Sinric defines***
#define MyApiKey ""  //TODO: Change to your sinric API Key. Your API Key is displayed on sinric.com dashboard
#define HEARTBEAT_INTERVAL 300000                         //send every 5 Minutes to sinric web

//***RoomLight defines***
#define PORT 80               //port number
#define DIM_TIME_STAMP 1000   //custom timeStamp in millis, the time to decrease soundTimer (kontrolliert grösse des Intervalls in MilliSekunden)
#define SOUND_TIME_STAMP 1000 //custom stamp, 1000 to decrease SOUND_TIMER each second
#define SOUND_TIMER 300       //start sound when soundTimer <= 0; in s for example: SOUND_TIMER = 300 => 5min rest to start sound 

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
WiFiClient client;
WiFiServer wifiServer(PORT);
WiFiUDP ntpUDP;               //also works with TCP
NTPClient timeClient(ntpUDP); //set NTP client to UDP wifi 

//-------------- setup --------------
void setup() 
{
  Serial.begin(9600);
  Serial.println("Booting");

  setupOTA();
  setupSinric();
  setupRoomLight();
}

//-------------- loop --------------
void loop() 
{
  ArduinoOTA.handle();  //OTA loop
  loopSinric();
  loopRoomLight();
  
  //use "TelnetStream2.println("...");" to communicate about putty
}
