--		Copyright 1994-2006,2010 by Daniel R. Grayson

use atomic;
use arithmetic;

export threadLocal interruptShield := false;
export threadLocal interruptPending := false;
export threadLocal alarmedFlag := false;
export threadLocal steppingFlag := false;


import threadLocal interruptedFlag:atomicField;


import threadLocal exceptionFlag:atomicField; -- indicates interrupt, stepping, or alarm

header "extern int libfac_interruptflag;"; -- declared in libfac/factor/version.cc, but not exported, with C++ linkage
getinterruptflag() ::= Ccode(int,"libfac_interruptflag");
setinterruptflag(n:int) ::= Ccode(void,"libfac_interruptflag = ",n,"");
export determineExceptionFlag():void := (
     store(exceptionFlag, test(interruptedFlag) || steppingFlag || alarmedFlag);
     setinterruptflag(int(load(interruptedFlag)));
     );
header "#include <unistd.h>";
alarm(x:uint) ::= Ccode(int,"alarm(",x,")");
export clearAlarm():void := alarm(uint(0));
export clearAllFlags():void := (
     setinterruptflag(0);
     store(exceptionFlag, false);
     compilerBarrier();
     store(interruptedFlag, false);
     steppingFlag = false;
     alarmedFlag = false;
     interruptPending = false;
     );
export setInterruptFlag():void := (
     --note ordering here, interrupt flag, then exception flag, then libfac interrupt flag. 
     store(interruptedFlag, true);
     --compiler barrier necessary to disable compiler reordering.  
     --On architectures that do not enforce memory write ordering, emit a memory barrier
     compilerBarrier();
     store(exceptionFlag, true);
     setinterruptflag(1);
     );
export setAlarmedFlag():void := (
     store(interruptedFlag, true); -- an alarm is an interrupt, as far as the engine is concerned
     alarmedFlag = true;
     store(exceptionFlag, true);
     );
export setSteppingFlag():void := (
     steppingFlag = true;
     store(exceptionFlag, true);
     );
export clearInterruptFlag():void := (
     --reverse previous order when undoing set.  
     setinterruptflag(0);
     store(interruptedFlag, false);
     compilerBarrier();
     determineExceptionFlag();
     );
export clearAlarmedFlag():void := (
     store(interruptedFlag, false);
     alarmedFlag = false;
     determineExceptionFlag();
     );
export stepCount := -1;
export microStepCount := -1;
export clearSteppingFlag():void := (
     stepCount = -1;
     microStepCount = -1;
     steppingFlag = false;
     determineExceptionFlag();
     );

-- Local Variables:
-- compile-command: "echo \"make: Entering directory \\`$M2BUILDDIR/Macaulay2/d'\" && echo \"make: Entering directory \\`$M2BUILDDIR/Macaulay2/d'\" && make -C $M2BUILDDIR/Macaulay2/d interrupts.o "
-- End:
