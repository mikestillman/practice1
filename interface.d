--		Copyright 1994-2004 by Daniel R. Grayson

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

-----------------------------------------------------------------------------
-- debugging

export rawGetErrorMessage(e:Expr):Expr := (
     s := Ccode(string, "(string)IM2_last_error_message()");
     if length(s) == 0 then nullE else Expr(s));
setupfun("rawGetErrorMessage",rawGetErrorMessage);

-----------------------------------------------------------------------------
-- random numbers

export rawSetRandomSeed(e:Expr):Expr := (
     when e is n:Integer do Expr(toInteger(Ccode(int, "rawSetRandomSeed(", "(M2_Integer)", n, ")")))
     else WrongArgInteger());
setupfun("rawSetRandomSeed",rawSetRandomSeed);

export rawSetRandomMax(e:Expr):Expr := (
     when e is n:Integer do (
	  Ccode(void,
	       "rawSetRandomMax(", 
	       "(M2_Integer)", n,
	       ")");
	  nullE)
     else WrongArgInteger());
setupfun("rawSetRandomMax",rawSetRandomMax);

export rawRandomInteger(e:Expr):Expr := (
     when e is n:Integer do Expr(Ccode(Integer, "(gmp_Integer)rawRandomInteger(", "(M2_Integer)", n, ")"))
     else WrongArgInteger());
setupfun("rawRandomInteger",rawRandomInteger);

-----------------------------------------------------------------------------
-- monomials

export rawVarMonomial(a:Expr):Expr := (
     when a
     is v:Integer do 
     if isInt(v) then toExpr(
	  Ccode(RawMonomialOrNull, 
	       "(engine_RawMonomialOrNull)rawVarMonomial(", toInt(v), ",1)" ))
     else WrongArgSmallInteger()
     is s:Sequence do 
     if length(s) == 2 then 
     when s.0 is v:Integer do 
     if isInt(v) then 
     when s.1 is e:Integer do 
     if isInt(e) then toExpr(Ccode(RawMonomialOrNull, 
	       "(engine_RawMonomialOrNull)rawVarMonomial(",
	       toInt(v), ",", toInt(e), ")" ))
     else WrongArgSmallInteger(2)
     else WrongArgInteger(2)
     else WrongArgSmallInteger(1)
     else WrongArgInteger(1)
     else WrongArg("an integer or a pair of integers")
     else WrongArg("an integer or a pair of integers")
     );
setupfun("rawVarMonomial",rawVarMonomial);

export rawSparseListFormMonomial(e:Expr):Expr := (
     when e 
     is x:RawMonomial do (
	  y := Ccode(RawArrayInt, "(engine_RawArrayInt)rawSparseListFormMonomial((Monomial*)",x,")" );
	  n := length(y)/2;
	  list(new Sequence len n do (
		    for i from length(y)-2 to 0 by -2 do (  -- we reverse the engine order
			 provide new Sequence len 2 do (
			      provide Expr(toInteger(y.i));
			      provide Expr(toInteger(y.(i+1))))))))
     else WrongArg("a raw monomial")
     );
setupfun("rawSparseListFormMonomial",rawSparseListFormMonomial);

export rawMakeMonomial(e:Expr):Expr := (
     -- accepts a list of pairs : {(2, 1), (3, 7), (5, 4)}
     -- we reverse the list before giving it to the engine
     when e
     is l:List do (
	  when isSequenceOfPairsOfSmallIntegers(l.v) is s:string do return WrongArg(s) else nothing;
	  m := Ccode(RawMonomialOrNull, 
	       "(engine_RawMonomialOrNull)rawMakeMonomial(",
	          "(M2_arrayint)", getReverseSequenceOfPairsOfSmallIntegers(l.v), 
	       ")" );
	  when m
	  is x:RawMonomial do Expr(x)
	  is null do buildErrorPacket(EngineError("raw monomial overflow"))
	  )
     else WrongArg("a list of pairs of integers"));
setupfun("rawMakeMonomial",rawMakeMonomial);

export rawMonomialIsOne(e:Expr):Expr := (
     when e is s:Sequence 
     do if length(s) == 2 
     then when s.0 is x:RawMonomial 
     do when s.1 is t:Integer 
     do if t === 1 
     then if Ccode(bool, "rawMonomialIsOne((Monomial*)",x,")") then True else False
     else WrongArg(2,"the integer 1")
     else WrongArgInteger(2)
     else WrongArg(1,"a raw monomial")
     else WrongNumArgs(2)
     else WrongNumArgs(2));
installMethod(Expr(EqualEqualS), rawMonomialClass,integerClass, Expr(CompiledFunction(rawMonomialIsOne,nextHash())));

export rawCompareMonomial(e:Expr):Expr := (
     when e is s:Sequence do if length(s) != 3 then WrongNumArgs(3) else
     when s.0 is M:RawMonoid do
     when s.1 is x:RawMonomial do
     when s.2 is y:RawMonomial do (
	  r := Ccode(int, "rawCompareMonomial((Monoid *)", M,",(Monomial *)", x, ",(Monomial *)", y, ")");
	  if r < 0 then LessE else if r > 0 then GreaterE else EqualEqualE
	  )
     else WrongArg(3,"a raw monomial")
     else WrongArg(2,"a raw monomial")
     else WrongArg(1,"a raw monoid")
     else WrongNumArgs(3));
setupfun("rawCompareMonomial",rawCompareMonomial);

export rawMonomialDivides(e:Expr):Expr := (
     when e is s:Sequence do if length(s) != 3 then WrongNumArgs(3) else
     when s.0 is M:RawMonoid do
     when s.1 is x:RawMonomial do
     when s.2 is y:RawMonomial do toExpr(Ccode(bool, "rawMonomialDivides((Monoid *)", M,",(Monomial *)", x, ",(Monomial *)", y, ")"))
     else WrongArg(3,"a raw monomial")
     else WrongArg(2,"a raw monomial")
     else WrongArg(1,"a raw monoid")
     else WrongNumArgs(3));
setupfun("rawMonomialDivides",rawMonomialDivides);

export rawMonomialDivide(e:Expr):Expr := (
     when e is s:Sequence do if length(s) != 3 then WrongNumArgs(3) else
     when s.0 is M:RawMonoid do
     when s.1 is x:RawMonomial do
     when s.2 is y:RawMonomial do toExpr(Ccode(RawMonomialOrNull, "(engine_RawMonomialOrNull)rawMonomialDivide((Monoid *)", M,",(Monomial *)", x, ",(Monomial *)", y, ")"))
     else WrongArg(3,"a raw monomial")
     else WrongArg(2,"a raw monomial")
     else WrongArg(1,"a raw monoid")
     else WrongNumArgs(3));
setupfun("rawMonomialDivides",rawMonomialDivides);

export rawRadical(e:Expr):Expr := (
     when e
     is x:RawMonomial do Expr(Ccode(RawMonomial, "(engine_RawMonomial)", "rawRadicalMonomial(", "(Monomial*)", x, ")" ) )
     is I:RawMonomialIdeal do Expr( Ccode(RawMonomialIdeal, "(engine_RawMonomialIdeal)", "rawRadicalMonomialIdeal(", "(MonomialIdeal *)", I, ")" ) )
     else WrongArg("a raw monomial or monomial ideal"));
setupfun("rawRadical",rawRadical);

export rawGCD(e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) != 2 then WrongNumArgs(2) else
     when s.0 is x:RawMonomial do (
     	  when s.1 is y:RawMonomial do Expr(Ccode(RawMonomial, "(engine_RawMonomial)rawGCD((Monomial*)",x,",","(Monomial*)",y,")"))
     	  else WrongArg(2,"a raw monomial"))
     is x:RawRingElement do (
     	  when s.1 is y:RawRingElement do toExpr(Ccode(RawRingElementOrNull, "(engine_RawRingElementOrNull)rawGCDRingElement((RingElement *)",x,",","(RingElement *)",y,")"))
     	  else WrongArg(2,"a raw ring element"))
     else WrongArg(1,"a raw monomial or ring element")
     else WrongNumArgs(2)
     );
setupfun("rawGCD",rawGCD);

export rawExtendedGCD(e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) != 2 then WrongNumArgs(2) else
     when s.0 is x:RawRingElement do (
     	  when s.1 is y:RawRingElement do (
	       a := x;
	       b := y;
	       ret := Ccode(RawRingElementOrNull, 
		    "(engine_RawRingElementOrNull)rawExtendedGCDRingElement(",
		    "(RingElement *)", x,",",
		    "(RingElement *)", y,",",
		    "(RingElement **)&", a,",",
		    "(RingElement **)&", b,
		    ")");
	       when ret is h:RawRingElement do Expr(Sequence(Expr(h),Expr(a),Expr(b)))
	       else engineErrorMessage()
	       )
     	  else WrongArg(2,"a raw ring element"))
     else WrongArg(1,"a raw ring element")
     else WrongNumArgs(2)
     );
setupfun("rawExtendedGCD",rawExtendedGCD);

export rawLCM(e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) != 2 then WrongNumArgs(2) else
     when s.0 is x:RawMonomial do
     when s.1 is y:RawMonomial do Expr(
	       Ccode(RawMonomial, "(engine_RawMonomial)rawLCM((Monomial*)",x,",","(Monomial*)",y,")"))
     else WrongArg(2,"a raw monomial")
     else WrongArg(1,"a raw monomial")
     else WrongArg("a pair of raw monomials")
     );
setupfun("rawLCM",rawLCM);

export rawSaturate(e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) != 2 then WrongNumArgs(2) else
     when s.0 is x:RawMonomial do
     when s.1 is y:RawMonomial do Expr(
	       Ccode(RawMonomial, "(engine_RawMonomial)rawSaturateMonomial((Monomial*)",x,",","(Monomial*)",y,")"))
     else WrongArg(2,"a raw monomial")
     else when s.0 is I:RawMonomialIdeal do 
     when s.1
     is y:RawMonomial do Expr(
	  Ccode(RawMonomialIdeal, "(engine_RawMonomialIdeal)rawSaturateMonomialIdeal1(",
	       "(MonomialIdeal *)", I, ",", "(Monomial *)", y, ")" ))
     is J:RawMonomialIdeal do toExpr(
	  Ccode(RawMonomialIdealOrNull, "(engine_RawMonomialIdealOrNull)rawSaturateMonomialIdeal2(",
	       "(MonomialIdeal *)", I, ",", "(MonomialIdeal *)", J, ")" ))
     else WrongArg(2,"a raw monomial or monomial ideal")
     else WrongArg(1,"a raw monomial or monomial ideal")
     else WrongNumArgs(2)
     );
setupfun("rawSaturate",rawSaturate);

export rawSyzygy(e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) != 2 then WrongNumArgs(2) else
     when s.0 is x:RawMonomial do
     when s.1 is y:RawMonomial do (
	  r := Ccode(RawMonomialPair, "(engine_RawMonomialPair)rawSyzygy((Monomial*)", x,",","(Monomial*)",y, ")");
	  Expr(Sequence(Expr(r.a), Expr(r.b))))
     else WrongArg(2,"a raw monomial")
     else WrongArg(1,"a raw monomial")
     else WrongArg("a pair of raw monomials")
     );
setupfun("rawSyzygy",rawSyzygy);

export rawColon(e:Expr):Expr := (
     when e is a:Sequence do 
     if length(a) == 2 then 
     when a.0 is x:RawMonomial do 
     when a.1 is y:RawMonomial do toExpr( Ccode(RawMonomialOrNull, "(engine_RawMonomialOrNull)rawColonMonomial(", "(Monomial *)", x, ",", "(Monomial *)", y, ")" ))
     else WrongArg(2,"a raw monomial")
     else when a.0 is I:RawMonomialIdeal do 
     when a.1
     is y:RawMonomial do Expr( Ccode(RawMonomialIdeal, "(engine_RawMonomialIdeal)rawColonMonomialIdeal1(", "(MonomialIdeal *)", I, ",", "(Monomial *)", y, ")" ))
     is J:RawMonomialIdeal do toExpr( Ccode(RawMonomialIdealOrNull, "(engine_RawMonomialIdealOrNull)rawColonMonomialIdeal2(", "(MonomialIdeal *)", I, ",", "(MonomialIdeal *)", J, ")" ))
     else WrongArg(2,"a raw monomial or monomial ideal")
     else WrongArg(1,"a raw monomial or monomial ideal")
     else WrongNumArgs(2)
     else WrongNumArgs(2));
setupfun("rawColon",rawColon);

-----------------------------------------------------------------------------
-- monomial orderings

PositionS := makeProtectedSymbolClosure("Position");
UpS := makeProtectedSymbolClosure("Up");
DownS := makeProtectedSymbolClosure("Down");
PositionMO(b:bool):RawMonomialOrdering := (		    -- b is true for Up, false for Down
     Ccode(RawMonomialOrdering, "(engine_RawMonomialOrdering)rawPositionMonomialOrdering(",b,")")
     );

LexS      := makeProtectedSymbolClosure("Lex");
LexSmallS := makeProtectedSymbolClosure("LexSmall");
LexTinyS  := makeProtectedSymbolClosure("LexTiny");
LexMO     (n:int):RawMonomialOrdering := Ccode(RawMonomialOrdering, "(engine_RawMonomialOrdering)rawLexMonomialOrdering(",n,",1)");
LexSmallMO(n:int):RawMonomialOrdering := Ccode(RawMonomialOrdering, "(engine_RawMonomialOrdering)rawLexMonomialOrdering(",n,",2)");
LexTinySMO (n:int):RawMonomialOrdering := Ccode(RawMonomialOrdering, "(engine_RawMonomialOrdering)rawLexMonomialOrdering(",n,",4)");

RevLexS := makeProtectedSymbolClosure("RevLex");
RevLexMO(n:int):RawMonomialOrdering := Ccode(RawMonomialOrdering, "(engine_RawMonomialOrdering)rawRevLexMonomialOrdering(",n,")");

GroupLexS := makeProtectedSymbolClosure("GroupLex");
GroupLexMO(n:int):RawMonomialOrdering := Ccode(RawMonomialOrdering, "(engine_RawMonomialOrdering)rawGroupLexMonomialOrdering(",n,")");

GroupRevLexS := makeProtectedSymbolClosure("GroupRevLex");
GroupRevLexMO(n:int):RawMonomialOrdering := Ccode(RawMonomialOrdering, "(engine_RawMonomialOrdering)rawGroupRevLexMonomialOrdering(",n,")");

NCLexS := makeProtectedSymbolClosure("NCLex");
NCLexMO(n:int):RawMonomialOrdering := Ccode(RawMonomialOrdering, "(engine_RawMonomialOrdering)rawNClexMonomialOrdering(",n,")");

GRevLexS := makeProtectedSymbolClosure("GRevLex");
GRevLexSmallS := makeProtectedSymbolClosure("GRevLexSmall");
GRevLexTinyS := makeProtectedSymbolClosure("GRevLexTiny");
GRevLexMO(n:array(int)):RawMonomialOrdering := Ccode(RawMonomialOrdering, "(engine_RawMonomialOrdering)rawGRevLexMonomialOrdering((M2_arrayint)",n,",1)");
GRevLexSmallMO(n:array(int)):RawMonomialOrdering := Ccode(RawMonomialOrdering, "(engine_RawMonomialOrdering)rawGRevLexMonomialOrdering((M2_arrayint)",n,",2)");
GRevLexTinySMO(n:array(int)):RawMonomialOrdering := Ccode(RawMonomialOrdering, "(engine_RawMonomialOrdering)rawGRevLexMonomialOrdering((M2_arrayint)",n,",4)");

WeightsS := makeProtectedSymbolClosure("Weights");
WeightsMO(n:array(int)):RawMonomialOrdering := Ccode(RawMonomialOrdering, "(engine_RawMonomialOrdering)rawWeightsMonomialOrdering((M2_arrayint)",n,")");

joinMO(s:RawMonomialOrderingArray):RawMonomialOrdering := ( Ccode(RawMonomialOrdering, "(engine_RawMonomialOrdering)rawJoinMonomialOrdering((MonomialOrdering_array)",s,")") );

arrayint := array(int);
funtype := fun1 or fun2 or fun3 or fun4;
funtypeornull := fun1 or fun2 or fun3 or fun4 or null;
fun1 := function():RawMonomialOrdering;
fun2 := function(int):RawMonomialOrdering;
fun3 := function(arrayint):RawMonomialOrdering;;
fun4 := function(bool):RawMonomialOrdering;;

Maker := { sym:SymbolClosure, fun:funtype };

makers := array(Maker)(
     Maker(PositionS,PositionMO),
     Maker(LexS,LexMO),
     Maker(LexSmallS,LexSmallMO),
     Maker(LexTinyS,LexTinySMO),
     Maker(RevLexS,RevLexMO),
     Maker(GroupLexS,GroupLexMO),
     Maker(GroupRevLexS,GroupRevLexMO),
     Maker(NCLexS,NCLexMO),
     Maker(GRevLexS,GRevLexMO),
     Maker(GRevLexSmallS,GRevLexSmallMO),
     Maker(GRevLexTinyS,GRevLexTinySMO),
     Maker(WeightsS,WeightsMO)
     );

getmaker(sym:SymbolClosure):funtypeornull := (
     foreach pair in makers do if sym == pair.sym then (
	  when pair.fun
	  is f:fun1 do return f
	  is f:fun2 do return f
	  is f:fun3 do return f
     	  is f:fun4 do return f
	  );
     null());

-- trivialMonomial := Ccode(RawMonomial, 
--      "(engine_RawMonomial)rawMakeMonomial(", "(M2_arrayint)", array(int)(), ")" 
--      );

export rawMonomialOrdering(e:Expr):Expr := (
     -- This routine gets an expression like this:
     -- { GRevLexSmallS => {1,2,3}, PositionS, LexTinyS => 4, LexS => 5, WeightsS => {1,2,3} }
     -- For GRevLexS, the weights are already provided by top level code.
     -- Each member of the sequence results in one monomial ordering, and the sequence
     -- is then "joined".
     -- The weights for grevlex have to be > 0.
     -- Limit the total number of variables to 2^15-1.
     when e is s:List do (
	  -- first check it
	  foreach spec in s.v do (
	       when spec is sp:List do
	       if sp.class == optionClass && length(sp.v)==2 then 
	       when sp.v.0 
	       is sym:SymbolClosure do (
		    when getmaker(sym)
		    is g:fun1 do (
			 if sp.v.1 != nullE
			 then return buildErrorPacket("expected option value to be 'null'");
			 )
		    is g:fun2 do (
			 if !isSmallInt(sp.v.1)
			 then return buildErrorPacket("expected option value to be a small integer");
			 )
		    is g:fun3 do (
			 if !isSequenceOfSmallIntegers(sp.v.1)
			 then return buildErrorPacket("expected option value to be a sequence of small integers");
			 )
		    is g:fun4 do (
			 if g == PositionMO then (
			      if !(sp.v.1 == UpS || sp.v.1 == DownS)
			      then return buildErrorPacket("expected option value to be Up or Down");
			      )
			 else (
			      if !(sp.v.1 == True || sp.v.1 == False)
			      then return buildErrorPacket("expected option value to be true or false");
			      ))
		    is null do return buildErrorPacket("expected option key '"+sym.symbol.word.name+"' to be a monomial ordering key")
		    )
	       else return buildErrorPacket("expected option key to be a symbol")
	       else return WrongArg("a list of options")
	       else return WrongArg("a list of options"));
	  -- then accumulate it
     	  Expr(joinMO(new RawMonomialOrderingArray len length(s.v) do (
	       foreach spec in s.v do
	       when spec is sp:List do
	       when sp.v.0 is sym:SymbolClosure do (
		    when getmaker(sym)
		    is g:fun1 do provide g()
		    is g:fun2 do provide g(getSmallInt(sp.v.1))
		    is g:fun3 do provide g(getSequenceOfSmallIntegers(sp.v.1))
		    is g:fun4 do provide g(if g == PositionMO then sp.v.1 == UpS else sp.v.1 == True)
		    is null do nothing
		    )
	       else nothing
	       else nothing;
	       provide PositionMO(true);		    -- just in case, to prevent a loop
	       ))))
     else WrongArg("a list of options"));
setupfun("rawMonomialOrdering",rawMonomialOrdering);

export rawProductMonomialOrdering(e:Expr):Expr := (
     when e
     is m:RawMonomialOrdering do e
     is s:Sequence do 
     if !isSequenceOfMonomialOrderings(s) 
     then WrongArg("a sequence of raw monomial orderings") 
     else Expr(Ccode(
	       RawMonomialOrdering, 
	       "(engine_RawMonomialOrdering)rawProductMonomialOrdering(",
	       "(MonomialOrdering_array)", getSequenceOfMonomialOrderings(s),
	       ")"
	       ))
     else WrongArg("a sequence of raw monomial orderings"));
