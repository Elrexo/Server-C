/*
 Version 0.1 - August 24 2020
 Tim Keicher
*/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h> //  https://github.com/kakopappa/sinric/wiki/How-to-add-dependency-libraries
#include <ArduinoJson.h> // https://github.com/kakopappa/sinric/wiki/How-to-add-dependency-libraries
#include <StreamString.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
WiFiClient client;

///////////WiFi Setup/////////////

#define MyApiKey "" // TODO: Change to your sinric API Key. Your API Key is displayed on sinric.com dashboard
#define MySSID "" // TODO: Change to your Wifi network SSID
#define MyWifiPassword "" // TODO: Change to your Wifi network password

/////////////////////////////////

#define HEARTBEAT_INTERVAL 300000 // 5 Minutes
#define SWITCH_VENTILATOR D1  //ventilator push bottom whos turns on/off about D1
#define IR_SEND_PIN D2        //send infared about Pin D2
#define VENTILATOR_ONE  D3    //should control ventilator one about D3 /maybe an other one
#define VENTILATOR_TWO  D4    //should control ventilator two about D4 /maybe an other one
#define PORT 80               //port number

//devices id:
String tableLightID = "5f43b237ff09dc1fbf835485";
String ventilatorID = "5f441b45ff09dc1fbf8362f7";

uint64_t heartbeatTimestamp = 0;
bool isConnected = false;
bool temp = 0;                        //temporary value, one means ventilator on, zero means ventilator off
long frequenz;                        //content the frequenz value
unsigned char currentLevel;           //content the current level of brightness. Level go from one to three.
const unsigned char maximumLevel = 3; //maximum level of brightness
const unsigned char minimalLevel = 1; //minimal level of brightness

//frequencies for controlling the table light
/*const long frequencyRed = 16738455;
const long frequencyBlue = 16756815;
const long frequencyGreen = 16750695;
const long frequencyOrange = 16726215;
const long frequencyLightBlue = 16743045;
const long frequencyYellow = 16730805;
const long frequencyPurple = 16716015;
const long frequencyPink = 16728765;
const long frequencyWhite = 16732845;
const long frequencyLightOn = 16753245;
const long frequencyLightOff = 16769565;
const long frequencyBrightnessUp = 16754775;
const long frequencyBrightnessDown = 16748655;*/

WiFiServer wifiServer(PORT);    //Wifi port
IRsend irsend(IR_SEND_PIN);     //in library IRsend.h, send about IR_SEND_PIN infrared

void setPowerStateOnServer(String deviceId, String value);

// deviceId is the ID assgined to your smart-home-device in sinric.com dashboard. Copy it from dashboard and paste it here

//updated tablelight on/off/color etc. from android app
void updateTableLight()
{
  Serial.print("IR-Signal send: ");
  Serial.println(frequenz);
  irsend.sendNEC(frequenz,32); //send frequenz to light
  switch(frequenz){
    case 16753245:
      setPowerStateOnServer(tableLightID,"ON");
      break;
    case 16769565:
      setPowerStateOnServer(tableLightID,"OFF");
      break;
    case 16754775:
      currentLevel++;
      break;
    case 16748655:
      currentLevel--;
      break;
  }
}

//updateted ventilator ON/OFF from android app
void updateVentilator()
{
  Serial.println("Received code:");
  Serial.println(frequenz);
  //use switch case if we want regulate every ventilator seperate
  if(frequenz==300 && temp==0)              //if requirements of vantilator on is true
  {
    temp = 1;
    digitalWrite(VENTILATOR_ONE,HIGH); //write both ventilators on. Maybe it will be seperated later 
    digitalWrite(VENTILATOR_TWO,HIGH);
    Serial.println("Ventilator: ON");
    setPowerStateOnServer(ventilatorID,"ON");
    delay(100);
  }
  else if(frequenz==300 && temp==1)       //if requirements of vantilator off is true
  {
    temp = 0;
    digitalWrite(VENTILATOR_ONE,LOW); //write both ventilators off. Maybe it will be seperated later 
    digitalWrite(VENTILATOR_TWO,LOW);
    Serial.println("Ventilator: OFF");
    setPowerStateOnServer(ventilatorID,"OFF");
    delay(100);
  }
  else                                //if anyone is going wrong
  {
    Serial.println("Temp or code have a false value. Temp isn't zero or one or code isn't 300.");
    Serial.println("Temp:"+temp);
  }
}

