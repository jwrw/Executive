/*
 * executive.h
 *
 *  Created on: 26 Jun 2018
 *      Author: jim
 */

#ifndef EXECUTIVE_H_
#define EXECUTIVE_H_

const long MIN_YIELD_TIME_MS = 1;
const int DEFAULT_MAX_TASKS = 10;

class Executive {

public:
	Executive(void);
	Executive(int maxTasks);
	~Executive();

	int addTask(unsigned long interval_ms, void (*doTask)(void), unsigned long timeToNext_ms);

	void yield(void);
	void delay(long delay_ms);

};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_EEPROM)
extern Executive Exec;
#endif

#endif /* EXECUTIVE_H_ */
