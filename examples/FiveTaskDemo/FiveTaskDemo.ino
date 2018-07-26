// FiveTaskDemo
//
// Example use of the Executive library
//
// Run five tasks at a variety of intervals
//
// The output is best viewed in the Serial Plotter which shows the tasks being called.
// It shows a 'blip' each time an Exec.delay() in the loop() has completed
//
// Notice the tasks keep to a steady schedule but that only one task is running at
// any one time.  This means that tasks may be blocked until another task completes.
//
// Keep the tasks with the shorter time periods at the top of the list to give them
// first chance when you call Exec.delay() and greater likelihood to run closer to their
// schedule. Other than that, Exec tries to just work on a first come, first served basis
// but if one of the tasks takes a long time to complete then all the other tasks should
// get a go before that task is next run.  If a task's desired run time is passed
// whilst Exec is calling a long running task or outside of the call to Exec.delay()
// then there is no concept of 'catching up'.  The task just gets run (once) when Exec is
// able to run it and Exec will aim to run it again after its defined time interval.
//
// Exec will run all the tasks at least once that are ready for execution on each call to Exec.delay()
// As Exec is dependent on task execution times, the actual delay you get may be more than you requested;
// it will never be less than you requested.

//
// See also FiveTaskDemoInterrupted for a more complex example that does a 
// more sophisticated display

#include <Executive.h>

//The setup function is called once at startup of the sketch
void setup() {
	Serial.begin(115200);
	while (!Serial) {
		; // wait for serial port to connect. Needed for native USB port only
	}

	setupTasks();
}

// The loop function is called in an endless loop
void loop() {
	Exec.delay(4000);
	Serial.println("2, 4, 6, 8, 10");

	Exec.delay(7234);
	Serial.println("2, 4, 6, 8, 10");

}

// run every 25ms
void doTask1() {
	delay(10);
	Serial.println("1, 2, 4, 6, 8");
}

// run every 80ms
void doTask2() {
	delay(30);
	Serial.println("0, 3, 4, 6, 8");
}

// run every 1s
void doTask3() {
	delay(10);
	Serial.println("0, 2, 5, 6, 8");
}

// run every 5s
void doTask4() {
	delay(500);
	Serial.println("0, 2, 4, 7, 8");
}

// run every 5s
void doTask5() {
	delay(250);
	Serial.println("0, 2, 4, 6, 9");
}

// This is put after all the 'doTask' routines have been defined
// (alternatively you need to declare the 'doTask' routines near the top of the module
// which is one of those messy C++ things that Arduino normally hides from you)
void setupTasks() {
	Exec.addTask(25, doTask1);
	Exec.addTask(80, doTask2);
	Exec.addTask(1000, doTask3);
	Exec.addTask(5000, doTask4);
	Exec.addTask(5000, doTask5, 2500); // every 5s but start after a 2.5 s delay
}
