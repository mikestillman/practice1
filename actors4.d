-- Copyright 1994 by Daniel R. Grayson

use C;
use system; 
use convertr;
use evaluate;
use common;
use binding;
use parser;
use lex;
use gmp;
use engine;
use nets;
use varnets;
use tokens;
use util;
use err;
use stdiop;
use ctype;
use stdio;
use vararray;
use varstrin;
use getline;
use strings;
use C;
use actors;
use actors2;
use basic;
use struct;
use objects;

internalName(s:string):string := (
     -- was "$" + s in 0.9.2
     s
     );

sleepfun(e:Expr):Expr := (
     when e
     is i:ZZ do (
	  if isInt(i)
	  then Expr(toInteger(sleep(toInt(i))))
	  else WrongArgSmallInteger(1))
     else WrongArgZZ(1));
setupfun("sleep",sleepfun);

forkfun(e:Expr):Expr := (
     when e
     is a:Sequence do (
	  if length(a) == 0
	  then Expr(toInteger(fork()))
	  else WrongNumArgs(0))
     else WrongNumArgs(0));
setupfun("fork",forkfun);

getpidfun(e:Expr):Expr := (
     when e
     is o:file do Expr(toInteger(o.pid))
     is a:Sequence do (
	  if length(a) == 0
	  then Expr(toInteger(getpid()))
	  else WrongNumArgs(0))
     else WrongNumArgs(0));
setupfun("processID",getpidfun);

getpgrpfun(e:Expr):Expr := (
     when e
     is a:Sequence do (
	  if length(a) == 0
	  then Expr(toInteger(getpgrp()))
	  else WrongNumArgs(0))
     else WrongNumArgs(0));
setupfun("groupID",getpgrpfun);

setpgidfun(e:Expr):Expr := (
     when e
     is a:Sequence do (
	  if length(a) == 2
	  then (
	       when a.0 is pid:ZZ do
	       if !isInt(pid) then WrongArgSmallInteger(1) else
	       when a.1 is pgid:ZZ do
	       if !isInt(pgid) then WrongArgSmallInteger(2) else (
		    r := setpgid(toInt(pid),toInt(pgid));
		    if r == ERROR
		    then buildErrorPacket("setGroupID: "+syserrmsg())
		    else nullE)
	       else WrongArgZZ(2)
	       else WrongArgZZ(1)
	       )
	  else WrongNumArgs(2))
     else WrongNumArgs(0));
setupfun("setGroupID",setpgidfun);

absfun(e:Expr):Expr := (
     when e
     is i:ZZ do Expr(abs(i))				    -- # typical value: abs, ZZ, ZZ
     is x:RR do Expr(if sign(x) then -x else x)		    -- # typical value: abs, RR, RR
     is x:CC do Expr(abs(x))				    -- # typical value: abs, CC, RR
     is r:QQ do Expr(abs(r))				    -- # typical value: abs, QQ, RR
     else WrongArg("a number, real or complex"));
setupfun("abs",absfun);

select(a:Sequence,f:Expr):Expr := (
     b := new array(bool) len length(a) do provide false;
     found := 0;
     foreach x at i in a do (
	  y := applyEE(f,x);
	  when y is err:Error do if err.message == breakMessage then return if err.value == dummyExpr then nullE else err.value else return y else nothing;
	  if y == True then (
	       b.i = true;
	       found = found + 1;
	       )
	  else if y != False then return buildErrorPacket("select: expected predicate to yield true or false");
	  );
     new Sequence len found do (
	  foreach p at i in b do if p then provide a.i));
foo := array(string)();
select(pat:string,rep:string,subj:string,ignorecase:bool):Expr := (
     r := regexselect(pat,rep,subj,foo,ignorecase);
     if r == foo then return buildErrorPacket("select: "+regexmatchErrorMessage);
     Expr(list(new Sequence len length(r) do foreach s in r do provide Expr(s))));
select(e:Expr,f:Expr,ignorecase:bool):Expr := (
     when e
     is pat:string do (
     	  when f is subj:string
	  do select(pat,"\\0",subj,ignorecase) 
     	  else WrongArgString(2)
	  )
     is obj:HashTable do (
	  if obj.mutable then return WrongArg(0+1,"an immutable hash table");
	  u := newHashTable(obj.class,obj.parent);
	  foreach bucket in obj.table do (
	       p := bucket;
	       while p != p.next do (
		    newvalue := applyEE(f,p.value);
		    when newvalue
		    is err:Error do if err.message == breakMessage then return if err.value == dummyExpr then nullE else err.value else return newvalue
		    else if newvalue == True 
		    then (storeInHashTable(u,p.key,p.hash,p.value);)
	  	    else if newvalue != False then return buildErrorPacket("select: expected predicate to yield true or false");
		    p = p.next;
		    ));
	  sethash(u,obj.mutable);
	  Expr(u))
     is a:Sequence do Expr(select(a,f))
     is b:List do (
	  c := select(b.v,f);
	  when c
	  is Error do c
	  is v:Sequence do list(b.class,v)
	  else e			  -- shouldn't happen
	  )
     else WrongArg(0+1,"a list or a string"));
select(n:int,a:Sequence,f:Expr):Expr := (
     b := new array(bool) len length(a) do provide false;
     found := 0;
     foreach x at i in a do (
	  if found < n then (
	       y := applyEE(f,x);
	       when y is err:Error do if err.message == breakMessage then return if err.value == dummyExpr then nullE else err.value else return y else nothing;
	       if y == True then (
		    b.i = true;
		    found = found + 1;
		    )
	       else if y != False then return buildErrorPacket("select: expected predicate to yield true or false");
	       )
	  else b.i = false);
     new Sequence len found do (
	  foreach p at i in b do if p then provide a.i));
select(n:Expr,e:Expr,f:Expr,ignorecase:bool):Expr := (
     when n is pat:string do
     when e is rep:string do
     when f is subj:string do select(pat,rep,subj,ignorecase)
     else WrongArgString(3)
     else WrongArgString(2)
     is nn:ZZ do
     if isInt(n) then
     when e
     is obj:HashTable do (
	  if obj.mutable then return WrongArg(1+1,"an immutable hash table");
	  u := newHashTable(obj.class,obj.parent);
	  nval := toInt(n);
	  if nval > 0 then
	  foreach bucket in obj.table do (
	       p := bucket;
	       while nval > 0 && p != p.next do (
		    newvalue := applyEE(f,p.value);
		    when newvalue 
		    is err:Error do if err.message == breakMessage then return if err.value == dummyExpr then nullE else err.value else return newvalue
		    else if newvalue == True 
		    then (
			 storeInHashTable(u,p.key,p.hash,p.value);
			 nval = nval-1;
			 )
	  	    else if newvalue != False then return buildErrorPacket("select: expected predicate to yield true or false");
		    p = p.next;
		    ));
	  sethash(u,obj.mutable);
	  Expr(u))
     is a:Sequence do Expr(select(toInt(n),a,f))
     is b:List do (
	  c := select(toInt(n),b.v,f);
	  when c
	  is Error do c
	  is v:Sequence do list(b.class,v)
	  else e			  -- shouldn't happen
	  )
     else WrongArg(1+1,"a list")
     else WrongArg(0+1,"an integer or string")
     else WrongArgZZ(0+1));
select(e:Expr):Expr := (
     ignorecase := false;
     when e is a:Sequence do (
	  if length(a) == 2
	  then select(a.0,a.1,ignorecase)
	  else if length(a) == 3
	  then select(a.0,a.1,a.2,ignorecase)
	  else WrongNumArgs(2,3))  -- could change this later
     else WrongNumArgs(2,3));
