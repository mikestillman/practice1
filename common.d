--		Copyright 1994-2003 by Daniel R. Grayson

use C;
use system;
use binding;
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
use basic;

export codePosition(c:Code):Position := (
     when c
     is f:binaryCode do f.position
     is f:adjacentCode do f.position
     is f:forCode do f.position
     is f:ifCode do f.position
     is f:newOfFromCode do f.position
     is f:newFromCode do f.position
     is f:newOfCode do f.position
     is f:newCode do f.position
     is f:whileListDoCode do f.position
     is f:whileListCode do f.position
     is f:whileDoCode do f.position
     is f:tryCode do f.position
     is f:functionCode do codePosition(f.parms)
     is f:globalAssignmentCode do f.position
     is f:globalMemoryReferenceCode do f.position
     is f:globalSymbolClosureCode do f.position
     is f:integerCode do f.position
     is f:localAssignmentCode do f.position
     is f:localMemoryReferenceCode do f.position
     is f:localSymbolClosureCode do f.position
     is f:multaryCode do f.position
     is f:nullCode do dummyPosition
     is f:newLocalFrameCode do codePosition(f.body)
     is f:parallelAssignmentCode do f.position
     is f:realCode do f.position
     is f:sequenceCode do f.position
     is f:listCode do f.position
     is f:arrayCode do f.position
     is f:stringCode do dummyPosition
     is f:ternaryCode do f.position
     is f:unaryCode do f.position
     );

export tostring(c:Code):string := (
     when c
     is x:arrayCode do concatenate(array(string)( "(array ", between(" ",new array(string) len length(x.z) do foreach s in x.z do provide tostring(s)), ")"))
     is x:binaryCode do concatenate(array(string)("(2-OP ",tostring(x.lhs)," ",tostring(x.rhs),")"))
     is x:adjacentCode do concatenate(array(string)("(adjacent ",tostring(x.lhs)," ",tostring(x.rhs),")"))
     is x:forCode do concatenate(array(string)(
	       "(for from: ",tostring(x.fromClause),
	       " to: ",tostring(x.toClause),
	       " when: ",tostring(x.whenClause),
	       " list: ",tostring(x.listClause),
	       " do: ",tostring(x.doClause),
	       ")"))
     is x:whileListDoCode do concatenate(array(string)( "(while ",tostring(x.predicate), " list: ",tostring(x.listClause), " do: ",tostring(x.doClause), ")"))
     is x:newOfFromCode do concatenate(array(string)( "(new ",tostring(x.newClause), " of: ",tostring(x.ofClause), " from: ",tostring(x.fromClause), ")"))
     is x:newFromCode do concatenate(array(string)( "(new ",tostring(x.newClause), " from: ",tostring(x.fromClause), ")"))
     is x:newOfCode do concatenate(array(string)( "(new ",tostring(x.newClause), " of: ",tostring(x.ofClause), ")"))
     is x:newCode do concatenate(array(string)( "(new ",tostring(x.newClause), ")"))
     is x:whileDoCode do concatenate(array(string)( "(while ",tostring(x.predicate), " do: ",tostring(x.doClause), ")"))
     is x:whileListCode do concatenate(array(string)( "(while ",tostring(x.predicate), " list: ",tostring(x.listClause), ")"))
     is x:functionCode do concatenate(array(string)(
	       "(function restargs: ",tostring(x.desc.restargs),
	       " numparms: ",tostring(x.desc.numparms),
	       " framesize: ",tostring(x.desc.framesize),
	       " frameID: ",tostring(x.desc.frameID),
	       " ",tostring(x.body),")"))
     is x:globalAssignmentCode do concatenate(array(string)("(= ",x.lhs.word.name," ",tostring(x.rhs),")"))
     is x:globalMemoryReferenceCode do concatenate(array(string)("(fetch ",tostring(x.frameindex),")"))
     is x:globalSymbolClosureCode  do join("'",x.symbol.word.name)
     is x:integerCode do tostring(x.x)
     is x:listCode do concatenate(array(string)( "(list ", between(" ",new array(string) len length(x.y) do foreach s in x.y do provide tostring(s)), ")"))
     is x:localAssignmentCode do concatenate(array(string)("(store ",tostring(x.frameindex)," ",tostring(x.nestingDepth)," ",tostring(x.rhs),")"))
     is x:localMemoryReferenceCode do concatenate(array(string)("(fetch ",tostring(x.frameindex)," ",tostring(x.nestingDepth),")"))
     is x:localSymbolClosureCode do concatenate(array(string)("(local ",x.symbol.word.name," nestingDepth: ",tostring(x.nestingDepth),")"))
     is x:multaryCode do concatenate(array(string)( "(OP ", between(" ",new array(string) len length(x.args) do foreach c in x.args do provide tostring(c)), ")" ))
     is x:newLocalFrameCode do concatenate(array(string)())
     is x:nullCode do "(null)"
     is x:parallelAssignmentCode do (
	  n := length(x.nestingDepth);
	  concatenate(
	       array(string)(
	       	    "(parallel= (",
		    between(" ",
			 new array(string) len length(x.nestingDepth) do 
			 for i from 0 to n-1 do 
			 if x.lhs.i == dummySymbol 
			 then provide concatenate(array(string)("(",tostring(x.frameindex.i)," ",tostring(x.nestingDepth.i),")"))
			 else provide join("'",x.lhs.i.word.name)),
		    ") ", tostring(x.rhs), ")" ) ) )
     is x:realCode do tostring(x.x)
     is x:sequenceCode do (
	  concatenate(array(string)(
		    "(sequence ",
		    between(" ",new array(string) len length(x.x) do foreach s in x.x do provide tostring(s)),
     	       	    ")")))
     is x:stringCode do concatenate(array(string)("\"",present(x.x),"\""))
     is x:ternaryCode do concatenate(array(string)("(3-OP ",tostring(x.arg1)," ",tostring(x.arg2)," ",tostring(x.arg3),")"))
     is x:ifCode do concatenate(array(string)("(if ",tostring(x.predicate)," then: ",tostring(x.thenClause)," else: ",tostring(x.elseClause),")"))
     is x:tryCode do concatenate(array(string)("(try ",tostring(x.code)," ",tostring(x.elseClause),")"))
     is x:unaryCode do concatenate(array(string)("(1-OP ",tostring(x.rhs),")"))
     );

export setup(word:Word):void := (
     makeSymbol(word,dummyPosition,globalDictionary);
     );
export setup(word:Word,fn:unop):void := (
     e := makeSymbol(word,dummyPosition,globalDictionary);
     e.unary = fn;
     );
export setup(word:Word,fn:binop):void := (
     e := makeSymbol(word,dummyPosition,globalDictionary);
     e.binary = fn;
     );
export setup(word:Word,fun1:unop,fun2:binop):void := (
     e := makeSymbol(word,dummyPosition,globalDictionary);
     e.unary = fun1;
     e.binary = fun2;
     );
export setup(word:Word,fun1:unop,fun2:unop):void := (
     e := makeSymbol(word,dummyPosition,globalDictionary);
     e.unary = fun1;
     e.postfix = fun2;
     );
export setup(e:SymbolClosure,fn:unop):void := (
     e.symbol.unary = fn;
     );
export setuppostfix(e:SymbolClosure,fn:unop):void := (
     e.symbol.postfix = fn;
     );
export setup(e:SymbolClosure,fn:binop):void := (
     e.symbol.binary = fn;
     );
export setup(e:SymbolClosure,fun1:unop,fun2:binop):void := (
     e.symbol.unary = fun1;
     e.symbol.binary = fun2;
     );
export setup(e:SymbolClosure,fun1:unop,fun2:unop):void := (
     e.symbol.unary = fun1;
     e.symbol.postfix = fun2;
     );
export setupop(s:SymbolClosure,fun:unop):void := s.symbol.unary = fun;
export setupfun(name:string,fun:unop):Symbol := (
     word := makeUniqueWord(name,
	  parseinfo(precSpace,precSpace,precSpace,parsefuns(unaryop, defaultbinary)));
     entry := makeSymbol(word,dummyPosition,globalDictionary);
     entry.unary = fun;
     entry.protected = true;
     entry);     
export setupfun(name:string,value:fun):Symbol := (
     word := makeUniqueWord(name,parseWORD);
     entry := makeSymbol(word,dummyPosition,globalDictionary);
     globalFrame.values.(entry.frameindex) = Expr(CompiledFunction(value,nextHash()));
     entry.protected = true;
     entry);
export setupvar(name:string,value:Expr):Symbol := (
     word := makeUniqueWord(name,parseWORD);
     when lookup(word,globalDictionary)
     is null do (
     	  entry := makeSymbol(word,dummyPosition,globalDictionary);
     	  globalFrame.values.(entry.frameindex) = value;
	  entry)
     is entry:Symbol do (
	  -- we are doing it again after loading data with loaddata()
	  -- or we are reassigning to o or oo in interpret.d
     	  globalFrame.values.(entry.frameindex) = value;
	  entry));
export setupconst(name:string,value:Expr):Symbol := (
     s := setupvar(name,value);
     s.protected = true;
     s);
setup(commaW,dummyBinaryFun);

export quoteit(name:string):string := "'" + name + "'";
export NotYet(desc:string):Expr := buildErrorPacket(desc + " not implemented yet");
export WrongArg(desc:string):Expr := buildErrorPacket("expected " + desc);
export WrongArg(n:int,desc:string):Expr := (
     buildErrorPacket("expected argument " + tostring(n) + " to be " + desc));
export WrongArgInteger():Expr := WrongArg("an integer");
export WrongArgInteger(n:int):Expr := WrongArg(n,"an integer");
export WrongArgSmallInteger():Expr := WrongArg("a small integer");
export WrongArgSmallInteger(n:int):Expr := WrongArg(n,"a small integer");
export WrongArgString():Expr := WrongArg("a string");
export WrongArgString(n:int):Expr := WrongArg(n,"a string");
export WrongArgBoolean():Expr := WrongArg("true or false");
export WrongArgBoolean(n:int):Expr := WrongArg(n,"true or false");
export ArgChanged(name:string,n:int):Expr := (
     buildErrorPacket(quoteit(name) + " expected argument " + tostring(n)
	  + " not to change its type during execution"));
export WrongNumArgs(name:string,n:int):Expr := (
     if n == 0
     then buildErrorPacket(quoteit(name) + " expected no arguments")
     else if n == 1
     then buildErrorPacket(quoteit(name) + " expected " + tostring(n) + " argument")
     else buildErrorPacket(quoteit(name) + " expected " + tostring(n) + " arguments")
     );
export WrongNumArgs(n:int):Expr := buildErrorPacket(
     if n == 0 then "expected no arguments"
     else if n == 1 then "expected " + tostring(n) + " argument"
     else "expected " + tostring(n) + " arguments"
     );
export WrongNumArgs(name:string,m:int,n:int):Expr := (
     if n == m+1
     then buildErrorPacket(quoteit(name) + " expected " 
	  + tostring(m) + " or "
	  + tostring(n) + " arguments")
     else buildErrorPacket(quoteit(name) + " expected " 
	  + tostring(m) + " to "
	  + tostring(n) + " arguments"));
export WrongNumArgs(m:int,n:int):Expr := (
     if n == m+1
     then buildErrorPacket("expected " + tostring(m) + " or " + tostring(n) + " arguments")
     else buildErrorPacket("expected " + tostring(m) + " to " + tostring(n) + " arguments"));
export TooFewArgs(name:string,m:int):Expr := (
     if m == 1
     then buildErrorPacket(quoteit(name) + " expected at least 1 argument")
     else buildErrorPacket(quoteit(name) + " expected at least " 
	  + tostring(m) + " arguments"));
export TooManyArgs(name:string,m:int):Expr := (
     if m == 1
     then buildErrorPacket(quoteit(name) + " expected at most 1 argument")
     else buildErrorPacket(quoteit(name) + " expected at most " 
	  + tostring(m) + " arguments"));
export errorDepth := 0;
export printErrorMessage(c:Code,message:string):Expr := (
     p := codePosition(c);
     if int(p.loadDepth) >= errorDepth
     then Expr(printError(Error(p,message,CodeClosureList(CodeClosure(noRecycle(localFrame),c),self),nullE,false)))
     else buildErrorPacket(message));

export returnFromFunction(z:Expr):Expr := when z is err:Error do if err.message == returnMessage then err.value else z else z;
export returnFromLoop(z:Expr):Expr     := when z is err:Error do if err.message == breakMessage  then err.value else z else z;

export WrongNumArgs(c:Code,wanted:int,got:int):Expr := (
     printErrorMessage(c, "expected " + tostring(wanted) + " argument"
	  + (if wanted == 1 then "" else "s") + ", but got "
	  + tostring(got)));

export MissingMethod(name:string,method:string):Expr := buildErrorPacket(quoteit(name) + " expected item to have a method for " + method);
export MissingMethod(method:SymbolClosure):Expr := buildErrorPacket("expected a method for "+quoteit(method.symbol.word.name));
export MissingMethodPair(method:string):Expr := buildErrorPacket("expected pair to have a method for "+quoteit(method));
export MissingMethodPair(method:SymbolClosure):Expr := buildErrorPacket("expected pair to have a method for " + quoteit(method.symbol.word.name));
export MissingMethodPair(method:SymbolClosure,left:Expr,right:Expr):Expr := buildErrorPacket( "expected pair to have a method for " + quoteit(method.symbol.word.name) );

-----------------------------------------------------------------------------
-- Database stuff
export dbmcheck(ret:int):Expr := (
     if ret == -1 then buildErrorPacket(dbmstrerror())
     else Expr(toInteger(ret)));
export dbmopenin(filename:string):Expr := (
     mutable := false;
     handle := dbmopen(filename,mutable);
     if handle == -1 
     then buildErrorPacket(dbmstrerror() + " : " + filename)
     else Expr(Database(filename,nextHash(),handle,true,mutable)));
export dbmopenout(filename:string):Expr := (
     mutable := true;
     handle := dbmopen(filename,mutable);
     if handle == -1 
     then buildErrorPacket(dbmstrerror() + " : " + filename)
     else Expr(Database(filename,nextHash(),handle,true,mutable)));
export dbmclose(f:Database):Expr := (
     if !f.isopen then return buildErrorPacket("database already closed");
     dbmclose(f.handle);
     f.isopen = false;
     Expr(toInteger(0)));
export dbmstore(f:Database,key:string,content:string):Expr := (
     if !f.isopen then return buildErrorPacket("database closed");
     if !f.mutable then return buildErrorPacket("database not mutable");
     ret := dbmstore(f.handle,key,content);
     if 0 == ret then Expr(content)
     else dbmcheck(ret));
export dbmquery(f:Database,key:string):Expr := (
     if !f.isopen then return buildErrorPacket("database closed");
     when dbmfetch(f.handle,key)
     is a:string do True
     else False);
export dbmfirst(f:Database):Expr := (
     if !f.isopen then return buildErrorPacket("database closed");
     when dbmfirst(f.handle)
     is a:string do Expr(a)
     else nullE);
export dbmreorganize(f:Database):Expr := (
     if !f.isopen then return buildErrorPacket("database closed");
     if !f.mutable then return buildErrorPacket("database not mutable");
     dbmcheck(dbmreorganize(f.handle)));


-- Local Variables:
-- compile-command: "make -C $M2BUILDDIR/Macaulay2/d "
-- End:
