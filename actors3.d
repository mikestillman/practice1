--		Copyright 1994 by Daniel R. Grayson

use C;
use system; 
use convertr;
use engine;
use binding;
use evaluate;
use common;
use parser;
use lex;
use gmp;
use nets;
use tokens;
use err;
use stdiop;
use ctype;
use stdio;
use varstrin;
use strings;
use C;
use actors;
use basic;
use struct;
use objects;
-----------------------------------------------------------------------------
isOption(c:HashTable,e:Expr):bool := (
     when e
     is b:List do b.class == optionClass && length(b.v) == 2 
     is q:HashTable do q.class == c
     else false
     );
numOptions(c:HashTable,w:Sequence):int := (
     n := 0;
     foreach x in w do if isOption(c,x) then n = n+1;
     n);
override(h:HashTable,v:Sequence,numopts:int):Expr := (
     numargs := length(v) - numopts;
     newargs := nullE;
     if numargs == 0 then (newargs = emptySequenceE;)
     else if numargs == 1 then foreach x in v do (if !isOption(h.class,x) then newargs = x)
     else (
	  newargs = Expr(
	       new Sequence len numargs do (
	       	    foreach x in v do if !isOption(h.class,x) then provide x));
	  );
     z := copy(h);
     z.mutable = true;
     foreach x in v do if isOption(h.class,x) then (
	  when x is b:List do (
	       key := b.v.0;
	       keyhash := hash(key);
	       r := storeInHashTableMustClobber(z,key,keyhash,b.v.1);
	       when r is Error do return r else nothing;
	       )
	  is y:HashTable do (
	       foreach bucket in y.table do (
		    q := bucket;
		    while q != bucketEnd do (
			 r := storeInHashTableMustClobber(z,q.key,q.hash,q.value);
			 when r is Error do return r else nothing;
			 q = q.next)))
	  else nothing;					    -- shouldn't occur
	  );
     sethash(z,h.mutable);
     Expr(Sequence(Expr(z),newargs)));
override(e:Expr):Expr := (
     when e is args:Sequence do (
	  if length(args) == 2 then (
	       when args.0 is h:HashTable do (
		    if h.mutable then WrongArg("an immutable hash table")
		    else when args.1 is v:Sequence do (
			 n := numOptions(h.class,v);
			 if n == 0 then e else override(h,v,n)
			 )
		    else (
			 if !isOption(h.class,args.1) then e
			 else override(h,Sequence(args.1),1)))
	       else WrongArg(1,"a hashtable"))
	  else WrongNumArgs(2))
     else WrongNumArgs(2));
setupfun("override",override);
-----------------------------------------------------------------------------
EqualEqualfun(lhs:Code,rhs:Code):Expr := (
     x := eval(lhs);
     when x is Error do x
     else (
     	  y := eval(rhs);
     	  when y is Error do y
	  else if x == y || equal(x,y) == True then True
	  else (
	       cx := Class(x);
	       cy := Class(y);
	       if cx == cy && (
		    cx == integerClass ||
		    cx == symbolClass ||
		    cx == rationalClass ||
		    cx == doubleClass ||
		    cx == bigRealClass ||
		    cx == booleanClass ||
		    cx == netClass ||
		    cx == stringClass ||
		    cx == functionClass ||
		    cx == booleanClass
		    )
	       then False
	       else (
		    method := lookupBinaryMethod(cx,cy,EqualEqualS);
		    if method == nullE 
		    then MissingMethodPair(EqualEqualS,x,y)
		    else apply(method,x,y)))));
setup(EqualEqualS,EqualEqualfun);
not(z:Expr):Expr := (
     when z is Error do z 
     else if z == True then False 
     else if z == False then True
     else buildErrorPacket("expected true or false"));
notequalfun(lhs:Code,rhs:Code):Expr := not(EqualEqualfun(lhs,rhs));
setup(NotEqualS,notequalfun);

compare(left:Expr,right:Expr):Expr := (
     if left == right then EqualE else
     when left
     is x:Integer do (
	  when right
	  is y:Real do 
	  if x < y.v then LessE else if x > y.v then GreaterE else EqualE
	  is y:Integer do
	  if x < y then LessE else if x > y then GreaterE else EqualE
	  is y:Rational do
	  if x < y then LessE else if x > y then GreaterE else EqualE
	  is y:BigReal do (
	       r := compare(toBigReal(x),y);
	       if r < 0 then LessE else if r > 0 then GreaterE else EqualE
	       )
     	  is Error do right
	  else binarymethod(left,right,QuestionS))
     is x:SymbolClosure do (
	  when right
	  is y:SymbolClosure do (
	       c := strcmp(x.symbol.word.name,y.symbol.word.name);
	       if c == 1 then LessE
	       else if c == -1 then GreaterE
	       else (
		    if x.symbol.hash < y.symbol.hash then GreaterE
		    else if x.symbol.hash > y.symbol.hash then LessE
		    else (
			 -- if we had a Sequence number stored in each fram
			 -- we could make the rest of the comparison well-defined
			 EqualE
			 )
		    )
	       )
	  else binarymethod(left,right,QuestionS))
     is x:Rational do (
	  when right
	  is y:Real do
	  if x < y.v then LessE else if x > y.v then GreaterE else EqualE
	  is y:Integer do 
	  if x < y then LessE else if x > y then GreaterE else EqualE
	  is y:Rational do
	  if x < y then LessE else if x > y then GreaterE else EqualE
	  is y:BigReal do (
	       r := compare(toBigReal(x),y);
	       if r < 0 then LessE else if r > 0 then GreaterE else EqualE
	       )
     	  is Error do right
	  else binarymethod(left,right,QuestionS))
     is x:BigReal do (
	  when right
	  is y:Real do (
	       r := compare(x,toBigReal(y.v));
	       if r < 0 then LessE else if r > 0 then GreaterE else EqualE
	       )
	  is y:Integer do (
	       r := compare(x,toBigReal(y));
	       if r < 0 then LessE else if r > 0 then GreaterE else EqualE
	       )
	  is y:Rational do (
	       r := compare(x,toBigReal(y));
	       if r < 0 then LessE else if r > 0 then GreaterE else EqualE
	       )
	  is y:BigReal do (
	       r := compare(x,y);
	       if r < 0 then LessE else if r > 0 then GreaterE else EqualE
	       )
     	  is Error do right
	  else binarymethod(left,right,QuestionS))
     is x:Real do (
	  when right
	  is y:Real do
	  if x.v < y.v then LessE else if x.v > y.v then GreaterE else EqualE
	  is y:Integer do
	  if x.v < y then LessE else if x.v > y then GreaterE else EqualE
	  is y:Rational do
	  if x.v < y then LessE else if x.v > y then GreaterE else EqualE
	  is y:BigReal do (
	       r := compare(toBigReal(x.v),y);
	       if r < 0 then LessE else if r > 0 then GreaterE else EqualE
	       )
     	  is Error do right
	  else binarymethod(left,right,QuestionS))
     is x:string do (
	  when right
	  is y:string do if x < y then LessE else if x > y then GreaterE else EqualE
     	  is Error do right
	  else binarymethod(left,right,QuestionS))
     is x:Net do (
	  when right
	  is y:Net do if x < y then LessE else if x > y then GreaterE else EqualE
     	  is Error do right
	  else binarymethod(left,right,QuestionS))
     is Error do left
     else (
	  when right
	  is Error do right
	  else binarymethod(left,right,QuestionS)));