setupfun("select",select);

any(f:Expr,n:int):Expr := (
     for i from 0 to n-1 do (
	  v := applyEE(f,Expr(toInteger(i)));
	  when v is err:Error do if err.message == breakMessage then return if err.value == dummyExpr then nullE else err.value else return v else nothing;
	  if v == True then return True;
	  if v != False then return buildErrorPacket("any: expected true or false");
	  );
     False);
any(f:Expr,obj:HashTable):Expr := (
     foreach bucket in obj.table do (
	  p := bucket;
	  while true do (
	       if p == p.next then break;
	       v := applyEEE(f,p.key,p.value);
	       when v is err:Error do if err.message == breakMessage then return if err.value == dummyExpr then nullE else err.value else return v else nothing;
	       if v == True then return True;
	       if v != False then return buildErrorPacket("any: expected true or false");
	       p = p.next;
	       ));
     False);
any(f:Expr,a:Sequence):Expr := (
     foreach x at i in a do (
	  y := applyEE(f,x);
	  when y is err:Error do if err.message == breakMessage then return if err.value == dummyExpr then nullE else err.value else return y else nothing;
	  if y == True then return True;
	  if y != False then return buildErrorPacket("any: expected true or false");
	  );
     False);
any(f:Expr,e:Expr):Expr := (
     when e
     is a:Sequence do Expr(any(f,a))
     is b:List do Expr(any(f,b.v))
     is i:ZZ do if isInt(i) then Expr(any(f,toInt(i))) else WrongArgSmallInteger(1)
     is c:HashTable do 
     if c.mutable then WrongArg(1,"an immutable hash table") else
     Expr(any(f,c))
     else WrongArg("a list or a hash table"));
any(f:Expr,a:Sequence,b:Sequence):Expr := (
     if length(a) != length(b) then return buildErrorPacket("expected lists of the same length");
     foreach x at i in a do (
	  y := applyEEE(f,x,b.i);
	  when y is err:Error do if err.message == breakMessage then return if err.value == dummyExpr then nullE else err.value else return y else nothing;
	  if y == True then return True;
	  if y != False then return buildErrorPacket("any: expected true or false");
	  );
     False);
any(f:Expr,a:Sequence,y:Expr):Expr := (
     when y
     is c:Sequence do Expr(any(f,a,c))
     is d:List do Expr(any(f,a,d.v))
     else WrongArg("a basic list or sequence"));
any(f:Expr,x:Expr,y:Expr):Expr := (
     when x
     is a:Sequence do Expr(any(f,a,y))
     is b:List do Expr(any(f,b.v,y))
     else WrongArg("a basic list or sequence"));
any(e:Expr):Expr := (
     when e is a:Sequence do (
	  if length(a) == 2 then any(a.1,a.0)
	  else if length(a) == 3 then any(a.2,a.0,a.1)
	  else WrongNumArgs(2,3))
     else WrongNumArgs(2));
setupfun("any",any);

--find(f:Expr,obj:HashTable):Expr := (
--     foreach bucket in obj.table do (
--	  p := bucket;
--	  while true do (
--	       if p == bucketEnd then break;
--	       r := apply(f,p.key,p.value);
--	       if r != nullE then return r;
--	       p = p.next;
--	       ));
--     nullE);
--find(f:Expr,a:Sequence):Expr := (
--     i := 0;
--     while i < length(a) do (
--	  r := apply(f,a.i);
--	  if r != nullE then return r;
--	  i = i+1;
--	  );
--     nullE);
--find(e:Expr):Expr := (
--     when e is a:Sequence do (
--	  if length(a) != 2
--	  then WrongNumArgs(2)
--	  else (
--	       f := a.1;
--	       x := a.0;
--	       when x
--	       is a:Sequence do Expr(find(f,a))
--	       is b:List do Expr(find(f,b.v))
--	       is c:HashTable do
--	       if c.mutable then WrongArg(1,"an immutable hash table") else
--	       Expr(find(f,c))
--	       else WrongArg(1+1,"a list")))
--     else WrongNumArgs(2));
--setupfun("find",find);

characters(e:Expr):Expr := (
     when e
     is s:string do list(
	  new Sequence len length(s) do (
	       foreach c in s do provide chars.(int(uchar(c)))))
     else buildErrorPacket("expects a string"));
setupfun("characters",characters);

ascii(e:Expr):Expr := (
     if isIntArray(e)
     then Expr((
	  v := toIntArray(e);
	  new string len length(v) do foreach i in v do provide char(i)))
     else 
     when e is s:string do list(
	  new Sequence len length(s) do (
	       foreach c in s do provide toInteger(int(uchar(c)))))
     is i:ZZ do (
	  if isInt(i)
	  then Expr(new string len 1 do provide char(toInt(i)))
	  else WrongArgSmallInteger(1))
     else buildErrorPacket("expects a string, a small integer, or an array of small integers"));
setupfun("ascii",ascii);

utf8(v:array(int)):Expr := (
     w := newvarstring(length(v)+10);
     foreach i in v do (
	  if (i &     ~0x7f) == 0 then w << char(i) else
	  if (i &    ~0x7ff) == 0 then w << char(0xc0 | (i >> 6)) << char(0x80 | (i & 0x3f)) else
	  if (i &   ~0xffff) == 0 then w << char(0xe0 | (i >> 12)) << char(0x80 | ((i >> 6) & 0x3f)) << char(0x80 | (i & 0x3f)) else
	  if (i & ~0x1fffff) == 0 then w << char(0xf0 | (i >> 18)) << char(0x80 | ((i >> 12) & 0x3f)) << char(0x80 | ((i >> 6) & 0x3f)) << char(0x80 | (i & 0x3f))
	  else ( return buildErrorPacket("encountered integer too large for utf-8 encoding"); w));
     Expr(takestring(w)));

erru():Expr := buildErrorPacket("string ended unexpectedly during utf-8 decoding");
errb():Expr := buildErrorPacket("unexpected byte encountered in utf-8 decoding");
utf8(y:Expr):Expr := (
     if isIntArray(y) then utf8(toIntArray(y))
     else when y is s:string do (
	  n := length(s);
	  x := newvararrayint(n+10);
	  i := 0;
	  while i < n do (
	       c := int(uchar(s.i));
	       if (c & 0x80) == 0 then (
		    x << c;
		    i = i+1;
		    )
	       else if (c & 0xe0) == 0xc0 then (
		    if i+1 < n then (
		    	 d := int(uchar(s.(i+1)));
			 if (d & 0xc0) != 0x80 then return errb();
		    	 x << (((c & ~0xe0) << 6) | (d & ~0xc0));
			 i = i+2;
	       		 )
		    else return erru();
		    )
	       else if (c & 0xf0) == 0xe0 then (
		    if i+2 < n then (
		    	 d := int(uchar(s.(i+1)));
			 if (d & 0xc0) != 0x80 then return errb();
		    	 e := int(uchar(s.(i+2)));
			 if (e & 0xc0) != 0x80 then return errb();
		    	 x << (((c & ~0xf0) << 12) | ((d & ~0xc0) << 6) | (e & ~0xc0));
			 i = i+3;
			 )
		    else return erru();
		    )
	       else if (c & 0xf8) == 0xf0 then (
		    if i+3 < n then (
		    	 d := int(uchar(s.(i+1)));
			 if (d & 0xc0) != 0x80 then return errb();
		    	 e := int(uchar(s.(i+2)));
			 if (e & 0xc0) != 0x80 then return errb();
		    	 f := int(uchar(s.(i+3)));
			 if (f & 0xc0) != 0x80 then return errb();
		    	 x << (((c & ~0xf8) << 18) | ((d & ~0xc0) << 12) | ((e & ~0xc0) << 6) | (f & ~0xc0));
			 i = i+4;
			 )
		    else return erru();
		    )
	       else return errb();
	       );
	  a := takearrayint(x);
     	  Expr(list(new Sequence len length(a) do foreach i in a do provide Expr(toInteger(i)))))
     is i:ZZ do (
	  if !isInt(i) then return WrongArgSmallInteger();
	  Expr(utf8(array(int)(toInt(i)))))
     else buildErrorPacket("expects a string, a small integer, or an array of small integers"));
setupfun("utf8",utf8);

export checknoargs(e:Expr):Expr := (
     when e
     is v:Sequence do (
	  if length(v) == 0 then e else WrongNumArgs(0)
	  )
     else WrongNumArgs(0)
     );

randomint(e:Expr):Expr := (
     when checknoargs(e) is f:Error do return Expr(f) else nothing;
     Expr(toInteger(randomint()))
     );
setupfun("randomint",randomint);

outseq(x:file,y:Sequence):Expr := (
     foreach z in y do (
	  when z
	  is w:List do (outseq(x,w.v);)
	  is Nothing do nothing
	  is y:SymbolClosure do (
	       x << if y.frame == globalFrame
	       then y.symbol.word.name
	       else internalName(y.symbol.word.name);
	       )
	  is y:ZZ do (
	       if isInt(y) 
	       then for toInt(y) do x << ' '
	       else return WrongArg(2,"a string, or list or sequence of strings and small integers, or null"))
	  is w:Sequence do (outseq(x,w);)
	  is s:string do (x << s;)
	  else return WrongArg(2,"a string, or list or sequence of strings and integers, or null"));
     Expr(x));
outstringfun(e:Expr):Expr := (
     when e
     is a:Sequence do (
	  if length(a) == 2 then (
	       when a.0 
	       is x:file do (
		    when a.1 
		    is y:string do Expr(x << y)
		    is Nothing do Expr(x)
		    is y:SymbolClosure do (
			 x << (
			      if y.frame == globalFrame
	  		      then y.symbol.word.name
	  		      else internalName(y.symbol.word.name)
			      );
			 Expr(x)
			 )
		    is y:List do outseq(x,y.v)
		    is n:Net do Expr(x << n)
		    is y:ZZ do (
			 if isInt(y) 
			 then (
			      for toInt(y) do x << ' ';
			      Expr(x))
			 else WrongArg(2,"a sequence or list of strings, nets, or nulls"))
		    is y:Sequence do outseq(x,y)
		    else WrongArg(2,"a sequence or list of strings, nets, or nulls"))
	       else WrongArg(1,"a file"))
	  else WrongNumArgs(2))
     else WrongNumArgs(2));
setupfun("printString",outstringfun);			 

lenseq(y:Sequence):int := (
     l := 0;
     foreach z in y do (
	  when z
	  is w:List do (
	       m := lenseq(w.v);
	       if m == -1 then return -1;
	       l = l + m;
	       )
	  is Nothing do nothing
	  is y:SymbolClosure do l = l + length(y.symbol.word.name)
	  is y:ZZ do (
	       if isInt(y) 
	       then l = l + toInt(y)
	       else return -1)
	  is w:Sequence do (
	       m := lenseq(w);
	       if m == -1 then return -1;
	       l = l + m;
	       )
	  is s:string do (
	       l = l + length(s);
	       )
	  else return -1);
     l);
stringlenfun(e:Expr):Expr := (
     when e
     is y:string do Expr(toInteger(length(y)))
     is Nothing do Expr(toInteger(0))
     is y:SymbolClosure do Expr(toInteger(length(y.symbol.word.name)))
     is y:List do (
	  l := lenseq(y.v);
	  if l == -1 
	  then WrongArg(1,"a string, or list or sequence of strings and integers, or null")
	  else Expr(toInteger(l))
	  )
     is y:ZZ do e
     is y:Sequence do (
	  l := lenseq(y);
	  if l == -1 
	  then WrongArg(1,"a string, or list or sequence of strings and integers, or null")
	  else Expr(toInteger(l))
	  )
     else WrongArg(1,"a string, or list or sequence of strings and integers, or null"));
setupfun("stringlen",stringlenfun);

stringcat2(a:Sequence,s:string,i:int):int := ( -- returns next available index
     foreach x in a do (
	  when x
	  is a:Sequence do i = stringcat2(a,s,i)
     	  is y:SymbolClosure do foreach c in y.symbol.word.name do (s.i = c; i = i+1;)
	  is a:List do i = stringcat2(a.v,s,i)
	  is n:ZZ do for toInt(n) do (s.i = ' '; i = i+1;)
	  is t:string do foreach c in t do (s.i = c; i = i+1;)
	  else nothing;
	  );
     i);
export stringcatseq(a:Sequence):Expr := (
     l := lenseq(a);
     if l == -1 
     then WrongArg("strings, integers, or symbols")
     else (
	  s := new string len l do provide ' ';
	  stringcat2(a,s,0);
	  Expr(s)));
stringcatfun(e:Expr):Expr := (
     when e
     is a:Sequence do stringcatseq(a)
     is a:List do stringcatseq(a.v)
     is Nothing do Expr("")
     is y:SymbolClosure do Expr(y.symbol.word.name)
     is n:ZZ do (
	  if isInt(n) then (
	       m := toInt(n);
	       if m >= 0 
	       then Expr(new string len m do provide ' ')
	       else buildErrorPacket("encountered negative integer")
	       )
	  else buildErrorPacket("encountered a large integer")
	  )
     is string do e
     else WrongArg("a sequence or list of strings, integers, or symbols"));
setupfun("concatenate",stringcatfun);

errorfun(e:Expr):Expr := (
     e = stringcatfun(e);
     when e
     is s:string do buildErrorPacket(s)
     else buildErrorPacket("expects a string or sequence of strings as its argument"));
setupfun("error",errorfun).protected = false;		    -- this will be replaced by a toplevel function that calls this one

mingleseq(a:Sequence):Expr := (
     n := length(a);
     b := new array(Sequence) len n do provide emptySequence;
     newlen := 0;
     for i from 0 to n-1 do (
	  when a.i
	  is d:Sequence do ( newlen = newlen + length(d); b.i = d; )
	  is d:List do ( newlen = newlen + length(d.v); b.i = d.v; )
	  else return WrongArg(i+1,"a list or sequence"));
     list ( new Sequence len newlen do
     	  for j from 0 to newlen-1 do for i from 0 to n-1 do
     	  if j < length(b.i) then provide b.i.j));
minglefun(e:Expr):Expr := (
     when e
     is a:Sequence do mingleseq(a)
     is a:List do mingleseq(a.v)
     is Error do e
     else WrongArg("a list or sequence"));
setupfun("mingle", minglefun);

packlist(v:Sequence,n:int):Expr := (
     d := length(v);
     i := 0;
     list(new Sequence len (d + n - 1) / n do 
     	  provide list(
	       new Sequence len if n < d-i then n else d-i do (
		    j := i;
		    i = i+1;
	       	    provide v.j))));
packfun(e:Expr):Expr := (
     when e
     is a:Sequence do (
     	  if length(a) == 2 then (
	       when a.0
	       is n:ZZ do (
		    if isInt(n)
		    then (
			 nn := toInt(n);
			 if nn > 0 then (
			      when a.1
			      is x:Sequence do packlist(x,nn)
			      is x:List do packlist(x.v,nn)
			      else WrongArg(1,"a list or sequence")
			      )
			 else if nn == 0 then (
			      when a.1
			      is x:Sequence do if length(x) == 0 then emptyList else WrongArg(1,"a positive integer")
			      is x:List do if length(x.v) == 0 then emptyList else WrongArg(1,"a positive integer")
			      else WrongArg(1,"a list or sequence")
			      )
			 else WrongArg(1,"a positive integer")
			 )
		    else WrongArgSmallInteger(1)
		    )
	       is x:Sequence do (
		    when a.1
		    is n:ZZ do (
			 if isInt(n) 
			 then (
			      nn := toInt(n);
			      if nn > 0 
			      then packlist(x,nn)
			      else WrongArg(2,"a positive integer"))
			 else WrongArgSmallInteger(2))
		    else WrongArgZZ(2))
	       is x:List do (
		    when a.1
		    is n:ZZ do (
			 if isInt(n) 
			 then (
			      nn := toInt(n);
			      if nn > 0 
			      then packlist(x.v,nn)
			      else WrongArg(2,"a positive integer"))
			 else WrongArgSmallInteger(2))
		    else WrongArgZZ(2))
	       else WrongArg(1,"a list or sequence"))
	  else WrongNumArgs(2))
     else WrongNumArgs(2));	  
setupfun("pack", packfun);

getenvfun(e:Expr):Expr := (
     when e
     is s:string do Expr(getenv(s))
     else WrongArgString());
setupfun("getenv",getenvfun);

getfun(e:Expr):Expr := (
     when e
     is f:file do (
	  if f.infd == -1
	  then WrongArg("an open input file")
	  else (
	       when readfile(f.infd)
	       is s:string do (
		    stat := closeIn(f); -- the user may close the output side of a pipe first, so we can find out now if the process exited normally
		    when stat is m:errmsg do buildErrorPacket(m.message)
		    else Expr(s))
	       else buildErrorPacket("unable to read file: "+syserrmsg())))
     is filename:string do (
	  when get(filename)
	  is e:errmsg do buildErrorPacket(e.message)
	  is s:string do Expr(s))
     else WrongArg("a string as filename"));
setupfun("get",getfun);

readprompt := "";
readpromptfun():string := readprompt;

import isReady(fd:int):int;
isReadyFun(e:Expr):Expr := (
     when e
     is f:file do toExpr ( 
	  f.input && !f.eof && ( f.insize > f.inindex || isReady(f.infd) > 0 ) 
	  ||
	  f.listener && (
	       f.connection != -1
	       ||
	       ( sd := acceptNonblocking(f.listenerfd); f.connection = sd; sd != -1 )
	       )
	  )
     else WrongArg("a file"));
setupfun("isReady",isReadyFun);

atEOFfun(e:Expr):Expr := (
     when e
     is f:file do toExpr ( !f.input || f.eof || f.insize == f.inindex && isReady(f.infd) > 0 
	  && 0 == filbuf(f) )
     else WrongArg("a file"));
setupfun("atEndOfFile",atEOFfun);

allInputFilesOrListeners(s:Sequence):bool := (
     foreach f in s do (
     	  when f is g:file do ( if !g.input && !g.listener then return false; )
     	  else return false;
	  );
     true);
allIntegers(s:Sequence):bool := (
     foreach f in s do (
     	  when f is g:ZZ do ( if !isInt(g) || toInt(g) < 0 then return false; )
     	  else return false;
	  );
     true);
numberReadyOnes(s:Sequence):int := (
     n := 0;
     foreach f in s do (
     	  when f is g:file do ( if g.input && (g.insize > g.inindex) then n = n+1; )
     	  else nothing;
	  );
     n);
readyOnes(s:Sequence):array(int) := (
     new array(int) len numberReadyOnes(s) do
     foreach f at i in s do
     when f is g:file do ( if g.input && (g.insize > g.inindex) then provide i; )
     else nothing
     );
fdlist(s:Sequence):array(int) := (
     new array(int) len length(s) do
     foreach f in s do 
     when f is g:file 
     do provide if g.input then g.infd else if g.listener then g.listenerfd else if g.output then g.outfd else -1
     else nothing
     );
wait(s:Sequence):Expr := (
     if allIntegers(s) then (
	  stats := waitNoHang(new array(int) len length(s) do foreach pid in s do provide toInt(pid));
	  Expr(list(new Sequence len length(s) do foreach status in stats do provide Expr(toInteger(status)))))
     else if !allInputFilesOrListeners(s) then WrongArg("a list of input files or listeners, or a list of small non-negative integers")
     else if numberReadyOnes(s) > 0 then list(toArrayExpr(readyOnes(s)))
     else list(toArrayExpr(select(fdlist(s))))
     );
wait(f:file):Expr := (
     if !f.input then WrongArg("an input file or list of input files")
     else if f.insize > f.inindex then nullE
     else ( filbuf(f); nullE ));
wait(e:Expr):Expr := (
     when e
     is f:List do wait(f.v)
     is v:Sequence do wait(v)
     is f:file do wait(f)
     is x:ZZ do (
	  if isInt(x) then (
	       ret := wait(toInt(x));
	       if ret == ERROR 
	       then buildErrorPacket("wait failed")
	       else Expr(toInteger(ret))
	       )
	  else WrongArgSmallInteger()
	  )
     else WrongArg("an integer, or an input file or list of input files")
     );
setupfun("wait",wait);

readE(f:file):Expr := (
     when read(f)
     is e:errmsg do buildErrorPacket(e.message)
     is s:string do Expr(s));

readfun(e:Expr):Expr := (
     when e
     is f:file do readE(f)
     is p:string do (
	  readprompt = p;
	  oldprompt := stdin.prompt;
	  stdin.prompt = readpromptfun;
	  r := getLine(stdin);
	  when r is e:errmsg do buildErrorPacket(e.message)
	  is s:string do (
	       stdin.prompt = oldprompt;
	       Expr(s)))
     is s:Sequence do (
	  if length(s) == 0
	  then readE(stdin)
	  else if length(s) == 2
	  then (
	       when s.0
	       is f:file do (
		    if ! f.input
		    then return WrongArg(1,"expected an input file");
		    when s.1
		    is n:ZZ do (
			 if isInt(n)
			 then (
			      nn := toInt(n);
			      if nn < 0
			      then return WrongArg(2,"a positive integer");
			      if f.inindex < f.insize
			      then (
				   nn = min(nn,f.insize - f.inindex);
				   o := f.inindex;
				   f.inindex = f.inindex + nn;
				   Expr(new string len nn do for i from 0 to nn-1 do provide f.inbuffer.(o+i))
				   )
			      else (
				   buf := new string len nn do provide ' ';
				   r := read(f.infd,buf,nn);
				   Expr(new string len r do for i from 0 to r-1 do provide buf.i)
				   )
			      )
			 else WrongArgSmallInteger(2)
			 )
		    else WrongArgZZ(2)
		    )
	       else WrongArg(1,"a file")
	       )
	  else WrongNumArgs(0,1))
     else WrongArg(1,"a file or a string"));
setupfun("read",readfun);

export setupargv():void := (
     setupconst("commandLine",toExpr(argv));
     setupconst("environment",toExpr(envp));
     );

substrfun(e:Expr):Expr := (
     when e is args:Sequence do
     if length(args) == 3 then
     when args.0
     is i:ZZ do if !isInt(i) then WrongArgSmallInteger(1) else (
	  when args.1 is j:ZZ do if !isInt(j) then WrongArgSmallInteger(2) else
	  when args.2 is s:string do Expr(substr(s,toInt(i),toInt(j)))
	  else WrongArgString(3)
	  else WrongArgZZ(2))
     is s:string do (
	  when args.1 is i:ZZ do if !isInt(i) then WrongArgSmallInteger(2) else
	  when args.2 is j:ZZ do if !isInt(j) then WrongArgSmallInteger(3) else Expr(substr(s,toInt(i),toInt(j)))
	  else WrongArgZZ(3)
	  else WrongArgZZ(2))
     else WrongArg(1,"a string or an integer")
     else if length(args) == 2 then
     when args.0
     is s:string do (
	  when args.1 
	  is i:ZZ do if !isInt(i) then WrongArgSmallInteger(2) else Expr(substr(s,toInt(i)))
	  else WrongArgZZ(2))
     is i:ZZ do if !isInt(i) then WrongArgSmallInteger(1) else (
	  when args.1
	  is s:string do Expr(substr(s,toInt(i)))
	  else WrongArgString(2))
     is pair:Sequence do (
	  if length(pair) != 2 then WrongArg(1,"a sequence of length 2") else
	  when pair.0 is i:ZZ do if !isInt(i) then WrongArg(1,"a pair of small integers") else (
	       when pair.1 is j:ZZ do if !isInt(j) then WrongArg(1,"a pair of small integers") else
	       when args.1 is s:string do Expr(substr(s,toInt(i),toInt(j)))
	       else WrongArgString(2)
	       else WrongArg(1,"a pair of integers"))
	  else WrongArg(1,"a pair of integers"))
     else WrongArg(1,"a string, an integer, or a pair of integers")
     else WrongNumArgs(2,3)
     else WrongNumArgs(2,3));
setupfun("substring",substrfun);
     
linesE(s:string):Expr := (
     v := lines(s);
     list(new Sequence len length(v) do foreach t in v do provide Expr(t)));
lines(s:string,c:char):Expr := (
     nlines := 1;
     i := 0;
     while true do (
	  j := index(s,i,c);
	  if j == -1 then (
     	       -- if i != length(s) then nlines = nlines + 1;
	       break;
	       );
	  i = j+1;
	  nlines = nlines + 1;
	  );
     i = 0;
     list(new Sequence len nlines do (
	       while true do (
		    j := index(s,i,c);
		    if j == -1 then (
			 -- if i != length(s) then provide Expr(substr(s,i));
			 provide Expr(substr(s,i));
			 break;
			 )
		    else (
			 provide Expr(substr(s,i,j-i));
			 i = j+1;
			 )))));
lines(s:string,c:char,d:char):Expr := (
     -- nlines := 0;
     nlines := 1;
     i := 0;
     while true do (
	  j := index(s,i,c,d);
	  if j == -1 then (
     	       -- if i != length(s) then nlines = nlines + 1;
	       break;
	       );
	  i = j+2;
	  nlines = nlines + 1;
	  );
     i = 0;
     list(new Sequence len nlines do (
	       while true do (
		    j := index(s,i,c,d);
	  	    if j == -1 then (
			 -- if i != length(s) then provide Expr(substr(s,i));
			 provide Expr(substr(s,i));
			 break;
			 )
		    else (
			 provide Expr(substr(s,i,j-i));
			 i = j+2;
			 )))));
lines(s:string,ch:string):Expr := (
     if length(ch) == 1 
     then Expr(lines(s,ch.0))
     else if length(ch) == 2
     then Expr(lines(s,ch.0,ch.1))
     else WrongArg(1,"a string of length 1 or 2")
     );
linesfun(e:Expr):Expr := (
     when e
     is a:Sequence do
     if length(a) == 2 then
     when a.1
     is s:string do 
     when a.0 is ch:string do lines(s,ch)
     else WrongArgString(1)
     else WrongArgString(2)
     else WrongNumArgs(2)
     is s:string do linesE(s)
     else WrongArgString());
setupfun("separate",linesfun);

tostring(m:MysqlConnectionWrapper):string := (
     Ccode(void, "extern string tostring2(const char *)");
     "<<MysqlConnection : " + (
	  when m.mysql 
	  is null 
	  do "closed" 
	  is n:MysqlConnection 
	  do Ccode(string, "tostring2(\n#if USE_MYSQL\n mysql_get_host_info((MYSQL*)", n, ")\n#else\n \"not present\"\n#endif\n)" )
	  )
     + ">>");

tostringfun(e:Expr):Expr := (
     Ccode(void, "extern string tostring2(const char *)");
     when e 
     is i:ZZ do Expr(tostring(i))
     is x:QQ do Expr(tostring(x))
     is s:string do e
     is q:SymbolClosure do Expr( if q.frame == globalFrame then q.symbol.word.name else internalName(q.symbol.word.name) )
     is f:file do Expr(f.filename)
     is b:Boolean do Expr(if b.v then "true" else "false")
     is Nothing do Expr("null")
     is f:Database do Expr(f.filename)
     is m:MysqlConnectionWrapper do Expr(tostring(m))
     is res:MysqlResultWrapper do Expr(
	  "<<MysqlResult : " 
	  + tostring(Ccode(int, "\n # if USE_MYSQL \n mysql_num_rows((MYSQL_RES *)", res.res, ") \n #else \n 0 \n #endif \n"))
	  + " by "
	  + tostring(Ccode(int, "\n # if USE_MYSQL \n mysql_num_fields((MYSQL_RES *)", res.res, ") \n #else \n 0 \n #endif \n"))
	  + ">>")
     is fld:MysqlFieldWrapper do Expr(
	  "<<MysqlField : " 
	  + Ccode(string,"tostring2(\n #if USE_MYSQL \n ((MYSQL_FIELD *)", fld.fld, ")->name \n #else \n \"\" \n #endif \n )")
	  + " : "
	  + tostring(Ccode(int,"\n # if USE_MYSQL \n ((MYSQL_FIELD *)", fld.fld, ")->type \n #else \n 0 \n #endif \n"))
	  + ">>")
     is Net do Expr("<<net>>")
     is CodeClosure do Expr("<<pseudocode>>")
     is functionCode do Expr("<<a function body>>")
     is CompiledFunction do Expr("<<a compiled function>>")
     is CompiledFunctionClosure do Expr("<<a compiled function closure>>")
     is CompiledFunctionBody do Expr("<<a compiled function body>>")
     is FunctionClosure do Expr("<<a function closure>>")
     is DictionaryClosure do Expr("<<a dictionary>>")
     is NetFile do Expr("<<a netfile>>")
     is x:RR do Expr(tostringRR(x))
     is z:CC do Expr(tostringCC(z))
     is Error do Expr("<<an error message>>")
     is Sequence do Expr("<<a sequence>>")
     is HashTable do Expr("<<a hash table>>")
     is List do Expr("<<a list>>")
     is s:SpecialExpr do tostringfun(s.e)
     is x:RawMonomial do Expr(Ccode(string, "(string)IM2_Monomial_to_string((Monomial*)",x,")" ))
     is x:RawFreeModule do Expr(Ccode(string, "(string)IM2_FreeModule_to_string((FreeModule*)",x,")" ))
     is x:RawMatrix do Expr(Ccode(string, "(string)IM2_Matrix_to_string((Matrix*)",x,")" ))
     is x:RawMutableMatrix do Expr(Ccode(string, "(string)IM2_MutableMatrix_to_string((MutableMatrix*)",x,")" ))
     is x:RawStraightLineProgram do Expr(Ccode(string, "(string)rawStraightLineProgramToString((StraightLineProgram*)",x,")" ))
     is x:RawPathTracker do Expr(Ccode(string, "(string)rawPathTrackerToString((PathTracker*)",x,")" ))
     is x:RawRingMap do Expr(Ccode(string, "(string)IM2_RingMap_to_string((RingMap*)",x,")" ))
     is x:RawMonomialOrdering do Expr(Ccode(string, "(string)IM2_MonomialOrdering_to_string((MonomialOrdering*)",x,")" ))
     is x:RawMonoid do Expr(Ccode(string, "(string)IM2_Monoid_to_string((Monoid*)",x,")" ))
     is x:RawRing do Expr(Ccode(string, "(string)IM2_Ring_to_string((Ring*)",x,")" ))
     is x:RawRingElement do Expr( Ccode(string, "(string)IM2_RingElement_to_string((RingElement*)",x,")" ) )
     is x:RawMonomialIdeal do Expr(
	  "<<raw monomial ideal>>"
	  -- Ccode(string, "IM2_MonomialIdeal_to_string((MonomialIdeal*)",x,")" )
	  )
     is c:RawComputation do Expr(Ccode(string, "(string)IM2_GB_to_string((Computation*)",c,")" ))
     is po:pythonObject do (
	  -- Expr("<<a python object>>")
	  str := Ccode(pythonObject,"(tokens_pythonObject)PyObject_Str((PyObject*)",po,")");
	  r := Expr(Ccode(string,"tostring2(PyString_AS_STRING(",str,"))"));
	  Ccode(void,"Py_DECREF((PyObject*)",str,")");
	  r)
     );
setupfun("simpleToString",tostringfun);

connectionCount(e:Expr):Expr := (
     when e is f:file do if f.listener then Expr(toInteger(f.numconns))
     else WrongArg(1,"an open socket listening for connections")
     else WrongArg(1,"a file")
     );
setupfun("connectionCount", connectionCount);

format(e:Expr):Expr := (
     when e
     is s:string do Expr("\"" + present(s) + "\"")
     is RR do format(Expr(Sequence(e)))
     is CC do format(Expr(Sequence(e)))
     is args:Sequence do (
	  s := printingPrecision;
	  ac := printingAccuracy;
	  l := printingLeadLimit;
	  t := printingTrailLimit;
	  sep := printingSeparator;
	  n := length(args);
	  if n == 0 || n > 6 then return WrongNumArgs(1,6);
	  if n > 1 then when args.0 is p:ZZ do if !isInt(p) then return WrongArgSmallInteger(2) else s = toInt(p)
	  is Nothing do nothing else return WrongArgZZ(1);
	  if n > 2 then when args.1 is p:ZZ do if !isInt(p) then return WrongArgSmallInteger(2) else ac = toInt(p)
	  is Nothing do nothing else return WrongArgZZ(2);
	  if n > 3 then when args.2 is p:ZZ do if !isInt(p) then return WrongArgSmallInteger(2) else l = toInt(p)
	  is Nothing do nothing else return WrongArgZZ(3);
	  if n > 4 then when args.3 is p:ZZ do if !isInt(p) then return WrongArgSmallInteger(2) else t = toInt(p)
	  is Nothing do nothing else return WrongArgZZ(4);
	  if n > 5 then when args.4 is p:string do sep = p else return WrongArgString(5);
	  when args.(n-1)
	  is x:RR do Expr(concatenate(format(s,ac,l,t,sep,x))) 
	  is z:CC do Expr(format(s,ac,l,t,sep,false,false,z))
	  else WrongArgRR(n)
	  )
     else WrongArg("string, or real number, integer, integer, integer, string"));
setupfun("format",format);

numfun(e:Expr):Expr := (
     when e
     is r:QQ do Expr(numerator(r))
     else WrongArg("a rational number"));
setupfun("numerator",numfun);
denfun(e:Expr):Expr := (
     when e
     is r:QQ do Expr(denominator(r))
     else WrongArg("a rational number"));
setupfun("denominator",denfun);

join(e:Expr):Expr := (
     when e
     is a:Sequence do (
	  n := length(a);
	  if n == 0 then return e;
	  newlen := 0;
	  foreach x in a do (
	       when x
	       is b:Sequence do (newlen = newlen + length(b);)
	       is c:List do (newlen = newlen + length(c.v);)
	       else return WrongArg("lists or sequences");
	       );
	  z := new Sequence len newlen do (
	       foreach x in a do (
		    when x
		    is b:Sequence do foreach y in b do provide y
		    is c:List do foreach y in c.v do provide y
		    else nothing;
		    );
	       );
	  when a.0
	  is Sequence do Expr(z)
	  is c:List do list(c.class,z,c.mutable)
	  else nullE			  -- shouldn't happen anyway
	  )
     is c:List do if c.mutable then Expr(copy(c)) else e
     else WrongArg("lists or sequences"));
setupfun("join",join);

instanceof(e:Expr):Expr := (
     when e
     is args:Sequence do (
	  when args.1
	  is y:HashTable do if ancestor(Class(args.0),y) then True else False
	  else WrongArg(2,"a hash table"))
     else WrongNumArgs(2));
setupfun("instance",instanceof);

ancestorfun(e:Expr):Expr := (
     when e
     is args:Sequence do (
	  x := args.1;					    -- args reversed
	  y := args.0;
	  if x==y then return True;
	  when x
	  is xx:HashTable do (
	       when y 
	       is yy:HashTable do if ancestor(xx,yy) then True else False
	       else WrongArg(1,"a hash table"))
	  else WrongArg(2,"a hash table"))
     else WrongNumArgs(2));
setupfun("ancestor",ancestorfun);

hadseq := false;
deeplen(a:Sequence):int := (
     n := 0;
     foreach x in a do (
	  when x
	  is b:Sequence do (
	       hadseq = true;
	       n = n + deeplen(b);
	       )
	  else (
	       n = n+1;
	       );
	  );
     n);
deepseq := emptySequence;
deepindex := 0;
deepinsert(a:Sequence):int := (
     n := 0;
     foreach x in a do (
	  when x
	  is b:Sequence do n = n + deepinsert(b)
	  else (
	       deepseq.deepindex = x;
	       deepindex = deepindex+1;
	       n = n+1;
	       );
	  );
     n);
export deepsplice(a:Sequence):Sequence := (
     -- warning: this returns its arg if no change is required.
     hadseq = false;
     newlen := deeplen(a);
     if hadseq then (
     	  deepseq = new Sequence len deeplen(a) do provide nullE;
     	  deepindex = 0;
     	  deepinsert(a);
     	  w := deepseq;
     	  deepseq = emptySequence;
     	  w)
     else a);
deepsplice(e:Expr):Expr := (
     when e
     is args:Sequence do Expr(deepsplice(args))
     is a:List do (
	  r := deepsplice(a.v);
	  if r == a.v 
	  then (
	       if a.mutable
	       then list(a.class,copy(r),a.mutable)
	       else e)
	  else list(a.class,r,a.mutable))
     else e);
setupfun("deepSplice",deepsplice);