setupfun("rawProductMonomialOrdering",rawProductMonomialOrdering);

export rawNumberOfVariables(e:Expr):Expr := (
     when e
     is m:RawMonomialOrdering do toExpr(Ccode( int, "rawNumberOfVariables(", "(MonomialOrdering *)", m, ")" ))
     else WrongArg("a monomial ordering"));
setupfun("rawNumberOfVariables",rawNumberOfVariables);

export rawNumberOfInvertibleVariables(e:Expr):Expr := (
     when e
     is m:RawMonomialOrdering do toExpr(Ccode( int, "rawNumberOfInvertibleVariables(", "(MonomialOrdering *)", m, ")" ))
     else WrongArg("a monomial ordering"));
setupfun("rawNumberOfInvertibleVariables",rawNumberOfInvertibleVariables);

-----------------------------------------------------------------------------
-- monoids

export rawMonoid(mo:RawMonomialOrdering,names:array(string),degreesRing:RawRing,degs:array(int)):Expr := (
     when Ccode(RawMonoidOrNull, 
	  "(engine_RawMonoidOrNull)IM2_Monoid_make(",
	      "(MonomialOrdering *)", mo, ",",
	      "(M2_stringarray)", names, ",",
	      "(Ring *)", degreesRing, ",",
	      "(M2_arrayint)", degs,
	  ")")
     is m:RawMonoid do Expr(m)
     is null do buildErrorPacket(EngineError("internal error: unexplained failure to make raw monoid"))
     );
export rawMonoid(e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) == 0 then Expr(Ccode(RawMonoid,"(engine_RawMonoid)IM2_Monoid_trivial()"))
     else if length(s) == 4 then 
     when s.0 is mo:RawMonomialOrdering do
     if isSequenceOfStrings(s.1) then (
	  names := getSequenceOfStrings(s.1);
	  when s.2 is degreesRing:RawRing do
	  if isSequenceOfSmallIntegers(s.3) then (
	       degs := getSequenceOfSmallIntegers(s.3);
	       rawMonoid(mo,names,degreesRing,degs))
	  else WrongArg(4,"a sequence of small integers (flattened degrees)")
	  else WrongArg(3,"the degrees ring"))
     else WrongArg(2,"a sequence of strings to be used as names")
     else WrongArg(1,"a monomial ordering")
     else buildErrorPacket("expected 0 or 4 arguments")
     else buildErrorPacket("expected 0 or 4 arguments")
     );
setupfun("rawMonoid",rawMonoid);

-----------------------------------------------------------------------------
-- rings

export rawZZ(e:Expr):Expr := (
     when e is s:Sequence do if length(s) == 0
     then Expr(Ccode(RawRing,"(engine_RawRing)IM2_Ring_ZZ()"))
     else WrongNumArgs(0)
     else WrongNumArgs(0)
     );
setupfun("rawZZ", rawZZ);

export rawQQ(e:Expr):Expr := (
     when e is s:Sequence do if length(s) == 0
     then Expr(Ccode(RawRing,"(engine_RawRing)IM2_Ring_QQ()"))
     else WrongNumArgs(0)
     else WrongNumArgs(0)
     );
setupfun("rawQQ", rawQQ);

export rawZZp(e:Expr):Expr := (
     when e is p:Integer do if !isInt(p) then WrongArgSmallInteger(1) else toExpr(Ccode(RawRingOrNull, "(engine_RawRingOrNull)IM2_Ring_ZZp(", toInt(p), ")" ))
     else WrongArgInteger());
setupfun("rawZZp", rawZZp);

export rawRR(e:Expr):Expr := (
     when e is epsilon:Real do toExpr(Ccode(RawRingOrNull, "(engine_RawRingOrNull)IM2_Ring_RR(", epsilon.v, ")" ))
     else WrongArg("a real number"));
setupfun("rawRR",rawRR);

export rawCC(e:Expr):Expr := (
     when e is epsilon:Real do toExpr(Ccode(RawRingOrNull, "(engine_RawRingOrNull)IM2_Ring_CC(", epsilon.v, ")" ))
     else WrongArg("a real number"));
setupfun("rawCC",rawCC);

export rawRRR(e:Expr):Expr := when e is s:Sequence do if length(s) != 0 then WrongNumArgs(0) else toExpr(Ccode(RawRingOrNull, "(engine_RawRingOrNull)IM2_Ring_RRR()" )) else WrongNumArgs(0);
setupfun("rawRRR",rawRRR);

export rawCCC(e:Expr):Expr := when e is s:Sequence do if length(s) != 0 then WrongNumArgs(0) else toExpr(Ccode(RawRingOrNull, "(engine_RawRingOrNull)IM2_Ring_CCC()" )) else WrongNumArgs(0);
setupfun("rawCCC",rawCCC);

export rawIndexIfVariable(e:Expr):Expr := (
     when e is f:RawRingElement do (
	  i := Ccode(int, "IM2_RingElement_index_if_var(", "(RingElement *)", f, ")" );
	  if i == -1 then nullE else toExpr(i))
     else WrongArg("a raw ring element"));
setupfun("rawIndexIfVariable",rawIndexIfVariable);

export rawIndices(e:Expr):Expr := (
     when e is f:RawRingElement do toExpr(Ccode(array(int), "(engine_RawArrayInt)IM2_RingElement_indices(", "(RingElement *)", f, ")" ))
     else WrongArg("a raw ring element"));
setupfun("rawIndices",rawIndices);

export rawPolynomialRing(e:Expr):Expr := (
     when e is a:Sequence do 
     if length(a) == 0 then Expr(Ccode( RawRing, "(engine_RawRing)IM2_Ring_trivial_polyring()" ))
     else if length(a) == 2 then 
     when a.0 is K:RawRing do 
     when a.1 is M:RawMonoid do toExpr(Ccode(RawRingOrNull,
	       "(engine_RawRingOrNull)IM2_Ring_polyring(",
	       "(Ring *)", K, ",",
	       "(Monoid *)", M,
	       ")"
	       ))
     else WrongArg(2,"a raw monoid")
     else WrongArg(1,"a raw ring")
     else WrongArg("0 or 2 arguments")
     else WrongArg("0 or 2 arguments"));
setupfun("rawPolynomialRing",rawPolynomialRing);

export rawSkewPolynomialRing(e:Expr):Expr := (
     when e is a:Sequence do 
     if length(a) == 2 then 
     when a.0 is R:RawRing do 
     if !isSequenceOfSmallIntegers(a.1) then WrongArg(2,"a sequence of small integers")
     else toExpr(Ccode(RawRingOrNull,
 	       "(engine_RawRingOrNull)IM2_Ring_skew_polyring(",
 	       "(Ring *)", R, ",",
 	       "(M2_arrayint)", getSequenceOfSmallIntegers(a.1), -- skew variables
 	       ")"
 	       ))
     else WrongArg(1,"a raw ring")
     else WrongNumArgs(2)
     else WrongNumArgs(2));
setupfun("rawSkewPolynomialRing",rawSkewPolynomialRing);

export rawWeylAlgebra(e:Expr):Expr := (
     when e is a:Sequence do 
     if length(a) == 4 then 
     when a.0 is R:RawRing do 
     if !isSequenceOfSmallIntegers(a.1) then WrongArg(2,"a sequence of small integers") else
     if !isSequenceOfSmallIntegers(a.2) then WrongArg(3,"a sequence of small integers") else
     if !isSmallInt(a.3) then WrongArgSmallInteger(4) else
     toExpr(Ccode(RawRingOrNull,
	       "(engine_RawRingOrNull)IM2_Ring_weyl_algebra(",
	       "(Ring *)", R, ",",
	       "(M2_arrayint)", getSequenceOfSmallIntegers(a.1), ",",  -- commvars
	       "(M2_arrayint)", getSequenceOfSmallIntegers(a.2), ",", -- diff vars
	       getSmallInt(a.3), -- homog var
	       ")"
	       ))
     else WrongArg(1,"a raw ring")
     else WrongNumArgs(4)
     else WrongNumArgs(4));
setupfun("rawWeylAlgebra",rawWeylAlgebra);

export rawSolvableAlgebra(e:Expr):Expr := (
     when e is a:Sequence do 
     if length(a) == 2 then 
     when a.0 is R:RawRing do 
     when a.1 is Q:RawMatrix do toExpr(Ccode(RawRingOrNull,
 	       "(engine_RawRingOrNull)IM2_Ring_solvable_algebra(",
 	       "(Ring *)", R, ",",
 	       "(Matrix*)", Q,				    -- how to rewrite x_j*x_i
 	       ")"
 	       ))
     else WrongArg(2,"a raw matrix")
     else WrongArg(1,"a raw ring")
     else WrongNumArgs(2)
     else WrongNumArgs(2));
setupfun("rawSolvableAlgebra",rawSolvableAlgebra);

export rawLocalRing(e:Expr):Expr := (			    -- localization at a prime ideal
     when e is a:Sequence do 
     if length(a) == 2 then 
     when a.0 is R:RawRing do 
     when a.1 is P:RawMatrix do toExpr(Ccode(RawRingOrNull,
 	       "(engine_RawRingOrNull)IM2_Ring_localization(",
 	       "(Ring *)", R, ",",
 	       "(Matrix*)", P,				    -- 1 by n matrix generating the prime ideal
 	       ")"
 	       ))
     else WrongArg(2,"a raw matrix")
     else WrongArg(1,"a raw ring")
     else WrongNumArgs(2)
     else WrongNumArgs(2));
setupfun("rawLocalRing",rawLocalRing);

export rawQuotientRing(e:Expr):Expr := (			    -- localization at a prime ideal
     when e is a:Sequence do 
     if length(a) == 2 then 
     when a.0 is R:RawRing do 
     when a.1 is I:RawMatrix do toExpr(Ccode(RawRingOrNull,
 	       "(engine_RawRingOrNull)IM2_Ring_quotient(",
 	       "(Ring *)", R, ",",
 	       "(Matrix*)", I,				    -- 1 by n matrix generating the ideal
 	       ")"
 	       ))
     is B:RawRing do toExpr(Ccode(RawRingOrNull,
 	       "(engine_RawRingOrNull)IM2_Ring_quotient1(",
 	       "(Ring *)", R, ",",
 	       "(Ring *)", B,				    -- 1 by n matrix generating the ideal
 	       ")"
 	       ))
     else WrongArg(2,"a raw matrix")
     else WrongArg(1,"a raw ring")
     else WrongNumArgs(2)
     else WrongNumArgs(2));
setupfun("rawQuotientRing",rawQuotientRing);

export rawGaloisField(e:Expr):Expr := (
     when e is f:RawRingElement do toExpr(Ccode(RawRingOrNull,"(engine_RawRingOrNull)rawGaloisField((RingElement *)", f, ")"))
     else WrongArg("a raw ring element"));
setupfun("rawGaloisField", rawGaloisField);

export rawFractionRing(e:Expr):Expr := (
     when e is R:RawRing do Expr(
	  Ccode(RawRing,"(engine_RawRing)IM2_Ring_frac(",
	       "(Ring *)", R,
	       ")"))
     else WrongArg("a raw ring")
     );
setupfun("rawFractionRing", rawFractionRing);

export rawAmbientRing(e:Expr):Expr := (
     when e is R:RawRing do toExpr(
	  Ccode(RawRingOrNull,"(engine_RawRingOrNull)rawAmbientRing(",
	       "(Ring *)", R,
	       ")"))
     else WrongArg("a raw ring")
     );
setupfun("rawAmbientRing", rawAmbientRing);

export rawDenominatorRing(e:Expr):Expr := (
     when e is R:RawRing do toExprOrNull(
	  Ccode(RawRingOrNull,"(engine_RawRingOrNull)rawDenominatorRing(",
	       "(Ring *)", R,
	       ")"))
     else WrongArg("a raw ring")
     );
setupfun("rawDenominatorRing", rawDenominatorRing);

export rawSchurRing(e:Expr):Expr := (
     when e is R:RawRing do toExpr(Ccode(RawRingOrNull, "(engine_RawRingOrNull)IM2_Ring_schur(", "(Ring *)", R, ")" ) )
     else WrongArg("a raw ring"));
setupfun("rawSchurRing",rawSchurRing);

export rawIsField(e:Expr):Expr := (
     when e is K:RawRing do toExpr(Ccode(bool, "IM2_Ring_is_field(", "(Ring *)", K, ")" ))
     else WrongArg("a raw ring"));
setupfun("rawIsField",rawIsField);

export rawDeclareField(e:Expr):Expr := (
     when e is K:RawRing do (
	  Ccode(void, "IM2_Ring_declare_field(", "(Ring *)", K, ")" );
	  nullE)
     else WrongArg("a raw ring"));
setupfun("rawDeclareField",rawDeclareField);

export rawGetNonUnit(e:Expr):Expr := (
     when e is K:RawRing do Expr(Ccode(RawRingElement, 
	       "(engine_RawRingElement)IM2_Ring_get_zero_divisor(", "(Ring *)", K, ")" ))
     else WrongArg("a raw ring"));
setupfun("rawGetNonUnit",rawGetNonUnit);

-----------------------------------------------------------------------------
-- ring elements

export rawRingVar(e:Expr):Expr := (
     when e is a:Sequence do 
     if length(a) == 2 then 
     when a.0 is R:RawRing do
     when a.1 is v:Integer do
     if !isInt(v) then WrongArgSmallInteger(2) else
     toExpr(Ccode(RawRingElementOrNull, "(engine_RawRingElementOrNull)IM2_RingElement_make_var(", "(Ring *)", R, ",", toInt(v), ")" ))
     else WrongArgInteger(2)
     else WrongArg(1,"a raw ring")
     else WrongNumArgs(2)
     else WrongNumArgs(2));
setupfun("rawRingVar",rawRingVar);

export rawFromNumber(e:Expr):Expr := (
     when e is s:Sequence do if length(s) == 2 then
     when s.0
     is R:RawRing do
     when s.1
     is n:Integer do Expr(Ccode( RawRingElement, "(engine_RawRingElement)IM2_RingElement_from_Integer(", "(Ring*)",R,",", "(M2_Integer)",n, ")"))
     is x:Rational do Expr(Ccode( RawRingElement, "(engine_RawRingElement)IM2_RingElement_from_rational(", "(Ring*)",R,",", "(M2_Rational)",x, ")"))
     is x:Real do (
	  when Ccode(RawRingElementOrNull, "(engine_RawRingElementOrNull)IM2_RingElement_from_double((Ring*)",R,",",x.v,")")
	  is r:RawRingElement do Expr(r)
	  is null do buildErrorPacket(EngineError("promoting real number to ring element: not implemented yet")))
     is x:Complex do (
	  when Ccode(RawRingElementOrNull, "(engine_RawRingElementOrNull)IM2_RingElement_from_complex((Ring*)",R,",(M2_CC)", x,")")
	  is r:RawRingElement do Expr(r)
	  is null do buildErrorPacket(EngineError("promoting real number to ring element: not implemented yet")))
     is x:RRR do (
	  when Ccode(RawRingElementOrNull, "(engine_RawRingElementOrNull)IM2_RingElement_from_BigReal((Ring*)",R,",(M2_RRR)",x,")")
	  is r:RawRingElement do Expr(r)
	  is null do
	  buildErrorPacket(EngineError("can't promote big real number to ring element")))
     else WrongArg(2,"an integer or real number or complex or big real")
     else WrongArg(1,"a raw ring")
     else WrongNumArgs(2)
     else WrongNumArgs(2)
     );
setupfun("rawFromNumber", rawFromNumber);

export rawMultiDegree(e:Expr):Expr := (
     when e
     is x:RawRingElement do toExpr(
	  Ccode(RawArrayIntOrNull, "(engine_RawArrayIntOrNull)IM2_RingElement_multidegree(",
	       "(RingElement*)",x, ")" ) )
     is x:RawFreeModule do toExpr(
	  Ccode(RawArrayIntOrNull, "(engine_RawArrayIntOrNull)IM2_FreeModule_get_degrees(",
	       "(FreeModule*)",x, ")" ) )
     is x:RawMatrix do toExpr(
	  Ccode(RawArrayIntOrNull, "(engine_RawArrayIntOrNull)IM2_Matrix_get_degree(",
	       "(Matrix*)",x, ")" ) )
     else WrongArg("a raw ring element")
     );
setupfun("rawMultiDegree",rawMultiDegree);

export rawDegree(e:Expr):Expr := (
     when e
     -- is x:RawMonomial do Expr(toInteger(Ccode(int, "IM2_Monomial_degree((Monomial*)",x,")")))
     is s:Sequence do 
     if length(s) != 2 then buildErrorPacket("expected 1 or 2 arguments") else
     when s.0 is x:RawRingElement do 
     if !isSequenceOfSmallIntegers(s.1) then WrongArg(2,"a sequence of small integers") else
     toExpr(
	  Ccode( IntegerPairOrNull, "(engine_IntegerPairOrNull)IM2_RingElement_degree(",
	       "(RingElement*)",x, ",",
	       "(M2_arrayint)", getSequenceOfSmallIntegers(s.1),
	       ")"
	       ))
     else WrongArg(1,"a raw ring element")
     else WrongArg("a raw monomial or a pair: raw ring element, list of weights")
     );
setupfun("rawDegree",rawDegree);

export rawTermCount(e:Expr):Expr := (
     when e
     is args:Sequence do
     if length(args) == 2 then
     when args.0 is nvars:Integer do
     if !isInt(nvars) then WrongArgSmallInteger(1) else
     when args.1 is x:RawRingElement do toExpr( Ccode( int, "IM2_RingElement_n_terms(", toInt(nvars), ",", "(RingElement*)",x, ")" ))
     else WrongArg(2,"a raw ring element")
     else WrongArgInteger(1)
     else WrongNumArgs(2)
     else WrongNumArgs(2)
     );
setupfun("rawTermCount",rawTermCount);

export rawIsHomogeneous(e:Expr):Expr := (
     when e
     is x:RawRingElement do toExpr( Ccode( bool, "IM2_RingElement_is_graded(", "(RingElement*)",x, ")" ))
     is x:RawMatrix do toExpr( Ccode( bool, "IM2_Matrix_is_graded(", "(Matrix*)",x, ")" ))
     else WrongArg("a raw ring element")
     );
setupfun("rawIsHomogeneous",rawIsHomogeneous);

export rawIsDense(e:Expr):Expr := (
     when e is x:RawMatrix do
     toExpr(Ccode( bool, "IM2_Matrix_is_implemented_as_dense(", "(Matrix*)",x, ")" ))
     else WrongArg("a raw matrix"));
setupfun("rawIsDense",rawIsDense);

export rawIsZero(e:Expr):Expr := (
     when e
     is x:RawRingElement do toExpr( Ccode( bool, "IM2_RingElement_is_zero(", "(RingElement*)",x, ")" ))
     is x:RawMatrix do toExpr( Ccode( bool, "IM2_Matrix_is_zero(", "(Matrix*)",x, ")" ))
     is x:RawMutableMatrix do toExpr( Ccode( bool, "IM2_MutableMatrix_is_zero(", "(MutableMatrix*)",x, ")" ))
     else WrongArg("a raw ring element or matrix or mutable matrix")
     );
setupfun("rawIsZero",rawIsZero);

export rawToInteger(e:Expr):Expr := (
     when e
     is x:RawRingElement do toExpr( 
	  Ccode( IntegerOrNull, "(engine_IntegerOrNull)IM2_RingElement_to_Integer(", "(RingElement*)",x, ")" ))
     else WrongArg("a raw ring element")
     );
setupfun("rawToInteger",rawToInteger);

export rawToRational(e:Expr):Expr := (
     when e
     is x:RawRingElement do toExpr( 
	  Ccode( RationalOrNull, "(engine_RationalOrNull)IM2_RingElement_to_rational(", "(RingElement*)",x, ")" ))
     else WrongArg("a raw ring element")
     );
setupfun("rawToRational",rawToRational);

export rawToReal(e:Expr):Expr := (
     when e
     is x:RawRingElement do Expr( 
	  Real(Ccode( double, "IM2_RingElement_to_double(", "(RingElement*)",x, ")" )))
     else WrongArg("a raw ring element")
     );
setupfun("rawToReal",rawToReal);

export rawToComplex(e:Expr):Expr := (
     when e
     is x:RawRingElement do toExpr( 
	  Ccode( ComplexOrNull, "(tokens_ComplexOrNull)IM2_RingElement_to_complex(", "(RingElement*)",x, ")" ))
     else WrongArg("a raw ring element")
     );
setupfun("rawToComplex",rawToComplex);

