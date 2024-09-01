#include "uthreads.h"
#include <cmath>
#include "Thread.h"
#include <iostream>
#include <csetjmp>
#include <queue>
#include <unordered_map>
#include <signal.h>
#include <sys/time.h>

#define MAX_THREAD_NUM 100 /* maximal number of threads */
#define SECOND_TO_MICRO 1000000  // Define SECOND_TO_MICRO as 1,000,000 microseconds
/**
 * @brief Switches the current execution context to a new state and handling
 * the thread switch.
 *
 * This function is responsible for switching the execution context to a
 * new state. It takes a `State` object as an argument, which defines the
 * new context to be set.
 * the function also handles the thread switch, and the thread that will be
 * executed next.
 *
 * @param state The new state to switch to.
 */
void switch_context (State state);

/**
 * @brief Initializes a timer with a specified interval.
 *
 * This function sets up a timer that will trigger after the specified
 * number of microseconds. The timer can be used for various time-based
 * operations within the application.
 *
 * @param usecs The interval in microseconds after which the timer should trigger.
 */
void initTimer (int usecs);

/**
 * @brief Sets up and initializes signal handling.
 *
 * this function sets up a set of signals that will be used for handling
 * the thread switching and signal handling.
 * The function also initializes the timer of signals.
 */
void signal_set_and_init ();

/**
 * @brief Signal handler for the timer signal.
 *
 * This function is called when the timer signal is triggered. It is
 * responsible for handling the signal and switching the thread context
 * to the next thread in the queue. and also updating the thread's quantum.
 * and the total quantum. and also take care of the sleeping threads.
 *
 * @param signum The signal number.
 */
void signalHandler (int signum);

/**
 * @brief Resets the timer to the initial value.
 *
 * This function resets the timer to the initial value. It is used when
 * the timer is triggered, and the signal handler is called.
 */
/**
 * @brief Resets the virtual timer for the thread library.
 *
 * This function sets the virtual timer for the process using the `setitimer` system call.
 * The timer is set to the value stored in the global `timer` variable. If the call to
 * `setitimer` fails, the function prints an error message and terminates the program.
 *
 * The virtual timer (ITIMER_VIRTUAL) decrements in real time only when the process is
 * executing, and delivers a SIGVTALRM signal when it expires.
 *
 * @throws Terminates the program with an error message if setting the timer fails.
 */
void reset_timer();

std::queue<Thread *> ready_queue;
std::priority_queue<int, std::vector<int>, std::greater<int>> minHeapId;
std::unordered_map<int, Thread *> threadsMap;
std::unordered_map<int, Thread *> blockedThreads;
std::unordered_map<Thread *, int> sleepingThreads;
Thread *currThread = nullptr;
int total_quantum_usecs = 0;
struct sigaction sigAct;
sigset_t SignalSetToBlock;
struct itimerval timer;
using namespace std;

void wakeSleepingThreads ()
{

  for (auto it = sleepingThreads.begin (); it != sleepingThreads.end ();)
  {
    sleepingThreads[it->first] = it->second - 1;
    if (it->second == 0 && it->first->getState () == SLEEP)
    {
      ready_queue.push (it->first);
      it->first->setState (READY);
      it = sleepingThreads.erase (it); // erase and update iterator
    }
    else if (it->second == 0 && it->first->getState () == BLOCKED_AND_SLEEP)
    {
      it->first->setState (BLOCKED);
      it = sleepingThreads.erase (it); // erase and update iterator
    }
    else
    {
      ++it; // increment iterator only if not erased
    }
  }

}

void signalHandler (int signum)
{
  // check if there are sleeping threads
  wakeSleepingThreads ();
  switch_context (READY);
}
Thread* remove_tid_from_ready_queue(int tid) {
  std::queue<Thread*> tempQueue;
  Thread* threadToRemove = nullptr;

  // Iterate through the original queue
  while (!ready_queue.empty()) {
    Thread* currentElement = ready_queue.front();
    ready_queue.pop();

    // Skip the element to remove
    if (currentElement->getID() != tid) {
      // Add the current element to the temporary queue
      tempQueue.push(currentElement);
    } else {
      threadToRemove = currentElement;
    }
  }

  // Swap the original queue with the temporary queue
  ready_queue.swap(tempQueue);

  return threadToRemove;
}


