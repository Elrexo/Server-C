/*
 Version 0.1 - August 24 2020
 Tim Keicher
*/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h> //  https://github.com/kakopappa/sinric/wiki/How-to-add-dependency-libraries
#include <ArduinoJson.h>      // https://github.com/kakopappa/sinric/wiki/How-to-add-dependency-libraries
#include <StreamString.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
WiFiClient client;

///////////WiFi Setup/////////////

#define MyApiKey "aacadef6-c35c-4ef0-85fc-b00d5a63acc1 "  // TODO: Change to your sinric API Key. Your API Key is displayed on sinric.com dashboard
#define MySSID "WLAN-149267"                              // TODO: Change to your Wifi network SSID
#define MyWifiPassword "70169424204811969647"             // TODO: Change to your Wifi network password

/////////////////////////////////

#define HEARTBEAT_INTERVAL 300000 // 5 Minutes
#define IR_SEND_PIN 2             //send infared about Pin D2
#define PORT 80                   //port number

#define maxVolume 30
#define minVolume 0

//frequencies
#define ON_OFF  3772793023
#define VOLUME_UP 3772833823
#define VOLUME_DOWN 3772829743
#define CHANNEL_UP 3772795063
#define CHANNEL_DOWN 3772778743
#define MUTE_UNMUTE 3772837903
#define SOURCE 3772809343
#define ENTER 3772782313
#define CROSS_UP 3772778233
#define CROSS_DOWN 3772810873
#define RETURN 3772783333
#define PRE_CH 3772827703
#define INFO 3772839943
#define TOOLS 3772830253
#define EXIT 3772822603
#define ROOM_LIGHT_ON 16111111  //if light already on, than it will be turning off too
#define ROOM_LIGHT_OFF 16555555 //hardcoded off just turn light off

long numbers[10] = {3772811383, //content the frequencies to send the numbers 0 to 9
                    3772784863,
                    3772817503,
                    3772801183,
                    3772780783,
                    3772813423,
                    3772797103,
                    3772788943,
                    3772821583,
                    3772805263};

//devices id:
String TVID = "5f458254ff09dc1fbf8392be";
String roomLightID = "5f47bd84d1e9084a708118ba";
String PCID = "5f6864dfd1e9084a708578de";

uint64_t heartbeatTimestamp = 0;
bool isConnected = false;
long frequenz;
bool tvState = false; // TV state for on/off
bool muteState = false;
unsigned char volumeState = 0;
signed char tvInputState = 1; // input at tv (HDMI, TV, AV, ...)
const char* host = "192.168.2.118"; //host ID for sending to pc

WiFiServer wifiServer(PORT);    //Wifi port
IRsend irsend(IR_SEND_PIN);     //in library IRsend.h, send about IR_SEND_PIN infrared

void setPowerStateOnServer(String deviceId, String value);

// deviceId is the ID assgined to your smart-home-device in sinric.com dashboard. Copy it from dashboard and paste it here

//change app received string into intager
int StringToInt(String req)
{
  String a_string = req.substring(req.length()-4, req.length());
  String b_string = req.substring(0, req.length()-4);
  long a_zahl = a_string.toInt();Serial.println(a_zahl);
  long b_zahl = b_string.toInt();Serial.println(b_zahl);
  long ergebnis = b_zahl*10000 + a_zahl;

  return ergebnis;
}

//send command to pc to change it into power saving mode
void sendPcPowerSavingMode(){
  WiFiClient client;
  
  if(!client.connect(host,PORT)){
    Serial.println("Connection to host failed");
    delay(1000);
    return;
  }
  Serial.println("Connected to server successful!");

  client.print("powerSavingMode");
  
  Serial.println("Disconnection...");
  client.stop();
}

//check for new input strings per TCP from android app 
void checkWiFiClient()
{
  WiFiClient client = wifiServer.available();

  if (client) 
  {
    while (client.connected()) 
    {
      while (client.available() > 0) 
      {
        String req = client.readStringUntil('X'); //try \r  REEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
        req.remove(0,2);
        if(req == "powerSavingMode"){
          Serial.println("Signal from Android!");
          sendPcPowerSavingMode();
        }
        else{
          frequenz = StringToInt(req);
          Serial.print("IR-Signal send: ");
          Serial.println(frequenz);
          
          if(frequenz > -1500000000){
            irsend.sendSAMSUNG(frequenz,32);
            Serial.println("Send Samsung");
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
            Serial.println("Send NEC");
          }
        }
        delay(10);
      }
    }
    client.stop();
    Serial.println("Client disconnected");

  }
}

//turn on devices with an Alexa command
void turnOn(String deviceId) {
  if (deviceId == TVID) // TV device
  {  
    if(!tvState){
      Serial.print("Turn on TV! Device id: ");
      Serial.println(deviceId);
      irsend.sendSAMSUNG(ON_OFF,32);
      tvState = true;
    }
    else{
      Serial.println("TV is already on.");
    }
  } 
  else if(deviceId == roomLightID){
    Serial.print("Turn on room light! Device id: ");
    Serial.println(deviceId);
    irsend.sendNEC(ROOM_LIGHT_ON,32);
  }
  else if(deviceId == PCID){
    sendPcPowerSavingMode();
  }
  else {
    Serial.print("Turn on for unknown device id: ");
    //Serial.println(deviceId);
  }     
}

//turn off devices with an Alexa command
void turnOff(String deviceId) {
   if (deviceId == TVID) // TV device
   {  
    if(tvState){
      Serial.print("Turn off TV! Device ID: ");
      Serial.println(deviceId);
      irsend.sendSAMSUNG(ON_OFF,32);
      tvState = false;
    }
    else{
      Serial.println("TV is already off.");
    }
   }
   else if(deviceId == roomLightID){
    Serial.print("Turn off room light! Device id: ");
    Serial.println(deviceId);
    irsend.sendNEC(ROOM_LIGHT_OFF,32);
  }
  else if(deviceId == PCID){
    sendPcPowerSavingMode();
  }
  else {
     Serial.print("Turn off for unknown device id: ");
     Serial.println(deviceId);    
  }
}

//send a sequence of numbers
void sendSequenceOfNumbers(char getLength, int getNumber){
  char numberArray[getLength];
  Serial.println(getNumber);
  sprintf(numberArray, "%d", getNumber);  //print getLength into a char array to get individual values
  
  for(unsigned char i=0; i < getLength; i++){
    irsend.sendSAMSUNG(numbers[numberArray[i]-48],32);  //use -48, because numberarrays content the asci-code numbers
    //Serial.println(numberArray[i]-48);
    delay(300);
  }
  Serial.println("Number: " + getNumber);
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      isConnected = false;    
      Serial.printf("[WSc] Webservice disconnected from sinric.com!\n");
      break;
    case WStype_CONNECTED: {
      isConnected = true;
      Serial.printf("[WSc] Service connected to sinric.com at url: %s\n", payload);
      Serial.printf("Waiting for commands from sinric.com ...\n");        
      }
      break;
    case WStype_TEXT: {
        Serial.printf("[WSc] get text: %s\n", payload);
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
            Serial.println("Volum up by to times.");
          }
          else if(value == "-10" && volumeState >= minVolume){  //reduced volume by 2 if volumeState not minimal
            irsend.sendSAMSUNG(VOLUME_DOWN,32);
            delay(300);
            irsend.sendSAMSUNG(VOLUME_DOWN,32);
            volumeState -= 2;
            Serial.println("Volum down by to times.");
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
            Serial.print("Volume increase by ");  //why does Serial.println("..." + differentVolume + "..."); not working? REEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
            Serial.print(differentVolume);
            Serial.println(" times.");
          }
          else if(volumeState > volumeNumber){
            for(signed char i=0; i > differentVolume; i--){
              irsend.sendSAMSUNG(VOLUME_DOWN,32);
              delay(300);
            }
            Serial.print("Volume reduced by ");
            Serial.print(differentVolume);
            Serial.println(" times.");
          }
          else{
            Serial.println("Volume already at this volume");
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
            Serial.println("Channel up!");
          }
          else{
            irsend.sendSAMSUNG(CHANNEL_DOWN,32);
            Serial.println("Channel down!");
          }
        }

        //+++++++++++++++++++++ change channel +++++++++++++++++++++++
        else if(action == "ChangeChannel"){
          String value = json["value"]["channel"]["number"];
          Serial.println(value);
          
          if(value != NULL){
            Serial.println("Number was received");
            char lengthNumber = value.length();
            int valueNumber = value.toInt();
            sendSequenceOfNumbers(lengthNumber,valueNumber);
          }
          
          else{
            value == json["value"]["channelMetadata"]["name"];
            Serial.println("Metadata was received");
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
          Serial.println(value);
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
              Serial.println("Input swtich up");
            }
            else if(tvInputState > inputNumber){
              for(signed char i=0; i > differentInputState; i--){
                irsend.sendSAMSUNG(CROSS_UP,32);
                delay(500);
              }
              Serial.println("Input switch down");
            }
            else{
              Serial.println("Input already on this input.");
            }
            irsend.sendSAMSUNG(ENTER,32);
            tvInputState = inputNumber;
          }
          
          else{
            Serial.println("Input number to high");
          }
        }
        
        //untility button
        else if(action == "Play"){
          irsend.sendSAMSUNG(ENTER,32);
          Serial.println("enter pressed");
        }
        else if(action == "Rewind"){
          irsend.sendSAMSUNG(PRE_CH,32);
          Serial.println("pre-ch pressed");
        }
        else if(action == "Previous"){
          irsend.sendSAMSUNG(RETURN,32);
          Serial.println("return pressed");
        }
        else if(action == "FastForward"){
          irsend.sendSAMSUNG(INFO,32);
          Serial.println("info pressed");
        }
        else if(action == "Stop"){
          Serial.println("sleep timer 30 in min");
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
          
        //there are no pissibilities to get information, set return button, send up, down, right, left command, activated sleep timer, set pre-ch (prev channel)
        /*
         * rewind
         * fast foward
         * next
         * previous
         * pause
         * stop
         * play
         * 
         * solutions:
         * - pre-ch:  action: changeChannel value: anything       //anything menas to looking for a situable command
         * - infotmation  action: changeChannel value: anything
         * - sleep timer  action: changeChannel value: anything
         * - return:  action: previous
         */
         
        else if (action == "test") {
            Serial.println("[WSc] received test command from sinric.com");
        }
      }
      break;
    case WStype_BIN:
      Serial.printf("[WSc] get binary length: %u\n", length);
      break;
  }
}

void setup() {
  Serial.begin(115200);
  
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

  // server address, port and URL
  webSocket.begin("iot.sinric.com", 80, "/");

  // event handler
  webSocket.onEvent(webSocketEvent);
  webSocket.setAuthorization("apikey", MyApiKey);
  
  // try again every 5000ms if connection has failed
  webSocket.setReconnectInterval(5000);   // If you see 'class WebSocketsClient' has no member named 'setReconnectInterval' error update arduinoWebSockets

  //ir-send 
  irsend.begin();

  //start wifi server on port "PORT"
  wifiServer.begin();
}

void loop() {
  webSocket.loop();
  
  if(isConnected) {
      uint64_t now = millis();
      
      // Send heartbeat in order to avoid disconnections during ISP resetting IPs over night. Thanks @MacSass
      if((now - heartbeatTimestamp) > HEARTBEAT_INTERVAL) {
          heartbeatTimestamp = now;
          webSocket.sendTXT("H");          
      }
  }   
  checkWiFiClient();
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