export rawLeadCoefficient(e:Expr):Expr := (
     when e
     is args:Sequence do
     if length(args) == 2 then
     when args.0 is coeffRing:RawRing do
     when args.1 is a:RawRingElement do toExpr( 
	  Ccode( RawRingElementOrNull, 
	       "(engine_RawRingElementOrNull)IM2_RingElement_lead_coeff(", "(Ring *)", coeffRing, ",", "(RingElement *)",a, ")" ))
     else WrongArg(2,"a raw ring element")
     else WrongArg(1,"a raw ring")
     else WrongNumArgs(2)
     else WrongNumArgs(2)
     );
setupfun("rawLeadCoefficient",rawLeadCoefficient);

export rawLeadMonomial(e:Expr):Expr := (
     when e
     is args:Sequence do
     if length(args) == 2 then
     when args.0 is nvars:Integer do if !isInt(nvars) then WrongArgSmallInteger(1) else
     when args.1 is x:RawRingElement do toExpr( 
	  Ccode( RawMonomialOrNull, 
	       "(engine_RawMonomialOrNull)IM2_RingElement_lead_monomial(",
	       toInt(nvars), ",",
	       "(RingElement*)",x, 
	       ")" ))
     else WrongArg(2,"a raw ring element")
     else WrongArg(1,"a raw ring")
     else WrongNumArgs(2)
     else WrongNumArgs(2)
     );
setupfun("rawLeadMonomial",rawLeadMonomial);

export rawPairs(e:Expr):Expr := (
     when e
     is args:Sequence do
     if length(args) == 2 then 
     when args.0 is coeffRing:RawRing do 
     when args.1 is x:RawRingElement do toExpr( 
	  Ccode( RawArrayPairOrNull, "(engine_RawArrayPairOrNull)IM2_RingElement_list_form(",
	       "(Ring *)", coeffRing, ",",
	       "(RingElement*)", x, 
	       ")" ))
     else WrongArg(2,"a raw ring element")
     else WrongArg(1,"a raw ring element")
     else WrongNumArgs(2)
     else WrongNumArgs(2)
     );
setupfun("rawPairs",rawPairs);

export ringElementMod(e:Expr):Expr := (
     when e is a:Sequence do 
     if length(a) == 2 then 
     when a.0 is x:RawRingElement do 
     when a.1 is y:RawRingElement do toExpr(x % y)
     else WrongArg(2,"a raw ring element")
     else WrongArg(1,"a raw ring element")
     else WrongNumArgs(2)
     else WrongNumArgs(2));
installMethod(PercentS,rawRingElementClass,rawRingElementClass,ringElementMod);

export rawDivMod(e:Expr):Expr := (
     when e is a:Sequence do 
     if length(a) == 2 then 
     when a.0 is x:RawRingElement do 
     when a.1 is y:RawRingElement do toExpr(
	  Ccode(RawRingElementPairOrNull,
	       "(engine_RawRingElementPairOrNull)",
	       "IM2_RingElement_divmod(",
	       "(RingElement *)", x, ",",
	       "(RingElement *)", y,
	       ")"
	       )
	  )
     else WrongArg(2,"a raw ring element")
     else WrongArg(1,"a raw ring element")
     else WrongNumArgs(2)
     else WrongNumArgs(2));
setupfun("rawDivMod",rawDivMod);

export rawPromote(e:Expr):Expr := (
     when e is a:Sequence do 
     if length(a) == 2 then 
     when a.0 is R:RawRing do 
     when a.1 is x:RawRingElement do toExpr(
	  Ccode(RawRingElementOrNull, 
		    "(engine_RawRingElementOrNull)IM2_RingElement_promote(",
		    "(Ring *)", R,
		    ",(RingElement *)", x,
		    ")" ))
     else WrongArg(2,"a raw ring element")
     else WrongArg(1,"a raw ring")
     else WrongNumArgs(2)
     else WrongNumArgs(2));
setupfun("rawPromote", rawPromote);

export rawLift(e:Expr):Expr := (
     when e is a:Sequence do 
     if length(a) == 2 then 
     when a.0 is R:RawRing do 
     when a.1 is x:RawRingElement do toExpr(
	  Ccode(RawRingElementOrNull, 
		    "(engine_RawRingElementOrNull)IM2_RingElement_lift(",
		    "(Ring *)", R,
		    ",(RingElement *)", x,
		    ")" ))
     else WrongArg(2,"a raw ring element")
     else WrongArg(1,"a raw ring")
     else WrongNumArgs(2)
     else WrongNumArgs(2));
setupfun("rawLift", rawLift);

export rawRing(e:Expr):Expr := (
     when e
     is x:RawRingElement do Expr(
	  Ccode(RawRing, "(engine_RawRing)IM2_RingElement_ring(", "(RingElement *)",x, ")" ))
     is x:RawFreeModule do Expr(
	  Ccode(RawRing, "(engine_RawRing)IM2_FreeModule_ring(", "(FreeModule *)",x, ")" ))
     else WrongArg("a raw ring element or free module")
     );
setupfun("rawRing", rawRing);

export rawHomogenize(e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) == 3 then (
	  when s.0
	  is a:RawRingElement do (
	       if !isSmallInt(s.1) then WrongArgSmallInteger(2) else
	       if !isSequenceOfSmallIntegers(s.2) then WrongArg(3,"a sequence of small integers") else
	       toExpr(Ccode(RawRingElementOrNull,
			 "(engine_RawRingElementOrNull)IM2_RingElement_homogenize(",
			 "(RingElement *)", a, ",",
			 getSmallInt(s.1), ",",		    -- var number v
			 "(M2_arrayint)", getSequenceOfSmallIntegers(s.2), -- weights
			 ")"
			 )
		    )
	       )
	  is M:RawMatrix do (
	       if !isSmallInt(s.1) then WrongArgSmallInteger(2) else
	       if !isSequenceOfSmallIntegers(s.2) then WrongArg(3,"a sequence of small integers") else
	       toExpr(Ccode(RawMatrixOrNull,
			 "(engine_RawMatrixOrNull)IM2_Matrix_homogenize(",
			 "(Matrix *)", M, ",",
			 getSmallInt(s.1), ",",		    -- var number v
			 "(M2_arrayint)", getSequenceOfSmallIntegers(s.2), -- weights
			 ")"
			 )
		    )
	       )
	  else WrongArg(1,"a raw ring element or matrix")
	  )
     else if length(s) == 4 then (
	  when s.0
	  is a:RawRingElement do (
	       if !isSmallInt(s.1) then WrongArgSmallInteger(2) else
	       if !isSmallInt(s.2) then WrongArgSmallInteger(3) else
	       if !isSequenceOfSmallIntegers(s.3) then WrongArg(4,"a sequence of small integers") else
	       toExpr(Ccode(RawRingElementOrNull,
			 "(engine_RawRingElementOrNull)IM2_RingElement_homogenize_to_degree(",
			 "(RingElement *)", a, ",",
			 getSmallInt(s.1), ",",		    -- var number v
			 getSmallInt(s.2), ",",		    -- target degree
			 "(M2_arrayint)", getSequenceOfSmallIntegers(s.3), -- weights
			 ")"
			 )
		    )
	       )
	  else WrongArg(1,"a raw ring element")
	  )
     else buildErrorPacket("expected 3 or 4 arguments")
     else buildErrorPacket("expected 3 or 4 arguments"));
setupfun("rawHomogenize",rawHomogenize);

export rawTerm(e:Expr):Expr := (
     when e is s:Sequence do 
     if length(s) == 3 then 
     when s.0 is R:RawRing do 
     when s.1 is a:RawRingElement do
     when s.2 is m:RawMonomial do toExpr(Ccode(RawRingElementOrNull,
	       "(engine_RawRingElementOrNull)IM2_RingElement_term(",
	       "(Ring *)", R, ",", "(RingElement *)", a, ",", "(Monomial *)", m, ")" ))
     else WrongArg(3,"a raw monomial")
     else WrongArg(2,"a raw ring element")
     else WrongArg(1,"a raw ring")
     else WrongNumArgs(3)
     else WrongNumArgs(3));
setupfun("rawTerm",rawTerm);

export rawGetTerms(e:Expr):Expr := (
     when e is s:Sequence do 
     if length(s) == 4 then 
     when s.0 is nvars:Integer do if !isInt(nvars) then WrongArgSmallInteger(1) else
     when s.1 is f:RawRingElement do (
	  when s.2 is lo:Integer do if !isInt(lo) then WrongArgSmallInteger(3) else
	  when s.3 is hi:Integer do if !isInt(hi) then WrongArgSmallInteger(4) else
	  Expr(Ccode(RawRingElement,
		    "(engine_RawRingElement)IM2_RingElement_get_terms(",
		    toInt(nvars), ",", "(RingElement *)", f, ",", toInt(lo), ",", toInt(hi), ")" ))
	  else WrongArgInteger(4)
	  else WrongArgInteger(3))
     else WrongArg(2,"a ring element")
     else WrongArgInteger(1)
     else WrongNumArgs(4)
     else WrongNumArgs(4));
setupfun("rawGetTerms",rawGetTerms);

export rawCoefficient(e:Expr):Expr := (
     when e is a:Sequence do 
     if length(a) == 3 then 
     when a.0 is coeffRing:RawRing do
     when a.1 is x:RawRingElement do 
     when a.2 is m:RawMonomial do toExpr(
	  Ccode(RawRingElementOrNull,"(engine_RawRingElementOrNull)",
	       "IM2_RingElement_get_coeff(",
	       "(Ring *)", coeffRing, ",",
	       "(RingElement *)", x, ",",
	       "(Monomial *)", m,
	       ")"))
     else WrongArg(3,"a raw monomial")
     else WrongArg(2,"a raw ring element")
     else WrongArg(1,"a raw ring")
     else WrongNumArgs(3)
     else WrongNumArgs(3));
setupfun("rawCoefficient",rawCoefficient);

export rawNumerator(e:Expr):Expr := (
     when e
     is x:RawRingElement do toExpr( 
	  Ccode( RawRingElementOrNull, 
	       "(engine_RawRingElementOrNull)IM2_RingElement_numerator(",
	       "(RingElement*)",x, ")" ))
     else WrongArg("a raw ring element"));
setupfun("rawNumerator",rawNumerator);

export rawDenominator(e:Expr):Expr := (
     when e
     is x:RawRingElement do toExpr( 
	  Ccode( RawRingElementOrNull, 
	       "(engine_RawRingElementOrNull)IM2_RingElement_denominator(",
	       "(RingElement*)",x, ")" ))
     else WrongArg("a raw ring element")
     );
setupfun("rawDenominator",rawDenominator);

export rawFraction(e:Expr):Expr := (
     when e is s:Sequence do 
     if length(s) == 3 then 
     when s.0 is R:RawRing do 
     when s.1 is x:RawRingElement do
     when s.2 is y:RawRingElement do toExpr(Ccode(RawRingElementOrNull,
	       "(engine_RawRingElementOrNull)IM2_RingElement_fraction(",
	       "(Ring *)", R, ",", "(RingElement *)", x, ",", "(RingElement *)", y, ")" ))
     else WrongArg(3,"a raw ring element")
     else WrongArg(2,"a raw ring element")
     else WrongArg(1,"a raw fraction ring")
     else WrongNumArgs(3)
     else WrongNumArgs(3));
setupfun("rawFraction",rawFraction);

export rawFactor(e:Expr):Expr := (
     when e
     is x:RawRingElement do (
	  resultFactors := RawRingElementArrayOrNull(NULL);
	  resultPowers  := RawArrayIntOrNull(NULL);
	  Ccode( void, "rawFactor(", 
	       "(RingElement *)", x, ",",
	       "(RingElement_array_OrNull **)&",resultFactors, ",",
	       "(M2_arrayint_OrNull *)&",resultPowers, ")" );
	  when resultFactors is null do engineErrorMessage() is f:RawRingElementArray do
	  when resultPowers is null do engineErrorMessage() is p:RawArrayInt do Expr(Sequence(toExpr(f),toExprSeq(p))))
     else WrongArg("a raw ring element")
     );
setupfun("rawFactor",rawFactor);

export rawCharSeries(e:Expr):Expr := (
     when e
     is x:RawMatrix do toExpr( Ccode( RawMatrixArrayOrNull, "(engine_RawMatrixArrayOrNull)rawCharSeries(", "(Matrix *)",x, ")" ))
     else WrongArg("a raw matrix")
     );
setupfun("rawCharSeries",rawCharSeries);

export rawIdealReorder(e:Expr):Expr := (
     when e
     is x:RawMatrix do toExpr( Ccode( RawArrayIntOrNull, "(engine_RawArrayIntOrNull)rawIdealReorder(", "(Matrix *)",x, ")" ))
     else WrongArg("a raw matrix")
     );
setupfun("rawIdealReorder",rawIdealReorder);

export rawPseudoRemainder(e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) != 2 then WrongNumArgs(2) else
     when s.0 is x:RawRingElement do 
     when s.1 is y:RawRingElement do toExpr(Ccode(RawRingElementOrNull, "(engine_RawRingElementOrNull)rawPseudoRemainder((RingElement *)",x,",","(RingElement *)",y,")"))
     else WrongArg(2,"a raw ring element")
     else WrongArg(1,"a raw ring element")
     else WrongNumArgs(2)
     );
setupfun("rawPseudoRemainder",rawPseudoRemainder);

export rawSchurDimension(e:Expr):Expr := (
     when e
     is x:RawRingElement do toExpr( Ccode( IntegerOrNull, "(engine_IntegerOrNull)rawSchurDimension(", "(RingElement*)",x, ")" ))
     else WrongArg("a raw ring element"));
setupfun("rawSchurDimension",rawSchurDimension);

export rawCompare(e:Expr):Expr := (
     when e is a:Sequence do 
     if length(a) == 2 then 
     when a.0 is x:RawRingElement do 
     when a.1 is y:RawRingElement do Expr(toInteger(Ccode(int, "rawRingElementCompare(", "(RingElement *)", x, ",", "(RingElement *)", y, ")" )))
     else WrongArg(2,"a raw ring element")
     else WrongArg(1,"a raw ring element")
     else WrongNumArgs(2)
     else WrongNumArgs(2));
setupfun("rawCompare",rawCompare);

-----------------------------------------------------------------------------
-- free modules

export rawRank(e:Expr):Expr := (
     when e
     is x:RawFreeModule do toExpr( Ccode( int, "IM2_FreeModule_rank(", "(FreeModule*)",x, ")" ))
     else WrongArg("a raw free module")
     );
setupfun("rawRank",rawRank);

export rawFreeModule(e:Expr):Expr := (
     when e
     is m:RawMatrix do toExpr(
	  Ccode(RawFreeModuleOrNull, "(engine_RawFreeModuleOrNull)IM2_FreeModule_make_schreyer(",
	       "(Matrix *)", m, ")" ) )
     is s:Sequence do
     if length(s) == 2 then (
	  when s.0
	  is R:RawRing do (
	       when s.1
	       is rank:Integer do (
		    if isInt(rank)
		    then toExpr( Ccode( RawFreeModuleOrNull, 
			      "(engine_RawFreeModuleOrNull)IM2_FreeModule_make(",
			      "(Ring*)",R, ",", toInt(rank), ")" ))
		    else WrongArg(2,"a small integer or a sequence of small integers"))
	       is degs:Sequence do (
		    if isSequenceOfSmallIntegers(degs)
		    then toExpr( Ccode( RawFreeModuleOrNull, 
			      "(engine_RawFreeModuleOrNull)IM2_FreeModule_make_degs(",
			      "(Ring*)", R, ",", 
			      "(M2_arrayint)", getSequenceOfSmallIntegers(degs),
			      ")" ))
		    else WrongArg(2,"a small integer or a sequence of small integers"))
	       else WrongArg(2,"a small integer or a sequence of small integers"))
	  else WrongArg(1,"a raw ring"))
     else WrongNumArgs(2)
     else WrongNumArgs(2));
setupfun("rawFreeModule",rawFreeModule);

export rawGetSchreyer(e:Expr):Expr := (
     when e
     is F:RawFreeModule do Expr(
	  Ccode(RawMatrix, "(engine_RawMatrix)IM2_FreeModule_get_schreyer(", "(FreeModule *)", F, ")" ) )
     else WrongArg("a raw free module"));
setupfun("rawGetSchreyer",rawGetSchreyer);

export rawZero(e:Expr):Expr := (
     when e is s:Sequence do if length(s) != 3 then WrongNumArgs(3) else
     when s.0 is F:RawFreeModule do
     when s.1 is G:RawFreeModule do
     when s.2 is preference:Integer do if !isInt(preference) then WrongArgSmallInteger(3) else
     toExpr(Ccode(RawMatrixOrNull,"(engine_RawMatrixOrNull)",
		    "IM2_Matrix_zero(",
		    "(FreeModule *)", F, ",",
		    "(FreeModule *)", G, ",",
		    toInt(preference),
		    ")" ) )
     else WrongArgInteger(3)
     else WrongArg(2,"a raw free module")
     else WrongArg(1,"a raw free module")
     else WrongNumArgs(3));
setupfun("rawZero",rawZero);

export rawExteriorPower(e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) == 2 then
     when s.0 is n:Integer do
     if !isInt(n) then WrongArgSmallInteger(1) else
     when s.1 is F:RawFreeModule do Expr(Ccode(RawFreeModule, "(engine_RawFreeModule)",
	       "IM2_FreeModule_exterior(",
	       toInt(n), ",",
	       "(FreeModule *)", F,
	       ")" ))
     else WrongArg(2,"a raw free module")
     else WrongArgInteger(1)
     else if length(s) == 3 then
     when s.0 is n:Integer do
     if !isInt(n) then WrongArgSmallInteger(1) else
     when s.1 is M:RawMatrix do
     when s.2 is strategy:Integer do
     if !isInt(strategy) then WrongArgSmallInteger(3) 
     else toExpr(Ccode(RawMatrixOrNull, "(engine_RawMatrixOrNull)",
	       "IM2_Matrix_exterior(",
	       toInt(n), ",",
	       "(Matrix *)", M, ",",
	       toInt(strategy),
	       ")" ))
     else WrongArgInteger(3)
     else WrongArg(2,"a raw matrix")
     else WrongArgInteger(1)
     else WrongNumArgs(2,3)
     else WrongNumArgs(2,3));
setupfun("rawExteriorPower",rawExteriorPower);

export rawSymmetricPower(e:Expr):Expr := (
     when e is s:Sequence do
     when s.0 is n:Integer do
     if !isInt(n) then WrongArgSmallInteger(1) else
     when s.1
     is F:RawFreeModule do Expr(Ccode(RawFreeModule, "(engine_RawFreeModule)",
	       "IM2_FreeModule_symm(",
	       toInt(n), ",",
	       "(FreeModule *)", F,
	       ")" ))
     is M:RawMatrix do toExpr(Ccode(RawMatrixOrNull, "(engine_RawMatrixOrNull)",
	       "IM2_Matrix_symm(",
	       toInt(n), ",",
	       "(Matrix *)", M,
	       ")" ))
     else WrongArg(2,"a raw matrix or free module")
     else WrongArgInteger(1)
     else WrongNumArgs(2));
setupfun("rawSymmetricPower",rawSymmetricPower);

export rawDual(e:Expr):Expr := (
     when e
     is F:RawFreeModule do Expr(Ccode(RawFreeModule, "(engine_RawFreeModule)",
	       "IM2_FreeModule_dual(", "(FreeModule *)", F, ")" ))
     is M:RawMatrix do Expr(Ccode(RawMatrix, "(engine_RawMatrix)",
	       "IM2_Matrix_transpose(", "(Matrix *)", M, ")" ))
     else WrongArg("a raw free module or matrix"));
setupfun("rawDual",rawDual);

export rawDirectSum(e:Expr):Expr := (
     if isSequenceOfMatrices(e) then toExpr(Ccode(RawMatrixOrNull, "(engine_RawMatrixOrNull)",
	       "IM2_Matrix_direct_sum(", 
	       "(Matrix_array *)", getSequenceOfMatrices(e),
	       ")"))
     else when e is s:Sequence do
     if length(s) == 2 then
     when s.0 is F:RawFreeModule do
     when s.1 is G:RawFreeModule do toExpr(Ccode(RawFreeModuleOrNull, "(engine_RawFreeModuleOrNull)",
	       "IM2_FreeModule_sum(", "(FreeModule *)", F, ",(FreeModule *)", G, ")" ))
     else WrongArg(2,"a raw free module")
     else WrongArg(1,"a raw matrix or free module")
     else WrongArg("a sequence of raw matrices or a pair of raw free modules")
     else WrongArg("a sequence of raw matrices or a pair of raw free modules")
     );
setupfun("rawDirectSum",rawDirectSum);

