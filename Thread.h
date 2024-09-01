//
// Created by home on 6/2/2024.
//

#ifndef _THREAD_H_
#define _THREAD_H_
#include <csetjmp>
#include <signal.h>

#define SUCCESS 0
#define FAILURE (-1)
#define STACK_SIZE 4096 /* stack size per thread (in bytes) */
#define ERROR "system error"

typedef void (*FuncLoc) (void);
enum State { RUNNING, BLOCKED, READY, SLEEP,TERMINATED,BLOCKED_AND_SLEEP };


class Thread
{
 private:
  int id;
  State state;
  int quantum;
  FuncLoc f;
  char* stack;
  sigjmp_buf threadEnv;


 public:
  explicit Thread(int id, FuncLoc f);
  explicit Thread(int id);
  ~Thread();
  void setState(State state);
  char* getStack() const;
  State getState() const;
  int getID() const;
  sigjmp_buf* getThreadEnv();
  int getQuantum() const;
  void increaseQuantum();
};








#endif //_THREAD_H_
