/*
AlexaSinric

Authors:        Tim Keicher

Created:        03.12.2020

Description:    Contains and handles all functions for room light.
*/

//handles RoomLight setup settings
void setupRoomLight(){
  //start wifi server on port "PORT"
  wifiServer.begin();

  //outputs/inputs
  pinMode(RELAY,OUTPUT);
  pinMode(SWITCH,INPUT);
}

//handle RoomLight loop settings
void loopRoomLight(){
  checkWiFiClient();
}

//***variables***
String roomLightID = "5f47bd84d1e9084a708118ba";
boolean relayState = false;   //reamin realy on or of

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

  /*
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
  }*/
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

