/*
TableLight

Authors:        Tim Keicher

Created:        02.12.2020

Description:    Contains and handles all functions for to turn on/off or change color by lights on desk.
                Handles the ventilator too.
*/

//handles TableLight setup settings
void setupTableLight(){
  //ir-send 
  irsend.begin();

  //start wifi server on port "PORT"
  wifiServer.begin();

  //outputs/inputs
  pinMode(VENTILATOR_ONE,OUTPUT);           //set VENTILATOR_ONE and VENTILATOR_TWO as an output
  pinMode(VENTILATOR_TWO,OUTPUT);
}

//handles TableLight loop settings
void loopTableLight(){
  checkWiFiClient();
}

//variables
String tableLightID = "5f43b237ff09dc1fbf835485";
String ventilatorID = "5f441b45ff09dc1fbf8362f7";
long frequenz;  //content the frequenz value

//updated tablelight on/off/color etc. from android app
void updateTableLight()
{
  USER_SERIAL.print("IR-Signal send: ");
  USER_SERIAL.println(frequenz);
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
  USER_SERIAL.println("Received code:");
  USER_SERIAL.println(frequenz);
  //use switch case if we want regulate every ventilator seperate
  if(frequenz==300 && temp==0)              //if requirements of vantilator on is true
  {
    temp = 1;
    digitalWrite(VENTILATOR_ONE,HIGH); //write both ventilators on. Maybe it will be seperated later 
    digitalWrite(VENTILATOR_TWO,HIGH);
    USER_SERIAL.println("Ventilator: ON");
    setPowerStateOnServer(ventilatorID,"ON");
    delay(100);
  }
  else if(frequenz==300 && temp==1)       //if requirements of vantilator off is true
  {
    temp = 0;
    digitalWrite(VENTILATOR_ONE,LOW); //write both ventilators off. Maybe it will be seperated later 
    digitalWrite(VENTILATOR_TWO,LOW);
    USER_SERIAL.println("Ventilator: OFF");
    setPowerStateOnServer(ventilatorID,"OFF");
    delay(100);
  }
  else                                //if anyone is going wrong
  {
    USER_SERIAL.println("Temp or code have a false value. Temp isn't zero or one or code isn't 300.");
    USER_SERIAL.println("Temp:"+temp);
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
    USER_SERIAL.println("Client disconnected");
  }
}

//trun on devices with an Alexa command
void turnOn(String deviceId) {
  if (deviceId == tableLightID) // TableLight
  {  
    USER_SERIAL.print("Turn on tableLight! Device id: ");
    USER_SERIAL.println(deviceId);
    irsend.sendNEC(16753245,32);
  } 
  else  if (deviceId == ventilatorID) // TableVentilator
  {  
    USER_SERIAL.print("Turn on ventilator! Device id: ");
    USER_SERIAL.println(deviceId);
    temp = 1;
    digitalWrite(VENTILATOR_ONE,HIGH); //write both ventilators on. Maybe it will be seperated later 
    digitalWrite(VENTILATOR_TWO,HIGH);
  } 
  else {
    USER_SERIAL.print("Turn on for unknown device id: ");
    USER_SERIAL.println(deviceId);    
  }     
}

//trun off devices with an Alexa command
void turnOff(String deviceId) {
   if (deviceId == tableLightID) // Device ID of first device
   {  
     USER_SERIAL.print("Turn off tableLight! Device ID: ");
     USER_SERIAL.println(deviceId);
     irsend.sendNEC(16769565,32);
   }
   else  if (deviceId == ventilatorID) // TableVentilator
  {  
    USER_SERIAL.print("Turn off ventilator! Device id: ");
    USER_SERIAL.println(deviceId);
    temp = 0;
    digitalWrite(VENTILATOR_ONE,LOW); //write both ventilators off. Maybe it will be seperated later 
    digitalWrite(VENTILATOR_TWO,LOW);
  } 
  else {
     USER_SERIAL.print("Turn off for unknown device id: ");
     USER_SERIAL.println(deviceId);    
  }
}
