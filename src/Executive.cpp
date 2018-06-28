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
} *tasks;

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
	tasks = malloc(sizeof *tasks * maxTasks);
}

//------------------------------------------------------------------------------
/**
 */
Executive::~Executive() {
	free(tasks);
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
int Executive::addTask(long timeToNext_ms, long interval_ms, void doTask(void)) {
	int taskNo = addTask(interval_ms, doTask);
	if(taskNo<0) return taskNo;

	tasks[taskNo].lastRun_ms = (timeToNext_ms>=interval_ms) ? millis() : millis() - (interval_ms - timeToNext_ms);
	return 0;
}

//------------------------------------------------------------------------------
/**
 *
 * @param interval_ms Interval between successive runs of the task
 * @param doTask The routine called to do the task
 * @return An index to the task or -1 if task could not be added (no room left in task table)
 */
int Executive::addTask(long interval_ms, void doTask(void)) {
	// TODO - tasks should really be sorted with shortest interval first
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