export rawSubmodule(e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) != 2 then WrongNumArgs(2) else
     when s.0 is M:RawFreeModule do
     if !isSequenceOfSmallIntegers(s.1) then WrongArg(2,"a sequence of small integers") else (
	  selection := getSequenceOfSmallIntegers(s.1);
	  toExpr(Ccode(RawFreeModuleOrNull, "(engine_RawFreeModuleOrNull)",
		    "IM2_FreeModule_submodule(",
		    "(FreeModule *)", M, ",",
		    "(M2_arrayint)", selection,
		    ")" ) ) )
     else WrongArg(1,"a raw free module")
     else WrongNumArgs(2));
setupfun("rawSubmodule",rawSubmodule);

-----------------------------------------------------------------------------
-- matrices

export rawIsEqual(e:Expr):Expr := (
     when e is s:Sequence do if length(s) != 2 then WrongNumArgs(2) else
     when s.0 is x:RawMatrix do
     when s.1 is y:RawMatrix do
     toExpr(Ccode(bool, "IM2_Matrix_is_equal((Matrix *)",x,",(Matrix *)",y,")"))
     else WrongArg(2,"a raw matrix")
     else
     when s.0 is x:RawMutableMatrix do
     when s.1 is y:RawMutableMatrix do
     toExpr(Ccode(bool, "IM2_MutableMatrix_is_equal((MutableMatrix *)",x,",(MutableMatrix *)",y,")"))
     else WrongArg(2,"a raw mutable matrix")
     else WrongArg(1,"a raw matrix or mutable matrix")
     else WrongNumArgs(2));
setupfun("rawIsEqual",rawIsEqual);

export rawSource(e:Expr):Expr := (
     when e
     is M:RawMatrix do Expr( Ccode( RawFreeModule, "(engine_RawFreeModule)IM2_Matrix_get_source(", "(Matrix*)",M, ")" ))
     else WrongArg("a raw matrix")
     );
setupfun("rawSource",rawSource);

export rawTarget(e:Expr):Expr := (
     when e
     is M:RawMatrix do Expr( Ccode( RawFreeModule, "(engine_RawFreeModule)IM2_Matrix_get_target(", "(Matrix*)",M, ")" ))
     is F:RawRingMap do Expr( Ccode( RawRing, "(engine_RawRing)IM2_RingMap_target(", "(RingMap *)",F, ")" ))
     else WrongArg("a raw matrix")
     );
setupfun("rawTarget",rawTarget);

export rawMatrix1(e:Expr):Expr := (
     when e is s:Sequence do 
     if length(s) != 4 then WrongNumArgs(4) else
     when s.0 is target:RawFreeModule do 
     when s.1 is ncols:Integer do if !isInt(ncols) then WrongArgSmallInteger(2) else 
     if isSequenceOfRingElements(s.2) then 
     when s.3 is preference:Integer do if !isInt(preference) then WrongArgSmallInteger(4) else
     toExpr(Ccode(RawMatrixOrNull, 
	       "(engine_RawMatrixOrNull)IM2_Matrix_make1(",
	       "(FreeModule*)", target, ",",
	       toInt(ncols), ",",
	       "(RingElement_array *)", getSequenceOfRingElements(s.2), ",", -- entries
	       toInt(preference),
	       ")"))
     else WrongArgInteger(4)
     else WrongArg(3,"a sequence of raw ring elements")
     else WrongArgInteger(2)
     else WrongArg(1,"a raw free module")
     else WrongNumArgs(4));
setupfun("rawMatrix1",rawMatrix1);

export rawMatrix2(e:Expr):Expr := (
     when e is s:Sequence do 
     if length(s) != 5 then WrongNumArgs(5) else
     when s.0 is target:RawFreeModule do 
     when s.1 is source:RawFreeModule do
     if !isSequenceOfSmallIntegers(s.2) then WrongArg(3,"a sequence of small integers") else
     if !isSequenceOfRingElements(s.3) then WrongArg(4,"a sequence of raw ring elements") else
     when s.4 is preference:Integer do if !isInt(preference) then WrongArgSmallInteger(5) else
     toExpr(Ccode(RawMatrixOrNull,
	       "(engine_RawMatrixOrNull)IM2_Matrix_make2(",
	       "(FreeModule*)", target, ",",
	       "(FreeModule*)", source, ",",
	       "(M2_arrayint)", getSequenceOfSmallIntegers(s.2), ",", -- deg
	       "(RingElement_array *)", getSequenceOfRingElements(s.3), ",", -- entries
	       toInt(preference),
	       ")"))
     else WrongArgInteger(5)
     else WrongArg(2,"a raw free module")
     else WrongArg(1,"a raw free module")
     else WrongNumArgs(5));
setupfun("rawMatrix2",rawMatrix2);

export rawMatrixRemake1(e:Expr):Expr := (
     when e is s:Sequence do 
     if length(s) != 3 then WrongNumArgs(3) else
     when s.0 is target:RawFreeModule do 
     when s.1 is M:RawMatrix do
     when s.2 is preference:Integer do if !isInt(preference) then WrongArgSmallInteger(3) else
     toExpr(Ccode(RawMatrixOrNull, "(engine_RawMatrixOrNull)IM2_Matrix_remake1(",
	       "(FreeModule*)", target, ",",
	       "(Matrix*)", M, ",",
	       toInt(preference),
	       ")"))
     else WrongArgInteger(3)
     else WrongArg(2,"a raw matrix")
     else WrongArg(1,"a raw free module")
     else WrongNumArgs(3));
setupfun("rawMatrixRemake1",rawMatrixRemake1);

export rawMatrixRemake2(e:Expr):Expr := (
     when e is s:Sequence do 
     if length(s) != 5 then WrongNumArgs(5) else
     when s.0 is target:RawFreeModule do 
     when s.1 is source:RawFreeModule do
     if !isSequenceOfSmallIntegers(s.2) then WrongArg(3,"a sequence of small integers") else
     when s.3 is M:RawMatrix do
     when s.4 is preference:Integer do if !isInt(preference) then WrongArgSmallInteger(5) else
     toExpr(Ccode(RawMatrixOrNull, 
	       "(engine_RawMatrixOrNull)IM2_Matrix_remake2(",
	       "(FreeModule*)", target, ",",
	       "(FreeModule*)", source, ",",
	       "(M2_arrayint)", getSequenceOfSmallIntegers(s.2), ",", -- deg
     	       "(Matrix *)", M, ",",
	       toInt(preference),
	       ")"))
     else WrongArgInteger(5)
     else WrongArg(4,"a raw matrix")
     else WrongArg(2,"a raw free module")
     else WrongArg(1,"a raw free module")
     else WrongNumArgs(5));
setupfun("rawMatrixRemake2",rawMatrixRemake2);

export rawSparseMatrix1(e:Expr):Expr := (
     when e is s:Sequence do 
     if length(s) != 6 then WrongNumArgs(6) else
     when s.0 is target:RawFreeModule do 
     when s.1 is ncols:Integer do if !isInt(ncols) then WrongArgSmallInteger(2) else 
     if isSequenceOfSmallIntegers(s.2) then
     if isSequenceOfSmallIntegers(s.3) then
     if isSequenceOfRingElements(s.4) then 
     when s.5 is preference:Integer do if !isInt(preference) then WrongArgSmallInteger(6) else
     toExpr(Ccode(RawMatrixOrNull, 
	       "(engine_RawMatrixOrNull)IM2_Matrix_make_sparse1(",
	       "(FreeModule*)", target, ",",
	       toInt(ncols), ",",
	       "(M2_arrayint)", getSequenceOfSmallIntegers(s.2), ",", -- rows
	       "(M2_arrayint)", getSequenceOfSmallIntegers(s.3), ",", -- cols
	       "(RingElement_array *)", getSequenceOfRingElements(s.4), ",", -- entries
	       toInt(preference),
	       ")"))
     else WrongArgInteger(6)
     else WrongArg(5,"a sequence of raw ring elements")
     else WrongArg(4,"a sequence of small integers")
     else WrongArg(3,"a sequence of small integers")
     else WrongArgInteger(2)
     else WrongArg(1,"a raw free module")
     else WrongNumArgs(6));
setupfun("rawSparseMatrix1",rawSparseMatrix1);

export rawMatrixRandom(e:Expr):Expr := (
     when e is s:Sequence do if length(s) != 6 then WrongNumArgs(6) else
     when s.0 is R:RawRing do 
     when s.1 is r:Integer do if !isInt(r) then WrongArgSmallInteger(2) else
     when s.2 is c:Integer do if !isInt(c) then WrongArgSmallInteger(3) else
     when s.3 is fractionNonZero:Real do
     when s.4 is specialType:Integer do if !isInt(specialType) then WrongArgSmallInteger(5) else
     when s.5 is preference:Integer do if !isInt(preference) then WrongArgSmallInteger(6) else
     toExpr(Ccode(RawMatrixOrNull, 
	       "(engine_RawMatrixOrNull)IM2_Matrix_random(",
     	       "(Ring *)", R, ",",
	       toInt(r), ",",
	       toInt(c), ",",
	       fractionNonZero.v, ",",
     	       toInt(specialType), ",",
	       toInt(preference),
	       ")"))
     else WrongArgInteger(6)
     else WrongArgInteger(5)
     else WrongArg(4,"a real number")
     else WrongArgInteger(3)
     else WrongArgInteger(2)
     else WrongArg(1,"a raw ring")
     else WrongNumArgs(6));
setupfun("rawMatrixRandom",rawMatrixRandom);

export rawSparseMatrix2(e:Expr):Expr := (
     when e is s:Sequence do 
     if length(s) != 7 then WrongNumArgs(7) else
     when s.0 is target:RawFreeModule do 
     when s.1 is source:RawFreeModule do
     if isSequenceOfSmallIntegers(s.2) then
     if isSequenceOfSmallIntegers(s.3) then
     if isSequenceOfSmallIntegers(s.4) then
     if isSequenceOfRingElements(s.5) then 
     when s.6 is preference:Integer do if !isInt(preference) then WrongArgSmallInteger(6) else
     toExpr(Ccode(RawMatrixOrNull, 
	       "(engine_RawMatrixOrNull)IM2_Matrix_make_sparse2(",
	       "(FreeModule*)", target, ",",
	       "(FreeModule*)", source, ",",
	       "(M2_arrayint)", getSequenceOfSmallIntegers(s.2), ",", -- deg
	       "(M2_arrayint)", getSequenceOfSmallIntegers(s.3), ",", -- rows
	       "(M2_arrayint)", getSequenceOfSmallIntegers(s.4), ",", -- cols
	       "(RingElement_array *)", getSequenceOfRingElements(s.5), ",", -- entries
	       toInt(preference),
	       ")"))
     else WrongArgInteger(7)
     else WrongArg(6,"a sequence of raw ring elements")
     else WrongArg(5,"a sequence of small integers")
     else WrongArg(4,"a sequence of small integers")
     else WrongArg(3,"a sequence of small integers")
     else WrongArg(2,"a raw free module")
     else WrongArg(1,"a raw free module")
     else WrongNumArgs(7));
setupfun("rawSparseMatrix2",rawSparseMatrix2);

export rawConcat(e:Expr):Expr := (
     if isSequenceOfMatrices(e) then
     toExpr(Ccode(RawMatrixOrNull, "(engine_RawMatrixOrNull)",
	       "IM2_Matrix_concat(", 
	       "(Matrix_array *)", getSequenceOfMatrices(e),
	       ")"))
     else WrongArg("a raw matrix or a sequence of raw matrices")
     );
setupfun("rawConcat",rawConcat);

export rawMatrixEntry(e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) == 4 then 
     when s.0 is M:RawMutableMatrix do
     when s.1 is r:Integer do if !isInt(r) then WrongArgSmallInteger(2) else
     when s.2 is c:Integer do if !isInt(c) then WrongArgSmallInteger(3) else 
     when s.3 is x:RawRingElement do (
	  if Ccode(bool, 
	       "IM2_MutableMatrix_set_entry(", "(MutableMatrix *)", M, ",", 
	       toInt(r), ",", toInt(c), ",", "(RingElement *)", x, ")" 
	       )
	  then nullE
	  else buildErrorPacket(EngineError("error setting raw matrix entry")))
     else WrongArg(4,"an raw ring element")
     else WrongArgInteger(3)
     else WrongArgInteger(2)
     else WrongArg(1,"a raw mutable matrix")
     else if length(s) != 3 then WrongNumArgs(3) else
     when s.0 is M:RawMutableMatrix do
     when s.1 is r:Integer do 
     if !isInt(r) then WrongArgSmallInteger(2) else
     when s.2 is c:Integer do 
     if !isInt(c) then WrongArgSmallInteger(3) else (
	  toExpr(Ccode(RawRingElementOrNull, "(engine_RawRingElementOrNull)", "IM2_MutableMatrix_get_entry(", "(MutableMatrix *)", M, ",", toInt(r), ",", toInt(c), ")" ) ) )
     else WrongArgInteger(3)
     else WrongArgInteger(2)
     else 
     when s.0 is M:RawMatrix do
     when s.1 is r:Integer do 
     if !isInt(r) then WrongArgSmallInteger(2) else
     when s.2 is c:Integer do 
     if !isInt(c) then WrongArgSmallInteger(3) else (
	  toExpr(Ccode(RawRingElementOrNull, "(engine_RawRingElementOrNull)", "IM2_Matrix_get_entry(", "(Matrix *)", M, ",", toInt(r), ",", toInt(c), ")" ) ) )
     else WrongArgInteger(3)
     else WrongArgInteger(2)
     else WrongArg(1,"a raw matrix or mutable matrix")
     else WrongNumArgs(3,4)
     );
setupfun("rawMatrixEntry",rawMatrixEntry);

export rawSortColumns(e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) != 3 then WrongNumArgs(3) else
     when s.0 is M:RawMatrix do
     when s.1 is deg_order:Integer do 
     if !isInt(deg_order) then WrongArgSmallInteger(2) else
     when s.2 is mon_order:Integer do 
     if !isInt(mon_order) then WrongArgSmallInteger(3) else (
	  toExpr(Ccode(RawArrayInt, "(engine_RawArrayInt)",
		    "IM2_Matrix_sort_columns(",
		    "(Matrix *)", M, ",",
		    toInt(deg_order), ",",
		    toInt(mon_order),
		    ")"
		    )
	       )
	  )
     else WrongArgInteger(3)
     else WrongArgInteger(2)
     else WrongArg(1,"a raw matrix")
     else WrongNumArgs(3)
     );
setupfun("rawSortColumns",rawSortColumns);

export rawEliminateVariables(e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) != 2 then WrongNumArgs(2) else
     when s.0 is nparts:Integer do if !isInt(nparts) then WrongArgSmallInteger(1) else 
     when s.1 is M:RawMatrix do (
	  toExpr(Ccode(RawArrayInt, "(engine_RawArrayInt)",
		    "IM2_Matrix_elim_vars(",
		    toInt(nparts), ",",
		    "(Matrix *)", M,
		    ")" ) ) )
     else WrongArg(2,"a raw matrix")
     else WrongArgInteger(1)
     else WrongNumArgs(2)
     );
setupfun("rawEliminateVariables",rawEliminateVariables);

export rawKeepVariables(e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) != 2 then WrongNumArgs(2) else
     when s.0 is nparts:Integer do if !isInt(nparts) then WrongArgSmallInteger(1) else 
     when s.1 is M:RawMatrix do (
	  toExpr(Ccode(RawArrayInt, "(engine_RawArrayInt)",
		    "IM2_Matrix_keep_vars(",
		    toInt(nparts), ",",
		    "(Matrix *)", M,
		    ")" ) ) )
     else WrongArg(2,"a raw matrix")
     else WrongArgInteger(1)
     else WrongNumArgs(2)
     );
setupfun("rawKeepVariables",rawKeepVariables);

-- rawRemoveContent(e:Expr):Expr := (
--      when e is M:RawMatrix do (
-- 	  toExpr(Ccode(RawMatrixOrNull, "(engine_RawMatrixOrNull)",
-- 		    "IM2_Matrix_remove_content(",
-- 		    "(Matrix *)", M,
-- 		    ")" ) ) )
--      else WrongArg("a raw matrix"));
-- setupfun("rawRemoveContent",rawRemoveContent);

export rawDivideByVariable(e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) != 3 then WrongNumArgs(3) else
     when s.0 is M:RawMatrix do
     when s.1 is var:Integer do 
     if !isInt(var) then WrongArgSmallInteger(2) else
     when s.2 is maxdegree:Integer do 
     if !isInt(maxdegree) then WrongArgSmallInteger(3) else (
	  toExpr(Ccode(RawMatrixAndInt, "(engine_RawMatrixAndInt)",
		    "IM2_Matrix_divide_by_var(",
		    "(Matrix *)", M, ",",
		    toInt(var), ",",
		    toInt(maxdegree),
		    ")" ) ) )
     else WrongArgInteger(3)
     else WrongArgInteger(2)
     else WrongArg(1,"a raw matrix")
     else WrongNumArgs(3)
     );
setupfun("rawDivideByVariable",rawDivideByVariable);

export rawMinors(e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) != 6 then WrongNumArgs(6) else
     when s.0 is p:Integer do if !isInt(p) then WrongArgSmallInteger(1) else
     when s.1 is M:RawMatrix do
     if !isSmallInt(s.2) then WrongArgSmallInteger(3) else
     if !isSmallInt(s.3) then WrongArgSmallInteger(4) else
     if !isNullOrSequenceOfSmallIntegers(s.4) then WrongArg(5,"null or a sequence of small integers") else
     if !isNullOrSequenceOfSmallIntegers(s.5) then WrongArg(5,"null or a sequence of small integers") else
     toExpr(Ccode(RawMatrixOrNull, "(engine_RawMatrixOrNull)",
	       "rawMinors(",
	       toInt(p), ",",
	       "(Matrix *)", M, ",",
	       getSmallInt(s.2), ",",		   -- strategy
	       getSmallInt(s.3), ",",	-- n_minors_to_compute
	       "(M2_arrayint_OrNull)", getNullOrSequenceOfSmallIntegers(s.4), ",",   -- first_row_set
	       "(M2_arrayint_OrNull)", getNullOrSequenceOfSmallIntegers(s.5), -- first_col_set
	       ")"
	       )
	  )
     else WrongArg(2,"a raw matrix")
     else WrongArgInteger(1)
     else WrongNumArgs(3)
     );
setupfun("rawMinors",rawMinors);

export rawInitial(e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) != 2 then WrongNumArgs(2) else
     when s.0 is p:Integer do if !isInt(p) then WrongArgSmallInteger(1) else
     when s.1 is M:RawMatrix do (
	  Expr(Ccode(RawMatrix, "(engine_RawMatrix)",
		    "IM2_Matrix_initial(",
		    toInt(p), ",",
		    "(Matrix *)", M,
		    ")"
		    )
	       )
	  )
     else WrongArg(2,"a raw matrix")
     else WrongArgInteger(1)
     else WrongNumArgs(2)
     );
setupfun("rawInitial",rawInitial);

export rawPfaffians(e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) != 2 then WrongNumArgs(2) else
     when s.0 is p:Integer do if !isInt(p) then WrongArgSmallInteger(1) else
     when s.1 is M:RawMatrix do (
	  toExpr(Ccode(RawMatrixOrNull, "(engine_RawMatrixOrNull)",
		    "IM2_Matrix_pfaffians(", toInt(p), ",", "(Matrix *)", M, ")" ) ) )
     else WrongArg(2,"a raw matrix")
     else WrongArgInteger(1)
     else WrongNumArgs(2)
     );
setupfun("rawPfaffians",rawPfaffians);

export rawTensor(e:Expr):Expr := (
     when e is s:Sequence do
     when s.0
     is f:RawMatrix do
     when s.1 is g:RawMatrix do (
	  toExpr(Ccode(RawMatrixOrNull,"(engine_RawMatrixOrNull)",
		    "IM2_Matrix_tensor(",
		    "(Matrix *)", f, ",",
		    "(Matrix *)", g,
		    ")"
		    )
	       )		    
	  )
     else WrongArg(2,"a raw matrix")
     is M:RawFreeModule do
     when s.1 is N:RawFreeModule do (
	  toExpr(Ccode(RawFreeModuleOrNull,"(engine_RawFreeModuleOrNull)",
		    "IM2_FreeModule_tensor(",
		    "(FreeModule *)", M, ",",
		    "(FreeModule *)", N,
		    ")"
		    )
	       )		    
	  )
     else WrongArg(2,"a raw free module")
     else WrongArg(1,"a raw matrix or free module")
     else WrongNumArgs(2)     
     );
setupfun("rawTensor",rawTensor);

export rawModuleTensor(e:Expr):Expr := (
     when e is s:Sequence do
     when s.0
     is f:RawMatrix do
     when s.1 is g:RawMatrix do toExpr(Ccode(RawMatrixOrNull,"(engine_RawMatrixOrNull)", "rawModuleTensor(", "(Matrix *)", f, ",", "(Matrix *)", g, ")" ) )
     else WrongArg(2,"a raw matrix")
     else WrongArg(1,"a raw matrix")
     else WrongNumArgs(2)     
     );
