/* ===========================================================================
 
Project:      SmarthomeMyRoom

==============================================================================
 
Filename:     Server_AlexaSinricTV1.1.ino
 
Authors:      Tim Keicher
 
Version:      1.1 vom 01.12.2020
 
Hardware:     esp8266-01, 3.3V/5V source, TIP31C, IR-transmitter
 
Software:     Arduino 1.8.10
 
Function:     This program control the room tv. It receive commands from alexa 
              and the associated app and send the matching frequency. The
              programm handles also the frequencies for the room recorder, but just
              might get commands from the app. 
 
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

//***TV includes***
#include <IRremoteESP8266.h>
#include <IRsend.h>

//***esp wifi defines***
#define STASSID ""                             //router name
#define STAPSK  ""                    //router password
#define USER_SERIAL TelnetStream2

//***Alexa Sinric defines***
#define MyApiKey ""  //TODO: Change to your sinric API Key. Your API Key is displayed on sinric.com dashboard
#define HEARTBEAT_INTERVAL 300000                         //send every 5 Minutes to sinric web

//***TV defines***
#define maxVolume 30              //safe point for max volum
#define minVolume 0               //safe point for min volum
#define IR_SEND_PIN 2             //send infared about Pin D2
#define PORT 80                   //port number

//frequencies
#define ON_OFF  3772793023
#define VOLUME_UP 3772833823
#define VOLUME_DOWN 3772829743
#define CHANNEL_UP 3772795063
#define CHANNEL_DOWN 3772778743
#define MUTE_UNMUTE 3772837903
#define SOURCE 3772809343
#define ENTER 3772782313
#define CROSS_UP 3772778233
#define CROSS_DOWN 3772810873
#define RETURN 3772783333
#define PRE_CH 3772827703
#define INFO 3772839943
#define TOOLS 3772830253
#define EXIT 3772822603
#define ROOM_LIGHT_ON 16111111  //if the light already on, than it will be turning off too
#define ROOM_LIGHT_OFF 16555555 //hardcoded off just turn light off

//***global variables***
long numbers[10] = {3772811383, //content the frequencies to send the numbers 0 to 9
                    3772784863,
                    3772817503,
                    3772801183,
                    3772780783,
                    3772813423,
                    3772797103,
                    3772788943,
                    3772821583,
                    3772805263};
                    
bool tvState = false;         // TV state for on/off
bool muteState = false;
unsigned char volumeState = 0;
signed char tvInputState = 1; // input at tv (HDMI, TV, AV, ...)

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
WiFiClient client;

WiFiServer wifiServer(PORT);    //create server
IRsend irsend(IR_SEND_PIN);     //in library IRsend.h, send about IR_SEND_PIN infrared

//-------------- setup --------------
void setup() 
{
  Serial.begin(115200);
  Serial.println("Booting");

  setupOTA();
  setupSinric();
  setupTV();
}

//-------------- loop --------------
void loop() 
{
  ArduinoOTA.handle();  //OTA loop
  loopSinric();
  loopTV();
  
  //use "TelnetStream2.println("...");" to communicate with putty
}
