/*
 * Executive.cpp
 *
 *  Created on: 26 Jun 2018
 *      Author: j-watson
 */

#include "Executive.h"

//------------------------------------------------------------------------------
/**
 * Minimal implementation of a class to replace std::Vector
 * Based on post by Gido on Arduino forum
 */
template <typename Data>
class SimpleVector {
  size_t vecSize;      // Stores no. of actually stored objects
  size_t vecCapacity;  // Stores allocated capacity
  Data *vecData;       // Stores data

 public:
  SimpleVector(int initialCapacity = 0)
      : vecSize(0), vecCapacity(initialCapacity), vecData(0) {
    if (initialCapacity > 0) resize(initialCapacity);
  };  // Default constructor

  SimpleVector(SimpleVector const &other)
      : vecSize(other.vecSize), vecCapacity(other.vecCapacity), vecData(0) {
    vecData = (Data *)malloc(vecCapacity * sizeof(Data));
    memcpy(vecData, other.vecData, vecSize * sizeof(Data));
  };  // Copy constuctor

  ~SimpleVector() { free(vecData); };  // Destructor

  SimpleVector &operator=(SimpleVector const &other) {
    free(vecData);
    vecSize = other.vecSize;
    vecCapacity = other.vecCapacity;
    vecData = (Data *)malloc(vecCapacity * sizeof(Data));
    memcpy(vecData, other.vecData, vecSize * sizeof(Data));
    return *this;
  };  // Needed for memory management

  void push_back(Data const &x) {
    if (vecSize >= vecCapacity) {
      if (!resize(vecCapacity + vecCapacity / 2 + 1)) return;
      // if could not expand by 50% then do nothing
      // the std::Vector implementation raises an exception but we can't
    }
    vecData[vecSize++] = x;
  };  // Adds  value. If needed, allocates more space

  size_t size() const { return vecSize; };  // Size getter
  size_t capacity() const { return vecCapacity; }

  //   Data const &operator[](size_t idx) const {
  //     return vecData[idx];
  //   };                                                     // Const getter

  Data &operator[](size_t idx) { return vecData[idx]; };  // Changeable getter

 private:
  // returns false if not enough space for resize
  bool resize(size_t newVecCapacity) {
    Data *data = (Data *)malloc(newVecCapacity * sizeof(Data));
    if (data == nullptr) {
      return false;
    } else {
      vecCapacity = newVecCapacity;
      memcpy(data, vecData, vecSize * sizeof(Data));
      free(vecData);
      vecData = data;
      return true;
    }
  };
};

//------------------------------------------------------------------------------
struct TaskEntry {
  unsigned long lastRun_ms;
  unsigned long interval_ms;
  void (*doTask)(void);
  bool oneShot;
  bool enabled;
};

SimpleVector<TaskEntry> *tasks;

//------------------------------------------------------------------------------
/**
 * Construct the Exec object
 * 
 * For Arduino, you should not need to use this constructor.  When you
 * include the header file (Executive.h) in your sketch you will automatically
 * get a global variable 'Exec' that you use to access the library functions. 
 *
 * V 1.1.0 The table now dynamically grows. MaxTasks is now the initial size of
 * the allocated table (optional)
 */
Executive::Executive(int maxTasks) {
  tasks = new SimpleVector<TaskEntry>(maxTasks);
}

//------------------------------------------------------------------------------
/**
 * Default destructor
 * 
 * Unless you have created an instance of Executive yourself (via 'new') you
 * are unlikely to need to use this yourself.
 */
Executive::~Executive() { delete tasks; }

//------------------------------------------------------------------------------
/**
 * Add a task (internal routine)
 *
 * @param timeToNext_ms Number of ms until first run of the task to do.  Must be
 * less than interval_ms otherwise taken as interval_ms
 * @param interval_ms Interval between successive runs of the task
 * @param doTask The routine called to do the task
 * @param oneShot True if this is to be a one-shot task
 * @return An index to the task or -1 if the task list could not be expanded
 */
int Executive::addTask(unsigned long interval_ms, void (*doTask)(void),
                       unsigned long timeToNext_ms, bool oneShot) {
  int slot;
  TaskEntry t = {.interval_ms = interval_ms,
                 .doTask = doTask,
                 .lastRun_ms = (timeToNext_ms >= interval_ms)
                                   ? millis()
                                   : millis() - (interval_ms - timeToNext_ms),
                 .enabled = true,
                 .oneShot = oneShot};

  for (slot = 0; slot < tasks->size(); slot++) {
    if ((*tasks)[slot].doTask == nullptr) {
      (*tasks)[slot] = t;  // re-use any empty slot
      return slot;
    }
  }

  // otherwise table grows
  tasks->push_back(t);
  if (slot >= tasks->capacity()) {
    // table could not be expanded
    return -1;
  } else {
    return slot;
  }
}

//------------------------------------------------------------------------------
/**
 * Add a  task to be performed at a regular interval
 *
 * @param timeToNext_ms Number of ms until first run of the task to do.  Must be
 * less than interval_ms otherwise taken as interval_ms
 * @param interval_ms Interval between successive runs of the task
 * @param doTask The routine called to do the task
 * @return An index to the task or -1 if task could not be added (no room left
 * the required task table)
 */