setupfun("rawModuleTensor",rawModuleTensor);

export rawBasis(e:Expr):Expr := (
     when e is s:Sequence do
     when s.0 is M:RawMatrix do
     if !isSequenceOfSmallIntegers(s.1) then WrongArg(2,"a sequence of small integers") else
     if !isSequenceOfSmallIntegers(s.2) then WrongArg(3,"a sequence of small integers") else
     if !isSequenceOfSmallIntegers(s.3) then WrongArg(4,"a sequence of small integers") else
     if !isSequenceOfSmallIntegers(s.4) then WrongArg(5,"a sequence of small integers") else
     when s.5 is doTruncation:Boolean do
     if !isSmallInt(s.6) then WrongArgSmallInteger(7)
     else toExpr(Ccode(RawMatrixOrNull,
	       "(engine_RawMatrixOrNull)rawBasis(",
	       "(Matrix*)", M, ",",
	       "(M2_arrayint)", getSequenceOfSmallIntegers(s.1), ",", -- lo_degree
	       "(M2_arrayint)", getSequenceOfSmallIntegers(s.2), ",", -- hi_degree
	       "(M2_arrayint)", getSequenceOfSmallIntegers(s.3), ",", -- wt
	       "(M2_arrayint)", getSequenceOfSmallIntegers(s.4), ",", -- vars
	       doTruncation == True, ",",
	       getSmallInt(s.6),			    -- limit
	       ")"
	       ))
     else WrongArgBoolean(6)
     else WrongArg(1,"a raw matrix")
     else WrongNumArgs(7));
setupfun("rawBasis",rawBasis);

export rawMatrixDiff(e:Expr):Expr := (
     when e is s:Sequence do
     when s.0 is f:RawMatrix do
     when s.1 is g:RawMatrix do (
	  toExpr(Ccode(RawMatrixOrNull,"(engine_RawMatrixOrNull)",
		    "IM2_Matrix_diff(",
		    "(Matrix *)", f, ",",
		    "(Matrix *)", g,
		    ")"
		    )
	       )		    
	  )
     else WrongArg(2,"a raw matrix")
     else WrongArg(1,"a raw matrix")
     else WrongNumArgs(2)     
     );
setupfun("rawMatrixDiff",rawMatrixDiff);

export rawMatrixContract(e:Expr):Expr := (
     when e is s:Sequence do
     when s.0 is f:RawMatrix do
     when s.1 is g:RawMatrix do (
	  toExpr(Ccode(RawMatrixOrNull,"(engine_RawMatrixOrNull)",
		    "IM2_Matrix_contract(",
		    "(Matrix *)", f, ",",
		    "(Matrix *)", g,
		    ")"
		    )
	       )		    
	  )
     else WrongArg(2,"a raw matrix")
     else WrongArg(1,"a raw matrix")
     else WrongNumArgs(2)     
     );
setupfun("rawMatrixContract",rawMatrixContract);

export rawIdentity(e:Expr):Expr := (
     when e is s:Sequence do if length(s) != 2 then WrongNumArgs(2) else
     when s.0 is F:RawFreeModule do
     when s.1 is preference:Integer do if !isInt(preference) then WrongArgSmallInteger(2) else
     Expr(Ccode(RawMatrix, "(engine_RawMatrix)",
	       "IM2_Matrix_identity(", 
	       "(FreeModule *)", F, ",",
	       toInt(preference),
	       ")" ))
     else WrongArgInteger(2)
     else WrongArg(1,"a raw free module")
     else WrongNumArgs(2));
setupfun("rawIdentity",rawIdentity);

export rawMutableIdentity(e:Expr):Expr := (
     when e is s:Sequence do if length(s) != 3 then WrongNumArgs(3) else
     when s.0 is R:RawRing do
     when s.1 is nrows:Integer do if !isInt(nrows) then WrongArgSmallInteger(2) else
     when s.2 is preferDense:Boolean do
     Expr(Ccode(RawMutableMatrix, "(engine_RawMutableMatrix)",
	       "IM2_MutableMatrix_identity(", 
	       "(Ring *)", R, ",",
	       toInt(nrows), ",",
	       preferDense == True,
	       ")" ))
     else WrongArgBoolean(3)
     else WrongArgInteger(2)
     else WrongArg(1,"a raw ring")
     else WrongNumArgs(3));
setupfun("rawMutableIdentity",rawMutableIdentity);

export rawMutableMatrix(e:Expr):Expr := (
     when e is s:Sequence do 
     if length(s) == 4 then
     when s.0 is R:RawRing do
     when s.1 is nrows:Integer do if !isInt(nrows) then WrongArgSmallInteger(2) else
     when s.2 is ncols:Integer do if !isInt(ncols) then WrongArgSmallInteger(3) else
     when s.3 is preferDense:Boolean do
     Expr(Ccode(RawMutableMatrix, "(engine_RawMutableMatrix)",
	       "IM2_MutableMatrix_make(", 
	       "(Ring *)", R, ",",
	       toInt(nrows), ",",
	       toInt(ncols), ",",
	       preferDense == True,
	       ")" ))
     else WrongArgBoolean(4)
     else WrongArgInteger(3)
     else WrongArgInteger(2)
     else WrongArg(1,"a raw ring")
     else if length(s) == 2 then
     when s.0 is M:RawMatrix do
     when s.1 is preferDense:Boolean do
     Expr(Ccode(RawMutableMatrix, "(engine_RawMutableMatrix)",
	       "IM2_MutableMatrix_from_matrix(",
	       "(Matrix *)", M, ",",
	       preferDense == True, ")"))
     else WrongArgBoolean(2)
     else
     when s.0 is M:RawMutableMatrix do
     when s.1 is preferDense:Boolean do
     Expr(Ccode(RawMutableMatrix, "(engine_RawMutableMatrix)",
	       "IM2_MutableMatrix_copy(",
	       "(MutableMatrix *)", M, ",",
	       preferDense == True, ")"))
     else WrongArgBoolean(2)
     else WrongArg(1,"a raw matrix or mutable matrix")
     else WrongArg("2 or 4 arguments")
     else WrongArg("2 or 4 arguments"));
setupfun("rawMutableMatrix",rawMutableMatrix);

export rawMatrix(e:Expr):Expr := (
     when e is M:RawMutableMatrix do Expr(Ccode(RawMatrix, "(engine_RawMatrix)", "IM2_MutableMatrix_to_matrix(", "(MutableMatrix *)", M, ")" ))
     else WrongArg("a raw mutable matrix"));
setupfun("rawMatrix",rawMatrix);

export rawMonomials(e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) != 2 then WrongNumArgs(2) else
     if !isSequenceOfSmallIntegers(s.0) then WrongArg(1,"a sequence of small integers") else 
     when s.1 is M:RawMatrix do (
	  vars := getSequenceOfSmallIntegers(s.0);
	  toExpr(Ccode(RawMatrixOrNull, "(engine_RawMatrixOrNull)",
		    "IM2_Matrix_monomials(",
		    "(M2_arrayint)", vars, ",",
		    "(Matrix *)", M,
		    ")" ))
	  )
     else WrongArg(2,"a raw matrix")
     else WrongNumArgs(2));
setupfun("rawMonomials",rawMonomials);

export rawCoefficients(e:Expr):Expr := (
     when e is s:Sequence do if length(s) != 3 then WrongNumArgs(3) else
     if isSequenceOfSmallIntegers(s.0) then
     when s.1 is monoms:RawMatrix do 
     when s.2 is M:RawMatrix do toExpr(
	  Ccode(RawMatrixOrNull, "(engine_RawMatrixOrNull)",
	       "rawCoefficients(",
	       "(M2_arrayint)", getSequenceOfSmallIntegers(s.0), ",",
	       "(Matrix *)", monoms, ",",
	       "(Matrix *)", M,
	       ")" ))
     else WrongArg(3,"a raw matrix")
     else WrongArg(2,"a raw matrix of monomials")
     else WrongArg(1,"a sequence of small integers")
     else WrongNumArgs(3));
setupfun("rawCoefficients",rawCoefficients);

export rawSubmatrix(e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) == 3 then
     when s.0 is M:RawMatrix do 
     if !isSequenceOfSmallIntegers(s.1) then WrongArg(2,"a sequence of small integers") else
     if !isSequenceOfSmallIntegers(s.2) then WrongArg(3,"a sequence of small integers") else (
	  rows := getSequenceOfSmallIntegers(s.1);
	  cols := getSequenceOfSmallIntegers(s.2);
	  toExpr(Ccode(RawMatrixOrNull, "(engine_RawMatrixOrNull)",
		    "IM2_Matrix_submatrix(",
		    "(Matrix *)", M, ",",
		    "(M2_arrayint)", rows, ",",
		    "(M2_arrayint)", cols,
		    ")" ) ) )
     else 
     when s.0 is M:RawMutableMatrix do 
     if !isSequenceOfSmallIntegers(s.1) then WrongArg(2,"a sequence of small integers") else
     if !isSequenceOfSmallIntegers(s.2) then WrongArg(3,"a sequence of small integers") else (
	  rows := getSequenceOfSmallIntegers(s.1);
	  cols := getSequenceOfSmallIntegers(s.2);
	  toExpr(Ccode(RawMutableMatrixOrNull, "(engine_RawMutableMatrixOrNull)",
		    "IM2_MutableMatrix_submatrix(",
		    "(MutableMatrix *)", M, ",",
		    "(M2_arrayint)", rows, ",",
		    "(M2_arrayint)", cols,
		    ")" ) ) )
     else 
     WrongArg(1,"a raw matrix or mutable matrix")
     else if length(s) == 2 then
     when s.0 is M:RawMatrix do 
     if !isSequenceOfSmallIntegers(s.1) then WrongArg(2,"a sequence of small integers") else (
	  cols := getSequenceOfSmallIntegers(s.1);
	  toExpr(Ccode(RawMatrixOrNull, "(engine_RawMatrixOrNull)",
		    "IM2_Matrix_submatrix1(",
		    "(Matrix *)", M, ",",
		    "(M2_arrayint)", cols,
		    ")" ) ) )
     else 
     when s.0 is M:RawMutableMatrix do 
     if !isSequenceOfSmallIntegers(s.1) then WrongArg(2,"a sequence of small integers") else (
	  cols := getSequenceOfSmallIntegers(s.1);
	  toExpr(Ccode(RawMutableMatrixOrNull, "(engine_RawMutableMatrixOrNull)",
		    "IM2_MutableMatrix_submatrix1(",
		    "(MutableMatrix *)", M, ",",
		    "(M2_arrayint)", cols,
		    ")" ) ) )
     else 
     WrongArg(1,"a raw matrix or mutable matrix")
     else WrongNumArgs(2,3)
     else WrongNumArgs(2,3));
setupfun("rawSubmatrix",rawSubmatrix);

export rawReshape(e:Expr):Expr := (
     when e is s:Sequence do 
     if length(s) == 3 then 
     when s.0 is M:RawMatrix do 
     when s.1 is F:RawFreeModule do
     when s.2 is G:RawFreeModule do toExpr(Ccode(RawMatrixOrNull,
	       "(engine_RawMatrixOrNull)IM2_Matrix_reshape(",
	       "(Matrix *)", M, ",", "(FreeModule *)", F, ",", "(FreeModule *)", G, ")" ))
     else WrongArg(3,"a raw free module")
     else WrongArg(2,"a raw free module")
     else WrongArg(1,"a raw matrix")
     else WrongNumArgs(3)
     else WrongNumArgs(3));
setupfun("rawReshape",rawReshape);

export rawFlip(e:Expr):Expr := (
     when e is s:Sequence do 
     if length(s) == 2 then 
     when s.0 is F:RawFreeModule do
     when s.1 is G:RawFreeModule do toExpr(Ccode(RawMatrixOrNull,
	       "(engine_RawMatrixOrNull)IM2_Matrix_flip(",
	       "(FreeModule *)", F, ",", "(FreeModule *)", G, ")" ))
     else WrongArg(2,"a raw free module")
     else WrongArg(1,"a raw free module")
     else WrongNumArgs(2)
     else WrongNumArgs(2));
setupfun("rawFlip",rawFlip);

export rawKoszul(e:Expr):Expr := (
     when e is s:Sequence do
     when s.0 is n:Integer do
     if !isInt(n) then WrongArgSmallInteger(1) else
     when s.1
     is F:RawMatrix do toExpr(Ccode(RawMatrixOrNull, "(engine_RawMatrixOrNull)",
	       "IM2_Matrix_koszul(", toInt(n), ",", "(Matrix *)", F, ")" ))
     else WrongArg(2,"a raw matrix")
     else WrongArgInteger(1)
     else WrongNumArgs(2));
setupfun("rawKoszul",rawKoszul);

export rawKoszulMonomials(e:Expr):Expr := (
     when e is s:Sequence do
     when s.0 is F:RawMatrix do 
     when s.1 is G:RawMatrix do 
     toExpr(Ccode(RawMatrixOrNull, "(engine_RawMatrixOrNull)",
	       "IM2_Matrix_koszul_monoms(", "(Matrix *)", F, ",", "(Matrix *)", G, ")" ))
     else WrongArg(2,"a raw matrix")
     else WrongArg(1,"a raw matrix")
     else WrongNumArgs(2));
setupfun("rawKoszulMonomials",rawKoszulMonomials);


export rawHilbert(e:Expr):Expr := (
     when e is M:RawMatrix do (
	  toExpr(Ccode(RawRingElementOrNull,"(engine_RawRingElementOrNull)",
		    "IM2_Matrix_Hilbert(",
		    "(Matrix *)", M, ")" ) ) )
     else WrongArg("a raw matrix")
     );
setupfun("rawHilbert",rawHilbert);

export rawInsertRows(e:Expr):Expr := (
     when e is s:Sequence do if length(s) != 3 then WrongNumArgs(3) else
     when s.0 is M:RawMutableMatrix do
     when s.1 is position:Integer do if !isInt(position) then WrongArgSmallInteger(2) else
     when s.2 is number:Integer do if !isInt(number) then WrongArgSmallInteger(3) else
     toExpr(Ccode(bool, "IM2_MutableMatrix_insert_rows(",
	       "(MutableMatrix *)", M, ",",
	       toInt(position), ",",
	       toInt(number),
	       ")" ) )
     else WrongArgInteger(3)
     else WrongArgInteger(2)
     else WrongArg(1,"a raw mutable matrix")
     else WrongNumArgs(3)
     );
setupfun("rawInsertRows",rawInsertRows);

export rawDeleteRows(e:Expr):Expr := (
     when e is s:Sequence do if length(s) != 3 then WrongNumArgs(3) else
     when s.0 is M:RawMutableMatrix do
     when s.1 is position:Integer do if !isInt(position) then WrongArgSmallInteger(2) else
     when s.2 is number:Integer do if !isInt(number) then WrongArgSmallInteger(3) else
     toExpr(Ccode(bool, "IM2_MutableMatrix_delete_rows(",
	       "(MutableMatrix *)", M, ",",
	       toInt(position), ",",
	       toInt(number),
	       ")" ) )
     else WrongArgInteger(3)
     else WrongArgInteger(2)
     else WrongArg(1,"a raw mutable matrix")
     else WrongNumArgs(3)
     );
setupfun("rawDeleteRows",rawDeleteRows);

export rawInsertColumns(e:Expr):Expr := (
     when e is s:Sequence do if length(s) != 3 then WrongNumArgs(3) else
     when s.0 is M:RawMutableMatrix do
     when s.1 is position:Integer do if !isInt(position) then WrongArgSmallInteger(2) else
     when s.2 is number:Integer do if !isInt(number) then WrongArgSmallInteger(3) else
     toExpr(Ccode(bool, "IM2_MutableMatrix_insert_columns(",
	       "(MutableMatrix *)", M, ",",
	       toInt(position), ",",
	       toInt(number),
	       ")" ) )
     else WrongArgInteger(3)
     else WrongArgInteger(2)
     else WrongArg(1,"a raw mutable matrix")
     else WrongNumArgs(3)
     );
setupfun("rawInsertColumns",rawInsertColumns);

export rawDeleteColumns(e:Expr):Expr := (
     when e is s:Sequence do if length(s) != 3 then WrongNumArgs(3) else
     when s.0 is M:RawMutableMatrix do
     when s.1 is position:Integer do if !isInt(position) then WrongArgSmallInteger(2) else
     when s.2 is number:Integer do if !isInt(number) then WrongArgSmallInteger(3) else
     toExpr(Ccode(bool, "IM2_MutableMatrix_delete_columns(",
	       "(MutableMatrix *)", M, ",",
	       toInt(position), ",",
	       toInt(number),
	       ")" ) )
     else WrongArgInteger(3)
     else WrongArgInteger(2)
     else WrongArg(1,"a raw mutable matrix")
     else WrongNumArgs(3)
     );
setupfun("rawDeleteColumns",rawDeleteColumns);

export rawSortColumns2(e:Expr):Expr := (
     when e is s:Sequence do if length(s) != 3 then WrongNumArgs(3) else
     when s.0 is M:RawMutableMatrix do
     when s.1 is position:Integer do if !isInt(position) then WrongArgSmallInteger(2) else
     when s.2 is number:Integer do if !isInt(number) then WrongArgSmallInteger(3) else
     toExpr(Ccode(bool, "IM2_MutableMatrix_sort_columns(",
	       "(MutableMatrix *)", M, ",",
	       toInt(position), ",",
	       toInt(number),
	       ")" ) )
     else WrongArgInteger(3)
     else WrongArgInteger(2)
     else WrongArg(1,"a raw mutable matrix")
     else WrongNumArgs(3)
     );
setupfun("rawSortColumns2",rawSortColumns2);

export rawPermuteRows(e:Expr):Expr := (
     when e is s:Sequence do if length(s) != 3 then WrongNumArgs(3) else
     when s.0 is M:RawMutableMatrix do
     when s.1 is start:Integer do if !isInt(start) then WrongArgSmallInteger(2) else
     if !isSequenceOfSmallIntegers(s.2) then WrongArg(3,"a sequence of small integers") else 
     toExpr(Ccode(bool, "IM2_MutableMatrix_row_permute(",
	       "(MutableMatrix *)", M, ",",
	       toInt(start), ",",
	       "(M2_arrayint)", getSequenceOfSmallIntegers(s.2),
	       ")" ) )
     else WrongArgInteger(2)
     else WrongArg(1,"a raw mutable matrix")
     else WrongNumArgs(3)
     );
setupfun("rawPermuteRows",rawPermuteRows);

export rawPermuteColumns(e:Expr):Expr := (
     when e is s:Sequence do if length(s) != 3 then WrongNumArgs(3) else
     when s.0 is M:RawMutableMatrix do
     when s.1 is start:Integer do if !isInt(start) then WrongArgSmallInteger(2) else
     if !isSequenceOfSmallIntegers(s.2) then WrongArg(3,"a sequence of small integers") else 
     toExpr(Ccode(bool, "IM2_MutableMatrix_column_permute(",
	       "(MutableMatrix *)", M, ",",
	       toInt(start), ",",
	       "(M2_arrayint)", getSequenceOfSmallIntegers(s.2),
	       ")" ) )
     else WrongArgInteger(2)
     else WrongArg(1,"a raw mutable matrix")
     else WrongNumArgs(3)
     );
setupfun("rawPermuteColumns",rawPermuteColumns);

export rawMatrixColumnOperation2(e:Expr):Expr := (
     when e is s:Sequence do if length(s) != 8 then WrongNumArgs(8) else
     when s.0 is M:RawMutableMatrix do
     when s.1 is c1:Integer do 
     when s.2 is c2:Integer do 
     when s.3 is a1:RawRingElement do
     when s.4 is a2:RawRingElement do
     when s.5 is b1:RawRingElement do
     when s.6 is b2:RawRingElement do
     when s.7 is opposite:Boolean do
     toExpr(Ccode(bool,"IM2_MutableMatrix_column_2by2(",
	       "(MutableMatrix *)", M, ",",
	       toInt(c1), ",",
	       toInt(c2), ",",
	       "(RingElement*)", a1, ",",
	       "(RingElement*)", a2, ",",
	       "(RingElement*)", b1, ",",
	       "(RingElement*)", b2, ",",
	       opposite.v,
	       ")"))     
     else WrongArgBoolean(8)
     else WrongArg(7,"a raw ring element")
     else WrongArg(6,"a raw ring element")
     else WrongArg(5,"a raw ring element")
     else WrongArg(4,"a raw ring element")
     else WrongArgInteger(3)
     else WrongArgInteger(2)
     else WrongArg(1,"a raw mutable matrix")
     else WrongNumArgs(8));
setupfun("rawMatrixColumnOperation2",rawMatrixColumnOperation2);

export rawMatrixRowOperation2(e:Expr):Expr := (
     when e is s:Sequence do if length(s) != 8 then WrongNumArgs(8) else
     when s.0 is M:RawMutableMatrix do
     when s.1 is r1:Integer do 
     when s.2 is r2:Integer do 
     when s.3 is a1:RawRingElement do
     when s.4 is a2:RawRingElement do
     when s.5 is b1:RawRingElement do
     when s.6 is b2:RawRingElement do
     when s.7 is opposite:Boolean do
     toExpr(Ccode(bool,"IM2_MutableMatrix_row_2by2(",
	       "(MutableMatrix *)", M, ",",
	       toInt(r1), ",",
	       toInt(r2), ",",
	       "(RingElement*)", a1, ",",
	       "(RingElement*)", a2, ",",
	       "(RingElement*)", b1, ",",
	       "(RingElement*)", b2, ",",
	       opposite.v,
	       ")"))     
     else WrongArgBoolean(8)
     else WrongArg(7,"a raw ring element")
     else WrongArg(6,"a raw ring element")
     else WrongArg(5,"a raw ring element")
     else WrongArg(4,"a raw ring element")
     else WrongArgInteger(3)
     else WrongArgInteger(2)
     else WrongArg(1,"a raw mutable matrix")
     else WrongNumArgs(8));
setupfun("rawMatrixRowOperation2",rawMatrixRowOperation2);

export rawWedgeProduct(e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) != 3 then WrongNumArgs(3) else
     when s.0 is p:Integer do if !isInt(p) then WrongArgSmallInteger(1) else
     when s.1 is q:Integer do if !isInt(q) then WrongArgSmallInteger(2) else
     when s.2 is M:RawFreeModule do toExpr(Ccode(RawMatrixOrNull, "(engine_RawMatrixOrNull)rawWedgeProduct(", toInt(p), ",", toInt(q), ",", "(FreeModule*)", M, ")" ))
     else WrongArg(3,"a raw free module")
     else WrongArgInteger(2)
     else WrongArgInteger(1)
     else WrongNumArgs(3));
setupfun("rawWedgeProduct",rawWedgeProduct);

export rawTopCoefficients(e:Expr):Expr := (
     when e
     is x:RawMatrix do toExpr( Ccode( RawMatrixPairOrNull, "(engine_RawMatrixPairOrNull)rawTopCoefficients(", "(Matrix *)",x, ")" ))
     else WrongArg("a raw matrix"));
setupfun("rawTopCoefficients",rawTopCoefficients);

export rawMatrixCompress(e:Expr):Expr := (
     when e
     is x:RawMatrix do Expr( Ccode( RawMatrix, "(engine_RawMatrix)rawMatrixCompress(", "(Matrix *)",x, ")" ))
     else WrongArg("a raw matrix"));
setupfun("rawMatrixCompress",rawMatrixCompress);

export rawRemoveMonomialFactors(e:Expr):Expr := (
     when e is s:Sequence do 
     when s.0 is m:RawMatrix do 
     if isBoolean(s.1) then Expr(Ccode(RawMatrix, "(engine_RawMatrix)rawRemoveMonomialFactors(", "(Matrix *)", m, ",", toBoolean(s.1), ")" ))
     else WrongArg(1,"true or false")
     else WrongArg(0,"a raw matrix")
     else WrongNumArgs(2));
setupfun("rawRemoveMonomialFactors",rawRemoveMonomialFactors);

export rawRemoveScalarMultiples(e:Expr):Expr := (
     when e is m:RawMatrix 
     do Expr(Ccode(RawMatrix, "(engine_RawMatrix)rawRemoveScalarMultiples(", "(Matrix *)", m, ")" ))
     else WrongArg(0,"a raw matrix"));
setupfun("rawRemoveScalarMultiples",rawRemoveScalarMultiples);

-----------------------------------------------------------------------------
-- monomial ideals

export rawMonomialIdealToMatrix(e:Expr):Expr := (
     when e
     is I:RawMonomialIdeal do Expr(Ccode(RawMatrix, "(engine_RawMatrix)", "IM2_MonomialIdeal_to_matrix(", "(MonomialIdeal *)", I, ")" ))
     else WrongArg("a raw monomial ideal")
     );
setupfun("rawMonomialIdealToMatrix",rawMonomialIdealToMatrix);

export rawMonomialIdeal(e:Expr):Expr := (
     when e is s:Sequence do
     when s.0 is m:RawMatrix do
     when s.1 is n:Integer do 
     if !isInt(n) then WrongArgSmallInteger(2) else 
     toExpr(Ccode(RawMonomialIdealOrNull, "(engine_RawMonomialIdealOrNull)",
	       "IM2_MonomialIdeal_make(", "(Matrix *)", m, ",", toInt(n), ")" ) )
     else WrongArgInteger(2)
     else WrongArg(1,"a raw matrix")
     else WrongNumArgs(2)
     );
setupfun("rawMonomialIdeal",rawMonomialIdeal);

export rawNumgens(e:Expr):Expr := (
     when e
     is I:RawMonomialIdeal do Expr( toInteger(
	       Ccode(int, "IM2_MonomialIdeal_n_gens(", "(MonomialIdeal *)", I, ")" )))
     else WrongArg("a raw free module"));
setupfun("rawNumgens",rawNumgens);

export rawIntersect(e:Expr):Expr := (
     when e is s:Sequence do
     when s.0 is I:RawMonomialIdeal do
     when s.1 is J:RawMonomialIdeal do
     toExpr(Ccode(RawMonomialIdealOrNull,"(engine_RawMonomialIdealOrNull)",
	       "IM2_MonomialIdeal_intersect(",
	       "(MonomialIdeal *)", I, ",",
	       "(MonomialIdeal *)", J,
	       ")"
	       )
	  )
     else WrongArg(2,"a raw monomial ideal")
     else WrongArg(1,"a raw monomial ideal")
     else WrongNumArgs(2)     
     );
setupfun("rawIntersect",rawIntersect);

export rawStronglyStableClosure(e:Expr):Expr := (
     when e is I:RawMonomialIdeal do Expr(
	  Ccode(RawMonomialIdeal,"(engine_RawMonomialIdeal)",
	       "IM2_MonomialIdeal_borel(", "(MonomialIdeal *)", I, ")" ) )
     else WrongArg("a raw monomial ideal"));
setupfun("rawStronglyStableClosure",rawStronglyStableClosure);

export rawIsStronglyStable(e:Expr):Expr := (
     when e is I:RawMonomialIdeal do toExpr(
	  Ccode(bool, "IM2_MonomialIdeal_is_borel(", "(MonomialIdeal *)", I, ")" ) )
     else WrongArg("a raw monomial ideal"));
setupfun("rawIsStronglyStable",rawIsStronglyStable);

export rawCodimension(e:Expr):Expr := (
     when e is I:RawMonomialIdeal do toExpr(
	  Ccode(int, "IM2_MonomialIdeal_codim(", "(MonomialIdeal *)", I, ")" ) )
     else WrongArg("a raw monomial ideal"));
setupfun("rawCodimension",rawCodimension);

export rawAssociatedPrimes(e:Expr):Expr := (
     when e is I:RawMonomialIdeal do Expr(
	  Ccode(RawMonomialIdeal,"(engine_RawMonomialIdeal)",
	       "IM2_MonomialIdeal_assprimes(", "(MonomialIdeal *)", I, ")" ) )
     else WrongArg("a raw monomial ideal"));
setupfun("rawAssociatedPrimes",rawAssociatedPrimes);

-----------------------------------------------------------------------------
-- ring maps

export rawRingMap(e:Expr):Expr := (
     when e
     is M:RawMatrix do Expr( Ccode( RawRingMap, "(engine_RawRingMap)IM2_RingMap_make1(", "(Matrix*)",M, ")" ))
     else WrongArg("a raw matrix")
     );
setupfun("rawRingMap",rawRingMap);

export rawRingMapEval(e:Expr):Expr := (
     when e
     is s:Sequence do
     if length(s) == 2 then 
     when s.0 is F:RawRingMap do 
     when s.1 is a:RawRingElement do (
	  toExpr(Ccode(RawRingElementOrNull,
		    "(engine_RawRingElementOrNull)",
		    "IM2_RingMap_eval_ringelem(",
		    "(RingMap *)", F, ",",
		    "(RingElement *)", a,
		    ")"
		    )
	       )
	  )
     else WrongArg(2,"a raw ring element")
     else WrongArg(1,"a raw ring map")
     else if length(s) == 3 then
     when s.0 is F:RawRingMap do 
     when s.1 is newTarget:RawFreeModule do
     when s.2
     is M:RawMatrix do (
	  toExpr(Ccode(RawMatrixOrNull,
		    "(engine_RawMatrixOrNull)",
		    "IM2_RingMap_eval_matrix(",
		    "(RingMap *)", F, ",",
		    "(FreeModule *)", newTarget, ",",
		    "(Matrix *)", M,
		    ")"
		    )
	       )
	  )
     else WrongArg(3,"a matrix")
     else WrongArg(2,"a raw free module")
     else WrongArg(1,"a raw ring map")
     else buildErrorPacket("expected 2 or 3 arguments")
     else buildErrorPacket("expected 2 or 3 arguments")
     );
setupfun("rawRingMapEval",rawRingMapEval);

export rawNumberOfRows(e:Expr):Expr := (
     when e
     is M:RawMatrix do toExpr(Ccode( int, "IM2_Matrix_n_rows(", "(Matrix *)", M, ")" ))
     is M:RawMutableMatrix do toExpr(Ccode( int, "IM2_MutableMatrix_n_rows(", "(MutableMatrix *)", M, ")" ))
     else WrongArg("a raw matrix"));
setupfun("rawNumberOfRows",rawNumberOfRows);

export rawNumberOfColumns(e:Expr):Expr := (
     when e
     is M:RawMatrix do toExpr(Ccode( int, "IM2_Matrix_n_cols(", "(Matrix *)", M, ")" ))
     is M:RawMutableMatrix do toExpr(Ccode( int, "IM2_MutableMatrix_n_cols(", "(MutableMatrix *)", M, ")" ))
     else WrongArg("a raw matrix"));
setupfun("rawNumberOfColumns",rawNumberOfColumns);

export rawRowChange(e:Expr):Expr := (
     when e
     is M:RawMutableMatrix do
     toExpr(Ccode( RawMutableMatrixOrNull, "(engine_RawMutableMatrixOrNull)", "IM2_MutableMatrix_get_row_change(", "(MutableMatrix *)", M, ")" ))
     is s:Sequence do
     when s.0 is M:RawMutableMatrix do
     when s.1 is rowChange:RawMutableMatrix do toExpr(Ccode( bool, "IM2_MutableMatrix_set_row_change(", "(MutableMatrix *)", M, ",", "(MutableMatrix *)", rowChange, ")" ))
     else WrongArg("a raw mutable matrix")
     else WrongArg("a raw mutable matrix")
     else WrongNumArgs(1,2));
setupfun("rawRowChange",rawRowChange);

export rawColumnChange(e:Expr):Expr := (
     when e
     is M:RawMutableMatrix do
     toExpr(Ccode( RawMutableMatrixOrNull, "(engine_RawMutableMatrixOrNull)", "IM2_MutableMatrix_get_col_change(", "(MutableMatrix *)", M, ")" ))
     is s:Sequence do
     when s.0 is M:RawMutableMatrix do
     when s.1 is colChange:RawMutableMatrix do toExpr( Ccode( bool, "IM2_MutableMatrix_set_col_change(", "(MutableMatrix *)", M, ",", "(MutableMatrix *)", colChange, ")" ))
     else WrongArg("a raw mutable matrix")
     else WrongArg("a raw mutable matrix")
     else WrongNumArgs(1,2));
setupfun("rawColumnChange",rawColumnChange);

export rawMatrixRowSwap(e:Expr):Expr := (
     when e is s:Sequence do if length(s) != 3 then WrongNumArgs(3) else
     when s.0 is M:RawMutableMatrix do
     when s.1 is i:Integer do if !isInt(i) then WrongArgSmallInteger(2) else
     when s.2 is j:Integer do if !isInt(j) then WrongArgSmallInteger(3) else (
	  if Ccode(bool, "IM2_MutableMatrix_row_swap(", "(MutableMatrix *)", M, ",", toInt(i), ",", toInt(j), ")" )
	  then nullE
	  else buildErrorPacket(EngineError("error swapping raw matrix rows")))
     else WrongArgInteger(3)
     else WrongArgInteger(2)
     else WrongArg(1,"a raw mutable matrix")
     else WrongNumArgs(3));
setupfun("rawMatrixRowSwap",rawMatrixRowSwap);

export rawMatrixColumnSwap(e:Expr):Expr := (
     when e is s:Sequence do if length(s) != 3 then WrongNumArgs(3) else
     when s.0 is M:RawMutableMatrix do
     when s.1 is i:Integer do if !isInt(i) then WrongArgSmallInteger(2) else
     when s.2 is j:Integer do if !isInt(j) then WrongArgSmallInteger(3) else (
	  if Ccode(bool, "IM2_MutableMatrix_column_swap(", "(MutableMatrix *)", M, ",", toInt(i), ",", toInt(j), ")" )
	  then nullE
	  else buildErrorPacket(EngineError("error swapping raw matrix columns")))
     else WrongArgInteger(3)
     else WrongArgInteger(2)
     else WrongArg(1,"a raw mutable matrix")
     else WrongNumArgs(3));
setupfun("rawMatrixColumnSwap",rawMatrixColumnSwap);

export rawColumnDotProduct(e:Expr):Expr := (
     when e is s:Sequence do if length(s) != 3 then WrongNumArgs(3) else
     when s.0 is M:RawMutableMatrix do
     when s.1 is c1:Integer do if !isInt(c1) then WrongArgSmallInteger(2) else
     when s.2 is c2:Integer do if !isInt(c2) then WrongArgSmallInteger(3) else
     Expr(Ccode(RawRingElement, "(engine_RawRingElement)IM2_Matrix_dot_product((MutableMatrix *)", M, ",", toInt(c1), ",", toInt(c2), ")" ))
     else WrongArgInteger(3)
     else WrongArgInteger(2)
     else WrongArg(1,"a raw mutable matrix")
     else WrongNumArgs(3));
setupfun("rawColumnDotProduct",rawColumnDotProduct);

export rawMatrixRowChange(e:Expr):Expr := (
     when e is s:Sequence do if length(s) != 5 then WrongNumArgs(5) else
     when s.0 is M:RawMutableMatrix do
     when s.1 is targetRow:Integer do if !isInt(targetRow) then WrongArgSmallInteger(2) else
     when s.2 is r:RawRingElement do
     when s.3 is sourceRow:Integer do if !isInt(sourceRow) then WrongArgSmallInteger(4) else 
     when s.4 is opposite:Boolean do (
	  if Ccode(bool, "IM2_MutableMatrix_row_operation(", "(MutableMatrix *)", M, ",", toInt(targetRow), ",", "(RingElement *)", r, ",", toInt(sourceRow), ",", opposite.v, ")" )
	  then nullE
	  else buildErrorPacket(EngineError("error changing raw matrix row")))
     else WrongArgBoolean(5)
     else WrongArgInteger(4)
     else WrongArg(3,"a raw ring element")
     else WrongArgInteger(2)
     else WrongArg(1,"a raw mutable matrix")
     else WrongNumArgs(5));
setupfun("rawMatrixRowChange",rawMatrixRowChange);

export rawMatrixColumnChange(e:Expr):Expr := (
     when e is s:Sequence do if length(s) != 5 then WrongNumArgs(5) else
     when s.0 is M:RawMutableMatrix do
     when s.1 is targetColumn:Integer do if !isInt(targetColumn) then WrongArgSmallInteger(2) else
     when s.2 is r:RawRingElement do
     when s.3 is sourceColumn:Integer do if !isInt(sourceColumn) then WrongArgSmallInteger(4) else 
     when s.4 is opposite:Boolean do (
	  if Ccode(bool, "IM2_MutableMatrix_column_operation(", "(MutableMatrix *)", M, ",", toInt(targetColumn), ",", "(RingElement *)", r, ",", toInt(sourceColumn), ",", opposite.v, ")" )
	  then nullE
	  else buildErrorPacket(EngineError("error changing raw matrix column")))
     else WrongArgBoolean(5)
     else WrongArgInteger(4)
     else WrongArg(3,"a raw ring element")
     else WrongArgInteger(2)
     else WrongArg(1,"a raw mutable matrix")
     else WrongNumArgs(5));
setupfun("rawMatrixColumnChange",rawMatrixColumnChange);

export rawMatrixRowScale(e:Expr):Expr := (
     when e is s:Sequence do if length(s) != 4 then WrongNumArgs(4) else
     when s.0 is M:RawMutableMatrix do
     when s.1 is r:RawRingElement do 
     when s.2 is targetRow:Integer do if !isInt(targetRow) then WrongArgSmallInteger(3) else
     when s.3 is opposite:Boolean do (
	  if Ccode(bool, "IM2_MutableMatrix_row_scale(", "(MutableMatrix *)", M, ",", "(RingElement *)", r, ",", toInt(targetRow), ",", opposite.v, ")" )
	  then nullE
	  else buildErrorPacket(EngineError("error scaling raw matrix row")))
     else WrongArgBoolean(4)
     else WrongArgInteger(3)
     else WrongArg(2,"a raw ring element")
     else WrongArg(1,"a raw mutable matrix")
     else WrongNumArgs(4));
setupfun("rawMatrixRowScale",rawMatrixRowScale);

export rawMatrixColumnScale(e:Expr):Expr := (
     when e is s:Sequence do if length(s) != 4 then WrongNumArgs(4) else
     when s.0 is M:RawMutableMatrix do
     when s.1 is r:RawRingElement do
     when s.2 is targetColumn:Integer do if !isInt(targetColumn) then WrongArgSmallInteger(3) else
     when s.3 is opposite:Boolean do (
	  if Ccode(bool, "IM2_MutableMatrix_column_scale(", "(MutableMatrix *)", M, ",", "(RingElement *)", r, ",", toInt(targetColumn), ",", opposite.v, ")" )
	  then nullE
	  else buildErrorPacket(EngineError("error scaling raw matrix column")))
     else WrongArgBoolean(4)
     else WrongArgInteger(3)
     else WrongArg(2,"a raw ring element")
     else WrongArg(1,"a raw mutable matrix")
     else WrongNumArgs(4));
setupfun("rawMatrixColumnScale",rawMatrixColumnScale);

-----------------------------------------------------------------------------
-- Groebner bases and resolutions and computations
-----------------------------------------------------------------------------

export rawStartComputation(e:Expr):Expr := (
     when e is c:RawComputation do (
	  Ccode(void, "rawStartComputation(", "(Computation *)", c, ")");
	  nullE)
     else WrongArg("a raw computation"));
setupfun("rawStartComputation",rawStartComputation);

export rawStatusResolution(e:Expr):Expr := (
     when e is G:RawComputation do (
	  completionDegree := 0;
	  completionLevel := 0;
	  ret := Ccode(int,"IM2_Resolution_status(",
	       "(Computation*)",G,",",
	       "&",completionDegree,",",
	       "&",completionLevel,
	       ")" );
	  if ret == -1 then buildErrorPacket(EngineError("unknown raw gb computation status error")) 
	  else Expr(Sequence(toInteger(ret),toInteger(completionDegree),toInteger(completionLevel)))
	  )
     else WrongArg("a raw computation")
     );
setupfun("rawStatusResolution", rawStatusResolution);

export rawGB(e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) != 8 then WrongNumArgs(8) else
     when s.0 is m:RawMatrix do
     when s.1 is collectSyz:Boolean do
     when s.2 is nRowsToKeep:Integer do if !isInt(nRowsToKeep) then WrongArgSmallInteger(3) else
     if isSequenceOfSmallIntegers(s.3) then
     when s.4 is useMaxDegree:Boolean do
     when s.5 is maxDegree:Integer do if !isInt(maxDegree) then WrongArgSmallInteger(5) else
     when s.6 is algorithm:Integer do if !isInt(algorithm) then WrongArgSmallInteger(6) else
     when s.7 is strategy:Integer do if !isInt(strategy) then WrongArgSmallInteger(7) else
     if !isInt(algorithm) then WrongArgSmallInteger(8) else
     toExpr(
	  Ccode(RawComputationOrNull,
	       "(engine_RawComputationOrNull)IM2_GB_make(",
		   "(Matrix*)",m,",",
		   isTrue(collectSyz),",",
		   toInt(nRowsToKeep),",",
     	       	   "(M2_arrayint)", getSequenceOfSmallIntegers(s.3), ",", -- gb degrees
		   isTrue(useMaxDegree),",",
		   toInt(maxDegree),",",
		   toInt(algorithm),",",
		   toInt(strategy),
	       ")"
	       )
	  )
     else WrongArgInteger(8)
     else WrongArgInteger(7)
     else WrongArgInteger(6)
     else WrongArgBoolean(5)
     else WrongArg(4,"a sequence of small integers")
     else WrongArgInteger(3)
     else WrongArgBoolean(2)
     else WrongArg(1,"a raw matrix")
     else WrongNumArgs(8)
     );
setupfun("rawGB",rawGB);

export rawResolution(e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) != 7 then WrongNumArgs(7) else
     when s.0 is m:RawMatrix do
     when s.1 is resolveCokernel:Boolean do
     when s.2 is maxLevel:Integer do if !isInt(maxLevel) then WrongArgSmallInteger(3) else
     when s.3 is useMaxSlantedDegree:Boolean do
     when s.4 is maxSlantedDegree:Integer do if !isInt(maxSlantedDegree) then WrongArgSmallInteger(5) else
     when s.5 is algorithm:Integer do if !isInt(algorithm) then WrongArgSmallInteger(6) else
     when s.6 is strategy:Integer do if !isInt(strategy) then WrongArgSmallInteger(7) else
     if !isInt(algorithm) then WrongArgSmallInteger(8) else
     toExpr(
	  Ccode(RawComputationOrNull,
	       "(engine_RawComputationOrNull)IM2_res_make(",
		   "(Matrix*)",m,",",
		   isTrue(resolveCokernel),",",
		   toInt(maxLevel),",",
		   isTrue(useMaxSlantedDegree),",",
		   toInt(maxSlantedDegree),",",
		   toInt(algorithm),",",
		   toInt(strategy),
	       ")"
	       )
	  )
     else WrongArgInteger(7)
     else WrongArgInteger(6)
     else WrongArgInteger(5)
     else WrongArgBoolean(4)
     else WrongArgInteger(3)
     else WrongArgBoolean(2)
     else WrongArg(1,"a raw matrix")
     else WrongNumArgs(7)
     );
setupfun("rawResolution",rawResolution);

export rawGBSetHilbertFunction(e:Expr):Expr := (
     when e is a:Sequence do 
     if length(a) == 2 then 
     when a.0 is G:RawComputation do 
     when a.1 is h:RawRingElement do toExpr(
	  Ccode(RawComputationOrNull, 
		    "(engine_RawComputationOrNull)IM2_GB_set_hilbert_function(",
		    "(Computation *)", G,
		    ",(RingElement *)", h,
		    ")" ))
     else WrongArg(2,"a raw ring element")
     else WrongArg(1,"a raw computation")
     else WrongNumArgs(2)
     else WrongNumArgs(2));
setupfun("rawGBSetHilbertFunction", rawGBSetHilbertFunction);

export rawGBForce(e:Expr):Expr := (
     when e is a:Sequence do 
     if length(a) == 4 then 
     when a.0 is m:RawMatrix do 
     when a.1 is gb:RawMatrix do 
     when a.2 is change:RawMatrix do 
     when a.3 is syz:RawMatrix do toExpr(
	  Ccode(RawComputationOrNull, 
		    "(engine_RawComputationOrNull)IM2_GB_force(",
		    "(Matrix *)", m,
		    ",(Matrix *)", gb,
		    ",(Matrix *)", change,
		    ",(Matrix *)", syz,
		    ")" ))
     else WrongArg(4,"a raw matrix")
     else WrongArg(3,"a raw matrix")
     else WrongArg(2,"a raw matrix")
     else WrongArg(1,"a raw matrix")
     else WrongNumArgs(4)
     else WrongNumArgs(4));
setupfun("rawGBForce", rawGBForce);

export rawGBSetStop(e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) != 11 then WrongNumArgs(11) else
     when s.0 is G:RawComputation do
     when s.1 is always_stop:Boolean do
     when s.2 is stop_after_degree:Boolean do
     if !isSequenceOfSmallIntegers(s.3) then WrongArg(4,"a sequence of small integers") else -- degree_limit
     when s.4 is basis_element_limit:Integer do
     if !isInt(basis_element_limit) then WrongArgSmallInteger(5) else
     when s.5 is syzygy_limit:Integer do
     if !isInt(syzygy_limit) then WrongArgSmallInteger(6) else
     when s.6 is pair_limit:Integer do
     if !isInt(pair_limit) then WrongArgSmallInteger(7) else
     when s.7 is codim_limit:Integer do
     if !isInt(codim_limit) then WrongArgSmallInteger(8) else
     when s.8 is subring_limit:Integer do
     if !isInt(subring_limit) then WrongArgSmallInteger(9) else
     when s.9 is just_min_gens:Boolean do
     if !isSequenceOfSmallIntegers(s.10) then WrongArg(11,"a sequence of small integers") else -- length_limit
     toExpr(
	  Ccode(RawComputationOrNull,"(engine_RawComputationOrNull)IM2_Computation_set_stop(",
		    "(Computation *)", G, ",",
		    True == always_stop, ",",
		    True == stop_after_degree, ",",
		    "(M2_arrayint)", getSequenceOfSmallIntegers(s.3), ",",
		    toInt(basis_element_limit), ",",
		    toInt(syzygy_limit), ",",
		    toInt(pair_limit), ",",
		    toInt(codim_limit), ",",
		    toInt(subring_limit), ",",
		    True == just_min_gens, ",",
		    "(M2_arrayint)", getSequenceOfSmallIntegers(s.10),
	       ")"
	       )
	  )
     else WrongArgBoolean(10)
     else WrongArgInteger(9)
     else WrongArgInteger(8)
     else WrongArgInteger(7)
     else WrongArgInteger(6)
     else WrongArgInteger(5)
     else WrongArgBoolean(3)
     else WrongArgBoolean(2)
     else WrongArg("a raw computation")
     else WrongNumArgs(11)
     );
setupfun("rawGBSetStop", rawGBSetStop);

export rawStatus1(e:Expr):Expr := (
     when e is G:RawComputation do 
     toExpr(Ccode(int, "rawStatus1(", "(Computation *)", G, ")" ))
     else WrongArg("a raw Groebner basis computation"));
setupfun("rawStatus1", rawStatus1);

export rawStatus2(e:Expr):Expr := (
     when e is G:RawComputation do 
     toExpr(Ccode(int, "rawStatus2(", "(Computation *)", G, ")" ))
     else WrongArg("a raw Groebner basis computation"));
setupfun("rawStatus2", rawStatus2);

export rawGBGetMatrix(e:Expr):Expr := (
     when e is G:RawComputation do 
     toExpr(Ccode(RawMatrixOrNull, "(engine_RawMatrixOrNull)rawGBGetMatrix(", "(Computation *)", G, ")" ))
     else WrongArg("a raw Groebner basis computation"));
setupfun("rawGBGetMatrix", rawGBGetMatrix);

export rawGBMinimalGenerators(e:Expr):Expr := (
     when e is G:RawComputation do 
     toExpr(Ccode(RawMatrixOrNull, "(engine_RawMatrixOrNull)rawGBMinimalGenerators(", "(Computation *)", G, ")" ))
     else WrongArg("a raw Groebner basis computation"));
setupfun("rawGBMinimalGenerators", rawGBMinimalGenerators);

export rawGBChangeOfBasis(e:Expr):Expr := (
     when e is G:RawComputation do 
     toExpr(Ccode(RawMatrixOrNull, "(engine_RawMatrixOrNull)rawGBChangeOfBasis(", "(Computation *)", G, ")" ))
     else WrongArg("a raw Groebner basis computation"));
setupfun("rawGBChangeOfBasis", rawGBChangeOfBasis);

export rawGBSyzygies(e:Expr):Expr := (
     when e is G:RawComputation do 
     toExpr(Ccode(RawMatrixOrNull, "(engine_RawMatrixOrNull)rawGBSyzygies(", "(Computation *)", G, ")" ))
     else WrongArg("a raw Groebner basis computation"));
setupfun("rawGBSyzygies", rawGBSyzygies);

export rawResolutionGetMatrix(e:Expr):Expr := (
     when e is a:Sequence do 
     if length(a) == 2 then 
     when a.0 is G:RawComputation do 
     when a.1 is level:Integer do
     if !isInt(level) then WrongArgSmallInteger(2) else
     toExpr(Ccode(RawMatrixOrNull, "(engine_RawMatrixOrNull)rawResolutionGetMatrix(", "(Computation *)", G, ",", toInt(level), ")" ))
     else WrongArgInteger(2)
     else WrongArg(1,"a raw computation")
     else WrongNumArgs(2)
     else WrongNumArgs(2));
setupfun("rawResolutionGetMatrix", rawResolutionGetMatrix);

export rawResolutionStatusLevel(e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) != 3 then WrongNumArgs(3) else
     when s.0 is G:RawComputation do
     when s.1 is level:Integer do
     if !isInt(level) then WrongArgSmallInteger(2) else
     when s.2 is minimize:Boolean do (
	  completionDegree := 0;
	  ret := Ccode(int,"IM2_Resolution_status_level(",
	       "(Computation*)",G,",",
	       toInt(level), ",",
	       True == minimize, ",",
	       "&",completionDegree,
	       ")"
	       );
	  if ret == -1 then buildErrorPacket(EngineError("unknown raw computation status/level error")) 
	  else Expr(Sequence(toInteger(ret),toInteger(completionDegree)))
	  )
     else WrongArgBoolean(3)
     else WrongArgInteger(2)
     else WrongArg(1,"a raw computation")
     else WrongNumArgs(3)
     );
setupfun("rawResolutionStatusLevel", rawResolutionStatusLevel);

export rawGBGetLeadTerms(e:Expr):Expr := (
     when e is a:Sequence do 
     if length(a) == 2 then 
     when a.0 is G:RawComputation do 
     when a.1 is nparts:Integer do
     if !isInt(nparts) then WrongArgSmallInteger(2) else
     toExpr( Ccode(RawMatrixOrNull, "(engine_RawMatrixOrNull)rawGBGetLeadTerms(", "(Computation *)", G, ",", toInt(nparts), ")" ))
     else WrongArgInteger(2)
     else WrongArg(1,"a raw computation")
     else WrongNumArgs(2)
     else WrongNumArgs(2));
setupfun("rawGBGetLeadTerms", rawGBGetLeadTerms);

export rawResolutionGetFree(e:Expr):Expr := (
     when e is a:Sequence do 
     if length(a) == 2 then 
     when a.0 is G:RawComputation do 
     when a.1 is level:Integer do
     if !isInt(level) then WrongArgSmallInteger(2) else
     toExpr( Ccode(RawFreeModuleOrNull, "(engine_RawFreeModuleOrNull)rawResolutionGetFree(", "(Computation *)", G, ",", toInt(level), ")" ))
     else WrongArgInteger(2)
     else WrongArg(1,"a raw computation")
     else WrongNumArgs(2)
     else WrongNumArgs(2));
setupfun("rawResolutionGetFree", rawResolutionGetFree);

export rawGBMatrixRemainder(e:Expr):Expr := (
     when e is a:Sequence do 
     if length(a) == 2 then 
     when a.0 is G:RawComputation do 
     when a.1 is m:RawMatrix do toExpr( Ccode(RawMatrixOrNull, "(engine_RawMatrixOrNull)rawGBMatrixRemainder(", "(Computation *)", G, ",", "(Matrix*)", m, ")" ))
     else WrongArg(2,"a raw matrix")
     else WrongArg(1,"a raw computation")
     else WrongNumArgs(2)
     else WrongNumArgs(2));
setupfun("rawGBMatrixRemainder", rawGBMatrixRemainder);

toSequence(a:RawMatrixOrNull,b:RawMatrixOrNull):Expr := (
     when a
     is null do buildErrorPacket(EngineError("unknown raw matrix lift engine error"))
     is A:RawMatrix do
     when b
     is null do buildErrorPacket(EngineError("unknown raw matrix lift engine error"))
     is B:RawMatrix do
     Expr(Sequence(Expr(A),Expr(B))));

export rawGBMatrixLift(e:Expr):Expr := (
     when e is a:Sequence do 
     if length(a) == 2 then 
     when a.0 is G:RawComputation do 
     when a.1 is m:RawMatrix do (
	  resultRemainder := RawMatrixOrNull(NULL);
	  resultQuotient := RawMatrixOrNull(NULL);
	  Ccode(void, "IM2_GB_matrix_lift(",
	       "(Computation *)", G, ",",
	       "(Matrix*)", m, ",",
	       "(Matrix**)&", resultRemainder, ",",
	       "(Matrix**)&", resultQuotient,
     	       -- I'm ignoring these messages for now:
	       --  ../../../Macaulay2/d/interface.d:2391: warning: dereferencing type-punned pointer will break strict-aliasing rules
	       --  ../../../Macaulay2/d/interface.d:2391: warning: dereferencing type-punned pointer will break strict-aliasing rules
	       --  similar error messages come from "gcc -c -O3 -Wall" on this:
	       --  struct A;
	       --  struct B;
	       --  extern void h();
	       --  void f(struct B *p) { h((struct A**)&p); }
	       ")" );
	  toSequence(resultRemainder,resultQuotient))
     else WrongArg(2,"a raw matrix")
     else WrongArg(1,"a raw Groebner basis")
     else WrongNumArgs(2)
     else WrongNumArgs(2));
setupfun("rawGBMatrixLift", rawGBMatrixLift);

export rawGBContains(e:Expr):Expr := (
     when e is a:Sequence do 
     if length(a) == 2 then 
     when a.0 is G:RawComputation do 
     when a.1 is m:RawMatrix do toExpr(
	  Ccode(int, 
		    "IM2_GB_contains(",
		    "(Computation *)", G, ",",
		    "(Matrix*)", m,		    
		    ")" ))
     else WrongArg(2,"a raw matrix")
     else WrongArg(1,"a raw Groebner basis")
     else WrongNumArgs(2)
     else WrongNumArgs(2));
setupfun("rawGBContains", rawGBContains);

export rawGBBetti(e:Expr):Expr := (
     when e is a:Sequence do 
     if length(a) == 2 then 
     when a.0 is G:RawComputation do 
     when a.1 is type:Integer do
     if !isInt(type) then WrongArgSmallInteger(2) else
     toExpr( Ccode(RawArrayInt, "(engine_RawArrayInt)rawResolutionBetti(", "(Computation *)", G, ",", toInt(type), ")" ))
     else WrongArgInteger(2)
     else WrongArg(1,"a raw computation")
     else WrongNumArgs(2)
     else WrongNumArgs(2));
setupfun("rawGBBetti", rawGBBetti);


export rawSolve(e:Expr):Expr := (
     -- rawSolve(A, b, x)
     when e is s:Sequence do
     if length(s) != 3 then WrongNumArgs(3) else
     when s.0 is A:RawMutableMatrix do 
     when s.1 is b:RawMutableMatrix do
     when s.2 is x:RawMutableMatrix do (
	  if Ccode(bool, 
	       "(M2_bool)rawSolve(", "(MutableMatrix *)", A, ",", 
		       "(MutableMatrix *)", b, ",", "(MutableMatrix *)", x, ")")
	  then nullE
	  else buildErrorPacket(EngineError("error calling lapack solve routine")))
     else WrongArg(3,"a raw mutable matrix")
     else WrongArg(2,"a raw mutable matrix")
     else WrongArg(1, "a raw mutable matrix")
     else WrongNumArgs(3));
setupfun("rawSolve", rawSolve);


-----------------------------------------------------------------------------
-- LLL
-----------------------------------------------------------------------------

export rawLLL(e:Expr):Expr := (
     when e is s:Sequence do if length(s) != 2 then WrongNumArgs(2) else
     when s.0 is M:RawMutableMatrix do
     when s.1 is threshold:RawRingElement do (
	  if Ccode(bool, 
	       "(M2_bool)IM2_LLL(", 
	       "(MutableMatrix *)", M, ",", 
	       "(RingElement *)", threshold,
	       ")")
	  then nullE
	  else buildErrorPacket(EngineError("error calling engine LLL routine")))
     else WrongArg(1,"a raw ring element")
     else WrongArg(0,"a mutable raw matrix")
     else WrongNumArgs(2));
setupfun("rawLLL",rawLLL);

-----------------------------------------------------------------------------
-- LAPACK 
-----------------------------------------------------------------------------

M2CC(e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) == 2 then
     when s.0 is re:Real do 
     when s.1 is im:Real do (
	  Expr(
	       Ccode(Complex, "(tokens_Complex)LP_make_M2_Complex(", re.v, ",", 
		                     	         	      im.v, ")"))
	  )
     else WrongArg(2, "a double")
     else WrongArg(1, "a double")
     else WrongNumArgs(2)
     else WrongNumArgs(2));
setupfun("M2CC", M2CC);

-- seqarg(e:Expr,n:int,f(s:Sequence):Expr):Expr := when e is s:Sequence do if length(s) == n then f(s) else WrongNumArgs(n) else WrongNumArgs(n);

export rawMatrixRR(e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) == 2 then
     when s.0 is nrows:Integer do if !isInt(nrows) then WrongArgSmallInteger(1) else
     when s.1 is ncols:Integer do if !isInt(ncols) then WrongArgSmallInteger(2) else
     Expr( Ccode(LMatrixRR, "(engine_LMatrixRR)LP_LMatrixRR_make(", toInt(nrows), ",", toInt(ncols), ")"))
     else WrongArgInteger(2)
     else WrongArgInteger(1)
     else WrongNumArgs(2)
     else WrongNumArgs(2));
setupfun("rawMatrixRR", rawMatrixRR);

export rawMatrixCC(e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) == 2 then
     when s.0 is nrows:Integer do if !isInt(nrows) then WrongArgSmallInteger(1) else
     when s.1 is ncols:Integer do if !isInt(ncols) then WrongArgSmallInteger(2) else (
	  Expr(
	       Ccode(LMatrixCC, "(engine_LMatrixCC)LP_LMatrixCC_make(", toInt(nrows), ",", 
		                                                   toInt(ncols), ")"))
	  )
     else WrongArgInteger(2)
     else WrongArgInteger(1)
     else WrongNumArgs(2)
     else WrongNumArgs(2));
setupfun("rawMatrixCC", rawMatrixCC);

export rawGetEpsilonMatrixRR(e:Expr):Expr := (
     when e is s:Sequence do if length(s) == 0
     then Expr(Ccode(Real,"(tokens_Real)LP_LMatrixRR_get_epsilon()"))
     else WrongNumArgs(0)
     else WrongNumArgs(0)
     );
setupfun("rawGetEpsilonMatrixRR", rawGetEpsilonMatrixRR);

export rawGetEpsilonMatrixCC(e:Expr):Expr := (
     when e is s:Sequence do if length(s) == 0
     then Expr(Ccode(Real,"(tokens_Real)LP_LMatrixCC_get_epsilon()"))
     else WrongNumArgs(0)
     else WrongNumArgs(0)
     );
setupfun("rawGetEpsilonMatrixCC", rawGetEpsilonMatrixCC);

export rawSetEpsilonMatrixRR(e:Expr):Expr := (
     when e is eps:Real do (
     	Ccode(void,"LP_LMatrixRR_set_epsilon(", eps.v, ")");
	nullE
	)
     else WrongArg("a real")
     );
setupfun("rawSetEpsilonMatrixRR", rawSetEpsilonMatrixRR);

export rawSetEpsilonMatrixCC(e:Expr):Expr := (
     when e is eps:Real do (
     	Ccode(void,"LP_LMatrixCC_set_epsilon(", eps.v, ")");
	nullE
	)
     else WrongArg("a real")
     );
setupfun("rawSetEpsilonMatrixCC", rawSetEpsilonMatrixCC);

export rawNumberOfRowsRR(e:Expr):Expr := (
     when e is M:LMatrixRR do
	  toExpr(
	       Ccode(int, "LP_LMatrixRR_nrows(", "(LMatrixRR *)", M, ")") 
	  )
     else WrongArg("a raw matrixRR"));
setupfun("rawNumberOfRowsRR", rawNumberOfRowsRR);

export rawNumberOfColumnsRR(e:Expr):Expr := (
     when e is M:LMatrixRR do
	  toExpr(
	       Ccode(int, "LP_LMatrixRR_ncols(", "(LMatrixRR *)", M, ")") 
	  )
     else WrongArg("a raw matrixRR"));
setupfun("rawNumberOfColumnsRR", rawNumberOfColumnsRR);

export rawNumberOfRowsCC(e:Expr):Expr := (
     when e is M:LMatrixCC do
	  toExpr(
	       Ccode(int, "LP_LMatrixCC_nrows(", "(LMatrixCC *)", M, ")") 
	  )
     else WrongArg("a raw matrixCC"));
setupfun("rawNumberOfRowsCC", rawNumberOfRowsCC);

export rawNumberOfColumnsCC(e:Expr):Expr := (
     when e is M:LMatrixCC do
	  toExpr(
	       Ccode(int, "LP_LMatrixCC_ncols(", "(LMatrixCC *)", M, ")") 
	  )
     else WrongArg("a raw matrixCC"));
setupfun("rawNumberOfColumnsCC", rawNumberOfColumnsCC);

export rawSetMatrixEntryRR(e:Expr):Expr := (
     -- rawSetEntry(M, r, c, double)
     when e is s:Sequence do
     if length(s) == 4 then 
     when s.0 is M:LMatrixRR do
     when s.1 is r:Integer do if !isInt(r) then WrongArgSmallInteger(2) else
     when s.2 is c:Integer do if !isInt(c) then WrongArgSmallInteger(3) else 
     when s.3 is x:Real do (
	  Ccode(void, 
	       "LP_LMatrixRR_set_entry(", "(LMatrixRR *)", M, ",", 
	       toInt(r), ",", toInt(c), ",", x.v, ")" 
	       );
	  nullE
          )
     else WrongArg(4,"a double")
     else WrongArgInteger(3)
     else WrongArgInteger(2)
     else WrongArg(1,"a raw matrixRR")
     else WrongNumArgs(4)
     else WrongNumArgs(4));
setupfun("rawSetMatrixEntryRR", rawSetMatrixEntryRR);

export rawSetMatrixEntryCC(e:Expr):Expr := (
     -- rawSetEntry(M, r, c, Complex)
     when e is s:Sequence do
     if length(s) == 4 then 
     when s.0 is M:LMatrixCC do
     when s.1 is r:Integer do if !isInt(r) then WrongArgSmallInteger(2) else
     when s.2 is c:Integer do if !isInt(c) then WrongArgSmallInteger(3) else 
     when s.3 is x:Complex do (
	  Ccode(void, 
	       "LP_LMatrixCC_set_entry(", "(LMatrixCC *)", M, ",", 
	       toInt(r), ",", toInt(c), ",", "(M2_CC)", x,")" 
	       );
	  nullE
          )
     else WrongArg(4,"a complex")
     else WrongArgInteger(3)
     else WrongArgInteger(2)
     else WrongArg(1,"a raw matrixCC")
     else WrongNumArgs(4)
     else WrongNumArgs(4));
setupfun("rawSetMatrixEntryCC", rawSetMatrixEntryCC);

export rawGetMatrixEntryRR(e:Expr):Expr := (
     -- rawGetEntry(M, r, c, double)
     when e is s:Sequence do
     if length(s) == 4 then 
     when s.0 is M:LMatrixRR do
     when s.1 is r:Integer do if !isInt(r) then WrongArgSmallInteger(2) else
     when s.2 is c:Integer do if !isInt(c) then WrongArgSmallInteger(3) else 
     when s.3 is x:Real do (
	  Ccode(void, 
	       "LP_LMatrixRR_get_entry(", "(LMatrixRR *)", M, ",", 
	       toInt(r), ",", toInt(c), ",", "(double *)", x, ")" 
	       );
	  nullE
          )
     else WrongArg(4,"a double")
     else WrongArgInteger(3)
     else WrongArgInteger(2)
     else WrongArg(1,"a raw matrixRR")
     else WrongNumArgs(4)
     else WrongNumArgs(4));
setupfun("rawGetMatrixEntryRR", rawGetMatrixEntryRR);

export rawGetMatrixEntryCC(e:Expr):Expr := (
     -- rawGetEntry(M, r, c, complex)
     when e is s:Sequence do
     if length(s) == 4 then 
     when s.0 is M:LMatrixCC do
     when s.1 is r:Integer do if !isInt(r) then WrongArgSmallInteger(2) else
     when s.2 is c:Integer do if !isInt(c) then WrongArgSmallInteger(3) else 
     when s.3 is x:Complex do (
	  Ccode(void, 
	       "LP_LMatrixCC_get_entry(", "(LMatrixCC *)", M, ",", 
	       toInt(r), ",", toInt(c), ",", "(M2_CC)", x,")" 
	       );
	  nullE
          )
     else WrongArg(4,"a complex")
     else WrongArgInteger(3)
     else WrongArgInteger(2)
     else WrongArg(1,"a raw matrixCC")
     else WrongNumArgs(4)
     else WrongNumArgs(4));
setupfun("rawGetMatrixEntryCC", rawGetMatrixEntryCC);

export rawSetMatrixValues(e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) == 3 then
     when s.1 is l:List do (
     when isSequenceOfPairsOfSmallIntegers(l.v) is t:string do return WrongArg(t) else nothing;
     	when s.0 
--- wait for IM2_MutableMatrix_set_values to be redesigned
-- 	is M:RawMutableMatrix do (
-- 		if isSequenceOfRingElements(s.2) then toExpr(
-- 		     Ccode(bool,
-- 			"IM2_MutableMatrix_set_values(",
-- 			"(MutableMatrix*)", M, ",",
-- 			"(M2_arrayint)", getSequenceOfPairsOfSmallIntegers(l.v), ",",
-- 			"(RingElement_array*)", getSequenceOfRingElements(s.2), ")"
-- 			))
-- 		else WrongArg(3, "a sequence of raw ring elements")
-- 	     )
     	is M:LMatrixRR do (
		if isSequenceOfReals(s.2) then (
		     Ccode(void,
			"LP_LMatrixRR_set_values(",
			"(LMatrixRR *)", M, ",",
			"(M2_arrayint)", getSequenceOfPairsOfSmallIntegers(l.v), ",",
			"(M2_double_array *)", getSequenceOfReals(s.2), ")"
			);
			nullE
			)
		else WrongArg(3, "a sequence of reals")
		)
	is M:LMatrixCC do (
		if isSequenceOfComplex(s.2) then (
		     Ccode(void,
			"LP_LMatrixCC_set_values(",
			"(LMatrixCC *)", M, ",",
			"(M2_arrayint)", getSequenceOfPairsOfSmallIntegers(l.v), ",",
			"(M2_complex_array *)", getSequenceOfComplex(s.2), ")"
			);
			nullE
			)
		else WrongArg(3, "a sequence of complexes")
		)
	else WrongArg(1, "a matrixRR or matrixCC")
	)
     else WrongArg(2, "a list of pairs of integers")
     else WrongNumArgs(3)
     else WrongNumArgs(3)
     );
setupfun("rawSetMatrixValues",rawSetMatrixValues);

export rawGetSubmatrix(e:Expr):Expr := (
     when e is s:Sequence do
     if length(s) == 3 then
     when s.0
     is M:LMatrixRR do (
     if !isSequenceOfSmallIntegers(s.1) then WrongArg(2,"a sequence of small integers") else
     if !isSequenceOfSmallIntegers(s.2) then WrongArg(3,"a sequence of small integers") else (
	  rows := getSequenceOfSmallIntegers(s.1);
	  cols := getSequenceOfSmallIntegers(s.2);
	  toExpr(Ccode(LMatrixRROrNull, "(engine_LMatrixRROrNull)",
		    "LP_LMatrixRR_get_submatrix(",
		    "(LMatrixRR *)", M, ",",
		    "(M2_arrayint)", rows, ",",
		    "(M2_arrayint)", cols,
		    ")" ) ) )
	)
     is M:LMatrixCC do (
     if !isSequenceOfSmallIntegers(s.1) then WrongArg(2,"a sequence of small integers") else
     if !isSequenceOfSmallIntegers(s.2) then WrongArg(3,"a sequence of small integers") else (
	  rows := getSequenceOfSmallIntegers(s.1);
	  cols := getSequenceOfSmallIntegers(s.2);
	  toExpr(Ccode(LMatrixCCOrNull, "(engine_LMatrixCCOrNull)",
		    "LP_LMatrixCC_get_submatrix(",
		    "(LMatrixCC *)", M, ",",
		    "(M2_arrayint)", rows, ",",
		    "(M2_arrayint)", cols,
		    ")" ) ) )
	)
     else WrongArg(1,"a raw matrixRR or raw matrixCC")
     else WrongNumArgs(3)
     else WrongNumArgs(3)
     );
setupfun("rawGetSubmatrix",rawGetSubmatrix);


export rawLU(e:Expr):Expr := (
     -- rawLU(M, L, U, P)
     when e is s:Sequence do
     if length(s) != 4 then WrongNumArgs(4) else
     when s.0
     is M:LMatrixRR do (
	when s.1 is L:LMatrixRR do
	when s.2 is U:LMatrixRR do
	when s.3 is P:LMatrixRR do (
	  toExpr(
	  Ccode(LMatrixRROrNull, "(engine_LMatrixRROrNull)LP_LMatrixRR_LU(", "(LMatrixRR *)", M, ",", 
			"(LMatrixRR *)", L, ",", "(LMatrixRR *)", U, ",",
			"(LMatrixRR *)", P, ")"))
	  )
	else WrongArg(4,"a raw matrixRR")
	else WrongArg(3,"a raw matrixRR")
	else WrongArg(2,"a raw matrixRR")
	)
      is M:LMatrixCC do (
	when s.1 is L:LMatrixCC do
	when s.2 is U:LMatrixCC do
	when s.3 is P:LMatrixRR do (
	  toExpr(
	  Ccode(LMatrixCCOrNull, "(engine_LMatrixCCOrNull)LP_LMatrixCC_LU(", "(LMatrixCC *)", M, ",", 
			"(LMatrixCC *)", L, ",", "(LMatrixCC *)", U, ",",
			"(LMatrixRR *)", P, ")"))
	  )
	else WrongArg(4,"a raw matrixRR")
	else WrongArg(3,"a raw matrixCC")
	else WrongArg(2,"a raw matrixCC")
	)
     else WrongArg(1, "a raw matrixRR or raw matrixCC")
     else WrongNumArgs(4));
setupfun("rawLU", rawLU);

export rawEigenvalues(e:Expr):Expr := (
     -- rawEigenvalues(M, eigs)
     when e is s:Sequence do
     if length(s) != 2 then WrongNumArgs(2) else
     when s.0 
     is M:LMatrixRR do (
        when s.1 is eigs:LMatrixCC do (
	toExpr(
	Ccode(LMatrixCCOrNull, "(engine_LMatrixCCOrNull)LP_LMatrixRR_eigenvalues(", "(LMatrixRR *)", M, ",", 
		            	"(LMatrixCC *)", eigs, ")"))
	)
        else WrongArg(2, "a raw matrixCC"))
     is N:LMatrixCC do (
        when s.1 is eigs:LMatrixCC do (
	toExpr(
	Ccode(LMatrixCCOrNull, "(engine_LMatrixCCOrNull)LP_LMatrixCC_eigenvalues(", "(LMatrixCC *)", N, ",", 
		            	"(LMatrixCC *)", eigs, ")"))
	)
        else WrongArg(2, "a raw matrixCC"))
     else WrongArg(1, "a raw matrixRR or raw matrixCC")
     else WrongNumArgs(2));
setupfun("rawEigenvalues", rawEigenvalues);

export rawEigenvectors(e:Expr):Expr := (
     -- rawEigenvalues(M, eigvals, eigvecs)
     when e is s:Sequence do
     if length(s) != 3 then WrongNumArgs(3) else
     when s.0 
     is M:LMatrixRR do (
     	when s.1 is eigvals:LMatrixCC do 
     	when s.2 is eigvecs:LMatrixCC do (
	toExpr(
	Ccode(LMatrixCCOrNull, "(engine_LMatrixCCOrNull)LP_LMatrixRR_eigenvectors(", "(LMatrixRR *)", M, ",", 
		       "(LMatrixCC *)", eigvals, ",", "(LMatrixCC *)", eigvecs, ")"))
	)
	else WrongArg(3,"a raw matrixCC")
	else WrongArg(2,"a raw matrixCC"))
     is N:LMatrixCC do (
     	when s.1 is eigvals:LMatrixCC do 
     	when s.2 is eigvecs:LMatrixCC do (
	toExpr(
	Ccode(LMatrixCCOrNull, "(engine_LMatrixCCOrNull)LP_LMatrixCC_eigenvectors(", "(LMatrixCC *)", N, ",", 
		       "(LMatrixCC *)", eigvals, ",", "(LMatrixCC *)", eigvecs, ")"))
	)
	else WrongArg(3,"a raw matrixCC")
	else WrongArg(2,"a raw matrixCC"))
     else WrongArg(1, "a raw matrixRR or raw matrixCC")
     else WrongNumArgs(3));
setupfun("rawEigenvectors", rawEigenvectors);

export rawEigenvaluesSymmetric(e:Expr):Expr := (
     -- rawEigenvaluesSymmetric(M, eigs)
     when e is s:Sequence do
     if length(s) == 2 then
     when s.0 
     is M:LMatrixRR do (
     	when s.1 is eigs:LMatrixRR do (
	  toExpr(
	  Ccode(LMatrixRROrNull, "(engine_LMatrixRROrNull)LP_LMatrixRR_eigenvalues_symmetric(", "(LMatrixRR *)", 
		M, ",", "(LMatrixRR *)", eigs, ")"))
	  )
       else WrongArg(2, "a raw matrixRR"))
     else WrongArg(1, "a raw matrixRR")
     else WrongNumArgs(2)
     else WrongNumArgs(2));
setupfun("rawEigenvaluesSymmetric", rawEigenvaluesSymmetric);

export rawEigenvectorsSymmetric(e:Expr):Expr := (
     -- rawEigenvectorsSymmetric(M, eigvals, eigvecs)
     when e is s:Sequence do
     if length(s) == 3 then
     when s.0 
     is M:LMatrixRR do (
     	when s.1 is eigvals:LMatrixRR do 
     	when s.2 is eigvecs:LMatrixRR do (
	  toExpr(
	  Ccode(LMatrixRROrNull, "(engine_LMatrixRROrNull)LP_LMatrixRR_eigenvectors_symmetric(", "(LMatrixRR *)", 
		M, ",", "(LMatrixRR *)", eigvals, ",", "(LMatrixRR *)", eigvecs, ")"))
	  )
       	else WrongArg(3, "a raw matrixRR")
       	else WrongArg(2, "a raw matrixRR"))
     else WrongArg(1,"a raw matrixRR")
     else WrongNumArgs(3)
     else WrongNumArgs(3));
setupfun("rawEigenvectorsSymmetric", rawEigenvectorsSymmetric);

export rawEigenvaluesHermitian(e:Expr):Expr := (
     -- rawEigenvaluesHermitian(M, eigs)
     when e is s:Sequence do
     if length(s) == 2 then
     when s.0 is M:LMatrixCC do
     when s.1 is eigs:LMatrixRR do (
	toExpr(
	Ccode(LMatrixRROrNull, "(engine_LMatrixRROrNull)LP_LMatrixCC_eigenvalues_hermitian(", "(LMatrixCC *)", 
		M, ",", "(LMatrixRR *)", eigs, ")"))
	)
     else WrongArg(2, "a raw matrixRR")
     else WrongArg(1, "a raw matrixCC")
     else WrongNumArgs(2)
     else WrongNumArgs(2));
setupfun("rawEigenvaluesHermitian", rawEigenvaluesHermitian);

export rawEigenvectorsHermitian(e:Expr):Expr := (
     -- rawEigenvectorsHermitian(M, eigvals, eigvecs)
     when e is s:Sequence do
     if length(s) == 3 then
     when s.0 is M:LMatrixCC do
     when s.1 is eigvals:LMatrixRR do 
     when s.2 is eigvecs:LMatrixCC do (
	toExpr(
	Ccode(LMatrixCCOrNull, "(engine_LMatrixCCOrNull)LP_LMatrixCC_eigenvectors_hermitian(", "(LMatrixCC *)", 
		M, ",", "(LMatrixRR *)", eigvals, ",", "(LMatrixCC *)", eigvecs, ")"))
	)
     else WrongArg(3, "a raw matrixCC")
     else WrongArg(2, "a raw matrixRR")
     else WrongArg(1, "a raw matrixCC")
     else WrongNumArgs(3)
     else WrongNumArgs(3));
setupfun("rawEigenvectorsHermitian", rawEigenvectorsHermitian);

export rawSVD(e:Expr):Expr := (
     -- rawSVD(M, Sigma, U, VT)
     when e is s:Sequence do
     if length(s) != 4 then WrongNumArgs(4) else
     when s.0 
     is M:LMatrixRR do (
	when s.1 is Sigma:LMatrixRR do
     	when s.2 is U:LMatrixRR do
     	when s.3 is VT:LMatrixRR do
	toExpr(
	Ccode(LMatrixRROrNull, "(engine_LMatrixRROrNull)LP_LMatrixRR_SVD(", "(LMatrixRR *)", M, ",", 
		       "(LMatrixRR *)", Sigma, ",", "(LMatrixRR *)", U, ",",
		       "(LMatrixRR *)", VT, ")"))
	else WrongArg(4, "a raw matrixRR")
	else WrongArg(3, "a raw matrixRR")
	else WrongArg(2, "a raw matrixRR")
	)
     is M:LMatrixCC do (
	when s.1 is Sigma:LMatrixRR do
     	when s.2 is U:LMatrixCC do
     	when s.3 is VT:LMatrixCC do
	toExpr(
	Ccode(LMatrixRROrNull, "(engine_LMatrixRROrNull)LP_LMatrixCC_SVD(", "(LMatrixCC *)", M, ",", 
		       "(LMatrixRR *)", Sigma, ",", "(LMatrixCC *)", U, ",",
		       "(LMatrixCC *)", VT, ")"))
	else WrongArg(4, "a raw matrixCC")
	else WrongArg(3, "a raw matrixCC")
	else WrongArg(2, "a raw matrixRR")
	)
     else WrongArg(1, "a raw matrixRR or raw matrixCC")
     else WrongNumArgs(4));
setupfun("rawSVD", rawSVD);

export rawLeastSquares(e:Expr):Expr := (
     -- rawLeastSquares(M, b, x)
     when e is s:Sequence do
     if length(s) != 3 then WrongNumArgs(3) else
     when s.0 
     is M:LMatrixRR do (
     	when s.1 is b:LMatrixRR do 
     	when s.2 is x:LMatrixRR do (
	toExpr(
	Ccode(LMatrixRROrNull, "(engine_LMatrixRROrNull)LP_LMatrixRR_least_squares(", "(LMatrixRR *)", M, ",", 
		       "(LMatrixRR *)", b, ",", "(LMatrixRR *)", x, ")"))
	)
	else WrongArg(3,"a raw matrixRR")
	else WrongArg(2,"a raw matrixRR"))
     is M:LMatrixCC do (
     	when s.1 is b:LMatrixCC do 
     	when s.2 is x:LMatrixCC do (
	toExpr(
	Ccode(LMatrixCCOrNull, "(engine_LMatrixCCOrNull)LP_LMatrixCC_least_squares(", "(LMatrixCC *)", M, ",", 
		       "(LMatrixCC *)", b, ",", "(LMatrixCC *)", x, ")"))
	)
	else WrongArg(3,"a raw matrixCC")
	else WrongArg(2,"a raw matrixCC"))
     else WrongArg(1, "a raw matrixRR or raw matrixCC")
     else WrongNumArgs(3));
setupfun("rawLeastSquares", rawLeastSquares);

export rawLeastSquaresDeficient(e:Expr):Expr := (
     -- rawLeastSquaresDeficient(M, b, x)
     when e is s:Sequence do
     if length(s) != 3 then WrongNumArgs(3) else
     when s.0 
     is M:LMatrixRR do (
     	when s.1 is b:LMatrixRR do 
     	when s.2 is x:LMatrixRR do (
	toExpr(
	Ccode(LMatrixRROrNull, "(engine_LMatrixRROrNull)LP_LMatrixRR_least_squares_deficient(", 
			"(LMatrixRR *)", M, ",", 
		        "(LMatrixRR *)", b, ",", "(LMatrixRR *)", x, ")"))
	)
	else WrongArg(3,"a raw matrixRR")
	else WrongArg(2,"a raw matrixRR"))
     is M:LMatrixCC do (
     	when s.1 is b:LMatrixCC do 
     	when s.2 is x:LMatrixCC do (
	toExpr(
	Ccode(LMatrixCCOrNull, "(engine_LMatrixCCOrNull)LP_LMatrixCC_least_squares_deficient(", 
			"(LMatrixCC *)", M, ",", 
		        "(LMatrixCC *)", b, ",", "(LMatrixCC *)", x, ")"))
	)
	else WrongArg(3,"a raw matrixCC")
	else WrongArg(2,"a raw matrixCC"))
     else WrongArg(1, "a raw matrixRR or raw matrixCC")
     else WrongNumArgs(3));
setupfun("rawLeastSquaresDeficient", rawLeastSquaresDeficient);

-- Local Variables:
-- compile-command: "make -C $M2BUILDDIR/Macaulay2/d "
-- End:
