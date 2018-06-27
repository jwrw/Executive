/*
 * Executive.cpp
 *
 *  Created on: 26 Jun 2018
 *      Author: jim
 */

#include "executive.h"

//------------------------------------------------------------------------------
struct taskEntry {
	unsigned long lastRun_ms;
	unsigned long interval_ms;
	void *doTask();
} tasks[DEFAULT_MAX_TASK_ENTRIES];

int nTasks = 0;

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
 * @return
 */
int Executive::addTask(long interval_ms, void doTask(void)) {
	if(nTasks< (sizeof tasks / sizeof tasks[0])) {
	tasks[nTasks].lastRun_ms = millis();
	tasks[nTasks].interval_ms = interval_ms;
	tasks[nTasks].doTask = doTask;
	nTasks++;
	}
	return nTasks;
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
	unsigned long sinceLast_ms;
	unsigned long next_ms;
	unsigned long nearestNext_ms;
	int nearestNextTask;

	for(int i=0; i<nTasks; i++) {
		sinceLast_ms = millis() - tasks[i].lastRun_ms;
		if(sinceLast_ms>=tasks[i].interval_ms) {
			tasks[i].lastRun_ms = millis();
			tasks[i].doTask();
		} else {
			next_ms = tasks[i].lastRun_ms + tasks[i].interval_ms;
			if(next_ms-millis() < nearestNext_ms-millis()) {
				nearestNext_ms = next_ms;
				nearestNextTask = i;
			}
		}



	}

}


