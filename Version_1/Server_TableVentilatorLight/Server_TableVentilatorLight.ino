#include "ESP8266WiFi.h"
#include <string.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

const char* ssid     = "WLAN-149267";           //router name
const char* password = "70169424204811969647";  //router password

//defines
#define SWITCH_VENTILATOR D1  //ventilator push bottom whos turns on/off 
#define IR_SEND_PIN D2        //send infared about Pin D2
#define VENTILATOR_ONE  D3    //should control ventilator one about D3 /maybe an other one
#define VENTILATOR_TWO  D4    //should control ventilator two about D4 /maybe an other one
#define PORT 80               //port number

long frequenz;                //content the frequenz value
bool temp = 0;                //temporary value, one means ventilator on, zero means ventilator off

WiFiServer wifiServer(PORT);    //Wifi port

IRsend irsend(IR_SEND_PIN); //in library IRsend.h, send about IR_SEND_PIN infrared

//functions
void updateVentilator()        //updateted ventilator ON/OFF
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
    delay(100);
  }
  else if(frequenz==300 && temp==1)       //if requirements of vantilator off is true
  {
    temp = 0;
    digitalWrite(VENTILATOR_ONE,LOW); //write both ventilators off. Maybe it will be seperated later 
    digitalWrite(VENTILATOR_TWO,LOW);
    Serial.println("Ventilator: OFF");
    delay(100);
  }
  else                                //if anyone is going wrong
  {
    Serial.println("Temp or code have a false value. Temp isn't zero or one or code isn't 300.");
    Serial.println("Temp:"+temp);
  }
}

/*void ventilatorButtonOn()             //turn ventilator on
{
  temp = 1;
  digitalWrite(VENTILATOR_ONE,HIGH);  //write both ventilators on. Maybe it will be seperated later 
  digitalWrite(VENTILATOR_TWO,HIGH);
  Serial.println("Ventilator: ON\nButton pressed!\n");
  delay(700);
}

void ventilatorButtonOff()          //turn ventilator off
{
  temp = 0;
  digitalWrite(VENTILATOR_ONE,LOW); //write both ventilators off. Maybe it will be seperated later 
  digitalWrite(VENTILATOR_TWO,LOW);
  Serial.println("Ventilator: OFF\nButton pressed!\n");
  delay(700);
}*/

void updateTableLight()        //updated tablelight on/off/color etc.
{
 Serial.print("IR-Signal send: ");
 Serial.println(frequenz);
 irsend.sendNEC(frequenz,32); //send frequenz to light
}

void setup() 
{
  Serial.begin(115200);

  delay(1000);                            //why? Do we need delay?

  WiFi.begin(ssid, password);             //start wifi

  irsend.begin();                         //start infrared

  while (WiFi.status() != WL_CONNECTED)   //as long as wifi is unconected
  {
    delay(1000);
    Serial.println("Connecting..");
  }
  Serial.print("Connected to WiFi. IP:");
  Serial.println(WiFi.localIP());        //print own ip-adress

  wifiServer.begin();

  pinMode(VENTILATOR_ONE,OUTPUT);           //set VENTILATOR_ONE and VENTILATOR_TWO as an output
  pinMode(VENTILATOR_TWO,OUTPUT);
  pinMode(SWITCH_VENTILATOR,INPUT_PULLUP);  //set SWITCH_VENTILATUR as an input
}

//main loop
void loop()
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
  /*else if(digitalRead(SWITCH_VENTILATOR)==HIGH && temp==0)  //if requirements of vantilator on is true
  {
    //ventilatorButtonOn();
      temp = 1;
      digitalWrite(VENTILATOR_ONE,HIGH);  //write both ventilators on. Maybe it will be seperated later 
      digitalWrite(VENTILATOR_TWO,HIGH);
      Serial.println("Ventilator: ON\nButton pressed!\n");
      delay(700);
  }
  else if(digitalRead(SWITCH_VENTILATOR)==HIGH && temp==1)  //if requirements of vantilator off is true
  {
    //ventilatorButtonOff();
      temp = 0;
      digitalWrite(VENTILATOR_ONE,LOW); //write both ventilators off. Maybe it will be seperated later 
      digitalWrite(VENTILATOR_TWO,LOW);
      Serial.println("Ventilator: OFF\nButton pressed!\n");
      delay(700);
  }*/
}