compareop(lhs:Code,rhs:Code):Expr := (
     x := eval(lhs);
     when x
     is Error do x
     else (
	  y := eval(rhs);
	  when y
	  is Error do y
	  else compare(x,y)));
setup(QuestionS,compareop);

lessfun1(rhs:Code):Expr := unarymethod(rhs,LessS);
lessfun2(lhs:Code,rhs:Code):Expr := (
     e := compareop(lhs,rhs);
     when e 
     is Error do e
     else if LessS.symbol === e then True else False
     );
setup(LessS,lessfun1,lessfun2);

greaterequalfun1(rhs:Code):Expr := unarymethod(rhs,GreaterEqualS);
greaterequalfun2(lhs:Code,rhs:Code):Expr := (
     e := compareop(lhs,rhs);
     when e 
     is Error do e
     else if GreaterS.symbol === e || EqualEqualS.symbol === e then True else False
     );
setup(GreaterEqualS,greaterequalfun1,greaterequalfun2);

greaterfun1(rhs:Code):Expr := unarymethod(rhs,GreaterS);
greaterfun2(lhs:Code,rhs:Code):Expr := (
     e := compareop(lhs,rhs);
     when e 
     is Error do e
     else if GreaterS.symbol === e then True else False
     );
setup(GreaterS,greaterfun1,greaterfun2);

lessequalfun1(rhs:Code):Expr := unarymethod(rhs,LessEqualS);
lessequalfun2(lhs:Code,rhs:Code):Expr := (
     e := compareop(lhs,rhs);
     when e 
     is Error do e
     else if LessS.symbol === e || EqualEqualS.symbol === e then True else False
     );
setup(LessEqualS,lessequalfun1,lessequalfun2);

mergepairs(xx:Expr,yy:Expr,f:Expr):Expr := (
     when xx is xl:List do
     when yy is yl:List do (
	  x := xl.v;
	  y := yl.v;
	  n := 0;
	  z := new Sequence len length(x)+length(y) do provide nullE;
	  i := 0;
	  j := 0;
	  while true do (
	       if i >= length(x) then (
		    while j < length(y) do (z.n = y.j; j = j+1; n = n+1; );
		    break;
		    )
	       else if j >= length(y) then (
		    while i < length(x) do (z.n = x.i; i = i+1; n = n+1; );
		    break;
		    );
	       when x.i
	       is xi:Sequence do 
	       if length(xi) != 2
	       then return WrongArg(1,"a list of pairs")
	       else
	       when y.j
	       is yj:Sequence do
	       if length(yj) != 2
	       then return WrongArg(2,"a list of pairs")
	       else (
		    c := compare(xi.0,yj.0);
		    when c is Error do return c else nothing;
		    if GreaterS.symbol === c then (
			 z.n = yj;
			 j = j+1;
			 n = n+1;
			 )
		    else if LessS.symbol === c then (
			 z.n = xi;
			 i = i+1;
			 n = n+1;
			 )
		    else (
			 z.n = Sequence(xi.0, apply(f,xi.1,yj.1));
			 i = i+1;
			 j = j+1;
			 n = n+1;
			 ))
	       else return WrongArg(2,"a list of pairs")
	       else return WrongArg(1,"a list of pairs"));
	  if n < length(x)+length(y)
	  then z = new Sequence len n do foreach a in z do provide a;
	  Expr(sethash(List(commonAncestor(xl.class,yl.class), z,0,false),xl.mutable | yl.mutable)))
     else WrongArg(2,"a list")
     else WrongArg(1,"a list"));
mergepairsfun(e:Expr):Expr := (
     when e
     is a:Sequence do
     if length(a) == 3 then
     mergepairs(a.0,a.1,a.2)
     else WrongNumArgs(3)
     else WrongNumArgs(3));
setupfun("mergePairs",mergepairsfun);

--rmergepairs(xx:Expr,yy:Expr,f:Expr):Expr := (
--     when xx is xl:List do
--     when yy is yl:List do (
--	  x := xl.v;
--	  y := yl.v;
--	  n := 0;
--	  z := new Sequence len length(x)+length(y) do provide nullE;
--	  i := 0;
--	  j := 0;
--	  while true do (
--	       if i >= length(x) then (
--		    while j < length(y) do (z.n = y.j; j = j+1; n = n+1; );
--		    break;
--		    )
--	       else if j >= length(y) then (
--		    while i < length(x) do (z.n = x.i; i = i+1; n = n+1; );
--		    break;
--		    );
--	       when x.i
--	       is xi:Sequence do 
--	       if length(xi) != 2
--	       then return WrongArg(1,"a list of pairs")
--	       else
--	       when y.j
--	       is yj:Sequence do
--	       if length(yj) != 2
--	       then return WrongArg(2,"a list of pairs")
--	       else (
--		    c := compare(xi.0,yj.0);
--		    when c is Error do return c else nothing;
--		    if LessS.symbol === c then (
--			 z.n = yj;
--			 j = j+1;
--			 n = n+1;
--			 )
--		    else if GreaterS.symbol === c then (
--			 z.n = xi;
--			 i = i+1;
--			 n = n+1;
--			 )
--		    else (
--			 z.n = Sequence(xi.0, apply(f,xi.1,yj.1));
--			 i = i+1;
--			 j = j+1;
--			 n = n+1;
--			 ))
--	       else return WrongArg(2,"a list of pairs")
--	       else return WrongArg(1,"a list of pairs"));
--	  if n < length(x)+length(y)
--	  then z = new Sequence len n do foreach a in z do provide a;
--	  Expr(sethash(List(commonAncestor(xl.class,yl.class), z,0,false),xl.mutable | yl.mutable)))
--     else WrongArg(2,"a list")
--     else WrongArg(1,"a list"));
--rmergepairsfun(e:Expr):Expr := (
--     when e
--     is a:Sequence do
--     if length(a) == 3 then
--     rmergepairs(a.0,a.1,a.2)
--     else WrongNumArgs(3)
--     else WrongNumArgs(3));
--setupfun("rmergepairs",rmergepairsfun);
bitxorfun(e:Expr):Expr := (
     when e is a:Sequence do
     if length(a) == 2 then
     when a.0
     is err:Error do Expr(err)
     is x:Integer do
     when a.1
     is err:Error do Expr(err)
     is y:Integer do Expr(x ^^ y)
     else WrongArgInteger(2)
     else WrongArgInteger(1)
     else WrongNumArgs(2)
     else WrongNumArgs(2));
