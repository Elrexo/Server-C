/* ===========================================================================
 
Project:      SmarthomeMyRoom

==============================================================================
 
Filename:     Server_AlexaSinricTableLight1.1.ino
 
Authors:      Tim Keicher
 
Version:      1.0 vom 02.12.2020
 
Hardware:     NodeMCU, 220V AC to 5V DC transmitter, IR-transmitter, TIP31C 
 
Software:     Arduino 1.8.10
 
Function:     TableLiths controling the table lights on the desk, turn on/off, 
              change color, change brightness. It control also the ventilator
              to turn it on or off. 
 
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

//***TableLights includes***
#include <IRremoteESP8266.h>
#include <IRsend.h>

//***esp wifi defines***
#define STASSID ""                             //router name
#define STAPSK  ""                    //router password
#define USER_SERIAL TelnetStream2

//***Alexa Sinric defines***
#define MyApiKey ""  //TODO: Change to your sinric API Key. Your API Key is displayed on sinric.com dashboard
#define HEARTBEAT_INTERVAL 300000                         //send every 5 Minutes to sinric web

//***TableLight defines***
#define SWITCH_VENTILATOR D1  //ventilator push bottom whos turns on/off about D1
#define IR_SEND_PIN D2        //send infared about Pin D2
#define VENTILATOR_ONE  D3    //should control ventilator one about D3 /maybe an other one
#define VENTILATOR_TWO  D4    //should control ventilator two about D4 /maybe an other one
#define PORT 80               //port number

//***global variables***
bool temp = 0;                        //temporary value, one means ventilator on, zero means ventilator off
unsigned char currentLevel;           //content the current level of brightness. Level go from one to three.
const unsigned char maximumLevel = 3; //maximum level of brightness
const unsigned char minimalLevel = 1; //minimal level of brightness

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
  setupTableLight();
  
  // put your setup code here, to run once:
}

//-------------- loop --------------
void loop() 
{
  ArduinoOTA.handle();  //OTA loop
  loopSinric();
  loopTableLight();
  
  // use "TelnetStream2.println("...");" to communicate with putty
}