int Executive::addTask(unsigned long interval_ms, void (*doTask)(void),
                       unsigned long timeToNext_ms) {
  return addTask(interval_ms, doTask, timeToNext_ms, false);
}

//------------------------------------------------------------------------------
/**
 * Add a  task to be performed just once
 *
 * @param timeToNext_ms Number of ms until first run of the task to do.
 * @param doTask The routine called to do the task
 * @return An index to the task or -1 if task could not be added (no room left
 * for the required task table)
 */
int Executive::addOneShotTask(void (*doTask)(void),
                              unsigned long timeToNext_ms) {
  int slot;
  return addTask(timeToNext_ms, doTask, timeToNext_ms, true);
}

//------------------------------------------------------------------------------
/**
 * Run the task that is most overdue to run.
 *
 * Only the most overdue task is run.  If there are no overdue tasks this
 * routine returns after a short delay (MIN_YIELD_TIME_MS).
 */
void Executive::yield(void) { Executive::delay(MIN_YIELD_TIME_MS); }

//------------------------------------------------------------------------------
/**
 * Hand over control to the Executive.  Run tasks to the schedule but never
 * return.
 */
void Executive::loop() { Executive::delay(DELAY_FOREVER); }

//------------------------------------------------------------------------------
/**
 * Run tasks in the schedule that need to be run in the next delay_ms
 * milliseconds. Return after at least delay_ms.  I.e. the function will call
 * the standard delay() function to use up any 'spare' time after any relevant
 * tasks have been run.
 *
 * Tasks are not run in parallel, so if a task become due to be run during the
 * execution of another task it will have to wait until the current task
 * completes before it can be started.
 *
 * When considering which task to run, the task with the earliest desired start
 * time will always take priority.  Where two tasks have the same desired start
 * time, the one earliest in the list will take precedence.  You should aim to
 * place the tasks with shortest intervals near the top of the list as they
 * stand the best chance of running to their desired interval.
 *
 * Immediately before running a task, the time at it is started is recorded. The
 * desired time of the next execution will be based on the actual time at which
 * the task is started and the task interval. There is no account taken of
 * whether the task was late in starting.  This effectively means that the task
 * interval is a minimum setting for the amount of time between successive calls
 * to the task function.  In the best case, where the delay() function has
 * control for most of the time and other scheduled functions don't interfere,
 * the interval time will be honoured and the task will operate at the requested
 * interval.  If the sketch spends a lot of its time outside the delay()
 * function or if there are other tasks that individually or in combination
 * might take longer then the interval, then Executive won't be able to keep to
 * the schedule and will work on a 'best endeavours' basis.
 *
 * One-shot tasks are treated equally to the interval tasks, except that they
 * are removed from the table immediately before this scheduler runs their
 * doTask() function.
 *
 * If a task is started and runs beyond the delay_ms window then this
 * Executive::delay() function will itself return as soon as that task
 * completes. In this case no further tasks will be started (even if they were
 * due to run in that delay_ms window).
 *
 * If, when delay() is called, there is a backlog of tasks that are due to be
 * run, then the tasks get run in the order of their desired run time.  Again,
 * if the end of the delay_ms is reached, then the delay() function will return
 * as soon as the current task finishes, even if there are other back-logged or
 * due to run tasks->  This behaviour does ensure that if there is a backlog of
 * tasks then at least one of these will get run for any call to delay().
 *
 * If there are no tasks due to run in the requested delay_ms window then the
 * Executive:delay() function just behaves like a standard delay() function.
 *
 * @param delay_ms The minimum amount of time to remain in this scheduling
 * function.  Return after this time has passed and a soon as any currently
 * executing task returns.  If the special value DELAY_FOREVER is used then the
 * control is passed entirely to Executive and is never returned back to the
 * calling sketch.
 *
 * @see Executive::loop()
 *
 */
