--		Copyright 1994 by Daniel R. Grayson
use C;
use system;
use stdio;
use stdiop;
use binding;
use strings;
use nets;
use tokens;
use err;
use stdio;
use gmp;
use basic;
use convertr;
use common;

export copy(e:Expr):Expr := (
     when e
     is a:List do if a.mutable then Expr(copy(a)) else e
     is o:HashTable do if o.mutable then Expr(copy(o)) else e
     else e);
setupfun("copy",copy);
reverse(e:Expr):Expr := (
     when e
     is a:Sequence do Expr(reverse(a))
     is a:List do Expr(reverse(a))
     else WrongArg("a list or sequence"));
setupfun("reverse",reverse);
export seq(e:Expr):Expr := Expr(Sequence(e));
-- setupfun("singleton",seq);
export splice(a:Sequence):Sequence := (
     -- warning - this function may return its argument without copying
     hadseq := false;
     newlen := length(a);
     if newlen == 0 then return a;
     if newlen == 1 then (
	  when a.0
	  is s:Sequence do return s
	  else return a; );
     foreach i in a do (
	  when i is ii:Sequence do (
	       hadseq = true; 
	       newlen = newlen + length(ii) - 1; )
     	  else nothing;
	  );
     if hadseq
     then new Sequence len newlen do
     foreach i in a do 
     when i is ii:Sequence 
     do foreach j in ii do provide j
     else provide i
     else a);
export splice(e:Expr):Expr := (
     when e
     is v:Sequence do Expr(splice(v))
     is a:List do list(
	  a.class,
	  if a.mutable then (
	       r := splice(a.v);
	       if r == a.v then copy(r) else r
	       )
	  else splice(a.v),
	  a.mutable)
     else e);
setupfun("splice",splice);
export accumulate(
     f0:function():Expr, f1:function(Expr):Expr,
     f2:function(Expr,Expr):Expr, e:Expr):Expr := (
     when e
     is a:Sequence do (
	  if length(a) == 0 then f0()
	  else if length(a) == 1 then f1(a.0)
	  else (
	       g := a.0;
	       for i from 1 to length(a)-1 do (
		    g = f2(g,a.i);
		    when g is Error do return g else nothing;
		    );
	       g))
     else f1(e));

export map(f:function(Expr):Expr,a:Sequence):Sequence := (
     new Sequence len length(a) do foreach x in a do provide f(x));
export join(v:Sequence,w:Sequence):Sequence := (
     new Sequence len length(v) + length(w) do (
	  foreach x in v do provide x;
	  foreach y in w do provide y));
export subarray(v:Sequence,start:int,leng:int):Sequence := (
     new Sequence len leng at i do provide v.(start+i));
export subarray(v:Sequence,leng:int):Sequence := subarray(v,0,leng);

export isInteger(e:Expr):bool := when e is Integer do true else false;
export isInt(e:Expr):bool := when e is i:Integer do isInt(i) else false;
export isIntArray(e:Sequence):bool := (
     foreach x in e do if !isInt(x) then return false;
     true);
export isIntArray(e:Expr):bool := (
     when e
     is a:Sequence do isIntArray(a)
     is b:List do isIntArray(b.v)
     else false);     
export toInt(e:Expr):int := (
     when e 
     is i:Integer do toInt(i)
     else (fatal("internal error"); 0));
export toIntArray(e:Sequence):array(int) := (
     new array(int) len length(e) do foreach x in e do provide toInt(x));
export toIntArray(e:Expr):array(int) := (
     when e
     is a:Sequence do toIntArray(a)
     is b:List do toIntArray(b.v)
     else (
	  fatal("internal error: toIntArray expected an array of ints");
	  array(int)()
	  )
     );
export toArrayExpr(v:array(int)):Sequence := (
     new Sequence len length(v) do foreach i in v do provide Expr(toInteger(i))
     );

export newlist(class:HashTable,v:Sequence):List := (
     x := List(class,v,0,false);
     x.hash = hash(x);
     x);
export basictype(o:HashTable):HashTable := (
     while true do (
	  if o.parent == thingClass then return o;
	  o = o.parent;
	  ));


-- Local Variables:
-- compile-command: "make -C $M2BUILDDIR/Macaulay2/d"
-- End:
