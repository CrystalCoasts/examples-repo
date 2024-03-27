#include <Arduino.h>

const int ledPin = 26;  // The GPIO pin for the LED

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin ledPin as an output.
  Serial.begin(115200); // Set the baud rate to 115200

  while (!Serial) {
    ; // Wait for serial port to connect. It is needed for some boards.
  }

  Serial.println("Led test");
  pinMode(ledPin, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(ledPin, HIGH); 
  Serial.print("HIGH, 500");  // turn the LED on (HIGH is the voltage level)
  delay(500);                       // wait for a second
  digitalWrite(ledPin, LOW);  
  Serial.print("LOW, 1000");   // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
}