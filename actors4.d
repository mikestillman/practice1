-- Copyright 1994 by Daniel R. Grayson

use C;
use system; 
use convertr;
use binding;
use parser;
use lex;
use gmp;
use engine;
use nets;
use tokens;
use util;
use err;
use stdiop;
use ctype;
use stdio;
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
     -- "$" + s						    -- old way, but now we can see these symbols sometimes
     s
     );

sleepfun(e:Expr):Expr := (
     when e
     is i:Integer do (
	  if isInt(i)
	  then Expr(toInteger(sleep(toInt(i))))
	  else WrongArgSmallInteger(1))
     else WrongArgInteger(1));
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
     is a:Sequence do (
	  if length(a) == 0
	  then Expr(toInteger(getpid()))
	  else WrongNumArgs(0))
     else WrongNumArgs(0));
-- processID is a function, in case we have threads!
setupfun("processID",getpidfun);

absfun(e:Expr):Expr := (
     when e
     is i:Integer do Expr(abs(i))
     is d:Real do Expr(Real(if d.v < 0. then -d.v else d.v))
     is r:Rational do Expr(abs(r))
     else WrongArg("a number, real or complex"));
setupfun("abs",absfun);

select(a:Sequence,f:Expr):Expr := (
     b := new array(bool) len length(a) do provide false;
     found := 0;
     foreach x at i in a do (
	  y := apply(f,x);
	  when y is Error do return(y) else nothing;
	  if y == True then (
	       b.i = true;
	       found = found + 1;
	       ));
     new Sequence len found do (
	  foreach p at i in b do if p then provide a.i));
select(e:Expr,f:Expr):Expr := (
     when e
     is obj:HashTable do (
	  if obj.mutable then return(WrongArg(0+1,"an immutable hash table"));
	  u := newHashTable(obj.class,obj.parent);
	  foreach bucket in obj.table do (
	       p := bucket;
	       while p != bucketEnd do (
		    newvalue := apply(f,p.value);
		    when newvalue 
		    is Error do return(newvalue)
		    else if newvalue == True 
		    then storeInHashTable(u,p.key,p.hash,p.value);
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
     else WrongArg(0+1,"a list"));
select(n:int,a:Sequence,f:Expr):Expr := (
     b := new array(bool) len length(a) do provide false;
     found := 0;
     foreach x at i in a do (
	  if found < n then (
	       y := apply(f,x);
	       when y is Error do return(y) else nothing;
	       if y == True then (
		    b.i = true;
		    found = found + 1;
		    ))
	  else b.i = false);
     new Sequence len found do (
	  foreach p at i in b do if p then provide a.i));
select(n:Expr,e:Expr,f:Expr):Expr := (
     when n is nn:Integer do
     if isInt(n) then
     when e
     is obj:HashTable do (
	  if obj.mutable then return(WrongArg(1+1,"an immutable hash table"));
	  u := newHashTable(obj.class,obj.parent);
	  nval := toInt(n);
	  if nval > 0 then
	  foreach bucket in obj.table do (
	       p := bucket;
	       while nval > 0 && p != bucketEnd do (
		    newvalue := apply(f,p.value);
		    when newvalue 
		    is Error do return(newvalue)
		    else if newvalue == True 
		    then (
			 storeInHashTable(u,p.key,p.hash,p.value);
			 nval = nval-1;
			 );
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
     else WrongArgSmallInteger(0+1)
     else WrongArgInteger(0+1));
select(e:Expr):Expr := (
     when e is a:Sequence do (
	  if length(a) == 2
	  then select(a.0,a.1)
	  else if length(a) == 3
	  then select(a.0,a.1,a.2)
	  else WrongNumArgs(2,3))  -- could change this later
     else WrongNumArgs(2,3));
setupfun("select",select);

any(f:Expr,obj:HashTable):Expr := (
     foreach bucket in obj.table do (
	  p := bucket;
	  while true do (
	       if p == bucketEnd then break;
	       v := apply(f,p.key,p.value);
	       when v is Error do return(v) else nothing;
	       if v == True then return(True);
	       p = p.next;
	       ));
     False);
any(f:Expr,a:Sequence):Expr := (
     foreach x at i in a do (
	  y := apply(f,x);
	  when y is Error do return(y) else nothing;
	  if y == True then return(True));
     False);
any(f:Expr,e:Expr):Expr := (
     when e
     is a:Sequence do Expr(any(f,a))
     is b:List do Expr(any(f,b.v))
     is c:HashTable do 
     if c.mutable then WrongArg(1,"an immutable hash table") else
     Expr(any(f,c))
     else WrongArg("a list"));
any(e:Expr):Expr := (
     when e is a:Sequence do (
	  if length(a) == 2
	  then any(a.1,a.0)
	  else WrongNumArgs(2))
     else WrongNumArgs(2));
setupfun("any",any);

--find(f:Expr,obj:HashTable):Expr := (
--     foreach bucket in obj.table do (
--	  p := bucket;
--	  while true do (
--	       if p == bucketEnd then break;
--	       r := apply(f,p.key,p.value);
--	       if r != nullE then return(r);
--	       p = p.next;
--	       ));
--     nullE);
--find(f:Expr,a:Sequence):Expr := (
--     i := 0;
--     while i < length(a) do (
--	  r := apply(f,a.i);
--	  if r != nullE then return(r);
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
     is i:Integer do (
	  if isInt(i)
	  then Expr(new string len 1 do provide char(toInt(i)))
	  else WrongArgSmallInteger(1))
     else buildErrorPacket("expects a string, a small integer, or an array of small integers"));
setupfun("ascii",ascii);

transnet(e:Expr):Expr := (
     if isIntArray(e)
     then Expr((
	       v := toIntArray(e);
	       new string len 4 * length(v) do foreach i in v do (
	       	    provide char(i >> 24);
	       	    provide char(i >> 16);
	       	    provide char(i >> 8);
	       	    provide char(i);
	       	    )))
     else when e is s:string do (
	  if length(s) % 4 != 0
	  then WrongArg("a string whose length is a multiple of 4")
	  else (
	       j := 0;
	       list(new Sequence len length(s)/4 do (
		    i := 0xff & int(s.j);
		    i = i << 8;
		    j = j+1;
		    i = i + (0xff & int(s.j));
		    i = i << 8;
		    j = j+1;
		    i = i + (0xff & int(s.j));
		    i = i << 8;
		    j = j+1;
		    i = i + (0xff & int(s.j));
		    j = j+1;
		    provide Expr(toInteger(i));
		    ))))
     else WrongArg("a string or array of small integers"));
setupfun("transnet",transnet);

export checknoargs(e:Expr):Expr := (
     when e
     is v:Sequence do (
	  if length(v) == 0 then e else WrongNumArgs(0)
	  )
     else WrongNumArgs(0)
     );

randomint(e:Expr):Expr := (
     when checknoargs(e) is f:Error do return(Expr(f)) else nothing;
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
	  is y:Integer do (
	       if isInt(y) 
	       then for toInt(y) do x << ' '
	       else return(WrongArg(2,"a string, or list or sequence of strings and small integers, or null")))
	  is w:Sequence do (outseq(x,w);)
	  is s:string do (x << s;)
	  else return(WrongArg(2,"a string, or list or sequence of strings and integers, or null")));
     Expr(x));
outstringfun(e:Expr):Expr := (
     when e
     is a:Sequence do (
	  if length(a) == 2 then (
	       when a.0 
	       is x:file do (
		    if x.outfd == -1 then return(WrongArg("an open output file"));
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
		    is y:Integer do (
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
	       if m == -1 then return(-1);
	       l = l + m;
	       )
	  is Nothing do nothing
	  is y:SymbolClosure do l = l + length(y.symbol.word.name)
	  is y:Integer do (
	       if isInt(y) 
	       then l = l + toInt(y)
	       else return(-1))
	  is w:Sequence do (
	       m := lenseq(w);
	       if m == -1 then return(-1);
	       l = l + m;
	       )
	  is s:string do (
	       l = l + length(s);
	       )
	  else return(-1));
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
     is y:Integer do e
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
	  is n:Integer do for toInt(n) do (s.i = ' '; i = i+1;)
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
     is n:Integer do (
	  if isInt(n) then (
	       m := toInt(n);
	       if m >= 0 
	       then Expr(new string len m do provide ' ')
	       else buildErrorPacket("encountered negative integer")
	       )
	  else buildErrorPacket("encountered a large integer")
	  )
     is string do e
     is Error do e
     else WrongArg("a sequence or list of strings, integers, or symbols"));
setupfun("concatenate",stringcatfun);

errorfun(e:Expr):Expr := (
     e = stringcatfun(e);
     when e
     is s:string do buildErrorPacket(s)
     else buildErrorPacket("expects a string or sequence of strings as its argument"));
setupfun("error",errorfun);

mingleseq(a:Sequence):Expr := (
     n := length(a);
     b := new array(Sequence) len n do provide emptySequence;
     newlen := 0;
     for i from 0 to n-1 do (
	  when a.i
	  is d:Sequence do ( newlen = newlen + length(d); b.i = d; )
	  is d:List do ( newlen = newlen + length(d.v); b.i = d.v; )
	  else return(WrongArg(i+1,"a list or sequence")));
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
	       is n:Integer do (
		    if isInt(n)
		    then (
			 nn := toInt(n);
			 if nn > 0
			 then (
			      when a.1
			      is x:Sequence do packlist(x,nn)
			      is x:List do packlist(x.v,nn)
			      else WrongArg(1,"a list or sequence")
			      )
			 else WrongArg(1,"a positive integer")
			 )
		    else WrongArgSmallInteger(1)
		    )
	       is x:Sequence do (
		    when a.1
		    is n:Integer do (
			 if isInt(n) 
			 then (
			      nn := toInt(n);
			      if nn > 0 
			      then packlist(x,nn)
			      else WrongArg(2,"a positive integer"))
			 else WrongArgSmallInteger(2))
		    else WrongArgInteger(2))
	       is x:List do (
		    when a.1
		    is n:Integer do (
			 if isInt(n) 
			 then (
			      nn := toInt(n);
			      if nn > 0 
			      then packlist(x.v,nn)
			      else WrongArg(2,"a positive integer"))
			 else WrongArgSmallInteger(2))
		    else WrongArgInteger(2))
	       else WrongArg(1,"a list or sequence"))
	  else WrongNumArgs(2))
     else WrongNumArgs(2));	  
setupfun("pack", packfun);

getenvfun(e:Expr):Expr := (
     when e
     is s:string do Expr(getenv(s))
     else WrongArgString());
setupfun("getenv",getenvfun);

getcwdfun(e:Expr):Expr := (
     when e
     is s:Sequence do
     if length(s) == 0
     then Expr(getcwd())
     else WrongNumArgs(0)
     else WrongNumArgs(0));
setupfun("currentDirectory",getcwdfun);

getfun(e:Expr):Expr := (
     when e
     is f:file do (
	  if f.infd == -1
	  then WrongArg("an open input file")
	  else (
	       when readfile(f.infd)
	       is s:string do (
		    closeIn(f);
		    Expr(s)
		    )
	       else buildErrorPacket("unable to read file: "+syserrmsg())
	       )
	  )
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

allInputFiles(s:Sequence):bool := (
     foreach f in s do (
     	  when f is g:file do ( if !g.input then return(false); )
     	  else return(false);
	  );
     true);
numberReadyOnes(s:Sequence):int := (
     n := 0;
     foreach f in s do (
     	  when f is g:file do ( if g.insize > g.inindex then n = n+1; )
     	  else nothing;
	  );
     n);
readyOnes(s:Sequence):array(int) := (
     new array(int) len numberReadyOnes(s) do
     foreach f at i in s do
     when f is g:file do ( if g.insize > g.inindex then provide i; )
     else nothing
     );
fdlist(s:Sequence):array(int) := (
     new array(int) len length(s) do
     foreach f in s do 
     when f is g:file do provide g.infd else nothing
     );
wait(s:Sequence):Expr := (
     if !allInputFiles(s) then WrongArg("an input file or list of input files")
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
     is f:file do wait(f)
     is x:Integer do (
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
	  r := getline(stdin);
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
		    then return(WrongArg(1,"expected an input file"));
		    when s.1
		    is n:Integer do (
			 if isInt(n)
			 then (
			      nn := toInt(n);
			      if nn < 0
			      then return(WrongArg(2,"a positive integer"));
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
		    else WrongArgInteger(2)
		    )
	       else WrongArg(1,"a file")
	       )
	  else WrongNumArgs(0,1))
     else WrongArg(1,"a file or a string"));
setupfun("read",readfun);

export setupargv():void := (
     setupconst("commandLine",toExpr(argv)).transient = true;
     setupconst("environment",toExpr(envp)).transient = true;
     );

-- setupargv();

substrfun(e:Expr):Expr := (
     when e is args:Sequence do
     if length(args) == 3 then
     when args.0
     is i:Integer do if !isInt(i) then WrongArgSmallInteger(1) else (
	  when args.1 is j:Integer do if !isInt(j) then WrongArgSmallInteger(2) else
	  when args.2 is s:string do Expr(substr(s,toInt(i),toInt(j)))
	  else WrongArgString(3)
	  else WrongArgInteger(2))
     is s:string do (
	  when args.1 is i:Integer do if !isInt(i) then WrongArgSmallInteger(2) else
	  when args.2 is j:Integer do if !isInt(j) then WrongArgSmallInteger(3) else Expr(substr(s,toInt(i),toInt(j)))
	  else WrongArgInteger(3)
	  else WrongArgInteger(2))
     else WrongArg(1,"a string or an integer")
     else if length(args) == 2 then
     when args.0
     is s:string do (
	  when args.1 
	  is i:Integer do if !isInt(i) then WrongArgSmallInteger(2) else Expr(substr(s,toInt(i)))
	  else WrongArgInteger(2))
     is i:Integer do if !isInt(i) then WrongArgSmallInteger(1) else (
	  when args.1
	  is s:string do Expr(substr(s,toInt(i)))
	  else WrongArgString(2))
     else WrongArg(1,"a string or an integer")
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

tostringfun(e:Expr):Expr := (
     when e 
     is i:Integer do Expr(tostring(i))
     is x:Real do Expr(tostring(x.v))
     is z:Complex do Expr(tostring(z.re) + " + ii * " + tostring(z.im))
     is x:Rational do Expr(tostring(x))
     is s:string do e
     is q:SymbolClosure do Expr(
	  if q.frame == globalFrame
	  then q.symbol.word.name
	  else internalName(q.symbol.word.name)
	  )
     is f:file do Expr(f.filename)
     is b:Boolean do Expr(if b.v then "true" else "false")
     is Nothing do Expr("null")
     is f:Database do Expr(f.filename)
     is Net do Expr("--net--")
     is CompiledFunction do Expr("--compiled function--")
     is CompiledFunctionClosure do Expr("--compiled function closure--")
     is FunctionClosure do Expr("--function closure--")
     is d:DictionaryClosure do Expr("--dictionary[" + tostring(d.dictionary.hash) + "]--")
     is x:BigReal do Expr(tostring(x))
     is Error do Expr("--error message--")
     is Sequence do Expr("--sequence--")
     is HashTable do Expr("--hash table--")
     is List do Expr("--list--")
     is x:LMatrixRR do (
	  Expr(Ccode(string, "LP_LMatrixRR_to_string((LMatrixRR*)",x,")" ))
	  )
     is x:LMatrixCC do (
	  Expr(Ccode(string, "LP_LMatrixCC_to_string((LMatrixCC*)",x,")" ))
	  )
     is x:RawMonomial do Expr(Ccode(string, "IM2_Monomial_to_string((Monomial*)",x,")" ))
     is x:RawFreeModule do Expr(Ccode(string, "IM2_FreeModule_to_string((FreeModule*)",x,")" ))
     is x:RawVector do Expr(Ccode(string, "IM2_Vector_to_string((Vector*)",x,")" ))
     is x:RawMatrix do (
	  Expr(Ccode(string, "IM2_Matrix_to_string((Matrix*)",x,")" ))
	  )
     is x:RawMutableMatrix do (
	  Expr(Ccode(string, "IM2_MutableMatrix_to_string((MutableMatrix*)",x,")" ))
	  )
     is x:RawRingMap do (
	  Expr(Ccode(string, "IM2_RingMap_to_string((RingMap*)",x,")" ))
	  )
     is x:RawMonomialOrdering do Expr(Ccode(string,
	       "IM2_MonomialOrdering_to_string((MonomialOrdering*)",x,")" ))
     is x:RawMonoid do Expr(Ccode(string, "IM2_Monoid_to_string((Monoid*)",x,")" ))
     is x:RawRing do Expr(Ccode(string, "IM2_Ring_to_string((Ring*)",x,")" ))
     is x:RawRingElement do Expr(
	  Ccode(string, "IM2_RingElement_to_string((RingElement*)",x,")" )
	  )
     is x:RawMonomialIdeal do Expr(
	  "--raw monomial ideal--"
	  -- Ccode(string, "IM2_MonomialIdeal_to_string((MonomialIdeal*)",x,")" )
	  )
     is BigComplex do Expr("--big complex number--")
     is c:RawComputation do Expr(Ccode(string, "IM2_GB_to_string((Computation*)",c,")" ))
     );
setupfun("string",tostringfun);

connectionCount(e:Expr):Expr := (
     when e is f:file do if f.listener then Expr(toInteger(f.numconns))
     else WrongArg(1,"an open socket listening for connections")
     else WrongArg(1,"a file")
     );
setupfun("connectionCount", connectionCount);

presentfun(e:Expr):Expr := (
     when e
     is s:string do Expr("\"" + present(s) + "\"")
     else WrongArgString(1));
setupfun("format",presentfun);

numfun(e:Expr):Expr := (
     when e
     is r:Rational do Expr(numerator(r))
     else WrongArg("a rational number"));
setupfun("numerator",numfun);
denfun(e:Expr):Expr := (
     when e
     is r:Rational do Expr(denominator(r))
     else WrongArg("a rational number"));
setupfun("denominator",denfun);

join(e:Expr):Expr := (
     when e
     is a:Sequence do (
	  n := length(a);
	  if n == 0 then return(e);
	  newlen := 0;
	  foreach x in a do (
	       when x
	       is b:Sequence do (newlen = newlen + length(b);)
	       is c:List do (newlen = newlen + length(c.v);)
	       else return(WrongArg("lists or sequences"));
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
	  else False)
     else WrongNumArgs(2));
setupfun("instance",instanceof);

ancestorfun(e:Expr):Expr := (
     when e
     is args:Sequence do (
	  x := args.0;
	  y := args.1;
	  if x==y then return(True);
	  when y 
	  is yy:HashTable do if ancestor(Parent(x),yy) then True else False
	  else False)
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
	  else return(WrongArgString(i+1)));
     exec(newargv);
     buildErrorPacket("exec failed"));
exec(e:Expr):Expr := (
     when e
     is a:Sequence do exec(a)
     is a:List do exec(a.v)
     is s:string do exec(Sequence(e))
     else WrongArg( "a string or a sequence or list of strings"));
setupfun("exec",exec);

extent := {filename:string, minline:int, mincol:int, maxline:int, maxcol:int};
code := extent("",0,0,0,0);

locate(e:Code):void;

lookat(p:Position):void := (
     if p == dummyPosition then return();
     code.filename = p.filename;
     if code.minline > int(p.line) then (
	  code.minline = int(p.line);
	  code.mincol = int(p.column);
	  )
     else if code.minline == int(p.line) && code.mincol > int(p.column) then (
	  code.mincol = int(p.column);
	  );
     if code.maxline < int(p.line) then (
	  code.maxline = int(p.line);
	  code.maxcol = int(p.column);
	  )
     else if code.maxline == int(p.line) && code.maxcol < int(p.column) then (
	  code.maxcol = int(p.column);
	  );
     );

locate(e:Code):void := (
     when e
     is f:localAssignmentCode do (lookat(f.position); locate(f.rhs);)
     is f:globalAssignmentCode do (lookat(f.position); locate(f.rhs);)
     is f:parallelAssignmentCode do (lookat(f.position); locate(f.rhs);)
     is f:localMemoryReferenceCode do lookat(f.position)
     is f:globalMemoryReferenceCode do lookat(f.position)
     is f:globalSymbolClosureCode do lookat(f.position)
     is f:localSymbolClosureCode do lookat(f.position)
     is f:unaryCode do (lookat(f.position); locate(f.rhs);)
     is f:binaryCode do (lookat(f.position); locate(f.lhs); locate(f.rhs);)
     is f:ternaryCode do (
	  lookat(f.position);
	  locate(f.arg1);
	  locate(f.arg2);
	  locate(f.arg3);)
     is f:multaryCode do ( lookat(f.position); foreach c in f.args do locate(c);)
     is f:forCode do (
	  lookat(f.position);
	  locate(f.fromClause);
	  locate(f.toClause);
	  locate(f.whenClause);
	  locate(f.listClause);
	  locate(f.doClause);
	  )
     is f:openDictionaryCode do locate(f.body)
     is f:functionCode do (locate(f.parms);locate(f.body);)
     is v:sequenceCode do foreach c in v.x do locate(c)
     is v:listCode do foreach c in v.y do locate(c)
     is v:arrayCode do foreach c in v.z do locate(c)
     is nullCode do nothing
     is v:realCode do lookat(v.position)
     is v:stringCode do lookat(v.position)
     is v:integerCode do lookat(v.position)
     );

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
		    minimizeFilename(p.filename),toInteger(int(p.line)),toInteger(int(p.line)))))
     is f:FunctionClosure do (
	  code.filename = "--unknown file name--";
	  code.minline = 1000000;
	  code.maxline = 0;
	  locate(f.model.parms);
	  locate(f.model.body);
	  if code.minline == 1000000 then (
	       code.minline = 0;
	       code.mincol = 0;
	       code.maxcol = 0;
	       );
	  Expr(Sequence(
		    Expr(minimizeFilename(code.filename)),
		    Expr(toInteger(code.minline)),
		    Expr(toInteger(code.maxline))
		    )))
     else WrongArg("a function, symbol, sequence, or null"));
setupfun("locate",locate);

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
		    is y:HashTable do (
			 if y.mutable && y.hash>h then ( h = y.hash; e = x; );
			 )
		    else nothing;
		    );
	       e))
     else nullE);
setupfun("youngest", youngest);

toBigReal(e:Expr):Expr := (
     when e
     is x:Rational do Expr(toBigReal(x))
     is x:Integer do Expr(toBigReal(x))
     is x:Real do Expr(toBigReal(x.v))
     else WrongArg("a number")
     );
setupfun("toBigReal",toBigReal);

precision(e:Expr):Expr := (
     when e 
     is x:BigReal do Expr(toInteger(precision(x)))
     else WrongArg("a big real number")
     );
setupfun("precision",precision);

setprec(e:Expr):Expr := (
     when e
     is i:Integer do (
	  if isInt(i) then Expr(toInteger(setprec(toInt(i))))
	  else WrongArgSmallInteger())
     else WrongArgInteger());
setupfun("setPrecision",setprec);