void Executive::delay(unsigned long delay_ms) {
  unsigned long start_ms = millis();

  do {
    unsigned long current_ms = millis();  // snapshot to treat all tasks equally
    signed long bestDelta_ms = 0x7fffffffL;
    int topTask = -1;
    for (int i = 0; i < tasks->size(); i++) {
      if ((*tasks)[i].doTask != nullptr && (*tasks)[i].enabled) {
        unsigned long nextRun_ms =
            (*tasks)[i].lastRun_ms + (*tasks)[i].interval_ms;
        signed long delta_ms = (signed long)(nextRun_ms - current_ms);
        if (delta_ms < bestDelta_ms) {
          bestDelta_ms = delta_ms;
          topTask = i;
        }
      }
    }

    if (topTask == -1) {
      // no tasks in task list - so behave as if nothing to do for a short time
      bestDelta_ms = MIN_YIELD_TIME_MS;
    }

    if (bestDelta_ms <= 0) {
      // task is due or overdue - start immediately
      (*tasks)[topTask].lastRun_ms = millis();
      void (*taskFn)(void) = (*tasks)[topTask].doTask;
      if ((*tasks)[topTask].oneShot) (*tasks)[topTask].doTask = nullptr;
      taskFn();
      ::yield();
      // Is there no more time left to do more?
      if (((signed long)(start_ms + delay_ms - millis())) <= 0) {
        break;
      }
    } else {
      // next task is in the future
      signed long deltaToEnd_ms = (signed long)(start_ms + delay_ms - millis());
      if (delay_ms != DELAY_FOREVER && bestDelta_ms >= deltaToEnd_ms) {
        // next task to run won't fall in the current delay_ms window
        if (deltaToEnd_ms >= 0) {
          ::delay(deltaToEnd_ms);
        }
        break;  // leave the function
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
 * @param taskNo Number of the task that is to be enabled.  The task number is
 * returned from addTask() and addOnShotTask().
 * @return The task no or -1 if the taskNo was not a valid task
 */
int Executive::enableTask(int taskNo) {
  if (taskNo >= tasks->size() || taskNo < 0 ||
      (*tasks)[taskNo].doTask == nullptr)
    return -1;

  (*tasks)[taskNo].lastRun_ms = millis() - (*tasks)[taskNo].interval_ms;
  (*tasks)[taskNo].enabled = true;
  return taskNo;
}
//------------------------------------------------------------------------------
/**
 * Disable the specified task from being scheduled (run)
 *
 * Task can be one-shot or continuous but must have been set up previously.  If
 * the task is one-shot then it must be disabled before it first runs or this
 * routine will return -1. One-shot tasks are deleted from the table immediately
 * before they are run.
 * @param taskNo Number of the task that is to be disabled.  The task number is
 * returned from addTask() and addOnShotTask().
 * @return The task no or -1 if the taskNo was not a valid task
 */
int Executive::disableTask(int taskNo) {
  if (taskNo >= tasks->size() || taskNo < 0 ||
      (*tasks)[taskNo].doTask == nullptr)
    return -1;
  (*tasks)[taskNo].enabled = false;
  return taskNo;
}
//------------------------------------------------------------------------------
/**
 * Modify the interval at which a task runs or the time until a one-shot is
 * executed
 *
 * For continuously running tasks, only the interval is changed, so if it is
 * shortened, the task may immediately become eligible to run (during the next
 * yield() or delay()).
 *
 * For one-shot tasks, the 'timer' is reset so it will become eligible after the
 *  interval_ms period.
 *
 * @param taskNo
 * @param interval_ms
 * @return The task no or -1 if the taskNo was not a valid task
 */
int Executive::modifyTaskInterval(int taskNo, unsigned long interval_ms) {
  if (taskNo >= tasks->size() || taskNo < 0 ||
      (*tasks)[taskNo].doTask == nullptr)
    return -1;
  (*tasks)[taskNo].interval_ms = interval_ms;
  if ((*tasks)[taskNo].oneShot) (*tasks)[taskNo].lastRun_ms = millis();
  return taskNo;
}

//------------------------------------------------------------------------------
/**
 * Change the time at which the task will next run
 *
 * Note that for one-shot tasks, this behaves the same as modifyTaskInterval()
 * and the internal 'timer' is effectively reset.
 *
 * For continuously running tasks, the timeToNext_ms should be less than or
 * equal to the interval_ms for the task.  If greater, then interval_ms is used.
 *
 * @param taskNo
 * @param timeToNext_ms
 * @return The task no or -1 if the taskNo was not a valid task
 */
int Executive::modifyTaskNextRun(int taskNo, unsigned long timeToNext_ms) {
  if (taskNo >= tasks->size() || taskNo < 0 ||
      (*tasks)[taskNo].doTask == nullptr)
    return -1;
  if ((*tasks)[taskNo].oneShot) {
    (*tasks)[taskNo].interval_ms = timeToNext_ms;
    (*tasks)[taskNo].lastRun_ms = millis();
  } else {
    if (timeToNext_ms > (*tasks)[taskNo].interval_ms)
      timeToNext_ms = (*tasks)[taskNo].interval_ms;
    (*tasks)[taskNo].lastRun_ms =
        millis() - ((*tasks)[taskNo].interval_ms - timeToNext_ms);
  }
  return taskNo;
}

//------------------------------------------------------------------------------
/**
 * Immediately removes a task from the task schedule
 *
 * Note that this doesn't stop a running task, it will just prevent the task
 * from being run again.
 *
 * After removing a task, its taskNo is not longer valid and should be discarded
 * as the taskNo may get re-used on a subsequent call to addTask().
 *
 * @param taskNo
 * @return The task no of the removed task (which is no longer useful) or -1 if
 * the taskNo was not a valid task.
 */
int Executive::removeTask(int taskNo) {
  if (taskNo >= tasks->size() || taskNo < 0 ||
      (*tasks)[taskNo].doTask == nullptr)
    return -1;
  (*tasks)[taskNo].doTask = nullptr;  // this flags the slot is free
  (*tasks)[taskNo].enabled = false;   // extra insurance
  return taskNo;
}
//------------------------------------------------------------------------------

#if !defined(NO_GLOBAL_INSTANCES)
Executive Exec(DEFAULT_MAX_TASKS);
#endif
