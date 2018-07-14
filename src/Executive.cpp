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
}*tasks;

int _nTasks = 0;
int _maxTasks = 0;

//------------------------------------------------------------------------------
/**
 *
 * @param maxTasks
 */
Executive::Executive(int maxTasks) {
	_maxTasks = maxTasks;
	tasks = new TaskEntry[maxTasks];
}

//------------------------------------------------------------------------------
/**
 */
Executive::~Executive() {
	delete[] tasks;
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
int Executive::addTask(unsigned long interval_ms, void (*doTask)(void),
		unsigned long timeToNext_ms) {
	if (_nTasks < _maxTasks) {
		tasks[_nTasks].interval_ms = interval_ms;
		tasks[_nTasks].doTask = doTask;
		tasks[_nTasks].lastRun_ms =
				(timeToNext_ms >= interval_ms) ?
						millis() : millis() - (interval_ms - timeToNext_ms);
		return _nTasks++;
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
		signed long bestDelta_ms = (signed long) ((tasks[0].lastRun_ms + tasks[0].interval_ms) - current_ms);
		int topTask = 0;
		for (int i = 1; i < _nTasks; i++) {
			unsigned long nextRun_ms = tasks[i].lastRun_ms + tasks[i].interval_ms;
			signed long delta_ms = (signed long) (nextRun_ms - current_ms);
			if (delta_ms < bestDelta_ms) {
				bestDelta_ms = delta_ms;
				topTask = i;
			}
		}

		if(bestDelta_ms<=0) {
			// task is due or overdue - start immediately
			tasks[topTask].lastRun_ms = millis();
			tasks[topTask].doTask();
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
#if !defined(NO_GLOBAL_INSTANCES)
Executive Exec(10);
#endif
