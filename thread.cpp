#define GC_PTHREADS 1
#include <iostream>
#include "style.hpp"
#include "mem.hpp"
#include "engine.h"
#include "ring.hpp"
#include "threadpool.hpp"
#include "thread.hpp"
#include "mutex.h"
#include "threadqueue.h"

extern "C" {
  extern void interp_process(), interp_process2(), interp_initializeLocalInterpState();
  extern void thread_rawRunSequence(struct threadLocalInterp*,void*);
}

void* outptr;
void rawTestThread(void* inptr)
{
  outptr = inptr;
  M2ThreadPool::m_Singleton->test();
}

void setCurrentThreadLocalInterp(struct threadLocalInterp* tli)
{
  M2Thread::getCurrentThread()->setThreadLocalInterp(tli);
}
struct threadLocalInterp* getCurrentThreadLocalInterp()
{
  struct threadLocalInterp* tli = M2Thread::getCurrentThread()->getThreadLocalInterp();
  return tli;
}
struct threadLocalInterp* getStartupThreadLocalInterp()
{
  struct threadLocalInterp* tli = M2ThreadPool::m_Singleton->startThread()->getThreadLocalInterp();
  return tli;
}

M2Thread::M2Thread(M2ThreadPool* pool, int id):m_ThreadId(id),m_ThreadPool(pool),m_ThreadLocalInterp(NULL)
{
  
}

void M2Thread::start()
{
  pthread_create(&m_Thread,0,sThreadEntryPoint,this);
}
void M2Thread::join()
{
  void* status;
  pthread_join(m_Thread, &status);
}
void* M2Thread::threadEntryPoint()
{
  setCurrentThread();
  if(this!=m_ThreadPool->startThread())
    {
      interp_initializeLocalInterpState();
       thread_rawRunSequence(getCurrentThreadLocalInterp(),outptr);
    }
  else
    {
      interp_process();
    }
  return NULL;
}
void posixDestructor(void*) { }
void M2Thread::setCurrentThread()
{
  pthread_setspecific(M2ThreadPool::m_Singleton->ThreadKey(),this);
}
M2Thread* M2Thread::getCurrentThread()
{
  return (M2Thread*) pthread_getspecific(M2ThreadPool::m_Singleton->ThreadKey());
}
