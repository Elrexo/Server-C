/*
TV

Authors:        Tim Keicher

Created:        15.11.2020

Description:    Contains and handles all functions for TV.
*/

//devices id:
String TVID = "5f458254ff09dc1fbf8392be";

//----- handle tv setup settings -----
void setupTV(){
   //ir-send 
  irsend.begin();

  
  //start wifi server on port "PORT"
  wifiServer.begin();
}

//----- handle tv loop settings -----
void loopTV(){
  checkWiFiClient();
}

//change app received string into intager
int StringToInt(String req)
{
  String a_string = req.substring(req.length()-5, req.length());
  String b_string = req.substring(0, req.length()-5);
  unsigned int a_zahl = a_string.toInt();
  unsigned int b_zahl = b_string.toInt();
  unsigned long ergebnis = b_zahl*100000 + a_zahl;

  return ergebnis;
}

//check for new input strings per TCP from android app 
void checkWiFiClient()
{
  WiFiClient client = wifiServer.available();

  if (client) 
  {
    while (client.connected()) 
    {
      USER_SERIAL.println("====================");
      USER_SERIAL.println("Client connectec!");
      while (client.available() > 0) 
      {
        String req = client.readStringUntil('X');
        req.remove(0,2);
        unsigned long frequenz = StringToInt(req);
        USER_SERIAL.print("IR-Signal send: ");
        USER_SERIAL.println(frequenz);
        
        if(frequenz > 3000000000){
          irsend.sendSAMSUNG(frequenz,32);
          USER_SERIAL.println("Send Samsung");
          switch(frequenz){
            case 3772833823:
              volumeState++;
              break;
            case 3772829743:
              volumeState--;
              break;
            case 3772793023:
              if(tvState = true){
                setPowerStateOnServer(TVID,"ON");
                tvState = false;
              }
              else{
                setPowerStateOnServer(TVID,"ON");
                tvState = true;
              }
              break;
            case 3772837903:
              if(muteState = true){
                muteState = false;
              }
              else{
                muteState = true;
              }
              break;
          }
          //current tvInputState  not possible
        }
        else
        {
          irsend.sendNEC(frequenz,32);
          USER_SERIAL.println("Send NEC");
        }
        delay(10);
      }
    }
    client.stop();
    USER_SERIAL.println("Client disconnected");
  }
}

//turn on devices with an Alexa command
void turnOn(String deviceId) {
  if (deviceId == TVID) // TV device
  {  
    if(!tvState){
      USER_SERIAL.print("Turn on TV! Device id: ");
      USER_SERIAL.println(deviceId);
      irsend.sendSAMSUNG(ON_OFF,32);
      tvState = true;
    }
    else{
      USER_SERIAL.println("TV is already on.");
    }
  } 
  else {
    USER_SERIAL.print("Turn on for unknown device id: ");
    //USER_SERIAL.println(deviceId);
  }     
}

//turn off devices with an Alexa command
void turnOff(String deviceId) {
   if (deviceId == TVID) // TV device
   {  
    if(tvState){
      USER_SERIAL.print("Turn off TV! Device ID: ");
      USER_SERIAL.println(deviceId);
      irsend.sendSAMSUNG(ON_OFF,32);
      tvState = false;
    }
    else{
      USER_SERIAL.println("TV is already off.");
    }
   }
  else {
     USER_SERIAL.print("Turn off for unknown device id: ");
     USER_SERIAL.println(deviceId);    
  }
}

//send a sequence of numbers
void sendSequenceOfNumbers(char getLength, int getNumber){
  char numberArray[getLength];
  USER_SERIAL.println(getNumber);
  sprintf(numberArray, "%d", getNumber);  //print getLength into a char array to get individual values
  
  for(unsigned char i=0; i < getLength; i++){
    irsend.sendSAMSUNG(numbers[numberArray[i]-48],32);  //use -48, because numberarrays content the asci-code numbers
    //USER_SERIAL.println(numberArray[i]-48);
    delay(300);
  }
  USER_SERIAL.println("Number: " + getNumber);
}
