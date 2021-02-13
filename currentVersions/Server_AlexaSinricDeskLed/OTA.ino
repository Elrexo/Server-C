/*
OTA

Authors:        Tim Keicher

Created:        01.12.2020

Description:    Contains and handles all functions for OTA install and upload.
*/

//***variables***
const char* ssid = STASSID;
const char* password = STAPSK;

//----- handle OTA setup settings -----
void setupOTA()
{
  OTApreference();

  //start wifi
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);
  USER_SERIAL.println();
  USER_SERIAL.print("Connecting to Wifi: ");
  USER_SERIAL.println(ssid);

  //waiting for Wifi connect
  while(WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    USER_SERIAL.print(".");
  }
  if(WiFiMulti.run() == WL_CONNECTED) {
    USER_SERIAL.println("");
    USER_SERIAL.print("WiFi connected. ");
    USER_SERIAL.print("IP address: ");
    USER_SERIAL.println(WiFi.localIP());
  }

  //start OTA
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    USER_SERIAL.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    USER_SERIAL.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    USER_SERIAL.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    USER_SERIAL.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      USER_SERIAL.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      USER_SERIAL.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      USER_SERIAL.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      USER_SERIAL.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      USER_SERIAL.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  TelnetStream2.begin();
  
  USER_SERIAL.println("Ready");
  USER_SERIAL.print("IP address: ");
  USER_SERIAL.println(WiFi.localIP());
}

//----- handles OTA preferences -----
void OTApreference()  //if you use spezific preferences change it here
{
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("ESP8266_DeskLED");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");
}
