use evaluate;
use expr;

header "#include \"../system/supervisorinterface.h\"";


taskCreatePush(f:function(TaskCellBody):null,tb:TaskCellBody) ::=  Ccode(taskPointer,
     "runM2Task((void *(*)(void *))(",f,"),(void *)(",tb,"))");
taskCreate(f:function(TaskCellBody):null,tb:TaskCellBody) ::=  Ccode(taskPointer,
     "createM2Task((void *(*)(void *))(",f,"),(void *)(",tb,"))");

export taskDone(tp:taskPointer) ::= Ccode(int, "taskDone(",tp,")")==1;
export taskStarted(tp:taskPointer) ::=Ccode(int, "taskStarted(",tp,")")==1;
pushTask(tp:taskPointer) ::=Ccode(void, "pushTask(",tp,")");
taskResult(tp:taskPointer) ::=Ccode(voidPointer, "taskResult(",tp,")");
export taskKeepRunning(tp:taskPointer) ::= Ccode(int, "taskKeepRunning(",tp,")")==1;
export taskRunning(tp:taskPointer) ::= Ccode(int, "taskRunning(",tp,")")==1;
taskInterrupt(tp:taskPointer) ::= Ccode(void, "taskInterrupt(",tp,")");
taskAddCancelTask(tp:taskPointer, cancel:taskPointer) ::=Ccode(void, "addCancelTask(",tp,",",cancel,")");
taskAddStartTask(tp:taskPointer, start:taskPointer) ::=Ccode(void, "addStartTask(",tp,",",start,")");
taskAddDependency(tp:taskPointer, dep:taskPointer) ::=Ccode(void, "addDependency(",tp,",",dep,")");


startup(tb:TaskCellBody):null := (
     --warning wrong return type
     f := tb.fun; tb.fun = nullE;
     x := tb.arg; tb.arg = nullE;
     if notify then stderr << "--thread " << " started" << endl;
     --add thread to supervisor

     r := applyEE(f,x);
     when r is err:Error do (
	  printError(err);
	  if notify then stderr << "--thread " << " ended, after an error" << endl;
	  )
     else (
     	  tb.returnValue = r;
     	  if notify then stderr << "--thread " << " ready, result available " << endl;
	  );
     compilerBarrier();
     null());

isFunction(e:Expr):bool := (
     when e
     is CompiledFunction do true
     is CompiledFunctionClosure do true
     is FunctionClosure do true
     is s:SpecialExpr do isFunction(s.e)
     else false);

cancelTask(tb:TaskCellBody):Expr := (
     if tb.resultRetrieved then return buildErrorPacket("thread result already retrieved");
     if taskDone(tb.task) then (
	  if notify then stderr << "task done, cancellation not needed" << endl;
	  return nullE;
	  );
     taskInterrupt(tb.task);
     nullE);

cancelTask(e:Expr):Expr := when e is c:TaskCell do cancelTask(c.body) else WrongArg("a thread");
-- # typical value: cancelTask, Thread, Nothing
setupfun("cancelTask",cancelTask);

taskCellFinalizer(tc:TaskCell,p:null):void := (
     -- It is not safe to call any routines that depend on initialization of global variables here,
     -- because this finalizer may be called early, before all initialization is done.
     -- It is safe to write to stderr, because we've made output to it not depend on global variables being
     -- initialized.
     if taskDone(tc.body.task) then return;
     if notify then stderr << "--cancelling inaccessible thread " << endl;
     when cancelTask(tc.body) is err:Error do (printError(err);) else nothing);

header "#include <signal.h>";

createTask2(fun:Expr,arg:Expr):Expr :=(
     if !isFunction(fun) then return WrongArg(1,"a function");
     tc := TaskCell(TaskCellBody(Ccode(taskPointer,"((void *)0)"), false, fun, arg, nullE ));
     Ccode(void, "{ sigset_t s, old; sigemptyset(&s); sigaddset(&s,SIGINT); sigprocmask(SIG_BLOCK,&s,&old)");
     -- we are careful not to give the new thread the pointer tc, which we finalize:
     tc.body.task=taskCreate(startup,tc.body);
     Ccode(void, "sigprocmask(SIG_SETMASK,&old,NULL); }");
     Ccode(void, "GC_REGISTER_FINALIZER(",tc,",(GC_finalization_proc)",taskCellFinalizer,",0,0,0)");
     Expr(tc));

createTask(e:Expr):Expr := (
     when e is args:Sequence do
     if length(args) == 2 then createTask2(args.0,args.1)
     else WrongNumArgs(1,2)
     else createTask2(e,emptySequenceE));

