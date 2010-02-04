#ifndef _threadpool_hh_
#define _threadpool_hh_

#include <vector>
#include "m2interface.hpp"

class M2Thread;

class M2ThreadPool
{
public:
  M2ThreadPool(int numThreads);
  pthread_key_t ThreadKey() { return m_ThreadKey; } 
  void createThreads();
  void joinThreads();
  ~M2ThreadPool();
  M2Thread* startThread() { return m_StartThread; }
  static M2ThreadPool* m_Singleton;
  void test();
protected:
  friend class M2Thread;
  void createThread(int threadId);
  pthread_key_t m_ThreadKey;
  const int m_NumPoolThreads;
  M2Thread* m_StartThread;
  std::vector<M2Thread*> m_PoolThreads;
};



struct M2FunctionSpecifier
{
public:
  ///Pointer to the native function location in memory or the m2 function
  void* m_F;
  ///Number of arguments the function takes
  int m_NumArgs;
  ///Is the function an interperted function or a native function
  bool m_InterpFunction;
  ///Is this RPCable 
  bool m_RPC;
  ///Function ID for RPC on native functions.  0 if not used.
  long m_NativeFunctionId;
  ///Pointers to Serialization routines for each of the arguments for RPC purposes.  NULL if not used.
  void** m_ArgSerializer;
};

inline struct M2FunctionSpecifier* createM2FunctionSpecifier(M2Expr* f)
{
  M2FunctionSpecifier* spec = (M2FunctionSpecifier*)GC_MALLOC(sizeof(M2FunctionSpecifier));
  spec->m_F = f;
  spec->m_NumArgs = 1;
  spec->m_InterpFunction = true;
  spec->m_RPC = true;
  spec->m_NativeFunctionId=0;
  spec->m_ArgSerializer = NULL;
  return spec;
}
inline struct M2FunctionSpecifier* createNativeFunctionSpecifier(void* f, int numArgs)
{
  M2FunctionSpecifier* spec = (M2FunctionSpecifier*)GC_MALLOC(sizeof(M2FunctionSpecifier));
  spec->m_F = f;
  spec->m_NumArgs = numArgs;
  spec->m_InterpFunction = false;
  spec->m_RPC = false;
  spec->m_NativeFunctionId=0;
  spec->m_ArgSerializer = NULL;
  return spec;
}




enum M2TaskArgumentPackageType
  {
    M2TaskArgumentPackageType_Sequence,
    M2TaskArgumentPackageType_List,
    M2TaskArgumentPackageType_Single,
  };

struct M2ThreadTaskGroupStruct;

struct M2TaskArgumentPackage;
struct M2TaskStruct
{
  struct M2TaskArgumentPackage* m_ArgPackage;
  void* m_Result;
  struct M2ThreadTaskGroupStruct* m_Group;
  M2Thread* m_CurrentThread;
  size_t m_ArgStart, m_ArgEnd;
  size_t m_ArgLocation;
  M2TaskArgumentPackageType m_ResultType;
};

struct M2TaskArgumentPackage
{
  M2FunctionSpecifier* m_FunctionSpec;
  void* m_Args;
  size_t m_Start, m_End;
  size_t m_Location;
  M2TaskArgumentPackageType m_Type;
};


