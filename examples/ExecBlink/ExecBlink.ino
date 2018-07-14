/*
  ExecBlink
  Turns on an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the Uno and
  Leonardo, it is attached to digital pin 13. If you're unsure what
  pin the on-board LED is connected to on your Arduino model, check
  the documentation at http://www.arduino.cc

  This example code is in the public domain.
 */

#include <Executive.h>

// Use names not numbers - then there is only one place you need to change it
const int PIN = 13;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialise digital pin for LED as an output.
  pinMode(PIN, OUTPUT);

  // Define our schedule
  setupTasks();
}

// the loop function runs over and over again forever
void loop() {
	Exec.loop();	// Hand control over to Exec (never returns)
}

void turnOn() {
	digitalWrite(PIN, LOW);
}

void turnOff() {
	digitalWrite(PIN, HIGH);
}

// Put this at the end of your sketch
void setupTasks() {
	Exec.addTask(2000, turnOn);	// Turn on every 2s
	Exec.addTask(2000, turnOff, 1000);  // Turn off every 2s, but start 1s later
}
