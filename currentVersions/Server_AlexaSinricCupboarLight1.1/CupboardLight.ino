/*
CupboardLight

Authors:        Tim Keicher

Created:        01.12.2020

Description:    Contains and handles all functions for CoupboardLight.
*/

//***variables***
unsigned char redColorValue, greenColorValue, blueColorValue;
String cupboardLightID = "5f4517a2ff09dc1fbf838481";

//handles setup settings
void setupCupboardLight(){
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

//handles loop settings
void loopCupboardLight(){

  checkWiFiClient();
}

//convert HTML to RGB
void HTMLtoRGB(String hexstring)
{
  int number = (int) strtol( &hexstring[1], NULL, 16);
  
  //split hexstring into  decimal red, green, blue values
  redColorValue = number >> 16;
  greenColorValue = number >> 8 & 0xFF;
  blueColorValue = number & 0xFF;
}

//write pins analog
void writePins(unsigned char colorArrayNumber){
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
        USER_SERIAL.print("HTML Code: ");
        req.remove(0,2);                          //remove the first both chars of the string req (always ff)
        USER_SERIAL.println(req);
        HTMLtoRGB(req);                           //maby give them as a pointer
        
        USER_SERIAL.print("R: ");USER_SERIAL.println(redColorValue);
        USER_SERIAL.print("G: ");USER_SERIAL.println(greenColorValue);
        USER_SERIAL.print("B: ");USER_SERIAL.println(blueColorValue);

        digitalWrite(RELAY,HIGH);
        
        //write color value into colors array
        colors[0][0] = redColorValue;
        colors[0][1] = greenColorValue;
        colors[0][2] = blueColorValue;
        writePins(0);
        
        if(redColorValue == 0 && greenColorValue == 0 && blueColorValue == 0){
          setPowerStateOnServer(cupboardLightID,"OFF");
          digitalWrite(RELAY,LOW);
        }
        else{
          setPowerStateOnServer(cupboardLightID,"ON");
        }
      }
      delay(10);
    }
    
    client.stop();
    USER_SERIAL.println("Client disconnected");  //stop recieve
  }
}

//trun on devices with an Alexa command
void turnOn(String deviceId) {
  if (deviceId == cupboardLightID) // cupboard light
  {  
    USER_SERIAL.print("Turn on cupboardLight! Device id: ");
    USER_SERIAL.println(deviceId);
    digitalWrite(RELAY,HIGH);
    writePins(6); // array row to turning on
  } 
  else {
    USER_SERIAL.print("Turn on for unknown device id: ");
    USER_SERIAL.println(deviceId);    
  }     
}

//trun off devices with an Alexa command
void turnOff(String deviceId) {
   if (deviceId == cupboardLightID) // cupboard light
   {  
     USER_SERIAL.print("Turn off cupboardLight! Device ID: ");
     USER_SERIAL.println(deviceId);
     digitalWrite(RELAY,LOW);
     writePins(11); //array row to turning on
   }
  else {
     USER_SERIAL.print("Turn off for unknown device id: ");
     USER_SERIAL.println(deviceId);    
  }
}