inline struct M2TaskArgumentPackage* createM2TaskArgumentPackage(struct M2FunctionSpecifier* fspec, void** args, size_t length)
{
  struct M2TaskArgumentPackage* package = (struct M2TaskArgumentPackage*) GC_MALLOC(sizeof(M2TaskArgumentPackage));
  package->m_Type = M2TaskArgumentPackageType_List;
  package->m_Args=args;
  package->m_FunctionSpec=fspec;
  package->m_End = length;
  package->m_Start = 0;
  package->m_Location = 0;
  return package;
}
inline struct M2TaskArgumentPackage* createM2TaskArgumentPackageFromSequence(struct M2FunctionSpecifier* fspec, struct M2Sequence* seq)
{
  struct M2TaskArgumentPackage* package = (struct M2TaskArgumentPackage*) GC_MALLOC(sizeof(M2TaskArgumentPackage));
  package->m_Type = M2TaskArgumentPackageType_Sequence;
  package->m_Args=seq;
  package->m_FunctionSpec=fspec;
  package->m_End = M2SequenceLength(seq);
  package->m_Start = 0;
  package->m_Location = 0;
  return package;
}
inline struct M2TaskArgumentPackage* createM2TaskArgumentPackageFromSingle(struct M2FunctionSpecifier* fspec, void* arg)
{
  struct M2TaskArgumentPackage* package = (struct M2TaskArgumentPackage*) GC_MALLOC(sizeof(M2TaskArgumentPackage));
  package->m_Type = M2TaskArgumentPackageType_Single;
  package->m_Args=arg;
  package->m_FunctionSpec=fspec;
  package->m_End = 1;
  package->m_Start = 0;
  package->m_Location = 0;
  return package;
}


inline struct M2TaskStruct* createM2Task(struct M2TaskArgumentPackage* argPack, struct M2ThreadTaskGroupStruct* group)
{
  size_t length = sizeof(struct M2TaskStruct*) + sizeof(void*)*argPack->m_FunctionSpec->m_NumArgs;
  struct M2TaskStruct* tts = (struct M2TaskStruct*) GC_MALLOC(length);
  tts->m_ArgPackage = argPack;
  tts->m_Result=NULL;
  tts->m_Group=group;
  tts->m_CurrentThread=NULL;
  return tts;
}

inline void** getArgumentPtr(struct M2TaskStruct* task, int i)
{
  return (void**)(((char*)task)+sizeof(struct M2TaskStruct)+sizeof(void*)*i);
}


struct M2ThreadTaskGroupStruct
{
  struct M2ThreadTaskGroupStruct* m_Parent;
  struct M2TaskStruct* m_Creator;
  size_t m_ThreadTaskComplete;
  size_t m_ThreadTaskSize;
  size_t m_ThreadTaskRequired;
  size_t m_TaskGroupComplete;
  size_t m_TaskGroupSize;
  size_t m_TaskGroupRequired;  
};

inline struct M2ThreadTaskGroupStruct* createM2ThreadTaskGroup(struct M2ThreadTaskGroupStruct* parent, struct M2TaskStruct* creator, size_t threadTaskSize, size_t threadTaskRequired, size_t taskGroupSize, size_t taskGroupRequired)
{
  size_t length = sizeof(struct M2ThreadTaskGroupStruct)+sizeof(M2TaskStruct*)*threadTaskSize+sizeof(struct M2ThreadTaskGroupStruct*)*taskGroupSize;
  struct M2ThreadTaskGroupStruct* tgs = (struct M2ThreadTaskGroupStruct*) GC_MALLOC(length);
  tgs->m_Parent = parent;
  tgs->m_Creator = creator;
  tgs->m_ThreadTaskComplete = 0;
  tgs->m_ThreadTaskSize = threadTaskSize;
  tgs->m_ThreadTaskRequired = threadTaskRequired;
  tgs->m_TaskGroupComplete = 0;
  tgs->m_TaskGroupSize = taskGroupSize;
  tgs->m_TaskGroupRequired = taskGroupRequired;
  
}

inline struct M2TaskStruct** getTaskGroupStartTask(struct M2ThreadTaskGroupStruct* tgs)
{
  return (M2TaskStruct**)(  ((char*)tgs)+sizeof(struct M2ThreadTaskGroupStruct));
}
inline struct M2ThreadTaskGroupStruct** getTaskGroupStartTaskGroup(struct M2ThreadTaskGroupStruct* tgs)
{
  return (M2ThreadTaskGroupStruct**)(  ((char*)tgs)+sizeof(struct M2ThreadTaskGroupStruct) + sizeof(M2TaskStruct*)*tgs->m_ThreadTaskSize);
}





#endif
