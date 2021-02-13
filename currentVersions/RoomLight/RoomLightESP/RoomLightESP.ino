#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFi.h>
#include <TelnetStream2.h>  //open TelnetStream communication with esp and Putty

#define MySSID "WLAN-149267" // TODO: Change to your Wifi network SSID
#define MyWifiPassword "70169424204811969647" // TODO: Change to your Wifi network password
#define PORT 80               //port number
#define USER_SERIAL TelnetStream2

ESP8266WiFiMulti WiFiMulti;
WiFiServer wifiServer(PORT);    //Wifi port
WiFiClient client;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Native USB only
  }
  
  WiFiMulti.addAP(MySSID, MyWifiPassword);
  Serial.println();
  Serial.print("Connecting to Wifi: ");
  Serial.println(MySSID);

  // Waiting for Wifi connect
  while(WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  if(WiFiMulti.run() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("WiFi connected. ");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }

  //start wifi server on port "PORT"
  wifiServer.begin();

  TelnetStream2.begin();
}

void loop() {
  WiFiClient client = wifiServer.available();     //client says wether wifi is available or not

  if (client)
  {
    USER_SERIAL.println("Client connected");
    while (client.connected())                    //check for conecting with anything
    {
      while (client.available() > 0)              //if arduino receive anything
      {
        String req = client.readStringUntil('X'); //read string until X   //try \r
        req.remove(0,2);                          //remove first both values
        //muss in ein char array umgewandelt werden
        USER_SERIAL.println("Received from Smartphone: " + req);
        Serial.print(req);                        //write to arduino nano by serial communication
      }
      delay(10);
    }
    client.stop();                                //disconected client
    USER_SERIAL.println("Client disconnected");
  }
}
