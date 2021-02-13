/*
 Version 0.1 - August 24 2020
 Tim Keicher
*/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h> //https://github.com/kakopappa/sinric/wiki/How-to-add-dependency-libraries
#include <ArduinoJson.h>      //https://github.com/kakopappa/sinric/wiki/How-to-add-dependency-libraries
#include <StreamString.h>

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
WiFiClient client;

///////////WiFi Setup/////////////

#define MyApiKey "aacadef6-c35c-4ef0-85fc-b00d5a63acc1 "  // TODO: Change to your sinric API Key. Your API Key is displayed on sinric.com dashboard
#define MySSID "WLAN-149267"                              // TODO: Change to your Wifi network SSID
#define MyWifiPassword "70169424204811969647"             // TODO: Change to your Wifi network password

/////////////////////////////////

#define HEARTBEAT_INTERVAL 300000 // 5 Minutes
#define PORT 80                   //port number

//pins
#define RELAY 2      //GPIO2
#define SWITCH 3     //GPIO0

//devices id:
String roomLightID = "5f47bd84d1e9084a708118ba";

uint64_t heartbeatTimestamp = 0;
boolean isConnected = false;
boolean relayState = false;   //reamin realy on or off

WiFiServer wifiServer(PORT);    //Wifi port

void setPowerStateOnServer(String deviceId, String value);

// deviceId is the ID assgined to your smart-home-device in sinric.com dashboard.

//++++++++++++++++++++++ functions for app +++++++++++++++++++++
//check for new input strings per TCP from android app 
void checkWiFiClient()
{
  WiFiClient client = wifiServer.available();
  //read and handle wifi input
  if (client) {
    while (client.connected()) {
      while (client.available() > 0) {
        String req = client.readStringUntil('X');
        req.remove(0,2);   
        long appCode = req.toInt();
        Serial.print("Receive integer: ");
        Serial.println(appCode);
        
        if(appCode<0 && !relayState){
          digitalWrite(RELAY,HIGH);
          relayState = true;
          Serial.println("Relay: HIGH\nTCP_Signal\n");
          setPowerStateOnServer(roomLightID,"ON");
        }
        else if(appCode<0 && relayState){
          digitalWrite(RELAY,LOW);
          relayState = false;
          Serial.println("Relay: LOW\nTCP_Signal\n");
          setPowerStateOnServer(roomLightID,"OFF");
        }
        else
          Serial.println("Error");
      }
      delay(10);
    }
    client.stop();
    Serial.println("Client disconnected");
  }
  //read and handle button input
  else if(digitalRead(SWITCH)== HIGH && !relayState){  //activate Relay by button
    digitalWrite(RELAY,HIGH);       //light on
    relayState = true;    
    setPowerStateOnServer(roomLightID,"ON");
    Serial.println("Relay: HIGH\nButton pressed!\n");

    delay(700);
  }
  else if(digitalRead(SWITCH)== HIGH && relayState){  //disactivate Relay by button
    digitalWrite(RELAY,LOW);        //light off
    relayState = false;
    setPowerStateOnServer(roomLightID,"OFF");
    Serial.println("Relay: LOW\nButton pressed!\n");
    delay(700);
  }
}

//trun on devices with an Alexa command
void turnOn(String deviceId) {
  if (deviceId == roomLightID) // room light
  {  
    Serial.print("Turn on RoomLight! Device id: ");
    Serial.println(deviceId);
    digitalWrite(RELAY,HIGH);
    relayState = true;
  } 
  else {
    Serial.print("Turn on for unknown device id: ");
    Serial.println(deviceId);    
  }     
}

//trun off devices with an Alexa command
void turnOff(String deviceId) {
   if (deviceId == roomLightID) // room light
   {  
     Serial.print("Turn off RoomLight! Device ID: ");
     Serial.println(deviceId);
     digitalWrite(RELAY,LOW);
     relayState = false;
   }
  else {
     Serial.print("Turn off for unknown device id: ");
     Serial.println(deviceId);    
  }
}

//++++++++++++++++++++++ functions for Alexa +++++++++++++++++++++
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
  pinMode(RELAY,OUTPUT);
  pinMode(SWITCH,INPUT);
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
