#include <Arduino.h>

const int ledPin = 26;

void setup(){
  Serial.print("Led demo");
  pinMode(ledPin, OUTPUT);
}


void loop(){
  digitalWrite(ledPin,HIGH);
  delay(1000);

  digitalWrite(ledPin,LOW);
  delay(1000);
}