exec(a:Sequence):Expr := (
     newargv := new array(string) len length(a) do provide "";
     foreach x at i in a do (
	  when x
	  is s:string do newargv.i = s
	  else return WrongArgString(i+1));
     exec(newargv);
     buildErrorPacket("exec failed"));
exec(e:Expr):Expr := (
     when e
     is a:Sequence do exec(a)
     is a:List do exec(a.v)
     is s:string do exec(Sequence(e))
     else WrongArg( "a string or a sequence or list of strings"));
setupfun("exec",exec);

youngest(e:Expr):Expr := (
     when e
     is y:HashTable do if !y.mutable then nullE else e
     is b:Sequence do (
	  if length(b) == 0
	  then nullE
	  else (
	       h := 0;
	       e = nullE;
	       foreach x in b do (
		    when x
		    is y:HashTable do if y.mutable && y.hash>h then ( h = y.hash; e = x; ) else nothing
		    is y:file do if y.hash>h then ( h = y.hash; e = x; ) else nothing
		    is y:CompiledFunctionClosure do if y.hash>h then ( h = y.hash; e = x; ) else nothing
		    is y:SymbolClosure do if y.symbol.hash>h then ( h = y.symbol.hash; e = x; ) else nothing
		    else nothing;
		    );
	       e))
     else nullE);
setupfun("youngest", youngest);

toRR(e:Expr):Expr := (
     when e
     is x:ZZ do Expr(toRR(x,defaultPrecision))
     is x:QQ do Expr(toRR(x,defaultPrecision))
     is RR do e
     is s:Sequence do (
	  if length(s) != 2 then WrongNumArgs(1,2) else
	  when s.0 is prec:ZZ do (
	       if !isULong(prec) then WrongArgSmallUInteger(1)
	       else (
	       	    when s.1 
     	       	    is x:ZZ do Expr(toRR(x,toULong(prec)))
	       	    is x:QQ do Expr(toRR(x,toULong(prec)))
     	       	    is x:RR do Expr(toRR(x,toULong(prec)))
		    else WrongArg(1,"an integral, rational, or real number")
		    )
	       )
	  else WrongArgZZ(1)
	  )
     else WrongArg("an integral, rational, or real number, or a pair"));
setupfun("toRR",toRR);

toCC(e:Expr):Expr := (
     when e
     is x:ZZ do Expr(toCC(x,defaultPrecision)) -- # typical value: toCC, ZZ, CC
     is x:QQ do Expr(toCC(x,defaultPrecision)) -- # typical value: toCC, QQ, CC
     is x:RR do Expr(toCC(x)) -- # typical value: toCC, RR, CC
     is CC do e -- # typical value: toCC, CC, CC
     is s:Sequence do (
	  if length(s) == 2 then (
	       when s.0 is prec:ZZ do (
		    if !isULong(prec) then WrongArgSmallUInteger(1)
		    else (
			 when s.1 
			 is x:ZZ do Expr(toCC(x,toULong(prec))) -- # typical value: toCC, ZZ, ZZ, CC
			 is x:QQ do Expr(toCC(x,toULong(prec))) -- # typical value: toCC, ZZ, QQ, CC
			 is x:RR do Expr(toCC(x,toULong(prec))) -- # typical value: toCC, ZZ, RR, CC
			 is x:CC do Expr(toCC(x,toULong(prec))) -- # typical value: toCC, ZZ, CC, CC
			 else WrongArg("a rational number, real number, or an integer")
			 )
		    )
	       is x:RR do (
		    when s.1 is y:RR do Expr(toCC(x,y))	    -- # typical value: toCC, RR, RR, CC
		    else WrongArgRR()
		    )
	       else WrongArgZZ(1)
	       )
	  else if length(s) == 3 then (
	       when s.0 is prec:ZZ do (
		    -- # typical value: toCC, ZZ, ZZ, ZZ, CC
		    -- # typical value: toCC, ZZ, ZZ, QQ, CC
		    -- # typical value: toCC, ZZ, ZZ, RR, CC
		    -- # typical value: toCC, ZZ, QQ, ZZ, CC
		    -- # typical value: toCC, ZZ, QQ, QQ, CC
		    -- # typical value: toCC, ZZ, QQ, RR, CC
		    -- # typical value: toCC, ZZ, RR, ZZ, CC
		    -- # typical value: toCC, ZZ, RR, QQ, CC
		    -- # typical value: toCC, ZZ, RR, RR, CC
		    if !isULong(prec) then WrongArgSmallUInteger(1)
		    else Expr(CC(
			      when s.1 
			      is x:QQ do toRR(x,toULong(prec))
			      is x:ZZ do toRR(x,toULong(prec))
			      is x:RR do toRR(x,toULong(prec))
			      else (
				   return WrongArg("a rational number, real number, or an integer");
				   toRR(0,toULong(prec)) -- dummy
				   )
			      ,
			      when s.2
			      is x:QQ do toRR(x,toULong(prec))
			      is x:ZZ do toRR(x,toULong(prec))
			      is x:RR do toRR(x,toULong(prec))
			      else (
				   return WrongArg("a rational number, real number, or an integer");
				   toRR(0,toULong(prec)) -- dummy
				   ))))
	       else WrongArgZZ(1))
	  else WrongNumArgs(1,3))
     else WrongArg("a real or complex number, or 2 or 3 arguments"));
setupfun("toCC",toCC);

precision(e:Expr):Expr := (
     when e 
     is x:RR do Expr(toInteger(precision(x)))
     is x:CC do Expr(toInteger(precision(x)))
     else WrongArgRR()
     );
setupfun("precision0",precision);

-- locate:

positionRange := {filename:string, minline:int, mincol:int, maxline:int, maxcol:int};
locatedCode := positionRange("",0,0,0,0);
lookat(p:Position):void := (
     if p == dummyPosition then return;
     locatedCode.filename = p.filename;
     if locatedCode.minline > int(p.line) then (
	  locatedCode.minline = int(p.line);
	  locatedCode.mincol = int(p.column);
	  )
     else if locatedCode.minline == int(p.line) && locatedCode.mincol > int(p.column) then (
	  locatedCode.mincol = int(p.column);
	  );
     if locatedCode.maxline < int(p.line) then (
	  locatedCode.maxline = int(p.line);
	  locatedCode.maxcol = int(p.column);
	  )
     else if locatedCode.maxline == int(p.line) && locatedCode.maxcol < int(p.column) then (
	  locatedCode.maxcol = int(p.column);
	  );
     );
