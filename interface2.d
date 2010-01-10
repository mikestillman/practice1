--		Copyright 2008, 2009 by Daniel R. Grayson

-- this file contains top level routines that call the C++ code in the engine

use C;
use system; 
use util;
use convertr;
use binding;
use nets;
use parser;
use lex;
use gmp;
use engine;
use util;
use tokens;
use err;
use stdiop;
use ctype;
use stdio;
use varstrin;
use strings;
use basic;
use struct;
use objects;
use evaluate;
use common;


-- straight line programs

export rawSLP(localInterpState:threadLocalInterp,e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) != 2 then WrongNumArgs(2)
     else when s.0 is M:RawMatrix do (
	  if !isSequenceOfSmallIntegers(s.1) then WrongArg(2,"a sequence of small integers") else
	  toExpr(Ccode(RawStraightLineProgramOrNull,
		    "(engine_RawStraightLineProgramOrNull)rawSLP(",
		    "(Matrix *)", M, ",",
		    "(M2_arrayint)", getSequenceOfSmallIntegers(s.1),
		    ")"
		    )))
     else WrongArgMatrix(1)
     else WrongNumArgs(2));
setupfun("rawSLP",rawSLP);

export rawEvaluateSLP(localInterpState:threadLocalInterp,e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) != 2 then WrongNumArgs(2)
     else when s.0 is slp:RawStraightLineProgram do (
	  when s.1 is M:RawMatrix do (
	       toExpr(Ccode(RawMatrixOrNull,
		    	 "(engine_RawMatrixOrNull)rawEvaluateSLP(",
		    	 "(StraightLineProgram *)", slp, ",",
		    	 "(Matrix *)", M,
		    	 ")"
		    	 )))
	  else WrongArgMatrix(1))
     else WrongArg(2,"a raw straight line program")
     else WrongNumArgs(2)
     );
setupfun("rawEvaluateSLP",rawEvaluateSLP);

export rawPathTrackerPrecookedSLPs(localInterpState:threadLocalInterp,e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) != 2 then WrongNumArgs(2)
     else when s.0 is slp1:RawStraightLineProgram do 
     	  when s.1 is slp2:RawStraightLineProgram do
	       toExpr(Ccode(RawPathTrackerOrNull,
			"(engine_RawPathTrackerOrNull)rawPathTrackerPrecookedSLPs(",
	       	   	"(StraightLineProgram *)", slp1, ",",
		   	"(StraightLineProgram *)", slp2, 
		   	")"
		    ))
     	  else WrongArg(2,"a raw straight line program")
     	  else WrongArg(1,"a raw straight line program")
     else WrongNumArgs(2)
     );
setupfun("rawPathTrackerPrecookedSLPs",rawPathTrackerPrecookedSLPs);

export rawPathTracker(localInterpState:threadLocalInterp,e:Expr):Expr := (
     when e is HH:RawMatrix do 
		toExpr(Ccode(RawPathTrackerOrNull,
		    "(engine_RawPathTrackerOrNull)rawPathTracker(",
		    "(Matrix *)", HH, 
		    ")"
		    ))
     else WrongArgMatrix()
     );
setupfun("rawPathTracker",rawPathTracker);

export rawSetParametersPT(localInterpState:threadLocalInterp,e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) != 12 then WrongNumArgs(11)
     else when s.0 is PT:RawPathTracker do 
	  when s.1 is isProj:Boolean do
	  when s.2 is initDt:RR do
	  when s.3 is minDt:RR do
	  when s.4 is dtIncreaseFactor:RR do 
	  when s.5 is dtDecreeaseFactor:RR do
	  when s.6 is numSuccessesBeforeIncrease:ZZ do
	  when s.7 is epsilon:RR do 
	  when s.8 is maxCorrSteps:ZZ do
	  when s.9 is endZoneFactor:RR do 
	  when s.10 is infinityThreshold:RR do 
	  when s.11 is predType:ZZ do 
	  (
	       Ccode(void,
		    	 "rawSetParametersPT(",
		    	 "(PathTracker *)", PT, ",",
			 toBoolean(s.1),",",
			 "(M2_RRR)", initDt,",",
			 "(M2_RRR)", minDt,",",
			 "(M2_RRR)", dtIncreaseFactor,",",
			 "(M2_RRR)", dtDecreeaseFactor,",",
			 toInt(s.6),",",
			 "(M2_RRR)", epsilon,",",
			 toInt(s.8),",",
			 "(M2_RRR)", endZoneFactor,",",
			 "(M2_RRR)", infinityThreshold,",",
			 toInt(s.11),
		    	 ")"
		    	 );
	       nullE)
	  else WrongArgZZ(12)
	  else WrongArgRR(11)
	  else WrongArgRR(10)
	  else WrongArgZZ(9)
	  else WrongArgRR(8)
	  else WrongArgZZ(7)
	  else WrongArgRR(6)
	  else WrongArgRR(5)
	  else WrongArgRR(4)
	  else WrongArgRR(3)
	  else WrongArgBoolean(2)
     	  else WrongArg(1,"a path tracker")
     else WrongNumArgs(12)
     );
setupfun("rawSetParametersPT",rawSetParametersPT);

