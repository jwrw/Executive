// FiveTaskDemo
//
// Example use of the Executive library
//
// Run five tasks at a variety of intervals
//
// The output is best viewed in the Arduino IDE's Serial Plotter which shows the tasks
// running (they each use a fixed delay() in this example). The trace also shows a
// 'blip' after each time the Exec.delay() is called.
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

#include <Executive.h>
#include <TimerOne.h>

volatile bool task1, task2, task3, task4, task5;

//The setup function is called once at startup of the sketch
void setup() {
	Serial.begin(115200);
	while (!Serial) {
		; // wait for serial port to connect. Needed for native USB port only
	}

	setupTasks();
	setupTimer1();
}

// You can hand over to Exec in big chunks of time
// or in much smaller ones - it depends if you
// need to do anything else in your loop() function but
// in many cases you won't.
void loop() {
	// Hand control to the executive for 2 seconds
	Exec.delay(2000);
	Serial.println("2,4,6,8,10");	// draw a blip on Serial Plotter

	// The delay time doesn't have to relate to the task timing intervals in
	// any way - you can call the Exec with any delay time
	Exec.delay(1234);
	Serial.println("2,4,6,8,10");	// draw a blip on Serial Plotter

	// The delay times can be much larger than your task intervals or can be much smaller
	// Exec will do its best to keep the schedule
	for(int i=0; i<10; i++) {
		Exec.delay(100);
		Serial.println("2,4,6,8,10");	// draw a blip on Serial Plotter
	}
}

// run every 25ms
void doTask1() {
	task1 = true;
	delay(10);
	task1 = false;
}

// run every 80ms
void doTask2() {
	task2 = true;
	delay(30);
	task2 = false;
}

// run every 1s
void doTask3() {
	task3 = true;
	delay(100);
	task3 = false;
}

// run every 5s
void doTask4() {
	task4 = true;
	delay(500);
	task4 = false;
}

// run every 5s
void doTask5() {
	task5 = true;
	delay(250);
	task5 = false;
}

// This is an 'Interrupt Service Routine' (ISR) that gets called
// each time the Timer1 timer expires.
// Please note, ISRs should be very short and using Serial inside one
// is best avoided.  We've done it here to keep the code simple.
void doOutput() {
	noInterrupts();
	bool t1 = task1;
	bool t2 = task2;
	bool t3 = task3;
	bool t4 = task4;
	bool t5 = task5;
	interrupts();

	Serial.print((t1) ? 1 : 0);
	Serial.print(",");
	Serial.print((t2) ? 3 : 2);
	Serial.print(",");
	Serial.print((t3) ? 5 : 4);
	Serial.print(",");
	Serial.print((t4) ? 7 : 6);
	Serial.print(",");
	Serial.println((t5) ? 9 : 8);
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

void setupTimer1() {
	  Timer1.initialize(10000);
	  Timer1.attachInterrupt(doOutput);
}
