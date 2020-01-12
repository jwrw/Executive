/*
 * executive.h
 *
 *  Created on: 26 Jun 2018
 *      Author: j-watson
 */

#ifndef EXECUTIVE_H_
#define EXECUTIVE_H_

#include "Arduino.h"

/**
 * The yield() function is actually the same as a short delay() of this number
 * of ms
 */
const long MIN_YIELD_TIME_MS = 1;

/**
 * The library maintains a fixed-size table of tasks so needs to know the
 * size of this.  If the caller doesn't specify anything then this size is used
 */
const int DEFAULT_MAX_TASKS = 10;

class Executive {
 public:
  /**
   * Use in a call to Exec.delay() this will cause an infinite delay.
   */
  const unsigned long DELAY_FOREVER = -1L;

  Executive(int maxTasks = DEFAULT_MAX_TASKS);
  ~Executive();

  int addTask(unsigned long interval_ms, void (*doTask)(void),
              unsigned long timeToNext_ms = 0);
  int addOneShotTask(void (*doTask)(void), unsigned long timeToNext_ms = 0);

  int enableTask(int taskNo);
  int disableTask(int taskNo);
  int modifyTaskInterval(int taskNo, unsigned long interval_ms);
  int modifyTaskNextRun(int taskNo, unsigned long timeToNext_ms);
  int removeTask(int taskNo);

  void yield(void);
  void delay(unsigned long delay_ms);
  void loop(void);

 private:
  int addTask(unsigned long interval_ms, void (*doTask)(void),
              unsigned long timeToNext_ms, bool oneShot);
};

#if !defined(NO_GLOBAL_INSTANCES)
extern Executive Exec;
#endif

#endif /* EXECUTIVE_H_ */
