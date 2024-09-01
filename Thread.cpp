//
// Created by home on 6/2/2024.
//

#include <csetjmp>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include "Thread.h"

#ifdef __x86_64__
/* code for 64 bit Intel arch */

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%fs:0x30,%0\n"
        "rol    $0x11,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
}

#else
/* code for 32 bit Intel arch */

typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5


/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
  address_t ret;
  asm volatile("xor    %%gs:0x18,%0\n"
               "rol    $0x9,%0\n"
      : "=g" (ret)
      : "0" (addr));
  return ret;
}


#endif


Thread::Thread(int id,FuncLoc f): id(id),f(f),state(READY),quantum(0){
  stack = new char[STACK_SIZE];
  address_t sp, pc;
  sp = (address_t)stack + STACK_SIZE - sizeof(address_t);
  pc = (address_t)f;
  sigsetjmp(threadEnv, 1);
  (threadEnv->__jmpbuf)[JB_SP] = translate_address(sp);
  (threadEnv->__jmpbuf)[JB_PC] = translate_address(pc);
  if (sigemptyset(&threadEnv->__saved_mask) == FAILURE)
  {
    perror (ERROR);
    exit(1);
  }

}



Thread::Thread(int id): id(id) ,quantum(0){
  stack = new char[STACK_SIZE];
}

void Thread::setState(State s){
  state = s;
}
State Thread::getState() const{
  return this-> state;
}

Thread::~Thread()
{
  delete[] stack;
  stack = nullptr;
}

int Thread::getID() const
{
  return id;
}
sigjmp_buf* Thread::getThreadEnv(){
  return &threadEnv;
}

int Thread::getQuantum () const
{
  return quantum;
}
void Thread::increaseQuantum()
{
  quantum++;
}
