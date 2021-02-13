/*
AlexaSinric

Authors:        Tim Keicher

Created:        03.12.2020

Last update:    03.02.2021

Description:    Contains and handles all functions for room light.
*/

//!!!WICHTIG: Nach permanenten Speicher schauen, da wenn der esp abstuertzt alle gespeicherten Alarme verloren gehen.
//            oder bei jedem Neustart einmal die Alarme abfragen.
// Sound (wecker) stoppen noch integrieren

//***variables***
String roomLightID = "5f47bd84d1e9084a708118ba";
String formattedDate;
String dayStamp;
String timeStamp;
unsigned long lastMillis = 0;
unsigned long currentMillis = 0;
unsigned int alarmControlTemp = 0;
unsigned int soundTimer = SOUND_TIMER;
char *splitedString;
char *h;
char *m;
char dimTemp = 0;
int sec = 0;
int Min = 0;
int hour = 0;
const char delimiter[] = ":";
boolean lightState = false;   //REEEEEEEEEEEEEEEEEE
boolean startDim = false;
boolean startSound = false;
boolean correctTime = true;   //true because it shall correct the time at the first esp start too
struct node *alarms = NULL;   //list who contains all alarms
struct node{                  //struct for a alarm node
        int hours;
        int minutes;
        struct node* next;
};

//handles RoomLight setup settings
void setupRoomLight(){
  //start wifi server on port "PORT"
  wifiServer.begin();

  setupNTPSettings();
}

//handle RoomLight loop settings
void loopRoomLight(){
  internTime();
  if(correctTime){    //check ntp server each 24h for time
    checkNTPServer();  
    changeTimeStampToInt();
    correctTime = false;
  }
  checkWiFiClient();
  checkAlarmClock(alarms);
  controlAlarm();
  //playAlarmSound(); //maby we send wake up code in controlAlarm
}

//handles NTP server setup settings
void setupNTPSettings(){
  // Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(3600);
}

//check for new input strings per TCP from android app 
void checkWiFiClient(){
  WiFiClient client = wifiServer.available();
  //read and handle wifi input
  if (client){
    USER_SERIAL.println("====================");
    USER_SERIAL.println("Client connected");
    while (client.connected()){
      while (client.available() > 0){
        String req = client.readStringUntil('X');
        req.remove(0,2);   
        USER_SERIAL.print("Received code: ");
        USER_SERIAL.println(req);
        char mala[31];
        req.toCharArray(mala,31);
        splitedString = strtok(mala,delimiter);
        
        //if ligt control manuel by app
        if(strcmp(splitedString,"dim") == 0){
          splitedString = strtok(NULL,delimiter);
          Serial.print(String(splitedString));  
          lightState = true;
        }
        
        //if turn light on
        else if(strcmp(splitedString,"ON") == 0 && !lightState){
          USER_SERIAL.println("Dimming 100%! Received app signal!");
          Serial.print("100");
          setPowerStateOnServer(roomLightID,"ON");
          lightState = true;
        }
        
        //if turn light off
        else if(strcmp(splitedString,"ON") == 0 && lightState){
          USER_SERIAL.println("Dimming 0%! Received app signal!");
          Serial.print("0");
          setPowerStateOnServer(roomLightID,"OFF");
          lightState = false;
        }
        else
          setAlarmClock();
      }
      delay(10);
    }
    client.stop();
    USER_SERIAL.println("Client disconnected");
  }
}

//trun on devices with an Alexa command
void turnOn(String deviceId) {
  if (deviceId == roomLightID){ // room light 
    USER_SERIAL.print("Turn on RoomLight! Device id: ");
    USER_SERIAL.println(deviceId);
    Serial.print("100");        // set light to 100%
  }else {
    USER_SERIAL.print("Turn on for unknown device id: ");
    USER_SERIAL.println(deviceId);
  }     
}

//trun off devices with an Alexa command
void turnOff(String deviceId) {
   if (deviceId == roomLightID){ // room light
     USER_SERIAL.print("Turn off RoomLight! Device ID: ");
     USER_SERIAL.println(deviceId);
     Serial.print("0");         // set light to 0%
   }else {
     USER_SERIAL.print("Turn off for unknown device id: ");
     USER_SERIAL.println(deviceId);    
  }
}

//set alarm setting if wifi receive an alarm time
void setAlarmClock(){
  if(strcmp(splitedString,"on") == 0){
    splitedString = strtok(NULL,delimiter);
    h = splitedString;
    splitedString = strtok(NULL,delimiter);
    m = splitedString;                     //rest of array "xx:Days"
    alarms = einfuegen(alarms, atoi(h), atoi(m));  //atoi change charArray to int
  }else if(strcmp(splitedString,"off") == 0){
    splitedString = strtok(NULL,delimiter);
    h = splitedString;
    splitedString = strtok(NULL,delimiter);
    m = splitedString;                     //rest of array "xx:Days"
    alarms = deleteElement(alarms, atoi(h), atoi(m));
  }else{
    USER_SERIAL.println("Error in setAlarmClock function!");
  }
}

//compare intern time and alarm clock setting //maybe more than one possible and weekday dependent
void checkAlarmClock(struct node* head){
  struct node *pt = head;
  while(pt != NULL){
    if(pt->hours == hour){
      if(pt->minutes == Min){
          startDim = true;
      }
    }
    pt = pt->next;
  }
}

//check current ntp server time and corecct intern time
void checkNTPServer(){
  while (!timeClient.update()){
    timeClient.forceUpdate();
  }
  // The formattedDate comes with the following format:
  // 2018-05-28T23:59:59Z
  // We need to extract date and time
  formattedDate = timeClient.getFormattedDate();
  USER_SERIAL.println(formattedDate);

  // Extract date
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  USER_SERIAL.print("DATE: ");
  USER_SERIAL.println(dayStamp);
  // Extract time
  timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 1);
  USER_SERIAL.print("HOUR: ");
  USER_SERIAL.println(timeStamp);
}

//intern time runs parallel to ntp and compare every hour
void internTime(){
 
  currentMillis = millis();
  
  /*if(currentMillis >= 0 || lastMillis < 0){ //&& wifi conected very importand !!! REEEEEEEEEEEEEEE check ntp server when overflow happens
    checkNTPServer(); //check ntp server for time and set hours, minutes, millis  REEEEEEEEEEEEEEE
    //what do to connect wifi again if it disconnected (research) [can we use if and do it in loo?] REEEEEEEEEEEEEEEEEE
  }*/
  if((currentMillis - lastMillis) >= 1000){ //increase sec every 1000 milli seconds
    sec++;
    lastMillis = currentMillis;
  }
  if(sec == 60){  //increase minutes every 60 seconds    
    Min++;
    sec = 0;
  }
  if(Min == 60){  //increase hours every 60 minutes
    hour++;
    Min = 0;
  }
  if(hour == 24){  //reset hours to zero after a day
    hour = 0;
    correctTime = true;
  }
}

//split timeStamp, change it from string to int and correct time
void changeTimeStampToInt(){
  //timeStampe format is 00:00:00 (h:m:s)
  //split them into 00 00 00 and change it seperatly to int

  //split timeStamp
  String hoursString = timeStamp.substring(0, 2);
  String minutesString = timeStamp.substring(3, 5);
  String secondsString = timeStamp.substring(6, 8);
  //change to int and correct time
  sec = secondsString.toInt();
  Min = minutesString.toInt();
  hour = hoursString.toInt();
  //USER_SERIAL.print("Seconds: ");USER_SERIAL.println(sec);
  //USER_SERIAL.print("Mintutes: ");USER_SERIAL.println(Min);
  //USER_SERIAL.print("Hours: ");USER_SERIAL.println(hour);
}

//add element to list
struct node *einfuegen(struct node *header, int hours, int minutes){  //---FUEGT NEUES ELEMENT ZWISCHEN ZWEI node EIN---
  struct node * newElement;                                           //NEUE ADRESSE DIE AUF DEN SPEICHER MIT WERT UND NEXT ZEIGT
  struct node * pt = header;                                          //PT ZEIGT ANFANGS AUF DIE LEERE LISTE; DANACH IMMER AUF DEN DER HOCHGEGEBEN WIRD   HEADER=HOCHGEGEBENER WERT                                                                                                                COPY HEADER FOR USING WHITHOUT DESTROINGY HEADER
  newElement = (struct node *)malloc(sizeof(*newElement));            //DYNAMISCHER SPEICHER
  if(newElement == NULL){                                             //CHECK FOR MEMORY CAPACITY
      printf("ERROR not enough storage!\n");
      exit(0);                                                        //GANZES PROGRAMM WIRD BEENDET
  }
  newElement->hours = hours;                                          //newElement DAS AUF DEN UNTERPUNKT WERT ZEIGT WIRD DER WERT EINGETRAGEN
  newElement->minutes = minutes;
  newElement->next = NULL;                                            //MACHT DEN ZEIGER ZUM NAECHSTEN ARRAY AUF NULL

  if(pt == NULL){                                                     //BEI LEERER LISTE
      return newElement;                                              //KOPIERT DIE ADRESSE DER LISTE AUF DIE ADRESSE AUF DIE DAS newElement ZEIGT / NEUES ELEMENT ZEIGT AUF node MIT WERT UND ADRESSE
  }
  while(pt->next != NULL){                                            //PRUEFT OB DER AKTUELLE ADRESSE AUF NULL IST
      pt = pt->next;                                                  //PT WIRD ZUR node; AUF DEM DIE ADRESSE VOM VORHERIGEM node ZEIGT
  }
  pt->next = newElement;                                              //ZEIGER DER NEUEN ADRESSE VOM VORHERIGEN node ZEIGT NUN AUF DEN NEUEN node
  return header;
}; 

//delete element from list
struct node *deleteElement(struct node *header, int deleteHours, int deleteMinutes){   //---LOESCHT EIN ELEMENT ZWISCHEN ZWEI node---
  struct node * pt = header;
  struct node * temp;

  if(pt == NULL){                                                                      //FALLS DIE LESTE LEER IST
      return header;
  }

  if(pt->hours == deleteHours && pt->minutes == deleteMinutes){                       //FALLS DIE ERSTE ZAHL GELOESCHT WERDEN SOLL
      temp = pt;
      header = pt->next;
      free(temp);
      return header;
  }

  while(pt->next != NULL){
      if(pt->next->hours == deleteHours && pt->next->minutes == deleteMinutes){
          temp = pt->next;
          pt->next = pt->next->next;
          free(temp);                                                                 //LOESCHT DAS ARRAY
          break;                                                                      //OHNE BREAK WERDEN ALLE GLEICHEN WERTE GELOESCHT
      }
      pt = pt->next;                                                                  //VERBINDET ARRAY DAVOR MIT DEM DANACH
  }
  return header;
};

//control time to increase light and play sound
void controlAlarm(){
  if(startDim){
    //soundTemp must defined one time in start sound timer (startDimming)
    if((currentMillis - alarmControlTemp) >= DIM_TIME_STAMP){ //becouse minus 1 by soundTimer every second
      alarmControlTemp = currentMillis;
      dimTemp++;
      Serial.print(String(dimTemp));
      if(dimTemp >= 100){
        dimTemp = 0;
        startDim = false;
        startSound = true;
      }
    }
  }else if(startSound){
    if((currentMillis - alarmControlTemp) >= SOUND_TIME_STAMP){
      alarmControlTemp = currentMillis;
      soundTimer--;
      if(soundTimer <= 0){
        startSound = false;
        soundTimer = SOUND_TIMER;
        //send wake up code for sound  
      }
    }
  }
}

//play sound for waking up
void playAlarmSound(){  //REEEEEEEEEEEEEEEEEEEEE
  if(startSound){
    //give signal for example  
  }else{
    //stop signal for epymple
  }
}

/*Ideen:
 * abfrage ob Licht an ist (wieviel %)
 * 
 */