setupfun("xor",bitxorfun);
semicolonfun(lhs:Code,rhs:Code):Expr := when eval(lhs) is err:Error do Expr(err) else eval(rhs);
setup(semicolonS,semicolonfun);
starfun(rhs:Code):Expr := unarymethod(rhs,StarS);
timesfun(lhs:Code,rhs:Code):Expr := (
     l := eval(lhs);
     when l is Error do l
     else (
     	  r := eval(rhs);
     	  when r is Error do r
	  else l*r));
setup(StarS,starfun,timesfun);

-- functions

sin(e:Expr):Expr := (
     when e
     is x:Real do Expr(Real(sin(x.v)))
     is x:Integer do Expr(Real(sin(toDouble(x))))
     is x:Rational do Expr(Real(sin(toDouble(x))))
     is x:Error do Expr(x)
     else buildErrorPacket("expected a number")
     );
setupfun("sin",sin);
cos(e:Expr):Expr := (
     when e
     is x:Real do Expr(Real(cos(x.v)))
     is x:Integer do Expr(Real(cos(toDouble(x))))
     is x:Rational do Expr(Real(cos(toDouble(x))))
     is x:Error do Expr(x)
     else buildErrorPacket("expected a number")
     );
setupfun("cos",cos);
tan(e:Expr):Expr := (
     when e
     is x:Real do Expr(Real(tan(x.v)))
     is x:Integer do Expr(Real(tan(toDouble(x))))
     is x:Rational do Expr(Real(tan(toDouble(x))))
     is x:Error do Expr(x)
     else buildErrorPacket("expected a number")
     );
setupfun("tan",tan);
acos(e:Expr):Expr := (
     when e
     is x:Real do Expr(Real(acos(x.v)))
     is x:Integer do Expr(Real(acos(toDouble(x))))
     is x:Rational do Expr(Real(acos(toDouble(x))))
     is x:Error do Expr(x)
     else buildErrorPacket("expected a number")
     );
setupfun("acos",acos);
asin(e:Expr):Expr := (
     when e
     is x:Real do Expr(Real(asin(x.v)))
     is x:Integer do Expr(Real(asin(toDouble(x))))
     is x:Rational do Expr(Real(asin(toDouble(x))))
     is x:Error do Expr(x)
     else buildErrorPacket("expected a number")
     );
setupfun("asin",asin);
atan(e:Expr):Expr := (
     when e
     is x:Real do Expr(Real(atan(x.v)))
     is x:Integer do Expr(Real(atan(toDouble(x))))
     is x:Rational do Expr(Real(atan(toDouble(x))))
     is x:Error do Expr(x)
     is a:Sequence do if length(a) != 2
     then WrongNumArgs(1,2)
     else when a.1
     is y:Real do when a.0
     is x:Real do Expr(Real(atan2(y.v,x.v)))
     is x:Integer do Expr(Real(atan2(y.v,toDouble(x))))
     is x:Rational do Expr(Real(atan2(y.v,toDouble(x))))
     else WrongArg(2,"a number")
     is y:Integer do when a.0
     is x:Real do Expr(Real(atan2(toDouble(y),x.v)))
     is x:Integer do Expr(Real(atan2(toDouble(y),toDouble(x))))
     is x:Rational do Expr(Real(atan2(toDouble(y),toDouble(x))))
     else WrongArg(2,"a number")
     is y:Rational do when a.0
     is x:Real do Expr(Real(atan2(toDouble(y),x.v)))
     is x:Integer do Expr(Real(atan2(toDouble(y),toDouble(x))))
     is x:Rational do Expr(Real(atan2(toDouble(y),toDouble(x))))
     else WrongArg(2,"a number")
     else WrongArg(1,"a number")
     else buildErrorPacket("expected a number or a pair of numbers")
     );
setupfun("atan",atan);
cosh(e:Expr):Expr := (
     when e
     is x:Real do Expr(Real(cosh(x.v)))
     is x:Integer do Expr(Real(cosh(toDouble(x))))
     is x:Rational do Expr(Real(cosh(toDouble(x))))
     is x:Error do Expr(x)
     else buildErrorPacket("expected a number")
     );
setupfun("cosh",cosh);
sinh(e:Expr):Expr := (
     when e
     is x:Real do Expr(Real(sinh(x.v)))
     is x:Integer do Expr(Real(sinh(toDouble(x))))
     is x:Rational do Expr(Real(sinh(toDouble(x))))
     is x:Error do Expr(x)
     else buildErrorPacket("expected a number")
     );
setupfun("sinh",sinh);
tanh(e:Expr):Expr := (
     when e
     is x:Real do Expr(Real(tanh(x.v)))
     is x:Integer do Expr(Real(tanh(toDouble(x))))
     is x:Rational do Expr(Real(tanh(toDouble(x))))
     is x:Error do Expr(x)
     else buildErrorPacket("expected a number")
     );
setupfun("tanh",tanh);
exp(e:Expr):Expr := (
     when e
     is x:Real do Expr(Real(exp(x.v)))
     is x:Integer do Expr(Real(exp(toDouble(x))))
     is x:Rational do Expr(Real(exp(toDouble(x))))
     is x:Error do Expr(x)
     else buildErrorPacket("expected a number")
     );
setupfun("exp",exp);
log(e:Expr):Expr := (
     when e
     is x:Real do Expr(Real(log(x.v)))
     is x:Integer do Expr(Real(log(toDouble(x))))
     is x:Rational do Expr(Real(log(toDouble(x))))
     is x:Error do Expr(x)
     else buildErrorPacket("expected a number")
     );
setupfun("log",log);
abs(x:double):double := if x < 0. then -x else x;
floor(e:Expr):Expr := (
     when e
     is x:Real do (
	  if finite(x.v) then Expr(Floor(x.v))
	  else WrongArg("a finite real number")
	  )
     is x:Rational do Expr(floor(x))
     is x:Error do Expr(x)
     else buildErrorPacket("expected a number")
     );
setupfun("floor",floor);

run(e:Expr):Expr := (
     when e
     is x:string do Expr(toInteger(run(x)))
     is x:Error do Expr(x)
     else buildErrorPacket("expected a string")
     );
setupfun("run",run);

sqrt(a:Expr):Expr := (
     when a
     is x:Real do Expr(Real(sqrt(x.v)))
     is x:BigReal do Expr(sqrt(x))
     is Error do a
     else WrongArg("a double or big real"));
setupfun("sqrt",sqrt);
map(a1:Sequence,a2:Sequence,f:Expr):Expr := (
     newlen := length(a1);
     if newlen != length(a2) then return WrongArg("lists of the same length");
     if newlen == 0 then return emptySequenceE;
     haderror := false;
     recursiondepth = recursiondepth + 1;
     if recursiondepth > recursionlimit then return RecursionLimit();
     errret := nullE;
     when f is fc:FunctionClosure do (
	  previousFrame := fc.frame;
	  model := fc.model;
	  desc := model.desc;
	  body := model.body;
	  frameID := desc.frameID;
	  numparms := desc.numparms;
	  framesize := desc.framesize;
	  -- since the function closure has no code inside it that makes
	  -- a closure, we can re-use its frame.
	  if desc.restargs then (	  -- x -> ...
	       saveLocalFrame := localFrame;
	       values := new Sequence len framesize do provide nullE;
	       localFrame = Frame(previousFrame,frameID,framesize,false,values);
	       ret := new Sequence len newlen at i do (
		    values.0 = Sequence(a1.i,a2.i);
		    tmp := eval(body);
		    when tmp is err:Error do (
			 if err.message == returnMessage
			 then provide err.value
			 else (
			      errret = tmp;
			      while true do provide nullE;
			      )
			 )
		    else provide tmp;
		    if localFrame.notrecyclable then (
			 values = new Sequence len framesize do provide nullE;
			 localFrame = Frame(previousFrame,frameID,framesize,false,values);
			 )
		    else for i from 1 to framesize - 1 do values.i = nullE
		    );
	       localFrame = saveLocalFrame;
	       recursiondepth = recursiondepth - 1;
	       if errret != nullE then errret else Expr(ret)
	       )
	  else (				  -- (x,y) -> ...
	       if numparms != 2 then WrongNumArgs(model.parms,numparms,2)
	       else (
		    saveLocalFrame := localFrame;
		    values := new Sequence len framesize do provide nullE;
		    localFrame = Frame(previousFrame,frameID,framesize,false,values);
		    ret := new Sequence len newlen at i do (
			 values.0 = a1.i;
			 values.1 = a2.i;
			 tmp := eval(body);
			 when tmp is err:Error do (
			      if err.message == returnMessage
			      then provide err.value
			      else (
				   errret = tmp;
				   while true do provide nullE;
				   )
			      )
			 else provide tmp;
			 if localFrame.notrecyclable then (
			      values = new Sequence len framesize do provide nullE;
			      localFrame = Frame(previousFrame,frameID,framesize,false,values);
			      )
			 else (
			      -- it would be faster to do a byte copy here:
			      for i from 2 to framesize - 1 do values.i = nullE;
			      );
			 );
		    localFrame = saveLocalFrame;
		    recursiondepth = recursiondepth - 1;
		    if errret != nullE then errret else Expr(ret)
		    )
	       )
	  )
     is cf:CompiledFunction do (	  -- compiled code
	  fn := cf.fn;
	  ret := new Sequence len newlen at i do (
	       tmp := fn(Expr(Sequence(a1.i,a2.i)));
	       when tmp is Error do (
		    errret = tmp;
		    while true do provide nullE; )
	       else provide tmp;
	       );
	  recursiondepth = recursiondepth - 1;
	  if errret != nullE then errret else Expr(ret)
	  )
     is cf:CompiledFunctionClosure do (	  -- compiled code closure
	  fn := cf.fn;
	  env := cf.env;
	  ret := new Sequence len newlen at i do (
	       tmp := fn(Expr(Sequence(a1.i,a2.i)),env);
	       when tmp is Error do (
		    errret = tmp;
		    while true do provide nullE; )
	       else provide tmp;
	       );
	  recursiondepth = recursiondepth - 1;
	  if errret != nullE then errret else Expr(ret)
	  )
     else WrongArg(2,"a function")
     );
map(a:Sequence,f:Expr):Expr := (
     newlen := length(a);
     if newlen == 0 then return emptySequenceE;
     haderror := false;
     recursiondepth = recursiondepth + 1;
     if recursiondepth > recursionlimit then return RecursionLimit();
     errret := nullE;
     when f is fc:FunctionClosure do (
	  previousFrame := fc.frame;
	  model := fc.model;
	  desc := model.desc;
	  body := model.body;
	  frameID := desc.frameID;
	  numparms := desc.numparms;
	  framesize := desc.framesize;
	  if desc.restargs then (	  -- x -> ...
	       saveLocalFrame := localFrame;
	       values := new Sequence len framesize do provide nullE;
	       localFrame = Frame(previousFrame,frameID,framesize,false,values);
	       ret := new Sequence len newlen do (
		    foreach arg in a do (
			 values.0 = arg;
			 tmp := eval(body);
			 when tmp is err:Error do (
			      if err.message == returnMessage
			      then provide err.value
			      else (
				   errret = tmp;
				   while true do provide nullE;
				   )
			      )
			 else provide tmp;
		    	 if localFrame.notrecyclable then (
			      values = new Sequence len framesize do provide nullE;
			      localFrame = Frame(previousFrame,frameID,framesize,false,values);
			      )
			 else for i from 1 to framesize - 1 do values.i = nullE;
			 )
		    );
	       localFrame = saveLocalFrame;
	       recursiondepth = recursiondepth - 1;
	       if errret != nullE then errret else Expr(ret)
	       )
	  else (				  -- (x,y) -> ...
	       if numparms == 1 then (
		    saveLocalFrame := localFrame;
		    values := new Sequence len framesize do provide nullE;
		    localFrame = Frame(previousFrame,frameID,framesize,false,values);
		    ret := new Sequence len newlen do (
			 if framesize == 1 then (
			      foreach arg in a do (
				   when arg is args:Sequence do (
					if 1 == length(args) then values.0 = args.0
					else (
					     errret = WrongNumArgs(model.parms,numparms,length(args));
					     while true do provide nullE;
					     )
					)
				   else values.0 = arg;
				   tmp := eval(body);
				   when tmp is err:Error do (
					if err.message == returnMessage
					then provide err.value
					else (
					     errret = tmp;
					     while true do provide nullE;
					     )
					)
				   else provide tmp;
		    		   if localFrame.notrecyclable then (
					values = new Sequence len framesize do provide nullE;
					localFrame = Frame(previousFrame,frameID,framesize,false,values);
					);
				   )
			      )
			 else (
			      foreach arg in a do (
				   when arg is args:Sequence do (
					if 1 == length(args) then values.0 = args.0
					else (
					     errret = WrongNumArgs(model.parms,numparms,length(args));
					     while true do provide nullE;
					     )
					)
				   else values.0 = arg;
				   tmp := eval(body);
				   when tmp is err:Error do (
					if err.message == returnMessage
					then provide err.value
					else (
					     errret = tmp;
					     while true do provide nullE;
					     )
					)
				   else provide tmp;
		    		   if localFrame.notrecyclable then (
					values = new Sequence len framesize do provide nullE;
					localFrame = Frame(previousFrame,frameID,framesize,false,values);
					)
				   else for i from 1 to framesize - 1 do values.i = nullE;
				   )
			      )
			 );
		    localFrame = saveLocalFrame;
		    recursiondepth = recursiondepth - 1;
		    if errret != nullE then errret else Expr(ret)
		    )
	       else (
		    if framesize == 0 then (
			 saveLocalFrame := localFrame;
			 localFrame = previousFrame;
			 ret := new Sequence len newlen do (
			      foreach arg in a do (
				   when arg is args:Sequence do (
					if 0 != length(args) then (
					     errret = WrongNumArgs(model.parms,0,length(args));
					     while true do provide nullE;
					     )
					)
				   else (
					errret = WrongNumArgs(model.parms,numparms,1);
					while true do provide nullE;
					);
				   tmp := eval(body);
				   when tmp is err:Error do (
					if err.message == returnMessage
					then provide err.value
					else (
					     errret = tmp;
					     while true do provide nullE;
					     )
					)
				   else provide tmp;
				   )
			      );
			 localFrame = saveLocalFrame;
			 recursiondepth = recursiondepth - 1;
			 if errret != nullE then errret else Expr(ret)
			 )
		    else (	  -- framesize != 0
			 saveLocalFrame := localFrame;
			 values := new Sequence len framesize do provide nullE;
			 localFrame = Frame(previousFrame,frameID,framesize,false,values);
			 ret := new Sequence len newlen do (
			      foreach arg in a do (
				   when arg is args:Sequence do (
					if numparms == length(args) then (
					     foreach x at i in args do values.i = x;
					     )
					else (
					     errret=WrongNumArgs(model.parms,numparms,length(args));
					     while true do provide nullE;
					     )
					)
				   else (
					errret = WrongNumArgs(model.parms,numparms,1);
					while true do provide nullE;
					);
				   tmp := eval(body);
				   when tmp is err:Error do (
					if err.message == returnMessage
					then provide err.value
					else (
					     errret = tmp;
					     while true do provide nullE;
					     )
					)
				   else provide tmp;
		    		   if localFrame.notrecyclable then (
					values = new Sequence len framesize do provide nullE;
					localFrame = Frame(previousFrame,frameID,framesize,false,values);
     	       	    	      	   	)
				   else for i from numparms to framesize - 1 do values.i = nullE;
				   ));
			 localFrame = saveLocalFrame;
			 recursiondepth = recursiondepth - 1;
			 if errret != nullE then errret else Expr(ret)
			 )
		    )
	       )
	  )
     is cf:CompiledFunction do (	  -- compiled code
	  fn := cf.fn;
	  ret := new Sequence len newlen do (
	       foreach arg in a do (
		    tmp := fn(arg);
		    when tmp is Error do (
			 errret = tmp;
			 while true do provide nullE; )
		    else provide tmp; ));
	  recursiondepth = recursiondepth - 1;
	  if errret != nullE then errret else Expr(ret)
	  )
     is cf:CompiledFunctionClosure do (	  -- compiled code closure
	  fn := cf.fn;
	  env := cf.env;
	  ret := new Sequence len newlen do (
	       foreach arg in a do (
		    tmp := fn(arg,env);
		    when tmp is Error do (
			 errret = tmp;
			 while true do provide nullE; )
		    else provide tmp; ));
	  recursiondepth = recursiondepth - 1;
	  if errret != nullE then errret else Expr(ret)
	  )
     else WrongArg(2,"a function")
     );
map(n:int,f:Expr):Expr := (
     if n <= 0 then return emptyList;
     haderror := false;
     errret := nullE;
     recursiondepth = recursiondepth + 1;
     saveLocalFrame := localFrame;
     b := new Sequence len n do (
	  if recursiondepth > recursionlimit then (
	       errret = RecursionLimit();
	       while true do provide nullE; )
	  else when f is fc:FunctionClosure do (
	       previousFrame := fc.frame;
	       model := fc.model;
	       desc := model.desc;
	       body := model.body;
	       frameID := desc.frameID;
	       numparms := desc.numparms;
	       framesize := desc.framesize;
	       if numparms != 1 then (
		    errret = WrongNumArgs(model.parms,numparms,1);
		    while true do provide nullE;
		    )
	       else (
		    values := new Sequence len framesize do provide nullE;
		    localFrame = Frame(previousFrame,frameID,framesize,false,values);
		    if framesize == 1 then (
			 for i from 0 to n-1 do (
			      values.0 = toInteger(i);
			      tmp := eval(body);
			      when tmp is err:Error do (
				   if err.message == returnMessage
				   then provide err.value
				   else (
					errret = tmp;
					while true do provide nullE;
					)
				   )
			      else provide tmp;
		    	      if localFrame.notrecyclable then (
				   values = new Sequence len framesize do provide nullE;
				   localFrame = Frame(previousFrame,frameID,framesize,false,values);
				   );				   
			      )
			 )
		    else (
			 for i from 0 to n-1 do (
			      values.0 = toInteger(i);
			      tmp := eval(body);
			      when tmp is err:Error do (
				   if err.message == returnMessage
				   then provide err.value
				   else (
					errret = tmp;
					while true do provide nullE;
					)
				   )
			      else provide tmp;
		    	      if localFrame.notrecyclable then (
				   values = new Sequence len framesize do provide nullE;
				   localFrame = Frame(previousFrame,frameID,framesize,false,values);
				   )
			      else for i from 1 to framesize - 1 do values.i = nullE;
			      )
			 )
		    )
	       )
	  is cf:CompiledFunction do (	  -- compiled code
	       fn := cf.fn;
	       for i from 0 to n-1 do (
		    tmp := fn(Expr(toInteger(i)));
		    when tmp is Error do (
			 errret = tmp;
			 while true do provide nullE; )
		    else provide tmp;))
	  is cf:CompiledFunctionClosure do (	  -- compiled code closure
	       fn := cf.fn;
	       env := cf.env;
	       for i from 0 to n-1 do (
		    tmp := fn(Expr(toInteger(i)),env);
		    when tmp is Error do (
			 errret = tmp;
			 while true do provide nullE; )
		    else provide tmp;))
	  else (
	       errret = WrongArg(2,"a function");
	       while true do provide nullE; ));
     localFrame = saveLocalFrame;
     recursiondepth = recursiondepth - 1;
     if errret != nullE then errret 
     else list(b));

map(e:Expr,f:Expr):Expr := (
     when e
     is a:Sequence do Expr(map(a,f))
--     is obj:HashTable do (
--	  if obj.mutable then return WrongArg("an immutable hash table");
--	  if ancestor(obj.class,Tally) then mapkeys(f,obj) else mapvalues(f,obj))
     is b:List do (
	  c := map(b.v,f);
	  when c is err:Error do if err.message == breakMessage then err.value else c
	  is v:Sequence do list(b.class,v,b.mutable)
	  else nullE			  -- will not happen
	  )
     is i:Integer do (
	  if !isInt(i)
	  then WrongArgSmallInteger()
	  else map(toInt(i),f))
     else WrongArg(1,"a list, sequence, or an integer"));
map(e1:Expr,e2:Expr,f:Expr):Expr := (
     when e1
     is a1:Sequence do (
	  when e2
	  is a2:Sequence do map(a1,a2,f)
	  is b2:List do (
	       c := map(a1,b2.v,f);
	       when c is err:Error do if err.message == breakMessage then err.value else c
	       is v:Sequence do list(b2.class,v,b2.mutable)
	       else nullE		  -- will not happen
	       )
	  else WrongArg(2,"a list or sequence"))
     is b1:List do (
	  when e2
	  is a2:Sequence do (
	       c := map(b1.v,a2,f);
	       when c is err:Error do if err.message == breakMessage then err.value else c
	       is v:Sequence do list(b1.class,v,b1.mutable)
	       else nullE		  -- will not happen
	       )
	  is b2:List do (
	       mutable := b1.mutable;
	       class := b1.class;
	       if class != b2.class then (
		    mutable = false;
		    class = listClass;
		    );
	       c := map(b1.v,b2.v,f);
	       when c is err:Error do if err.message == breakMessage then err.value else c
	       is v:Sequence do list(class,v,mutable)
	       else nullE		  -- will not happen
	       )
	  else WrongArg(2,"a list or sequence"))
     else WrongArg(1,"a list or sequence"));
map(e:Expr):Expr := (
     when e is a:Sequence do (
	  if length(a) == 2
	  then map(a.0,a.1)
	  else if length(a) == 3
	  then map(a.0,a.1,a.2)
	  else WrongNumArgs(2,3))
     else WrongNumArgs(2,3));
setupfun("apply",map);

scan(n:int,f:Expr):Expr := (
     if n <= 0 then return emptySequenceE;
     if recursiondepth > recursionlimit then (
     	  recursiondepth = recursiondepth - 1;
	  RecursionLimit())
     else when f is fc:FunctionClosure do (
     	  recursiondepth = recursiondepth + 1;
     	  saveLocalFrame := localFrame;
	  previousFrame := fc.frame;
	  model := fc.model;
	  desc := model.desc;
	  body := model.body;
	  frameID := desc.frameID;
	  numparms := desc.numparms;
	  framesize := desc.framesize;
	  if numparms != 1 then (
     	       recursiondepth = recursiondepth - 1;
	       return WrongNumArgs(model.parms,numparms,1);
	       );
	  if framesize == 1 then (
	       values := new Sequence len framesize do provide nullE;
	       localFrame = Frame(previousFrame,frameID,framesize,false,values);
	       for i from 0 to n-1 do (
		    values.0 = toInteger(i);
		    tmp := eval(body);
		    when tmp is err:Error do (
			 if err.message != returnMessage then (
			      recursiondepth = recursiondepth - 1;
			      localFrame = saveLocalFrame;
			      return returnFromLoop(tmp); 
			      )
			 )
		    else nothing;
		    if localFrame.notrecyclable then (
			 values = new Sequence len framesize do provide nullE;
			 localFrame = Frame(previousFrame,frameID,framesize,false,values);
			 );
		    );
	       localFrame = saveLocalFrame;
	       recursiondepth = recursiondepth - 1;
	       nullE)
	  else (
	       values := new Sequence len framesize do provide nullE;
	       localFrame = Frame(previousFrame,frameID,framesize,false,values);
	       for i from 0 to n-1 do (
		    values.0 = toInteger(i);
		    tmp := eval(body);
		    when tmp is err:Error do (
			 if err.message != returnMessage then (
			      recursiondepth = recursiondepth - 1;
			      localFrame = saveLocalFrame;
			      return returnFromLoop(tmp); 
			      )
			 )
		    else nothing;
		    if localFrame.notrecyclable then (
			 values = new Sequence len framesize do provide nullE;
			 localFrame = Frame(previousFrame,frameID,framesize,false,values);
			 )
		    else for i from 1 to framesize - 1 do values.i = nullE;
		    );
	       localFrame = saveLocalFrame;
	       recursiondepth = recursiondepth - 1;
	       nullE))
     is cf:CompiledFunction do (	  -- compiled code
     	  recursiondepth = recursiondepth + 1;
     	  saveLocalFrame := localFrame;
	  fn := cf.fn;
	  for i from 0 to n-1 do (
	       tmp := fn(Expr(toInteger(i)));
	       when tmp is Error do (
		    recursiondepth = recursiondepth - 1;
		    localFrame = saveLocalFrame;
		    return tmp; 
		    )
	       else nothing; 
	       );
	  localFrame = saveLocalFrame;
	  recursiondepth = recursiondepth - 1;
	  nullE)
     is cf:CompiledFunctionClosure do (	  -- compiled code closure
     	  recursiondepth = recursiondepth + 1;
     	  saveLocalFrame := localFrame;
	  fn := cf.fn;
	  env := cf.env;
	  for i from 0 to n-1 do (
	       tmp := fn(Expr(toInteger(i)),env);
	       when tmp is Error do (
		    recursiondepth = recursiondepth - 1;
		    localFrame = saveLocalFrame;
		    return tmp; 
		    )
	       else nothing; 
	       );
	  localFrame = saveLocalFrame;
	  recursiondepth = recursiondepth - 1;
	  nullE)
     else WrongArg(2,"a function"));

scan(a:Sequence,f:Expr):Expr := (
     oldlen := length(a);
     if oldlen == 0 then return nullE;
     recursiondepth = recursiondepth + 1;
     saveLocalFrame := localFrame;
     if recursiondepth > recursionlimit then (
	  recursiondepth = recursiondepth - 1;
	  return RecursionLimit();
	  );
     when f is fc:FunctionClosure do (
	  previousFrame := fc.frame;
	  model := fc.model;
	  desc := model.desc;
	  body := model.body;
	  frameID := desc.frameID;
	  numparms := desc.numparms;
	  framesize := desc.framesize;
	  if desc.restargs then (	  -- x -> ...
	       values := new Sequence len framesize do provide nullE;
	       localFrame = Frame(previousFrame,frameID,framesize,false,values);
	       foreach arg in a do (
		    values.0 = arg;
		    tmp := eval(body);
		    when tmp is err:Error do (
			 if err.message != returnMessage then (
			      recursiondepth = recursiondepth - 1;
			      localFrame = saveLocalFrame;
			      return returnFromLoop(tmp);
			      )
			 )
		    else nothing;
		    if localFrame.notrecyclable then (
			 values = new Sequence len framesize do provide nullE;
			 localFrame = Frame(previousFrame,frameID,framesize,false,values);
			 )
		    else for i from 1 to framesize - 1 do values.i = nullE;
		    )
	       )
	  else (				  -- (x,y) -> ...
	       if numparms == 1 then (
		    values := new Sequence len framesize do provide nullE;
		    localFrame = Frame(previousFrame,frameID,framesize,false,values);
		    if framesize == 1 then (
			 foreach arg in a do (
			      when arg is args:Sequence do (
				   if 1 == length(args) then values.0 = args.0
				   else (
					recursiondepth = recursiondepth - 1;
					localFrame = saveLocalFrame;
					return WrongNumArgs(model.parms,numparms,length(args));
					)
				   )
			      else values.0 = arg;
			      tmp := eval(body);
			      when tmp is err:Error do (
				   if err.message != returnMessage then (
					recursiondepth = recursiondepth - 1;
					localFrame = saveLocalFrame;
					return returnFromLoop(tmp);
					)
				   )
			      else nothing;
		    	      if localFrame.notrecyclable then (
				   values = new Sequence len framesize do provide nullE;
				   localFrame = Frame(previousFrame,frameID,framesize,false,values);
				   );
			      )
			 )
		    else (
			 foreach arg in a do (
			      when arg is args:Sequence do (
				   if 1 == length(args) then values.0 = args.0
				   else (
					recursiondepth = recursiondepth - 1;
					localFrame = saveLocalFrame;
					return WrongNumArgs(model.parms,numparms,length(args));
					)
				   )
			      else values.0 = arg;
			      tmp := eval(body);
			      when tmp is err:Error do (
				   if err.message != returnMessage then (
					recursiondepth = recursiondepth - 1;
					localFrame = saveLocalFrame;
					return returnFromLoop(tmp);
					)
				   )
			      else nothing;
		    	      if localFrame.notrecyclable then (
				   values = new Sequence len framesize do provide nullE;
				   localFrame = Frame(previousFrame,frameID,framesize,false,values);
				   )
			      else for i from 1 to framesize - 1 do values.i = nullE;
			      )
			 )
		    )
	       else (
		    if framesize == 0 then (
			 localFrame = previousFrame;
			 foreach arg in a do (
			      when arg is args:Sequence do (
				   if 0 != length(args) then (
					recursiondepth = recursiondepth - 1;
					localFrame = saveLocalFrame;
					return WrongNumArgs(model.parms,0,length(args));
					)
				   )
			      else (
				   recursiondepth = recursiondepth - 1;
				   localFrame = saveLocalFrame;
				   return WrongNumArgs(model.parms,numparms,1);
				   );
			      tmp := eval(body);
			      when tmp is err:Error do (
				   if err.message != returnMessage then (
					recursiondepth = recursiondepth - 1;
					localFrame = saveLocalFrame;
					return returnFromLoop(tmp);
					)
				   )
			      else nothing))
		    else (	  -- framesize != 0
			 values := new Sequence len framesize do provide nullE;
			 localFrame = Frame(previousFrame,frameID,framesize,false,values);
			 foreach arg in a do (
			      when arg is args:Sequence do (
				   if numparms == length(args) then (
					foreach x at i in args do values.i = x;
					)
				   else (
					recursiondepth = recursiondepth - 1;
					localFrame = saveLocalFrame;
					return WrongNumArgs(model.parms,numparms,length(args));
					)
				   )
			      else (
				   recursiondepth = recursiondepth - 1;
				   localFrame = saveLocalFrame;
				   return WrongNumArgs(model.parms,numparms,1);
				   );
			      tmp := eval(body);
			      when tmp is err:Error do (
				   if err.message != returnMessage then (
					recursiondepth = recursiondepth - 1;
					localFrame = saveLocalFrame;
					return returnFromLoop(tmp);
					)
				   )
			      else nothing;
		    	      if localFrame.notrecyclable then (
				   values = new Sequence len framesize do provide nullE;
				   localFrame = Frame(previousFrame,frameID,framesize,false,values);
				   )
			      else for i from numparms to framesize - 1 do values.i = nullE;
			      )))))
     is cf:CompiledFunction do (	  -- compiled code
	  fn := cf.fn;
	  foreach arg in a do (
	       tmp := fn(arg);
	       when tmp is Error do (
		    recursiondepth = recursiondepth - 1;
		    localFrame = saveLocalFrame;
		    return tmp;
		    )
	       else nothing; ))
     is cf:CompiledFunctionClosure do (	  -- compiled code
	  fn := cf.fn;
	  env := cf.env;
	  foreach arg in a do (
	       tmp := fn(arg,env);
	       when tmp is Error do (
		    recursiondepth = recursiondepth - 1;
		    localFrame = saveLocalFrame;
		    return tmp;
		    )
	       else nothing; ))
     else (
	  recursiondepth = recursiondepth - 1;
	  localFrame = saveLocalFrame;
	  return WrongArg(2,"a function");
	  );
     localFrame = saveLocalFrame;
     recursiondepth = recursiondepth - 1;
     nullE);

-- scan(a:Sequence,f:Expr):Expr := (
--      foreach x in a do (
-- 	  y := apply(f,x);
-- 	  when y is Error do return y else nothing;
-- 	  );
--      nullE);

scan(a1:Sequence,a2:Sequence,f:Expr):Expr := (
     newlen := length(a1);
     if newlen != length(a2) then return WrongArg("lists of the same length");
     if newlen == 0 then return nullE;
     haderror := false;
     recursiondepth = recursiondepth + 1;
     if recursiondepth > recursionlimit then return RecursionLimit();
     when f is fc:FunctionClosure do (
	  previousFrame := fc.frame;
	  model := fc.model;
	  desc := model.desc;
	  body := model.body;
	  frameID := desc.frameID;
	  numparms := desc.numparms;
	  framesize := desc.framesize;
	  if desc.restargs then (	  -- x -> ...
	       saveLocalFrame := localFrame;
	       values := new Sequence len framesize do provide nullE;
	       localFrame = Frame(previousFrame,frameID,framesize,false,values);
	       for i from 0 to newlen - 1 do (
		    values.0 = Sequence(a1.i,a2.i);
		    tmp := eval(body);
		    when tmp is err:Error do (
			 if err.message != returnMessage then (
			      -- stash
			      localFrame = saveLocalFrame;
			      recursiondepth = recursiondepth - 1;
			      return returnFromLoop(tmp);
			      )
			 )
		    else nothing;
		    if localFrame.notrecyclable then (
			 values = new Sequence len framesize do provide nullE;
			 localFrame = Frame(previousFrame,frameID,framesize,false,values);
			 )
		    else for i from 1 to framesize - 1 do values.i = nullE;
		    );
	       -- stash
	       localFrame = saveLocalFrame;
	       recursiondepth = recursiondepth - 1;
	       nullE)
	  else (				  -- (x,y) -> ...
	       if numparms != 2 then WrongNumArgs(model.parms,numparms,2)
	       else (
		    saveLocalFrame := localFrame;
		    values := new Sequence len framesize do provide nullE;
		    localFrame = Frame(previousFrame,frameID,framesize,false,values);
		    for i from 0 to newlen - 1 do (
			 values.0 = a1.i;
			 values.1 = a2.i;
			 tmp := eval(body);
			 when tmp is err:Error do (
			      if err.message != returnMessage then (
				   -- stash
				   localFrame = saveLocalFrame;
				   recursiondepth = recursiondepth - 1;
				   return returnFromLoop(tmp);
				   )
			      )
			 else nothing;
		    	 if localFrame.notrecyclable then (
			      values = new Sequence len framesize do provide nullE;
			      localFrame = Frame(previousFrame,frameID,framesize,false,values);
			      )
			 else (
			      -- it would be faster to do a byte copy here:
			      for i from 2 to framesize - 1 do values.i = nullE;
			      );
			 );
		    localFrame = saveLocalFrame;
		    recursiondepth = recursiondepth - 1;
		    nullE ) ) )
     is cf:CompiledFunction do (	  -- compiled code
	  fn := cf.fn;
	  for i from 0 to newlen - 1 do (
	       tmp := fn(Expr(Sequence(a1.i,a2.i)));
	       when tmp is Error do (
	  	    recursiondepth = recursiondepth - 1;
		    return tmp;
		    )
	       else nothing;
	       );
	  recursiondepth = recursiondepth - 1;
	  nullE
	  )
     is cf:CompiledFunctionClosure do (	  -- compiled code closure
	  fn := cf.fn;
	  env := cf.env;
	  for i from 0 to newlen - 1 do (
	       tmp := fn(Expr(Sequence(a1.i,a2.i)),env);
	       when tmp is Error do (
	  	    recursiondepth = recursiondepth - 1;
		    return tmp;
		    )
	       else nothing;
	       );
	  recursiondepth = recursiondepth - 1;
	  nullE)
     else WrongArg(2,"a function")
     );
scan(e1:Expr,e2:Expr,f:Expr):Expr := (
     when e1
     is a1:Sequence do (
	  when e2
	  is a2:Sequence do scan(a1,a2,f)
	  is b2:List do scan(a1,b2.v,f)
	  else WrongArg(2,"a list or sequence"))
     is b1:List do (
	  when e2
	  is a2:Sequence do scan(b1.v,a2,f)
	  is b2:List do scan(b1.v,b2.v,f)
	  else WrongArg(2,"a list or sequence"))
     else WrongArg(1,"a list or sequence"));
scan(e:Expr,f:Expr):Expr := (
     when e
     is a:Sequence do scan(a,f)
     is b:List do scan(b.v,f)
     is i:Integer do (
	  if !isInt(i)
	  then WrongArgSmallInteger(1)
	  else scan(toInt(i),f))
     else buildErrorPacket("scan expects a list"));
scan(e:Expr):Expr := (
     when e is a:Sequence do (
	  if length(a) == 2
	  then scan(a.0,a.1)
	  else if length(a) == 3
	  then scan(a.0,a.1,a.2)
	  else WrongNumArgs(2))
     else WrongNumArgs(2));
setupfun("scan",scan);
gcd(x:Expr,y:Expr):Expr := (
     when x
     is a:Integer do (
	  when y
	  is b:Integer do Expr(gcd(a,b))
	  else buildErrorPacket("expected an integer"))
     else buildErrorPacket("expected an integer"));
gcdfun(e:Expr):Expr := accumulate(plus0,plus1,gcd,e);
setupfun("gcd",gcdfun);

setup(EqualW);
setup(ColonEqualW);

toSequence(e:Expr):Expr := (
     when e
     is Sequence do e
     is b:List do (
	  if b.mutable
	  then Expr(new Sequence len length(b.v) do foreach i in b.v do provide i)
	  else Expr(b.v)
	  )
     else WrongArg("a list or sequence"));
setupfun("toSequence",toSequence);

sequencefun(e:Expr):Expr := (
     when e
     is a:Sequence do e
     else Expr(Sequence(e)));
setupfun("sequence",sequencefun);

iteratedApply(lhs:Code,rhs:Code):Expr := (
     -- f ## (x,y,z) becomes ((f x) y) z
     f := eval(lhs);
     when f is Error do f else (
	  arg := eval(rhs);
	  when arg
	  is Error do arg
	  is args:Sequence do (
	       foreach x in args do (
		    f = apply(f,x);
		    when f is Error do return f else nothing;
		    );
	       f)
	  else apply(f,arg)));
setup(SharpSharpS,iteratedApply);


-- Local Variables:
-- compile-command: "make -C $M2BUILDDIR/Macaulay2/d "
-- End:
