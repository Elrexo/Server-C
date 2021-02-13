/*
AlexaSinric

Authors:        Tim Keicher

Created:        02.12.2020

Description:    Contains and handles all functions for AlexaSinric.
*/
 
//***variables***
uint64_t heartbeatTimestamp = 0;
bool isConnected = false;   //is conneced?
String firstDeviceID = "";  //deviceId is the ID assgined to your smart-home-device in sinric.com dashboard. Copy it from dashboard and paste it here. You can add more than one device

//***prototyp declaration***
void setPowerStateOnServer(String deviceId, String value);

//----- turn on devices with an Alexa command -----
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      isConnected = false;    
      USER_SERIAL.printf("[WSc] Webservice disconnected from sinric.com!\n");
      break;
    case WStype_CONNECTED: {
      isConnected = true;
      USER_SERIAL.printf("[WSc] Service connected to sinric.com at url: %s\n", payload);
      USER_SERIAL.printf("Waiting for commands from sinric.com ...\n");        
      }
      break;
    case WStype_TEXT: {
        USER_SERIAL.printf("[WSc] get text: %s\n", payload);
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
                USER_SERIAL.println("freguency for brightness up was sending");
              }
            }
            else if(currentLevel > level){
              for(signed char i=0; i > differentLevel; i--){
                irsend.sendNEC(16748655,32);  //send brightness down
                delay(10);
                USER_SERIAL.println("freguency for brightness down was sending");
              }
            }
            else{
              USER_SERIAL.println("Brightness already at this level!");
            }
            currentLevel = level;
        }
        
        //+++++++++++++++++ own set color function ++++++++++++++++++++++++++++
        else if (action == "SetColor"){ // set Color on light
            String hue = json["value"]["hue"];
            int colorChoose = hue.toInt();
            switch(colorChoose){
              case 0:
                USER_SERIAL.println("red");
                irsend.sendNEC(16738455,32);
                break;   
              case 39:
                USER_SERIAL.println("orange");
                irsend.sendNEC(16726215,32);
                break;
              case 60:
                USER_SERIAL.println("yellow");
                irsend.sendNEC(16730805,32);
                break;
              case 120:
                USER_SERIAL.println("green");
                irsend.sendNEC(16750695,32);
                break;
              case 194:
                USER_SERIAL.println("light blue");
                irsend.sendNEC(16743045,32);
                break;
              case 240:
                USER_SERIAL.println("blue");
                irsend.sendNEC(16756815,32);
                break;
              case 277:
                USER_SERIAL.println("purple");
                irsend.sendNEC(16716015,32);
                break;                     
              case 348:
                USER_SERIAL.println("pink");
                irsend.sendNEC(16728765,32);
                break;
              default:
                USER_SERIAL.println("Color not declared!");
                break;              
            }
        }
        else if(action == "SetColorTemperature"){ //just for setting color back to white
          String value = json["value"];
          if(value == "4000"){
            USER_SERIAL.println("white");
            irsend.sendNEC(16732845,32);
          }
        }
        /*else if (action == "SetTargetTemperature") {  //I haven't any temperature settings
            String deviceId = json ["deviceId"];     
            String action = json ["action"];
            String value = json ["value"];
        }*/
        else if (action == "test") {
            USER_SERIAL.println("[WSc] received test command from sinric.com");
        }
      }
      break;
    case WStype_BIN:
      USER_SERIAL.printf("[WSc] get binary length: %u\n", length);
      break;
  }
}

//----- handle Sinric setup settings -----
void setupSinric() {
  // server address, port and URL
  webSocket.begin("iot.sinric.com", 80, "/");

  // event handler
  webSocket.onEvent(webSocketEvent);
  webSocket.setAuthorization("apikey", MyApiKey);
  
  // try again every 5000ms if connection has failed
  webSocket.setReconnectInterval(5000);   // If you see 'class WebSocketsClient' has no member named 'setReconnectInterval' error update arduinoWebSockets
}

//----- handle Sinric loop settings -----
void loopSinric() {
  webSocket.loop();
  
  if(isConnected) {
    uint64_t now = millis();
    
    // Send heartbeat in order to avoid disconnections during ISP resetting IPs over night. Thanks @MacSass
    if((now - heartbeatTimestamp) > HEARTBEAT_INTERVAL) {
        heartbeatTimestamp = now;
        webSocket.sendTXT("H");          
    }
  }   
}

// If you are going to use a push button to on/off the switch manually, use this function to update the status on the server
// so it will reflect on Alexa app.
// eg: setPowerStateOnServer("deviceid", "ON")

// Call ONLY If status changed. DO NOT CALL THIS IN loop() and overload the server. 

//----- set state on sinric server -----
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
