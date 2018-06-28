/*
 * Executive.cpp
 *
 *  Created on: 26 Jun 2018
 *      Author: jim
 */

#include "Executive.h"

//------------------------------------------------------------------------------
struct taskEntry {
	long lastRun_ms;
	long interval_ms;
	void *doTask();
} tasks[DEFAULT_MAX_TASK_ENTRIES];

int nTasks;

//------------------------------------------------------------------------------
/**
 *
 */
Executive::Executive() {
}

//------------------------------------------------------------------------------
/**
 *
 * @param maxTasks
 */
Executive::Executive(int maxTasks) {
}

//------------------------------------------------------------------------------
/**
 *
 * @param timeToNext
 * @param interval_ms
 * @param doTask
 * @return
 */
int Executive::addTask(long timeToNext, long interval_ms, void doTask(void)) {

	return 0;
}

//------------------------------------------------------------------------------
/**
 *
 * @param interval_ms
 * @param doTask
 * @return An index to the task or -1 if task could not be added (no room left in task table)
 */
int Executive::addTask(long interval_ms, void doTask(void)) {
	if(nTasks< (sizeof tasks / sizeof tasks[0])) {
		tasks[nTasks].lastRun_ms = millis();
		tasks[nTasks].interval_ms = interval_ms;
		tasks[nTasks].doTask = doTask;

		return nTasks++;
	} else {
		return -1;	// table is full
	}

}

//------------------------------------------------------------------------------
/**
 *
 */
void Executive::yield(void) {
	delay(MIN_YIELD_TIME_MS);
}

//------------------------------------------------------------------------------
/**
 *
 * @param delay_ms
 */
void Executive::delay(long delay_ms) {
	if(delay_ms < MIN_YIELD_TIME_MS) {

	}

}