//check for new input strings per TCP from android app 
void checkWiFiClient()
{
    WiFiClient client = wifiServer.available();     //client says wether wifi is available or not

  if (client)
  {
    while (client.connected())                    //check for conecting with anything
    {
      while (client.available() > 0)              //if arduino receive anything
      {
        String req = client.readStringUntil('X'); //read string until X   //try \r
        req.remove(0,2);                          //remove first both values
        frequenz = req.toInt();                   //string to integer
        
        if(frequenz <= 500)                       //less or equal 1 was determined in smarthome-app, normaly it should be 300
        {
          updateVentilator();
        }
        else
        {
          updateTableLight();
        }
      }
      delay(10);
    }
    client.stop();                                //disconected client
    Serial.println("Client disconnected");
  }
}

//trun on devices with an Alexa command
void turnOn(String deviceId) {
  if (deviceId == tableLightID) // TableLight
  {  
    Serial.print("Turn on tableLight! Device id: ");
    Serial.println(deviceId);
    irsend.sendNEC(16753245,32);
  } 
  else  if (deviceId == ventilatorID) // TableVentilator
  {  
    Serial.print("Turn on ventilator! Device id: ");
    Serial.println(deviceId);
    temp = 1;
    digitalWrite(VENTILATOR_ONE,HIGH); //write both ventilators on. Maybe it will be seperated later 
    digitalWrite(VENTILATOR_TWO,HIGH);
  } 
  else {
    Serial.print("Turn on for unknown device id: ");
    Serial.println(deviceId);    
  }     
}

//trun off devices with an Alexa command
void turnOff(String deviceId) {
   if (deviceId == tableLightID) // Device ID of first device
   {  
     Serial.print("Turn off tableLight! Device ID: ");
     Serial.println(deviceId);
     irsend.sendNEC(16769565,32);
   }
   else  if (deviceId == ventilatorID) // TableVentilator
  {  
    Serial.print("Turn off ventilator! Device id: ");
    Serial.println(deviceId);
    temp = 0;
    digitalWrite(VENTILATOR_ONE,LOW); //write both ventilators off. Maybe it will be seperated later 
    digitalWrite(VENTILATOR_TWO,LOW);
  } 
  else {
     Serial.print("Turn off for unknown device id: ");
     Serial.println(deviceId);    
  }
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      isConnected = false;    
      Serial.printf("[WSc] Webservice disconnected from sinric.com!\n");
      break;
    case WStype_CONNECTED: {
      isConnected = true;
      Serial.printf("[WSc] Service connected to sinric.com at url: %s\n", payload);
      Serial.printf("Waiting for commands from sinric.com ...\n");        
      }
      break;
    case WStype_TEXT: {
        Serial.printf("[WSc] get text: %s\n", payload);
        // Example payloads

        // For Switch or Light device types
        // {"deviceId": xxxx, "action": "setPowerState", value: "ON"} // https://developer.amazon.com/docs/device-apis/alexa-powercontroller.html

        // For Light device type
        // Look at the light example in github
          
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject((char*)payload); 
        String deviceId = json ["deviceId"];     
        String action = json ["action"];

        //+++++++++++++++++ switch ON/OFF devices ++++++++++++++++++++++++++++++++
        if(action == "setPowerState") { // Switch or Light
            String value = json ["value"];
            if(value == "ON") {
                turnOn(deviceId);
            } else {
                turnOff(deviceId);
            }
        }

        //++++++++++++++++++ own function for Brightness +++++++++++++++++++++++++
        else if (action == "SetBrightness"){  // set Brightness in on of tree levels
            String value = json["value"];
            unsigned char level = value.toInt();
            if(level > maximumLevel){       //check and set level of maximum level, if called level higher than maximum level (three)
              level = maximumLevel;
            }
            else if(level < minimalLevel){  //check and set level of minimal level, if called level higher than minimal level (one)
              level = minimalLevel;
            }
            signed char differentLevel = level - currentLevel;  //anythings must be wrong with level equal currentLevel
            if(currentLevel < level){
              for(unsigned char i=0; i < differentLevel; i++){
                irsend.sendNEC(16754775,32);  //send brightness up
                delay(10);
                Serial.println("freguency for brightness up was sending");
              }
            }
            else if(currentLevel > level){
              for(signed char i=0; i > differentLevel; i--){
                irsend.sendNEC(16748655,32);  //send brightness down
                delay(10);
                Serial.println("freguency for brightness down was sending");
              }
            }
            else{
              Serial.println("Brightness already at this level!");
            }
            currentLevel = level;
        }
        
        //+++++++++++++++++ own set color function ++++++++++++++++++++++++++++
        else if (action == "SetColor"){ // set Color on light
            String hue = json["value"]["hue"];
            int colorChoose = hue.toInt();
            switch(colorChoose){
              case 0:
                Serial.println("red");
                irsend.sendNEC(16738455,32);
                break;   
              case 39:
                Serial.println("orange");
                irsend.sendNEC(16726215,32);
                break;
              case 60:
                Serial.println("yellow");
                irsend.sendNEC(16730805,32);
                break;
              case 120:
                Serial.println("green");
                irsend.sendNEC(16750695,32);
                break;
              case 194:
                Serial.println("light blue");
                irsend.sendNEC(16743045,32);
                break;
              case 240:
                Serial.println("blue");
                irsend.sendNEC(16756815,32);
                break;
              case 277:
                Serial.println("purple");
                irsend.sendNEC(16716015,32);
                break;                     
              case 348:
                Serial.println("pink");
                irsend.sendNEC(16728765,32);
                break;
              default:
                Serial.println("Color not declared!");
                break;              
            }
        }
        else if(action == "SetColorTemperature"){ //just for setting color back to white
          String value = json["value"];
          if(value == "4000"){
            Serial.println("white");
            irsend.sendNEC(16732845,32);
          }
        }
        /*else if (action == "SetTargetTemperature") {  //I haven't any temperature settings
            String deviceId = json ["deviceId"];     
            String action = json ["action"];
            String value = json ["value"];
        }*/
        else if (action == "test") {
            Serial.println("[WSc] received test command from sinric.com");
        }
      }
      break;
    case WStype_BIN:
      Serial.printf("[WSc] get binary length: %u\n", length);
      break;
  }
}

void setup() {
  Serial.begin(115200);
  
  WiFiMulti.addAP(MySSID, MyWifiPassword);
  Serial.println();
  Serial.print("Connecting to Wifi: ");
  Serial.println(MySSID);

  // Waiting for Wifi connect
  while(WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  if(WiFiMulti.run() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("WiFi connected. ");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }

  // server address, port and URL
  webSocket.begin("iot.sinric.com", 80, "/");

  // event handler
  webSocket.onEvent(webSocketEvent);
  webSocket.setAuthorization("apikey", MyApiKey);
  
  // try again every 5000ms if connection has failed
  webSocket.setReconnectInterval(5000);   // If you see 'class WebSocketsClient' has no member named 'setReconnectInterval' error update arduinoWebSockets

  //ir-send 
  irsend.begin();

  //start wifi server on port "PORT"
  wifiServer.begin();

  //outputs/inputs
  pinMode(VENTILATOR_ONE,OUTPUT);           //set VENTILATOR_ONE and VENTILATOR_TWO as an output
  pinMode(VENTILATOR_TWO,OUTPUT);
}

void loop() {
  webSocket.loop();
  
  if(isConnected) {
      uint64_t now = millis();
      
      // Send heartbeat in order to avoid disconnections during ISP resetting IPs over night. Thanks @MacSass
      if((now - heartbeatTimestamp) > HEARTBEAT_INTERVAL) {
          heartbeatTimestamp = now;
          webSocket.sendTXT("H");          
      }
  }   
  checkWiFiClient();
}

// If you are going to use a push button to on/off the switch manually, use this function to update the status on the server
// so it will reflect on Alexa app.
// eg: setPowerStateOnServer("deviceid", "ON")

// Call ONLY If status changed. DO NOT CALL THIS IN loop() and overload the server. 

void setPowerStateOnServer(String deviceId, String value) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["deviceId"] = deviceId;
  root["action"] = "setPowerState";
  root["value"] = value;
  StreamString databuf;
  root.printTo(databuf);
  
  webSocket.sendTXT(databuf);
}
