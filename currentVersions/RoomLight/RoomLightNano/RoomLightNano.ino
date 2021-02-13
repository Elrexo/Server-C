/* ===========================================================================
 
Project:      SmarthomeMyRoom

==============================================================================
 
Filename:     RoomLightNano.ino
 
Authors:      Tim Keicher
 
Version:      1.0 vom 03.02.2021

Last update:  03.02.2021
 
Hardware:     Arduino nano, light dimming modul AC 230V 
 
Software:     Arduino 1.8.10
 
Function:     Dimming room light by dimming modul. Get dimming value by a 
              software serial interface from.
 
Buttonfunction:   - 
 
Moduls:       -
 
=============================================================================*/

//***includes***
#include <SoftwareSerial.h>
#include <RBDdimmer.h>

//***defines***
#define RX 7
#define TX 6          //here not in use just for transmitting
#define DIM_PIN  9    
#define ZERO_CROSS 2  //not changable

//***create objects***
SoftwareSerial mySerial(RX,TX);
dimmerLamp dimmer(DIM_PIN);

//***global variables***
boolean stringReady = false;
String dimmingValue;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Native USB only
  }  
  mySerial.begin(9600);
  while (!mySerial) {
    ; // wait for serial port to connect. Needed for Native USB only
  }
  dimmer.begin(NORMAL_MODE, ON);
}

void loop() {
  while(mySerial.available()){
    dimmingValue = mySerial.readString();
    stringReady = true;
  }
  if(stringReady){
    Serial.println("=================");
    Serial.println("Received String: " + dimmingValue);
    int intDimValue = dimmingValue.toInt();
    if(intDimValue >= 0 && intDimValue <= 100){
      dimmer.setPower(intDimValue);
      Serial.println(intDimValue);
    }
    stringReady = false;
  }
}
