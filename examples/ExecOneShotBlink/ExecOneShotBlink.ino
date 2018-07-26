/*
  ExecBlink
  Turns on an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the Uno and
  Leonardo, it is attached to digital pin 13. If you're unsure what
  pin the on-board LED is connected to on your Arduino model, check
  the documentation at http://www.arduino.cc

  This is a better version of the ExecBlink example and uses a
  one-shot task to turn off the LED.  It's overkill in this example but
  in a complex sketch it's a really good way to get something to happen in the future
  without having to check and manage it in your code.

  This example code is in the public domain.
 */

#include <Executive.h>

// Use names not numbers - then there is only one place you need to change it
const int PIN = 13;	// 'normal' Arduino
// const int PIN = 17;	// Sparkfun Pro Micro


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

void turnOff() {
	digitalWrite(PIN, HIGH);
}

// note that the turnOn() function references the turnOff() function
// To avoid Arduino compile errors it is placed after the turnOff() function definition.
// [There are other, more robust, ways to accomplish the same thing
//  (see topics such as 'declaring functions').
//  Arduino normally hides this messiness from you.]
void turnOn() {
	digitalWrite(PIN, LOW);
	Exec.addOneShotTask(turnOff, 1000);  // Turn off after a second
}

// Put this at the end of your sketch
void setupTasks() {
	Exec.addTask(2000, turnOn);	// Turn on every 2s
}
