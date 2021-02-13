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

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
WiFiClient client;

///////////WiFi Setup/////////////

#define MyApiKey "aacadef6-c35c-4ef0-85fc-b00d5a63acc1 " // TODO: Change to your sinric API Key. Your API Key is displayed on sinric.com dashboard
#define MySSID "WLAN-149267" // TODO: Change to your Wifi network SSID
#define MyWifiPassword "70169424204811969647" // TODO: Change to your Wifi network password

/////////////////////////////////

#define HEARTBEAT_INTERVAL 300000 // 5 Minutes
#define PORT 80               //port number

//pins
#define RED_PIN 2      //GPIO0
#define BLUE_PIN 3     //GPIO2
#define GREEN_PIN 0    //GPIO3
#define RELAY 1        //GPIOTX

//maximal color output
#define MAX_RED 255   //maximal analog RED_PIN output255
#define MAX_GREEN 60  //maximal analog GREEN_PIN output 60 // rgb=255 doesn´t look white
#define MAX_BLUE 100  //maximal analog BLUE_PIN output 100

//devices id:
String ledStringID = "5f81d9a7a471de16f91e7e2e";

uint64_t heartbeatTimestamp = 0;
bool isConnected = false;

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
                     
unsigned char redColorValue, greenColorValue, blueColorValue;

WiFiServer wifiServer(PORT);    //Wifi port

void setPowerStateOnServer(String deviceId, String value);

// deviceId is the ID assgined to your smart-home-device in sinric.com dashboard. Copy it from dashboard and paste it here

//++++++++++++++++++++++ functions for app +++++++++++++++++++++

void HTMLtoRGB(String hexstring)  //convert HTML to RGB
{
  int number = (int) strtol( &hexstring[1], NULL, 16);
  
  //split hexstring into  decimal red, green, blue values
  redColorValue = number >> 16;
  greenColorValue = number >> 8 & 0xFF;
  blueColorValue = number & 0xFF;
}

void writePins(unsigned char colorArrayNumber){ //write pins analog
        analogWrite(RED_PIN, map(colors[colorArrayNumber][0], 0, 255, 0, MAX_RED));  //map(colors[0], 0, 255, 0, MAX_RED)
        analogWrite(GREEN_PIN, map(colors[colorArrayNumber][1], 0, 255, 0, MAX_GREEN));
        analogWrite(BLUE_PIN, map(colors[colorArrayNumber][2], 0, 255, 0, MAX_BLUE));
}

//check for new input strings per TCP from android app 
void checkWiFiClient()
{
  WiFiClient client = wifiServer.available();

  if (client) {

    while (client.connected()) {  //if everything connected to esp8266
      while (client.available() > 0) {  //as long as get input
        
        String req = client.readStringUntil('X'); // String zerlegen und in char packen
        Serial.print("HTML Code: ");
        req.remove(0,2);                          //remove the first both chars of the string req (always ff)
        Serial.println(req);
        HTMLtoRGB(req);                           //maby give them as a pointer
        
        Serial.print("R: ");Serial.println(redColorValue);
        Serial.print("G: ");Serial.println(greenColorValue);
        Serial.print("B: ");Serial.println(blueColorValue);

        digitalWrite(RELAY,HIGH);
        
        //write color value into colors array
        colors[0][0] = redColorValue;
        colors[0][1] = greenColorValue;
        colors[0][2] = blueColorValue;
        writePins(0);
        
        if(redColorValue == 0 && greenColorValue == 0 && blueColorValue == 0){
          setPowerStateOnServer(ledStringID,"OFF");
          digitalWrite(RELAY,LOW);
        }
        else{
          setPowerStateOnServer(ledStringID,"ON");
        }
      }
      delay(10);
    }
    
    client.stop();
    Serial.println("Client disconnected");  //stop recieve
  }
}

//trun on devices with an Alexa command
void turnOn(String deviceId) {
  if (deviceId == ledStringID) // cupboard light
  {  
    Serial.print("Turn on cupboardLight! Device id: ");
    Serial.println(deviceId);
    digitalWrite(RELAY,HIGH);
    writePins(6); // array row to turning on
  } 
  else {
    Serial.print("Turn on for unknown device id: ");
    Serial.println(deviceId);    
  }     
}

//trun off devices with an Alexa command
void turnOff(String deviceId) {
   if (deviceId == ledStringID) // cupboard light
   {  
     Serial.print("Turn off cupboardLight! Device ID: ");
     Serial.println(deviceId);
     digitalWrite(RELAY,LOW);
     writePins(11); //array row to turning on
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

        //+++++++++++++++++ own set color function ++++++++++++++++++++++++++++
        else if (action == "SetColor"){ // set Color on light
            String hue = json["value"]["hue"];
            int colorChoose = hue.toInt();
            switch(colorChoose){
              case 0:
                Serial.println("red");
                writePins(1);
                break;   
              case 39:
                Serial.println("orange");
                writePins(2);
                break;
              case 60:
                Serial.println("yellow");
                writePins(3);
                break;
              case 120:
                Serial.println("green");
                writePins(4);
                break;
              case 194:
                Serial.println("light blue");
                writePins(5);
                break;
              case 240:
                Serial.println("blue");
                writePins(6);
                break;
              case 300:
                Serial.println("violet");
                writePins(7);
                break;
              case 277:
                Serial.println("purple");
                writePins(8);
                break;                     
              case 348:
                Serial.println("pink");
                writePins(9);
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
            writePins(10);
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

  //start wifi server on port "PORT"
  wifiServer.begin();

  //outputs/inputs
  pinMode(RED_PIN,OUTPUT);
  pinMode(GREEN_PIN,OUTPUT);
  pinMode(BLUE_PIN,OUTPUT);
  pinMode(RELAY,OUTPUT);

  digitalWrite(RELAY,LOW);
  writePins(11);  // set red- green- blue- pin low. Do it one times whenn server was going on.
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
