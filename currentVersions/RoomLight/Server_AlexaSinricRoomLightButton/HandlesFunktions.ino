/*
HandlesFunctios

Authors:        Tim Keicher

Created:        1.0 vom 24.01.2021

Last Change:    04.02.2021

Description:    Contains and handles all functions to check humanity, 
                check temperature and send them back to the android app 
                after wifi request. Check button and control party 
                light too.
*/

//=== variables ===
int pinDHT11 = SENSOR;
SimpleDHT11 dht11;
byte temperature;
byte humidity; 
String sendMessageBack;

//=== set setup functions ===
void functionsSetup(){
  //ir-send 
  irsend.begin();

  //start wifi server on port "PORT"
  wifiServer.begin();

  //outputs/inputs
  pinMode(BUTTON,INPUT); 
}

//=== set loop functions ===
void functionsLoop(){
  checkWifiClient();
  checkButton();
}

//check wifi client
void checkWifiClient(){
  WiFiClient client = wifiServer.available();             //client says wether wifi is available or not

  if (client)
  {
    while (client.connected())                            //check for conecting with anything
    {
      while (client.available() > 0)                      //if arduino receive anything
      {
        String req = client.readStringUntil('X');         //read string until X   //try \r
        req.remove(0,2);                                  //remove first both values
        long frequency = req.toInt();                     //string to integer
        USER_SERIAL.println("=================================");
        USER_SERIAL.print("Received signal: ");
        USER_SERIAL.println(frequency);
        USER_SERIAL.print("Remote IP-Address: ");
        USER_SERIAL.println(client.remoteIP());
        
        if(frequency <= 1000){                            //less or equal 1000 was determined in smarthome-app
          checkTemperatureHumidity();
          sendMessageBack = String((int)temperature) + ":" + String((int)humidity);
          USER_SERIAL.print("Send Message: ");
          USER_SERIAL.println(sendMessageBack);
        }else{
          irsend.sendNEC(frequency,32);
          USER_SERIAL.println("Frequency was send!");
          if(frequency == onOffFrequency && !partyState){ //check and refresh on/off state
            partyState = true;
            setPowerStateOnServer(firstDeviceID,"ON");
            USER_SERIAL.println("partyState: true");
          }else if(frequency == onOffFrequency && partyState){
            partyState = false;
            setPowerStateOnServer(firstDeviceID,"OFF");
            USER_SERIAL.println("partyState: false");
          }
        }
        client.print(sendMessageBack);
      }
      delay(10);
    }
    client.stop();                                        //disconected client
    USER_SERIAL.println("Client disconnected");
  }
}

//read and handle button input
void checkButton(){    
  if(digitalRead(BUTTON) == HIGH){  //check wheter we need high or slow REEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
    USER_SERIAL.println("=================================");
    USER_SERIAL.println("Button pushed!");

    WiFiClient client;
    
    if (!client.connect(ROOM_LIGHT_IP, PORT)) {
        Serial.println("Connection to host failed");
        delay(1000);
        return;
    }
    USER_SERIAL.println("Connecting to IP: " + ROOM_LIGHT_IP + " on PORT: " + PORT + " successful!");
    client.print("11ONX");  //we use 11 because the room light esp cut off the first both characker of the string
    client.flush();
    client.stop();
    USER_SERIAL.println("Client disconnected!");  
    delay(700);
  }
}

//check temperature and humidity by sensor
void checkTemperatureHumidity(){
  // start working...
  USER_SERIAL.println("Sample DHT11...");
  
  // read with raw sample data.
  temperature = 0;
  humidity = 0;
  byte data[40] = {0};
  if (dht11.read(pinDHT11, &temperature, &humidity, data)) {
    USER_SERIAL.print("Read DHT11 failed");
    return;
  }
  
  USER_SERIAL.print("Sample RAW Bits: ");
  for (int i = 0; i < 40; i++) {
    USER_SERIAL.print((int)data[i]);
    if (i > 0 && ((i + 1) % 4) == 0) {
      USER_SERIAL.print(' ');
    }
  }
  USER_SERIAL.println("");
  
  USER_SERIAL.print("Sample OK: ");
  USER_SERIAL.print((int)temperature); USER_SERIAL.print(" *C, ");
  USER_SERIAL.print((int)humidity); USER_SERIAL.println(" %");
}

/*  if we need to change the host ipAddress in a String for connecting
String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ;
}*/