locate(x:Token):void := lookat(position(x));
locate(e:Code):void := (
     when e
     is nullCode do nothing
     is v:adjacentCode do (lookat(v.position); locate(v.lhs); locate(v.rhs);)
     is v:arrayCode do foreach c in v.z do locate(c)
     is v:Error do lookat(v.position)
     is v:semiCode do foreach c in v.w do locate(c)
     is v:binaryCode do (lookat(v.position); locate(v.lhs); locate(v.rhs);)
     is v:forCode do ( lookat(v.position); locate(v.fromClause); locate(v.toClause); locate(v.whenClause); locate(v.listClause); locate(v.doClause); )
     is v:functionCode do (locate(v.arrow);locate(v.body);)
     is v:globalAssignmentCode do (lookat(v.position); locate(v.rhs);)
     is v:globalMemoryReferenceCode do lookat(v.position)
     is v:globalSymbolClosureCode do lookat(v.position)
     is v:ifCode do ( lookat(v.position); locate(v.predicate); locate(v.thenClause); locate(v.elseClause); )
     is v:integerCode do lookat(v.position)
     is v:listCode do foreach c in v.y do locate(c)
     is v:localAssignmentCode do (lookat(v.position); locate(v.rhs);)
     is v:localMemoryReferenceCode do lookat(v.position)
     is v:localSymbolClosureCode do lookat(v.position)
     is v:multaryCode do ( lookat(v.position); foreach c in v.args do locate(c);)
     is v:newCode do ( lookat(v.position); locate(v.newClause); )
     is v:newFromCode do ( lookat(v.position); locate(v.newClause); locate(v.fromClause); )
     is v:newLocalFrameCode do locate(v.body)
     is v:newOfCode do ( lookat(v.position); locate(v.newClause); locate(v.ofClause); )
     is v:newOfFromCode do ( lookat(v.position); locate(v.newClause); locate(v.ofClause); locate(v.fromClause); )
     is v:parallelAssignmentCode do (lookat(v.position); locate(v.rhs);)
     is v:realCode do lookat(v.position)
     is v:sequenceCode do foreach c in v.x do locate(c)
     is v:stringCode do nothing
     is v:ternaryCode do ( lookat(v.position); locate(v.arg1); locate(v.arg2); locate(v.arg3);)
     is v:tryCode do ( lookat(v.position); locate(v.code); locate(v.thenClause); locate(v.elseClause); )
     is v:catchCode do ( lookat(v.position); locate(v.code); )
     is v:unaryCode do (lookat(v.position); locate(v.rhs);)
     is v:whileDoCode do ( lookat(v.position); locate(v.predicate); locate(v.doClause); )
     is v:whileListCode do ( lookat(v.position); locate(v.predicate); locate(v.listClause); )
     is v:whileListDoCode do ( lookat(v.position); locate(v.predicate); locate(v.listClause); locate(v.doClause); )
     );
locate0():void := (
     locatedCode.filename = "{*unknown file name*}";
     locatedCode.minline = 1000000;
     locatedCode.maxline = 0;
     );
locate1():void := (
     if locatedCode.minline == 1000000 then (
	  locatedCode.minline = 0;
	  locatedCode.mincol = 0;
	  locatedCode.maxcol = 0;
	  ));
locate2(c:Code):Expr := (
     locate1();
     p := codePosition(c);
     Expr(Sequence(
	       Expr(verifyMinimizeFilename(locatedCode.filename)),
	       Expr(toInteger(locatedCode.minline)),
	       Expr(toInteger(locatedCode.mincol)),
	       Expr(toInteger(locatedCode.maxline)),
	       Expr(toInteger(locatedCode.maxcol)),
	       Expr(toInteger(int(p.line))),
	       Expr(toInteger(int(p.column)))
	       )));
locate(e:Expr):Expr := (
     when e
     is Nothing do nullE
     is Sequence do locate(lookupfun(e))
     is CompiledFunction do nullE
     is CompiledFunctionClosure do nullE
     is s:SymbolClosure do (
	  p := s.symbol.position;
	  if p == dummyPosition
	  then nullE
	  else Expr(
	       Sequence(
		    verifyMinimizeFilename(p.filename),
		    toInteger(int(p.line)),toInteger(int(p.column)),
		    toInteger(int(p.line)),toInteger(int(p.column)+length(s.symbol.word.name)-1),
		    toInteger(int(p.line)),toInteger(int(p.column))
		    )))
     is c:CodeClosure do (
	  locate0();
	  locate(c.code);
	  locate2(c.code))
     is s:SpecialExpr do locate(s.e)
     is f:FunctionClosure do (
	  locate0();
	  locate(f.model.arrow);
	  locate(f.model.body);
	  locate2(f.model.body))
     else WrongArg("a function, symbol, sequence, or null"));
setupfun("locate",locate);

export bar := 1;
export storeInHashTableWithCollisionHandler(x:HashTable,key:Expr,value:Expr,handler:Expr):Expr := (
     if !x.mutable then return buildErrorPacket("attempted to modify an immutable hash table");
     h := hash(key);
     hmod := int(h & (length(x.table)-1));
     p := x.table.hmod;
     while p != p.next do (
	  if p.key == key || equal(p.key,key)==True 
	  then (
	       ret := applyEEE(handler,p.value,value);
	       return when ret is Error do ret else (p.value = ret; ret));
	  p = p.next);
     if 4 * x.numEntries == 3 * length(x.table) -- SEE ABOVE
     then (
	  enlarge(x);
	  hmod = int(h & (length(x.table)-1));
	  );
     x.numEntries = x.numEntries + 1;
     x.table.hmod = KeyValuePair(key,h,value,x.table.hmod);
     value);
toHashTableWithCollisionHandler(v:Sequence,handler:Expr):Expr := (
     o := newHashTable(hashTableClass,nothingClass);
     foreach e at i in v do (
	  when e
	  is Nothing do nothing
	  is pair:Sequence do (
	       if length(pair) == 2 
	       then (storeInHashTableWithCollisionHandler(o,pair.0,pair.1,handler);)
	       else return toHashTableError(i))
	  is z:List do (
	       pair := z.v;
	       if length(pair) == 2 
	       then (storeInHashTableWithCollisionHandler(o,pair.0,pair.1,handler);)
	       else return toHashTableError(i))
	  else return toHashTableError(i));
     sethash(o,false);
     Expr(o));
toHashTable(e:Expr):Expr := (
     when e
     is w:List do toHashTable(w.v)
     is s:Sequence do (
	  if length(s) != 2 then return WrongNumArgs(1,2);
	  when s.0
	  is FunctionClosure do nothing
	  is CompiledFunction do nothing
	  is CompiledFunctionClosure do nothing
	  else return WrongArg(1,"a function");
	  when s.1 is w:List do toHashTableWithCollisionHandler(w.v,s.0)
	  else WrongArg(2,"a list"))
     else WrongArg("a list"));
setupfun("hashTable",toHashTable);

powermod(e:Expr):Expr := (
     when e is s:Sequence do 
     if length(s) == 3 then
     when s.0 is base:ZZ do 
     when s.1 is exp:ZZ do
     when s.2 is mod:ZZ do
     Expr(powermod(base,exp,mod))
     else WrongArgZZ(3)
     else WrongArgZZ(2)
     else WrongArgZZ(1)
     else WrongNumArgs(3)
     else WrongNumArgs(3)
     );
setupfun("powermod",powermod);

partsRR(x:Expr):Expr := (
     when x is xx:RR do (
	  p := Ccode(ulong,"(unsigned long)((__mpfr_struct *)",xx,")->_mpfr_prec");
	  sz := 8 * Ccode(int,"sizeof(*((__mpfr_struct *)",xx,")->_mpfr_d)");
	  n := (p+sz-1)/sz;
	  numbits := n * sz;
	  prec := toInteger(p);
	  sgn := toInteger(Ccode(long,"(long)((__mpfr_struct *)",xx,")->_mpfr_sign"));
	  expt := toInteger(Ccode(long,"(long)((__mpfr_struct *)",xx,")->_mpfr_exp"));
	  m := toInteger(0);
	  for i from int(n)-1 to 0 by -1 do (
	       m = (m << sz) + toInteger(Ccode(ulong,"(unsigned long)((__mpfr_struct *)",xx,")->_mpfr_d[",i,"]"));
	       );
	  Expr(Sequence(prec,sgn,expt,m,toInteger(numbits))))
     else WrongArg("a real number"));
setupfun("partsRR",partsRR);

-- Local Variables:
-- compile-command: "make -C $M2BUILDDIR/Macaulay2/d "
-- End:
