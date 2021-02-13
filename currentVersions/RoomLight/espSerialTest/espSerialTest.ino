String sendStrin = "hallo Luki";

void setup() {  
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Native USB only
  }
}

void loop() {
  Serial.print(sendStrin);
  delay(2000);
}
