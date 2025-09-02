#include <Servo.h>

Servo iServoMo;

int potpin = 0;  //babasang AnalogPIN sa pihitan (gitna)
int val;        //taga-salo ng value ni pihitan


void setup()
{
  Serial.begin(9600);
  iServoMo.attach(9); //(anong pin, stick sa 500, 2500 muna)
}

void loop()
{
	val = analogRead(potpin);  //kuha value ng pihitan
   
  	//i-mapa ang kontrol (current ni val, ,,orig FROM .. ,,magiging...)
  	val = map(val, 0, 1023, 180, 0); 
  	// kung baligtad ang kontrol, baligtarin ang mundo nya sa map
    delay(1000);
  	iServoMo.write(val);  // utos angel kontrol sa servo kumporme kay val
    
  	//eme nlang ito
  	
  	Serial.println(val);
}