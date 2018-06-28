/*
 * Executive.cpp
 *
 *  Created on: 26 Jun 2018
 *      Author: jim
 */

#include "executive.h"

//------------------------------------------------------------------------------
struct TaskEntry {
	unsigned long lastRun_ms;
	unsigned long interval_ms;
	void *doTask();
}*tasks;

int nTasks = 0;

//------------------------------------------------------------------------------
/**
 *
 */
Executive::Executive() {
	Executive(DEFAULT_MAX_TASK_ENTRIES);
}

//------------------------------------------------------------------------------
/**
 *
 * @param maxTasks
 */
Executive::Executive(int maxTasks) {
	tasks = new struct TaskEntry[maxTasks];
}

//------------------------------------------------------------------------------
/**
 */
Executive::~Executive() {
	delete [] tasks;
}

//------------------------------------------------------------------------------
/**
 *
 * @param timeToNext_ms Number of ms until first run of the task to do.  Must be less
 * than interval_ms otherwise taken as interval_ms
 * @param interval_ms Interval between successive runs of the task
 * @param doTask The routine called to do the task
 * @return An index to the task or -1 if task could not be added (no room left in task table)
 */
int Executive::addTask(unsigned long interval_ms, void &doTask(void),
		unsigned long timeToNext_ms = 0) {
	if (nTasks < (sizeof tasks / sizeof tasks[0])) {
		tasks[nTasks].interval_ms = interval_ms;
		tasks[nTasks].doTask = &doTask;
		tasks[nTasks].lastRun_ms =
				(timeToNext_ms >= interval_ms) ?
						millis() : millis() - (interval_ms - timeToNext_ms);
		return nTasks++;
	} else {
		return -1;
	}
}

//------------------------------------------------------------------------------
/**
 *
 */
void Executive::yield(void) {
	Executive::delay(MIN_YIELD_TIME_MS);
}

//------------------------------------------------------------------------------
/**
 *
 * @param delay_ms
 */
void Executive::delay(long delay_ms) {
	volatile unsigned long start_ms = millis();

	bool taskWasRun;
	do {
		taskWasRun = false;
		unsigned long nearestNext_ms = millis() + tasks[0].interval_ms;

		for (int i = 0; i < nTasks; i++) {
			unsigned long sinceLast_ms = millis() - tasks[i].lastRun_ms;
			if (sinceLast_ms >= tasks[i].interval_ms) {
				tasks[i].lastRun_ms = millis();
				tasks[i].doTask();
				taskWasRun = true;
				yield();
				break;
			} else {
				unsigned long next_ms = tasks[i].lastRun_ms
						+ tasks[i].interval_ms;
				unsigned long current_ms = millis();
				if (next_ms - current_ms < nearestNext_ms - current_ms) {
					nearestNext_ms = next_ms;
				}
			}
		}
		unsigned long timeToNext_ms = nearestNext_ms - millis();
		unsigned long remaining_ms = millis() - start_ms;
		if (timeToNext_ms < remaining_ms) {
			delay(timeToNext_ms);
		} else {
			delay(remaining_ms);
			break;
		}
	} while (taskWasRun);

}