export rawLaunchPT(localInterpState:threadLocalInterp,e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) != 2 then WrongNumArgs(2)
     else when s.0 is PT:RawPathTracker do 
	  when s.1 is startSols:RawMatrix do (
	       Ccode(void,
		    	 "rawLaunchPT(",
		    	 "(PathTracker *)", PT, ",",
	                 "(Matrix *)", startSols, 
		    	 ")"
	       );
	       nullE)
	  else WrongArgMatrix(2)
     	  else WrongArg(1,"a path tracker")
     else WrongNumArgs(2)
     );
setupfun("rawLaunchPT",rawLaunchPT);

export rawGetSolutionPT(localInterpState:threadLocalInterp,e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) != 2 then WrongNumArgs(2)
     else when s.0 is PT:RawPathTracker do 
          when s.1 is solN:ZZ do 
		toExpr(Ccode(RawMatrixOrNull,
		    "(engine_RawMatrixOrNull)rawGetSolutionPT(",
		    "(PathTracker *)", PT, ",",
		    toInt(s.1), 		    
		    ")"
		    ))
          else WrongArgZZ(2) 
          else WrongArg(1,"a path tracker")
     else WrongNumArgs(2)
     );
setupfun("rawGetSolutionPT",rawGetSolutionPT);

export rawGetAllSolutionsPT(localInterpState:threadLocalInterp,e:Expr):Expr := (
     when e is PT:RawPathTracker  do 
		toExpr(Ccode(RawMatrixOrNull,
		    "(engine_RawMatrixOrNull)rawGetAllSolutionsPT(",
		    "(PathTracker *)", PT, 
		    ")"
		    ))
     else WrongArg("a path tracker")
     );
setupfun("rawGetAllSolutionsPT",rawGetAllSolutionsPT);

export rawGetSolutionStatusPT(localInterpState:threadLocalInterp,e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) != 2 then WrongNumArgs(2)
     else when s.0 is PT:RawPathTracker do 
          when s.1 is solN:ZZ do 
		toExpr(Ccode(int,
		    "rawGetSolutionStatusPT(",
		    "(PathTracker *)", PT, ",",
		    toInt(s.1), 		    
		    ")"
		    ))
          else WrongArgZZ(2) 
          else WrongArg(1,"a path tracker")
     else WrongNumArgs(2)
     );
setupfun("rawGetSolutionStatusPT",rawGetSolutionStatusPT);

export rawGetSolutionLastTvaluePT(localInterpState:threadLocalInterp,e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) != 2 then WrongNumArgs(2)
     else when s.0 is PT:RawPathTracker do 
          when s.1 is solN:ZZ do 
		toExpr(Ccode(RRorNull, "(engine_RRorNull)rawGetSolutionLastTvaluePT(",
		    "(PathTracker *)", PT, ",",
		    toInt(s.1), 		    
		    ")"
		    ))
          else WrongArgZZ(2) 
          else WrongArg(1,"a path tracker")
     else WrongNumArgs(2)
     );
setupfun("rawGetSolutionLastTvaluePT",rawGetSolutionLastTvaluePT);

export rawGetSolutionStepsPT(localInterpState:threadLocalInterp,e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) != 2 then WrongNumArgs(2)
     else when s.0 is PT:RawPathTracker do 
          when s.1 is solN:ZZ do 
		toExpr(Ccode(int,
		    "rawGetSolutionStepsPT(",
		    "(PathTracker *)", PT, ",",
		    toInt(s.1), 		    
		    ")"
		    ))
          else WrongArgZZ(2) 
          else WrongArg(1,"a path tracker")
     else WrongNumArgs(2)
     );
setupfun("rawGetSolutionStepsPT",rawGetSolutionStepsPT);

export rawGetSolutionRcondPT(localInterpState:threadLocalInterp,e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) != 2 then WrongNumArgs(2)
     else when s.0 is PT:RawPathTracker do 
          when s.1 is solN:ZZ do 
		toExpr(Ccode(RRorNull, "(engine_RRorNull)rawGetSolutionRcondPT(",
		    "(PathTracker *)", PT, ",",
		    toInt(s.1), 		    
		    ")"
		    ))
          else WrongArgZZ(2) 
          else WrongArg(1,"a path tracker")
     else WrongNumArgs(2)
     );
setupfun("rawGetSolutionRcondPT",rawGetSolutionRcondPT);

export rawRefinePT(localInterpState:threadLocalInterp,e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) != 4 then WrongNumArgs(4)
     else when s.0 is PT:RawPathTracker do 
	  when s.1 is sols:RawMatrix do 
	  when s.2 is tolerance:RR do
          when s.3 is maxSteps:ZZ do 
	       toExpr(Ccode(RawMatrixOrNull,
		    "(engine_RawMatrixOrNull)rawRefinePT(",
		    	 "(PathTracker *)", PT, ",",
	                 "(Matrix *)", sols, ",",
			 "(M2_RRR)", tolerance,",",
                         toInt(s.3),
		    	 ")"
	       ))
          else WrongArgZZ(4)
          else WrongArgRR(3)
	  else WrongArgMatrix(2)
     	  else WrongArg(1,"a path tracker")
     else WrongNumArgs(4)
     );
setupfun("rawRefinePT",rawRefinePT);

