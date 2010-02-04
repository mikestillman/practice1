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
#include "m2interface.hpp"

extern "C" {
  extern void interp_process(), interp_process2(), interp_initializeLocalInterpState();
  extern void thread_rawRunSequence(struct threadLocalInterp*,void*);
}

struct M2Sequence* outptr;
void rawTestThread(void* inptr)
{
  outptr = (M2Sequence*)inptr;
  if(!M2ExprIsSequence((M2Expr*)inptr))
    {
      std::cout << "Bad argument to raw test thread" << std::endl;
      return;
    }
  std::cout << "Sequence length is " << M2SequenceLength((M2Sequence*)((M2Expr*)outptr)->ptr_) << std::endl;
  M2ThreadPool::m_Singleton->test();
}

bool rawQueueTask(void* inexpr)
{
  M2Expr* expr = (M2Expr*)inexpr;
  if(!M2ExprIsSequence(expr))
    {
      return false;
    }
  return true;
}
bool rawQueueTaskList(void* inexpr)
{
  M2Expr* expr = (M2Expr*)inexpr;
  if(!M2ExprIsSequence(expr))
    {
      return false;
    }
  if(M2SequenceLength((M2Sequence*)expr)<2)
    return false;
  if(!M2ExprIsSequence(M2SequenceGetExpr((M2Sequence*)expr,1)))
     return false;
  struct M2Sequence* args = (M2Sequence*)M2SequenceGetExpr((M2Sequence*)expr,1);
  size_t numArgs = M2SequenceLength(args);
  struct M2ThreadTaskGroupStruct* tg = createM2ThreadTaskGroup(NULL,NULL,numArgs,numArgs,0,0);
  struct M2FunctionSpecifier* fspec = createM2FunctionSpecifier(M2SequenceGetExpr((M2Sequence*)expr,0));
  struct M2TaskArgumentPackage* argPackage = createM2TaskArgumentPackageFromSequence(fspec,args);
  for(int i = 0; i<numArgs; ++i)
    {
      struct M2TaskStruct* ts = createM2Task(argPackage,tg);
      ts->m_ArgStart = ts->m_ArgLocation=i;
      ts->m_ArgEnd = i+1;
      getTaskGroupStartTask(tg)[i]=ts;
    }
  return true;
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

M2Thread::M2Thread(M2ThreadPool* pool, int id):m_ThreadId(id),m_ThreadPool(pool),m_ThreadLocalInterp(NULL),m_CurrentTask(NULL)
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