int uthread_init (int quantum_usecs)
{
  if (quantum_usecs <= 0)
  {
    std::cerr << "thread library error: Non-positive quantum_usecs"
              << std::endl;
    return FAILURE;
  }
  // initialize the minHeapId
  for (int i = 1; i < MAX_THREAD_NUM; i++)
  {
    minHeapId.push (i);
  }
  currThread = new Thread (0); // the main thread
  threadsMap[0] = currThread;

  sigAct.sa_handler = signalHandler;
  sigAct.sa_flags = 0; // default handling signals
  if (sigemptyset (&sigAct.sa_mask) == FAILURE)
  { // initialize an empty signal set.
    perror (ERROR);
    exit (1);
  }
  // setting up signal handler for sigvtalrm
  if (sigaction (SIGVTALRM, &sigAct, nullptr) == FAILURE)
  {
    perror (ERROR);
    exit (1);
  }
  // Initialize the timer with the provided quantum length
  initTimer (quantum_usecs);
  currThread->setState (RUNNING);
  switch_context (READY);

  signal_set_and_init ();
  return SUCCESS; // Success
}

void signal_set_and_init ()
{
  if (sigemptyset (&SignalSetToBlock) == FAILURE) //we didn't succeed to init
    // the set
  {
    perror (ERROR);
    exit (1);
  }
  if (sigaddset (&SignalSetToBlock, SIGVTALRM)
      == FAILURE) //we didn't succeed to
    // add
    // the timer alarm signal
  {
    perror (ERROR);
    exit (1);
  }

  reset_timer ();
}
void reset_timer()
{
  if (setitimer (ITIMER_VIRTUAL, &timer, nullptr) == FAILURE)
  {
    perror (ERROR);
    exit (1);
  }
}

void switch_context (State state)
{
  total_quantum_usecs++;
  if (ready_queue.empty ())
  {
    currThread->increaseQuantum ();
  }
  else
  {
    int ret = 0;
    Thread *nextThread = ready_queue.front ();
    ready_queue.pop ();
    if (state == TERMINATED)
    {
      threadsMap.erase (currThread->getID ());
      minHeapId.push (currThread->getID ());
      delete currThread;
    }
    else if (state == BLOCKED)
    {
      blockedThreads[currThread->getID ()] = currThread;
      currThread->setState (BLOCKED);
      ret = sigsetjmp(*(currThread->getThreadEnv ()), 1);
    }
    else if (state == READY)
    {
      ready_queue.push (currThread);
      currThread->setState (READY);
      ret = sigsetjmp(*(currThread->getThreadEnv ()), 1);
    }
    else if (state == SLEEP)
    {
      currThread->setState (SLEEP);
      ret = sigsetjmp(*(currThread->getThreadEnv ()), 1);
    }
    else if (state == BLOCKED_AND_SLEEP)
    {
      blockedThreads[currThread->getID ()] = currThread;
      currThread->setState (BLOCKED_AND_SLEEP);
      ret = sigsetjmp(*(currThread->getThreadEnv ()), 1);
    }
    if (ret == 0)
    {
      currThread = nextThread;
      currThread->increaseQuantum ();
      currThread->setState (RUNNING);
      siglongjmp (*(currThread->getThreadEnv ()), 1);
    }
  }
  reset_timer();
}

// Function to initialize the timer
void initTimer (int quantum_usecs)
{
  int seconds = std::floor (quantum_usecs / SECOND_TO_MICRO);
  int microseconds = quantum_usecs - seconds * SECOND_TO_MICRO;

  timer.it_value.tv_sec = seconds;  // First time interval, seconds part
  timer.it_value.tv_usec = microseconds; // First time interval,
  // microseconds part
  timer.it_interval.tv_sec = seconds;  // Following time intervals, seconds
  // part
  timer.it_interval.tv_usec = microseconds; // Following time intervals,
  // microseconds part
}

void blockTimerSignal ()
{
  if (sigprocmask (SIG_BLOCK, &SignalSetToBlock, nullptr) == FAILURE)
  {
    perror (ERROR);
    exit (1);
  }
}

void resumeTimerSignal ()
{
  if (sigprocmask (SIG_UNBLOCK, &SignalSetToBlock, nullptr) == FAILURE)
  {
    perror (ERROR);
    exit (1);
  }
}

int uthread_spawn (thread_entry_point entry_point)
{
  blockTimerSignal ();
  if (minHeapId.empty ())
  {
    std::cerr << "thread library error: Too many threads" << std::endl;
    resumeTimerSignal ();
    return FAILURE;
  }
  int id = minHeapId.top ();
  minHeapId.pop ();
  Thread *newThread = new Thread (id, entry_point);
  threadsMap[id] = newThread;
  ready_queue.push (newThread);
  resumeTimerSignal ();
  return id;
}

int uthread_terminate (int tid)
{
  blockTimerSignal ();


  if (tid == 0) // If true: exits program
  {
    Thread* main_thread;
    // Terminate all threads:
    for (auto threadIt = threadsMap.begin (); threadIt != threadsMap.end ();)
    {
      Thread *tmp = nullptr;
      if (threadIt->second->getState () == READY)
      {
        remove_tid_from_ready_queue (threadIt->second->getID ());
        tmp = threadIt->second;

        threadIt = threadsMap.erase (threadIt);
        delete tmp;
      }
      else if (threadIt->second->getState () == RUNNING)
      {
        tmp = threadIt->second;
        threadIt = threadsMap.erase (threadIt);
        main_thread = tmp;
        break;
      }
      else if (threadIt->second->getState () == BLOCKED)
      {
        blockedThreads.erase (threadIt->second->getID ());
        tmp = threadIt->second;
        threadIt = threadsMap.erase (threadIt);
        delete tmp;
      }
      else if (threadIt->second->getState () == SLEEP)
      {
        sleepingThreads.erase (threadIt->second);
        tmp = threadIt->second;
        threadIt = threadsMap.erase (threadIt);
        delete tmp;
      }
      else if (threadIt->second->getState () == BLOCKED_AND_SLEEP)
      {
        blockedThreads.erase (threadIt->second->getID ());
        sleepingThreads.erase (threadIt->second);
        tmp = threadIt->second;
        threadIt = threadsMap.erase (threadIt);
        delete tmp;
      }
      else
      {
        ++threadIt;
      }
    }
    delete main_thread;
    resumeTimerSignal ();
    exit (0);
  }

  if (threadsMap.count (tid) == 0) // If true: thread doesn't exist
  {
    std::cerr << "thread library error: tid doesn't exist so it can't be "
                 "terminated" << std::endl;
    resumeTimerSignal ();
    return FAILURE;
  }

  if (threadsMap[tid]->getState ()
      == RUNNING) // If true: deletes thread + jumps to next thread
  {
    switch_context (TERMINATED);
    // Execution won't return here, as switch_context will handle the termination.
  }

  // If in ready or blocked: remove from lists (Ready/blockedThreads + threadsMap), and delete thread
  Thread *tmp = threadsMap[tid];

  if (tmp->getState () == READY)
  {
    remove_tid_from_ready_queue (tid);
  }
  else if (tmp->getState () == BLOCKED)
  {
    blockedThreads.erase (tid);
  }
  else if (tmp->getState () == SLEEP)
  {
    sleepingThreads.erase (tmp);
  }
  else if (tmp->getState () == BLOCKED_AND_SLEEP)
  {
    blockedThreads.erase (tid);
    sleepingThreads.erase (tmp);
  }

  threadsMap.erase (tid);
  minHeapId.push (tid);
  delete tmp;

  resumeTimerSignal ();
  return SUCCESS;
}

