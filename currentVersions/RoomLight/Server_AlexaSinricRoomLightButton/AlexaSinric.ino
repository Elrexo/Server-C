/*
AlexaSinric

Authors:        Tim Keicher

Created:        1.0 vom 25.01.2021

Description:    Contains and handles all functions for AlexaSinric.
*/
 
//***variables***
uint64_t heartbeatTimestamp = 0;
bool isConnected = false;   //is conneced?
String firstDeviceID = "600e801a98845d1692f0317b";  //deviceId is the ID assgined to your smart-home-device in sinric.com dashboard. Copy it from dashboard and paste it here. You can add more than one device

//***prototyp declaration***
void setPowerStateOnServer(String deviceId, String value);

//----- turn on or off devices with an Alexa command -----
void turnOnOff(String deviceId) {
  if (deviceId == firstDeviceID) //Device ID of first device, you can add more than one, use else if
  {  
    USER_SERIAL.print("Turn on device id: ");
    USER_SERIAL.println(deviceId);
    irsend.sendNEC(onOffFrequency,32);
  } 
  else {
    USER_SERIAL.print("Turn on for unknown device id: ");
    USER_SERIAL.println(deviceId);    
  }     
}

//----- handle web socket events -----
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

        //+++++ set power state +++++
        if(action == "setPowerState") { // Switch or Light
            String value = json ["value"];
            if(value == "ON" && !partyState) {
                turnOnOff(deviceId);
                partyState = true;
            }else if(value == "OFF" && partyState){
                turnOnOff(deviceId);
                partyState = false;
            }
        }
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
