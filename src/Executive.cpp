/*
 * Executive.cpp
 *
 *  Created on: 26 Jun 2018
 *      Author: jim
 */

#include "Executive.h"

//------------------------------------------------------------------------------
struct TaskEntry {
	unsigned long lastRun_ms;
	unsigned long interval_ms;
	void (*doTask)(void);
	bool oneShot;
	bool enabled;
}*tasks;

int _maxTasks = 0;

//------------------------------------------------------------------------------
/**
 *
 * @param maxTasks
 */
Executive::Executive(int maxTasks) {
	_maxTasks = maxTasks;
	tasks = new TaskEntry[maxTasks];
	for(int slot=0; slot<_maxTasks; slot++) {
		tasks[slot].doTask = nullptr;
		tasks[slot].enabled = false;
	}
}

//------------------------------------------------------------------------------
/**
 */
Executive::~Executive() {
	delete[] tasks;
}

//------------------------------------------------------------------------------
/**
 * Add a new task to be performed at a regular interval
 *
 * @param timeToNext_ms Number of ms until first run of the task to do.  Must be less
 * than interval_ms otherwise taken as interval_ms
 * @param interval_ms Interval between successive runs of the task
 * @param doTask The routine called to do the task
 * @return An index to the task or -1 if task could not be added (no room left in task table)
 */
int Executive::addTask(unsigned long interval_ms, void (*doTask)(void),
		unsigned long timeToNext_ms) {
	int slot;
	for(slot=0; slot<_maxTasks; slot++) {
		if(tasks[slot].doTask==nullptr) break;
	}

	if (slot < _maxTasks) {
		tasks[slot].interval_ms = interval_ms;
		tasks[slot].doTask = doTask;
		tasks[slot].lastRun_ms =
				(timeToNext_ms >= interval_ms) ?
						millis() : millis() - (interval_ms - timeToNext_ms);
		tasks[slot].enabled = true;
		tasks[slot].oneShot = false;
		return slot;
	} else {
		return -1;
	}
}

//------------------------------------------------------------------------------
/**
 * Add a new task to be performed just once
 *
 * @param timeToNext_ms Number of ms until first run of the task to do.
 * @param doTask The routine called to do the task
 * @return An index to the task or -1 if task could not be added (no room left in task table)
 */