int uthread_block (int tid)
{
  // Implement thread blocking logic here
  blockTimerSignal ();
  if (tid == 0)
  {
    std::cerr << "thread library error: can't block main main thread "
              << std::endl;
    resumeTimerSignal ();
    return FAILURE;
  }
  if (threadsMap.count (tid) == 0)
  {
    std::cerr << "thread library error: no such thread" << std::endl;
    resumeTimerSignal ();
    return FAILURE;
  }
  Thread *threadToBlock = threadsMap[tid];
  if (threadToBlock->getState () == BLOCKED)
  {
    resumeTimerSignal ();
    return SUCCESS;
  }
  if (threadToBlock->getState () == READY)
  {
    // Create a temporary queue to hold non-matching elements

    // Element found, move it to blocked threads and update state
    blockedThreads[tid] = remove_tid_from_ready_queue (tid);
    blockedThreads[tid]->setState (BLOCKED);

    resumeTimerSignal ();
    return SUCCESS;
  }
  if (threadToBlock->getState () == RUNNING)
  {
//		move to blockedThreads + update state in thread
    switch_context (BLOCKED);
    resumeTimerSignal ();
    return SUCCESS;
  }
  if (threadToBlock->getState () == SLEEP)
  {
    blockedThreads[tid] = threadToBlock;
    threadToBlock->setState (BLOCKED_AND_SLEEP);
    resumeTimerSignal ();
    return SUCCESS;

  }

  std::cerr
      << "thread library error: while blocking the thread state was invalid"
      << std::endl;
  resumeTimerSignal ();
  return FAILURE; // Placeholder
}

int uthread_resume (int tid)
{
  blockTimerSignal ();

  // Check if thread exists in the thread map
  if (threadsMap.count (tid) == 0)
  {
    // Thread does not exist, report error
    std::cerr << "thread library error: invalid tid to resume" << std::endl;
    resumeTimerSignal ();
    return FAILURE;
  }

  Thread *tmp = threadsMap[tid];

  // No action needed if already in READY or RUNNING state
  if (tmp->getState () == READY || tmp->getState () == RUNNING)
  {
    resumeTimerSignal ();
    return SUCCESS;
  }

  // Assuming no bugs, thread should be in blockedThreads
  auto it = blockedThreads.find (tid);
  if (it == blockedThreads.end ())
  {
    // Thread not found in blocked threads (handle potential error)
    // ... (e.g., log an error or return a specific error code)
    std::cerr << "thread library error: thread wasnt in blockedThreads "
                 "alogth he isnt ready or" <<
              "running" << std::endl;

    resumeTimerSignal ();
    return FAILURE; // Or some appropriate error code
  }

  // Remove thread from blocked threads using iterator
  blockedThreads.erase (it);
  if (tmp->getState () == BLOCKED_AND_SLEEP)
  {
    tmp->setState (SLEEP);
  }
  else
  {
    tmp->setState (READY);
    ready_queue.push (tmp);
  }
  // Move thread to ready queue and update state

  resumeTimerSignal ();
  return SUCCESS;
}

int uthread_sleep (int num_quantums)
{
  // Implement thread sleeping logic here
  blockTimerSignal ();
  if (num_quantums <= 0)
  {
    std::cerr << "thread library error: num_quantums must be positive"
              << std::endl;
    resumeTimerSignal ();
    return FAILURE;
  }
  int tid = uthread_get_tid ();
  if (tid == 0)
  {
    std::cerr << "thread library error: main thread cannot sleep" << std::endl;
    resumeTimerSignal ();
    return FAILURE;
  }
  if (threadsMap.count (tid) == 0)
  {
    std::cerr << "thread library error: thread not found" << std::endl;
    resumeTimerSignal ();
    return FAILURE;
  }
  Thread *threadToSleep = threadsMap[tid];
  sleepingThreads[threadToSleep] = num_quantums;
  threadToSleep->getState () == BLOCKED ? switch_context (BLOCKED_AND_SLEEP)
                                        : switch_context (SLEEP);
  resumeTimerSignal ();
  return -1; // Placeholder
}

int uthread_get_tid ()
{

  return currThread->getID ();
}

int uthread_get_total_quantums ()
{
  // Implement getting total quantums logic here
  return total_quantum_usecs;
}

int uthread_get_quantums (int tid)
{
  blockTimerSignal (); // Block timer signals to ensure consistency
  if (tid < 0 || tid > MAX_THREAD_NUM)
  {
    std::cerr << "thread library error: tid not in range" << std::endl;
    resumeTimerSignal ();
    return FAILURE;
  }

  auto thread = threadsMap.find (tid);
  if (thread == threadsMap.end ())
  { // If the thread with given tid does not
    // exist - can happen if the right range but no thread with that id
    std::cerr << "thread library error: thread not found" << std::endl;
    resumeTimerSignal ();
    return FAILURE;
  }
  int quantums = thread->second->getQuantum (); // getQuantum() returns the number of quantums for the thread
  resumeTimerSignal (); // Resume timer signals before returning
  return quantums;
}

