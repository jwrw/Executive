// FiveTaskDemo
//
// Example use of the Executive library
//
// Run four tasks at a variety of intervals
//

#include "Executive.h";
#include "Arduino.h"

//The setup function is called once at startup of the sketch
void setup()
{
	Serial.begin(115200);
}

// The loop function is called in an endless loop
void loop()
{
	Exec.delay(10);
}
