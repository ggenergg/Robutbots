/*  
De-Pihit na motor, +5V sa power ni Servo at Kanang pin ni POTTY (Variable Resistor),
GND sa GND ni Servo at Kaliwang pin ni POTTY
A0(1,2,3,etc.) sa Gitna ni Potty
Orange pin ni Servo sa may tilde (~) ni Uno

*/

#include <Servo.h>  //pag wala nito, HU U si servo kay UNO

Servo iServoMo;  //Ayaw mautusan ni Servo, gusto ipasa ang utos (Instance) kay iServoMo

int potpin = 0;  //babasang AnalogPIN sa pihitan (gitna)
int val;        //taga-salo ng value ni pihitan


void setup()
{
  Serial.begin(9600);
  iServoMo.attach(9); //(anong pin, kung magwala... set PWMs 00, 2500 muna)
}

void loop()
{
	val = analogRead(potpin);  //kuha value ng pihitan
   
  	//i-mapa ang kontrol (current ni val, ,,orig FROM .. ,,magiging...)
  	val = map(val, 0, 1023, 180, 0); 
  	// kung baligtad ang kontrol, baligtarin ang mundo nya sa map
    
	
  	iServoMo.write(val);  // utos angel kontrol sa servo kumporme kay val
    
  	//eme nlang ito
  	Serial.println(val);
	delay(1000); // lakihan ang value kapag tinoyo si Uno, Servo, or both.
}
