
#include "IRremote.h"

#define array_1 5         // delay
#define switch_1 6        // light_switch
#define LIGHT_OFF 299286929
int receiver = 4;        // Signal Pin of IR receiver to Arduino Digital Pin 11 doesn't work use pin 4
long licht_an=701091283;  // Signal light on by 3772784863 (owne value) long 64 bit int 32 bit
int halter=0;             // hold relay on power

/*-----( Declare objects )-----*/
IRrecv irrecv(receiver);     // create instance of 'irrecv'
decode_results results;      // create instance of 'decode_results'


void setup()   /*----( SETUP: RUNS ONCE )----*/
{
  pinMode(array_1,OUTPUT);
  pinMode(switch_1,INPUT_PULLUP);
  Serial.begin(9600);
  Serial.println("IR Receiver Button Decode"); 
  irrecv.enableIRIn(); // Start the receiver
  results.value = 0;
}/*--(end setup )---*/


void loop(){
  if (irrecv.decode(&results)){   //check for IR_signal
    Serial.println(results.value);
    if(results.value == licht_an && halter == 0){   //activate Relay by IR_signal
      halter = 1;
      digitalWrite(array_1,HIGH);       //light on
      Serial.println("Relay: HIGH \nIR_signal\n");
      delay(700);
    }
    else if(results.value == licht_an && halter == 1){  //disactivate Relay by IR_signal
      halter = 0;
      digitalWrite(array_1,LOW);        //light off
      Serial.println("Relay: LOW\nIRsignal\n");
      delay(700);
    }
    else if(results.value == LIGHT_OFF){
      halter = 0;
      digitalWrite(array_1,LOW);
      Serial.println("Relay: LOW! Hardcoded off!");
      delay(700);
    }
    else{
      Serial.println("No function in value!\n");
    }
    irrecv.resume(); // receive the next value  
  }
  /*else if(digitalRead(switch_1) == HIGH && halter==0){  //activate Relay by button
      halter = 1;
      digitalWrite(array_1,HIGH);       //light on
      Serial.println("Relay: HIGH\nButton pressed!\n");
      delay(700);
  }
  else if(digitalRead(switch_1) == HIGH && halter == 1){  //disactivate Relay by button
      halter = 0;
      digitalWrite(array_1,LOW);        //light off
      Serial.println("Relay: LOW\nButton pressed!\n");
      delay(700);
  }*/
}