int Executive::addOneShotTask(void (*doTask)(void), unsigned long timeToNext_ms) {
	int slot;
	for(slot=0; slot<_maxTasks; slot++) {
		if(tasks[slot].doTask==nullptr) break;
	}

	if (slot < _maxTasks) {
		tasks[slot].interval_ms = timeToNext_ms;
		tasks[slot].doTask = doTask;
		tasks[slot].lastRun_ms = millis();
		tasks[slot].enabled = true;
		tasks[slot].oneShot = true;
		return slot;
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
 * Hand over control to the Executive.  Run tasks to the schedule but never
 * return.
 */
void Executive::loop() {
	Executive::delay(DELAY_FOREVER);
}

//------------------------------------------------------------------------------
/**
 * Run tasks in the schedule that need to be run in the next delay_ms milliseconds.
 * Return after at least delay_ms.  I.e. the function will call the standard delay()
 * function to use up any 'spare' time after any relevant tasks have been run.
 *
 * Tasks are not run in parallel, so if a task become due to be run during the execution of
 * another task it will have to wait until the current task completes before it
 * can be started.
 *
 * When considering which task to run, the task with the earliest desired start time will
 * always take priority.  Where two tasks have the same desired start time, the one earliest in the list
 * will take precedence.  You should aim to place the tasks with shortest intervals near the
 * top of the list as they stand the best chance of running to their desired interval.
 *
 * Immediately before running a task, the time at it is started is recorded.  The desired time of the
 * next execution will be based on the actual time at which the task is started and the task interval.
 * There is no account taken of whether the task was late in starting.  This effectively means that the
 * task interval is a minimum setting for the amount of time between successive calls to the
 * task function.  In the best case, where the delay() function has control for most of the time
 * and other scheduled functions don't interfere, the interval time will be honoured and the task will
 * operate at the requested interval.  If the sketch spends a lot of its time outside the delay() function
 * or if there are other tasks that individually or in combination might take longer then the interval,
 * then Executive won't be able to keep to the schedule and will work on a 'best endeavours' basis.
 *
 * One-shot tasks are treated equally to the interval tasks, except that they are removed from the table
 * immediately before this scheduler runs their doTask() function.
 *
 * If a task is started and runs beyond the delay_ms window then this Executive::delay()
 * function will itself return as soon as that task completes. In this case no further tasks
 * will be started (even if they were due to run in that delay_ms window).
 *
 * If, when delay() is called, there is a backlog of tasks that are due to be run, then
 * the tasks get run in the order of their desired run time.  Again, if the end of the delay_ms
 * is reached, then the delay() function will return as soon as the current task finishes,
 * even if there are other back-logged or due to run tasks.  This behaviour does ensure
 * that if there is a backlog of tasks then at least one of these will get run for
 * any call to delay().
 *
 * If there are no tasks due to run in the requested delay_ms window then the Executive:delay()
 * function just behaves like a standard delay() function.
 *
 * @param delay_ms The minimum amount of time to remain in this scheduling function.  Return
 * after this time has passed and a soon as any currently executing task returns.  If the special value
 * DELAY_FOREVER is used then the control is passed entirely to Executive
 * and is never returned back to the calling sketch.
 *
 * @see Executive::loop()
 *
 */
void Executive::delay(unsigned long delay_ms) {
	unsigned long start_ms = millis();

	do {
		unsigned long current_ms = millis();// snapshot to treat all tasks equally
		signed long bestDelta_ms = 0x7fffffffL;
		int topTask = -1;
		for (int i = 0; i < _maxTasks; i++) {
			if(tasks[i].doTask != nullptr && tasks[i].enabled) {
				unsigned long nextRun_ms = tasks[i].lastRun_ms + tasks[i].interval_ms;
				signed long delta_ms = (signed long) (nextRun_ms - current_ms);
				if (delta_ms < bestDelta_ms) {
					bestDelta_ms = delta_ms;
					topTask = i;
				}
			}
		}

		if(topTask==-1) {
			// no tasks in task list - so behave as if nothing to do for a short time
			bestDelta_ms = MIN_YIELD_TIME_MS;
		}

		if(bestDelta_ms<=0) {
			// task is due or overdue - start immediately
			tasks[topTask].lastRun_ms = millis();
			void (*taskFn)(void) = tasks[topTask].doTask;
			if(tasks[topTask].oneShot) tasks[topTask].doTask = nullptr;
			taskFn();
			::yield();
			// Is there no more time left to do more?
			if(((signed long)(start_ms + delay_ms - millis())) <= 0) {
				break;
			}
		} else {
			// next task is in the future 
			signed long deltaToEnd_ms = (signed long)(start_ms + delay_ms - millis());
			if(delay_ms != DELAY_FOREVER && bestDelta_ms >= deltaToEnd_ms) {
				// next task to run won't fall in the current delay_ms window
				if(deltaToEnd_ms>=0) {
					::delay(deltaToEnd_ms);
				}
				break;		// leave the function
			} else {
				// wait until time to run the next task
				::delay(bestDelta_ms);
				// we could run the task here - but instead loop around
				// and re-do the task list check in case anything
				// has changed
			}
		}
	} while (true);
}
//------------------------------------------------------------------------------
/**
 * Enable the specified task and make eligible to run immediately
 *
 * Task can be one-shot or continuous but must have been set up previously.
 * @param taskNo Number of the task that is to be enabled.  The task number is returned from addTask() and addOnShotTask().
 * @return The task no or -1 if the taskNo was not a valid task
 */
int Executive::enableTask(int taskNo) {
	if(taskNo>=_maxTasks || taskNo<0 || tasks[taskNo].doTask==nullptr) return -1;

	tasks[taskNo].lastRun_ms = millis() - tasks[taskNo].interval_ms;
	tasks[taskNo].enabled = true;
	return taskNo;
}
//------------------------------------------------------------------------------
/**
 * Disable the specified task from being scheduled (run)
 *
 * Task can be one-shot or continuous but must have been set up previously.  If the task is one-shot then
 * it must be disabled before it first runs or this routine will return -1. One-shot tasks are deleted from the
 * table immediately before they are run.
 * @param taskNo Number of the task that is to be disabled.  The task number is returned from addTask() and addOnShotTask().
 * @return The task no or -1 if the taskNo was not a valid task
 */
int disableTask(int taskNo) {
	if(taskNo>=_maxTasks || taskNo<0 || tasks[taskNo].doTask==nullptr) return -1;
	tasks[taskNo].enabled = false;
	return taskNo;
}
//------------------------------------------------------------------------------
/**
 * Modify the interval at which a task runs or the time until a one-shot is executed
 *
 * For continuously running tasks, only the interval is changed, so if it is shortened,
 * the task may immediately become eligible to run (during the next yield() or delay()).
 *
 * For one-shot tasks, the 'timer' is reset so it will become eligible after the new
 * interval_ms period.
 *
 * @param taskNo
 * @param interval_ms
 * @return The task no or -1 if the taskNo was not a valid task
 */
int Executive::modifyTaskInterval(int taskNo, unsigned long interval_ms) {
	if(taskNo>=_maxTasks || taskNo<0 || tasks[taskNo].doTask==nullptr) return -1;
	tasks[taskNo].interval_ms = interval_ms;
	if(tasks[taskNo].oneShot) tasks[taskNo].lastRun_ms = millis();
	return taskNo;
}

//------------------------------------------------------------------------------
/**
 * Change the time at which the task will next run
 *
 * Note that for one-shot tasks, this behaves the same as modifyTaskInterval()
 * and the internal 'timer' is effectively reset.
 *
 * For continuously running tasks, the timeToNext_ms should be less than or equal to the interval_ms for
 * the task.  If greater, then interval_ms is used.
 *
 * @param taskNo
 * @param timeToNext_ms
 * @return The task no or -1 if the taskNo was not a valid task
 */
int Executive::modifyTaskNextRun(int taskNo, unsigned long timeToNext_ms) {
	if(taskNo>=_maxTasks || taskNo<0 || tasks[taskNo].doTask==nullptr) return -1;
	if(tasks[taskNo].oneShot) {
		tasks[taskNo].interval_ms = timeToNext_ms;
		tasks[taskNo].lastRun_ms = millis();
	} else {
		if(timeToNext_ms>tasks[taskNo].interval_ms) timeToNext_ms = tasks[taskNo].interval_ms;
		tasks[taskNo].lastRun_ms = millis() - (tasks[taskNo].interval_ms - timeToNext_ms);
	}
	return taskNo;
}

//------------------------------------------------------------------------------
/**
 * Immediately removes a task from the task schedule
 *
 * Note that this doesn't stop a running task, it will just prevent the task from
 * being run again.
 *
 * After removing a task, its taskNo is not longer valid and should be discarded
 * as the taskNo may get re-used on a subsequent call to addTask().
 *
 * @param taskNo
 * @return The task no of the removed task (which is no longer useful) or -1 if the taskNo was not a valid task.
 */
int Executive::removeTask(int taskNo) {
	if(taskNo>=_maxTasks || taskNo<0 || tasks[taskNo].doTask==nullptr) return -1;
	tasks[taskNo].doTask = nullptr;	// this flags the slot is free
	tasks[taskNo].enabled = false;	// extra insurance
	return taskNo;
}
//------------------------------------------------------------------------------

#if !defined(NO_GLOBAL_INSTANCES)
Executive Exec(DEFAULT_MAX_TASKS);
#endif