setupfun("createTask",createTask);

addStartTask2(e1:Expr,e2:Expr):Expr := (
     when e1 is task:TaskCell do (
     when e2 is start:TaskCell do (
     taskAddStartTask(task.body.task,start.body.task); nullE)
     else WrongArg("Expect 2 tasks as arguments"))
     else WrongArg("Expect 2 tasks as arguments")
);

addStartTaskM2(e:Expr):Expr := (
     when e is args:Sequence do
     if length(args) == 2 then (addStartTask2(args.0,args.1); nullE)
     else WrongNumArgs(1,2)
     else WrongArg("Expect 2 tasks as arguments")
);
setupfun("addStartTask",addStartTaskM2);

addDependencyTask2(e1:Expr,e2:Expr):Expr := (
     when e1 is task:TaskCell do (
     when e2 is dep:TaskCell do (
     taskAddDependency(task.body.task,dep.body.task); nullE)
     else WrongArg("Expect 2 tasks as arguments"))
     else WrongArg("Expect 2 tasks as arguments")
);

addDependencyTaskM2(e:Expr):Expr := (
     when e is args:Sequence do
     if length(args) == 2 then (addDependencyTask2(args.0,args.1); nullE)
     else WrongNumArgs(1,2)
     else WrongArg("Expect 2 tasks as arguments")
);
setupfun("addDependencyTask",addDependencyTaskM2);


addCancelTask2(e1:Expr,e2:Expr):Expr := (
     when e1 is task:TaskCell do (
     when e2 is cancel:TaskCell do (
     taskAddCancelTask(task.body.task,cancel.body.task); nullE)
     else WrongArg("Expect 2 tasks as arguments"))
     else WrongArg("Expect 2 tasks as arguments")
);

addCancelTaskM2(e:Expr):Expr := (
     when e is args:Sequence do
     if length(args) == 2 then (addCancelTask2(args.0,args.1); nullE)
     else WrongNumArgs(1,2)
     else WrongArg("Expect 2 tasks as arguments")
);
setupfun("addCancelTask",addCancelTaskM2);


schedule2(fun:Expr,arg:Expr):Expr := (
     if !isFunction(fun) then return WrongArg(1,"a function");
     tc := TaskCell(TaskCellBody(Ccode(taskPointer,"((void *)0)"), false, fun, arg, nullE ));
     Ccode(void, "{ sigset_t s, old; sigemptyset(&s); sigaddset(&s,SIGINT); sigprocmask(SIG_BLOCK,&s,&old)");
     -- we are careful not to give the new thread the pointer tc, which we finalize:
     tc.body.task=taskCreatePush(startup,tc.body);
     Ccode(void, "sigprocmask(SIG_SETMASK,&old,NULL); }");
     Ccode(void, "GC_REGISTER_FINALIZER(",tc,",(GC_finalization_proc)",taskCellFinalizer,",0,0,0)");
     Expr(tc));

schedule1(task:TaskCell):Expr := (
     if taskStarted(task.body.task) then
     WrongArg("A task that hasn't started")
     else (
     pushTask(task.body.task); 
     Expr(task)
     )
);

schedule(e:Expr):Expr := (
     when e 
     is task:TaskCell do schedule1(task)
     is args:Sequence do
     if length(args) == 2 then schedule2(args.0,args.1)
     else WrongNumArgs(1,2)
     else schedule2(e,emptySequenceE));
-- # typical value: schedule, Function, Thread
-- # typical value: schedule, Function, Thing, Thread
setupfun("schedule",schedule);	   

taskResult(e:Expr):Expr := (
     when e is c:TaskCell do
     if c.body.resultRetrieved then buildErrorPacket("task result already retrieved")
     else if !taskKeepRunning(c.body.task) then buildErrorPacket("task canceled")
     else if !taskDone(c.body.task) then buildErrorPacket("task not done yet")
     else (
	  r := c.body.returnValue;
	  c.body.returnValue = nullE;
	  c.body.resultRetrieved = true;
	  c.body.task = nullTaskPointer();
	  r)
     else WrongArg("a task"));
-- # typical value: taskResult, Thread, Thing
setupfun("taskResult",taskResult);


setupfun("setIOSyncronized",setIOSyncronized);
setupfun("setIOUnSyncronized",setIOUnSyncronized);
setupfun("setIOExclusive",setIOExclusive);

-- Local Variables:
-- compile-command: "echo \"make: Entering directory \\`$M2BUILDDIR/Macaulay2/d'\" && make -C $M2BUILDDIR/Macaulay2/d pthread.o "
-- End:
