/*
AlexaSinric

Authors:        Tim Keicher

Created:        15.11.2020

Description:    Contains and handles all functions for AlexaSinric.
*/
 
//***variables***
uint64_t heartbeatTimestamp = 0;
bool isConnected = false;

//***prototyp declaration***
void setPowerStateOnServer(String deviceId, String value);

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

        //+++++++++++++++++++++ set power state ++++++++++++++++++++
        if(action == "setPowerState") { // Switch or Light
            String value = json ["value"];
            if(value == "ON") {
                turnOn(deviceId);
            } else {
                turnOff(deviceId);
            }
        }

        //+++++++++++++++++++++ adjust volume +++++++++++++++++++++
        else if(action == "AdjustVolume"){
          String value = json ["value"]["volume"];
          if(value == "10"  && volumeState <= maxVolume){ //increase volume by 2 if volumeState not maximal
            irsend.sendSAMSUNG(VOLUME_UP,32);
            delay(300);
            irsend.sendSAMSUNG(VOLUME_UP,32);
            volumeState += 2;
            USER_SERIAL.println("Volum up by to times.");
          }
          else if(value == "-10" && volumeState >= minVolume){  //reduced volume by 2 if volumeState not minimal
            irsend.sendSAMSUNG(VOLUME_DOWN,32);
            delay(300);
            irsend.sendSAMSUNG(VOLUME_DOWN,32);
            volumeState -= 2;
            USER_SERIAL.println("Volum down by to times.");
          }
        }
        
        //+++++++++++++++++ set volume +++++++++++++++++++++++++++
        else if(action == "SetVolume"){
          String value = json ["value"]["volume"];
          unsigned char volumeNumber = value.toInt();
          if(volumeNumber >= maxVolume){  //set volume to maximal if there are higher than maxVolume
            volumeNumber = maxVolume;     //minimal value cann't be less than zero
          }
          signed char differentVolume = volumeNumber - volumeState;
          if(volumeState < volumeNumber){
            for(signed char i=0; i < differentVolume; i++){
              irsend.sendSAMSUNG(VOLUME_UP,32);
              delay(300);
            }
            USER_SERIAL.print("Volume increase by ");  //why does USER_SERIAL.println("..." + differentVolume + "..."); not working? REEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
            USER_SERIAL.print(differentVolume);
            USER_SERIAL.println(" times.");
          }
          else if(volumeState > volumeNumber){
            for(signed char i=0; i > differentVolume; i--){
              irsend.sendSAMSUNG(VOLUME_DOWN,32);
              delay(300);
            }
            USER_SERIAL.print("Volume reduced by ");
            USER_SERIAL.print(differentVolume);
            USER_SERIAL.println(" times.");
          }
          else{
            USER_SERIAL.println("Volume already at this volume");
          }
          volumeState = volumeNumber;
        }

        //+++++++++++++++++++++ set mute +++++++++++++++++++++++++++++++
        else if(action == "SetMute"){
          String value = json["value"];
          irsend.sendSAMSUNG(MUTE_UNMUTE,32);
          if(value == "true"){
            muteState = true;
          }
          else{
            muteState = false;
          }
        }

        //++++++++++++++++++++ skip channel ++++++++++++++++++++++++++++
        else if(action == "SkipChannels"){
          String value = json ["value"]["channelCount"];
          if(value == "1"){
            irsend.sendSAMSUNG(CHANNEL_UP,32);
            USER_SERIAL.println("Channel up!");
          }
          else{
            irsend.sendSAMSUNG(CHANNEL_DOWN,32);
            USER_SERIAL.println("Channel down!");
          }
        }

        //+++++++++++++++++++++ change channel +++++++++++++++++++++++
        else if(action == "ChangeChannel"){
          String value = json["value"]["channel"]["number"];
          USER_SERIAL.println(value);
          
          if(value != NULL){
            USER_SERIAL.println("Number was received");
            char lengthNumber = value.length();
            int valueNumber = value.toInt();
            sendSequenceOfNumbers(lengthNumber,valueNumber);
          }
          
          else{
            value == json["value"]["channelMetadata"]["name"];
            USER_SERIAL.println("Metadata was received");
            //if...else if (sat1, zdf, kabel eins, 3sat, tele5) not at working because alexa haven't a god control
            if(value == "DMAX"){
              sendSequenceOfNumbers(3,747);
            }
            else if(value =="comedy central viva"){
              sendSequenceOfNumbers(2,14);
            }
            else if(value == "pro tv int"){ //pro 7
              sendSequenceOfNumbers(1,6);
            }
            else if(value == "Disney Channel"){
              sendSequenceOfNumbers(2,12);
            }
            else if(value == "nickelodeon"){
              sendSequenceOfNumbers(3,589);
            }
            else if(value == "fox"){  //vox
              sendSequenceOfNumbers(1,4);
            }
            else if(value == "n-tv"){
              sendSequenceOfNumbers(1,9);
            }
            else if(value == "n. twenty four"){ //n24
              sendSequenceOfNumbers(2,10);
            }
            else if(value == "DELUXE MUSIC"){
              sendSequenceOfNumbers(2,13);
            }
            else if(value == "pro sieben max"){
              sendSequenceOfNumbers(3,239);
            }
            else if(value == "SUPER RTL"){
              sendSequenceOfNumbers(3,614);
            }
            else if(value == "rtl"){
              sendSequenceOfNumbers(1,2);
            }
            else if(value == "rtl 2"){
              sendSequenceOfNumbers(1,3);
            }
          }
        }

        //++++++++++++++++++++ select input ++++++++++++++++++++++++++
        //tvInputState just working if input only changed by tv commands
        //if one command per app or fire tv input state is wrong and cann't being corrected
        //there is one bulshit way to corrected:  Say(...to input "number"...) as long as
        //                                        input is out of area of validity. After them
        //                                        there will be appear a screen with an ok button.
        //                                        Activat tis button and are finished.
        else if(action == "SelectInput"){ //say: alexa, switch input to input "number" on tv
          String value = json ["value"]["input"];
          value.remove(0,6);
          USER_SERIAL.println(value);
          unsigned char inputNumber = value.toInt();
          
          if(inputNumber <= 5){ //input number cann't be negativ or zero
            signed char differentInputState = inputNumber - tvInputState;
            irsend.sendSAMSUNG(SOURCE,32);
            delay(500);
            
            if(tvInputState < inputNumber){
              for(signed char i=0; i < differentInputState; i++){
                irsend.sendSAMSUNG(CROSS_DOWN,32);
                delay(500);
              }
              USER_SERIAL.println("Input swtich up");
            }
            else if(tvInputState > inputNumber){
              for(signed char i=0; i > differentInputState; i--){
                irsend.sendSAMSUNG(CROSS_UP,32);
                delay(500);
              }
              USER_SERIAL.println("Input switch down");
            }
            else{
              USER_SERIAL.println("Input already on this input.");
            }
            irsend.sendSAMSUNG(ENTER,32);
            tvInputState = inputNumber;
          }
          
          else{
            USER_SERIAL.println("Input number to high");
          }
        }
        
        //untility button
        else if(action == "Play"){
          irsend.sendSAMSUNG(ENTER,32);
          USER_SERIAL.println("enter pressed");
        }
        else if(action == "Rewind"){
          irsend.sendSAMSUNG(PRE_CH,32);
          USER_SERIAL.println("pre-ch pressed");
        }
        else if(action == "Previous"){
          irsend.sendSAMSUNG(RETURN,32);
          USER_SERIAL.println("return pressed");
        }
        else if(action == "FastForward"){
          irsend.sendSAMSUNG(INFO,32);
          USER_SERIAL.println("info pressed");
        }
        else if(action == "Stop"){
          USER_SERIAL.println("sleep timer 30 in min");
          irsend.sendSAMSUNG(TOOLS,32);
          delay(300);
          for(char i=0; i<4; i++){
            irsend.sendSAMSUNG(CROSS_DOWN,32);
            delay(300);
          }
          irsend.sendSAMSUNG(ENTER,32);
          delay(300);
          irsend.sendSAMSUNG(CROSS_DOWN,32);
          delay(300);
          irsend.sendSAMSUNG(ENTER,32);
          delay(300);
          irsend.sendSAMSUNG(EXIT,32);
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
