--		Copyright 1994-2000 by Daniel R. Grayson

newStartupMethod := true;				    -- for testing purposes

use C;
use system;
use actors;
use convertr;
use binding;
use actors2;
use actors3;
use actors4;
use actors5;
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
use objects;
use basic;
use struct;
use texmacs;

import dirname(s:string):string;

setGlobalVariable(x:Symbol,y:Expr):void := globalFrame.values.(x.frameindex) = y;
getGlobalVariable(x:Symbol):Expr := globalFrame.values.(x.frameindex);

currentFileName := setupvar("currentFileName", nullE);
currentFileDirectory := setupvar("currentFileDirectory", Expr("./"));
update(err:Error,prefix:string,f:Code):Expr := (
     if err.position == dummyPosition
     then printErrorMessage(f,prefix + ": " + err.message)
     else printErrorMessage(f,prefix + ": --backtrace-- ")
     );
stmtno := 0;
linefun(e:Expr):Expr := (
     when e
     is n:Integer do (
	  if isInt(n) then (
	       old := Expr(toInteger(stmtno));
	       nn := toInt(n);
	       if nn >= 0 then (
		    stmtno = nn;
	       	    old)
	       else WrongArg(1,"a non-negative integer"))
	  else WrongArgSmallInteger(1)
	  )
     is a:Sequence do (
	  if length(a) == 0
     	  then Expr(toInteger(stmtno))
     	  else WrongNumArgs(0))
     else WrongNumArgs(0)
     );
setupfun("lineNumber",linefun);
laststmtno := -1;
-- PrimaryPrompt := makeProtectedSymbolClosure("PrimaryPrompt");
-- SecondaryPrompt := makeProtectedSymbolClosure("SecondaryPrompt");
Print := makeProtectedSymbolClosure("Print");
NoPrint := makeProtectedSymbolClosure("NoPrint");
endInput := makeProtectedSymbolClosure("endInput");
(x:Position) === (y:Position) : bool := (
     x == y || x.filename === y.filename && x.line == y.line && x.column == y.column
     );
PrintOut(g:Expr,semi:bool,f:Code):Expr := (
     methodname := if semi then NoPrint else Print;
     method := lookup(Class(g),methodname);
     if method == nullE 
     then printErrorMessage(f,"no method for '" + methodname.symbol.word.name + "'")
     else apply(method,g)
     );
errorReportS := setupconst("report",Expr(emptySequence));
errorReportS.protected = false;
readeval4(file:TokenFile,printout:bool,AbortIfError:bool,dictionary:Dictionary):Expr := (
     returnvalue := nullE;
     lastvalue := nullE;
     while true do (
     	  if printout then stmtno = stmtno + 1;
	  interrupted = false;
	  interruptPending = false;
	  while peektoken(file,true).word == newlineW do (
	       -- laststmtno = -1; -- so there will be a new prompt after a blank line
	       -- but now we don't like so many extra prompts
	       interrupted = false;
	       interruptPending = false;
	       gettoken(file,true);
	       );
	  interrupted = false;
	  interruptPending = false;
	  parsed := parse(file,semicolonW.parse.precedence,true);
	  if equal(parsed,wordEOF) then break;
	  returnvalue = nullE;
	  if parsed == errorTree then (
	       if fileError(file) then return(buildErrorPacket(fileErrorMessage(file)));
	       if AbortIfError then return(buildErrorPacket("--backtrace--"));
	       )
	  else (
	       s := gettoken(file,true);  -- get the semicolon
	       if !(s.word == semicolonW || s.word == newlineW)
	       then (
		    printErrorMessage(s.position,"syntax error");
		    if AbortIfError 
		    then return(Expr(Error(s.position,"syntax error",emptySequence,nullE)));
		    )
	       else (
		    if localBind(parsed,dictionary) -- assign scopes to tokens, look up
		    then (		  
			 f := convert(parsed); -- convert to runnable code
			 lastvalue = eval(f);	  -- run it
			 if lastvalue == endInput then return(lastvalue);
			 when lastvalue is err:Error do (
			      if err.message == returnMessage
			      || err.message == breakMessage
			      then lastvalue = err.value;
			      )
			 else nothing;
			 when lastvalue is err:Error do (
			      setGlobalVariable(errorReportS, err.report);
			      if AbortIfError then return(lastvalue);
			      lastvalue = nullE;
			      )
			 else (
			      if s.word != semicolonW then returnvalue = lastvalue;
			      if printout then (
				   g := PrintOut(lastvalue,s.word == semicolonW,f);
				   when g is err:Error do (
					g = update(err,"at print",f);
					when g is err2:Error do (
					     setGlobalVariable(errorReportS, err2.report);
					     )
					else nothing;
					if AbortIfError then return(g);
					)
				   else nothing; ) ) )
		    else if isatty(file) 
		    then flush(file)
		    else return(buildErrorPacket("error while loading file")); ); );
	  lastvalue = nullE; );
     -- now we let filbuf handle all prompting:
     -- if isatty(file) then stdout << endl;
     returnvalue);
readeval3(file:TokenFile,printout:bool,AbortIfError:bool,dictionary:Dictionary):Expr := (
     savecf := getGlobalVariable(currentFileName);
      savecd := getGlobalVariable(currentFileDirectory);
       setGlobalVariable(currentFileName,Expr(file.posFile.file.filename));
       setGlobalVariable(currentFileDirectory,Expr(dirname(file.posFile.file.filename)));
       ret := readeval4(file,printout,AbortIfError,dictionary);
      setGlobalVariable(currentFileDirectory,savecd);
     setGlobalVariable(currentFileName,savecf);
     ret);
     
readeval2(file:TokenFile,printout:bool,AbortIfError:bool):Expr := (
     -- wrap a new dictionary around the file
     saveLocalFrame := localFrame;
     dictionary := newLocalDictionary();	  -- don't nest the scopes of files loaded; (dictionary.transient is set to false)
     localFrame = newLocalFrame(dictionary);
     ret := readeval3(file,printout,AbortIfError,dictionary);
     localFrame = saveLocalFrame;
     ret);
readeval(file:TokenFile):Expr := readeval2(file,false,true);
export StopIfError := false;
stopIfError(e:Expr):Expr := (
     ret := toExpr(StopIfError);
     if e == True then (StopIfError = true; ret)
     else if e == False then (StopIfError = false; ret)
     else WrongArg("true or false")
     );
setupfun("stopIfError",stopIfError);

InputPrompt := makeProtectedSymbolClosure("InputPrompt");
InputContinuationPrompt := makeProtectedSymbolClosure("InputContinuationPrompt");

topLevelPrompt():string := (
     method := lookup(integerClass,if stmtno == laststmtno then InputContinuationPrompt else (laststmtno = stmtno; InputPrompt));
     if method == nullE then ""
     else when apply(method,toExpr(stmtno)) is s:string do s
     is n:Integer do if isInt(n) then blanks(toInt(n)) else ""
     else "\n<--bad prompt--> : " -- unfortunately, we are not printing the error message!
     );

loadprint(s:string,StopIfError:bool):Expr := (
     when openTokenFile(s)
     is errmsg do False
     is file:TokenFile do (
	  if file.posFile.file != stdin then file.posFile.file.echo = true;
	  setprompt(file,topLevelPrompt);
     	  saveLocalFrame := localFrame;
	  d := newLocalDictionary();
     	  localFrame = newLocalFrame(d);
	  r := readeval3(file,true,StopIfError,d);
     	  localFrame = saveLocalFrame;
	  t := (
	       if s === "-"			 -- whether it's stdin
	       then (
		    file.posFile.file.eof = false; -- erase eof indication so we can try again (e.g., recursive calls to topLevel)
		    0
		    )
	       else close(file));
	  when r is Error do r 
	  else (
	       if t == ERROR
	       then buildErrorPacket("error closing file") 
	       else True)));
load(s:string):Expr := (
     when openTokenFile(s)
     is e:errmsg do buildErrorPacket(e.message)
     is file:TokenFile do (
	  r := readeval(file);
	  t := if !(s==="-") then close(file) else 0;
	  when r is Error do r
	  else (
	       if t == ERROR
	       then buildErrorPacket("error closing file") 
 	       else nullE)));

load(e:Expr):Expr := (
     when e
     is s:string do load(s)
     else buildErrorPacket("expected string as file name"));
setupfun("simpleLoad",load);

input(e:Expr):Expr := (
     when e
     is s:string do (
	  -- we should have a way of setting normal prompts while inputting
	  ret := loadprint(s,true);
	  laststmtno = -1;
	  ret)
     else buildErrorPacket("expected string as file name"));
setupfun("simpleInput",input);

stringTokenFile(name:string,contents:string):TokenFile := (
     TokenFile(
	  makePosFile(
	  file(nextHash(),     	    	  -- hash
	       name,	 		  -- filename
	       0,			  -- pid
	       false,	       	    	  -- error
	       "",     	    	      	  -- message
	       false,	       	    	  -- listener
	       NOFD,   	    	          -- listenerfd
	       NOFD,	      	   	  -- connection
	       0,     	   	     	  -- numconns
	       true,			  -- input
	       NOFD,			  -- infd
	       false,			  -- inisatty
	       contents,		  -- inbuffer
	       0,			  -- inindex
	       length(contents),	  -- insize
	       true,			  -- eof
	       false,	  		  -- promptq
	       noprompt,		  -- prompt
     	       true,	       	    	  -- bol
	       false,			  -- echo
	       false,			  -- output
	       NOFD,			  -- outfd
	       false,			  -- outisatty
	       "",			  -- outbuffer
	       0,			  -- outindex
	       0,     	   	     	  -- outbol
	       false,	       	    	  -- hadNet
	       dummyNetList,   	      	  -- nets
	       0		          -- bytesWritten
	       )),
	  NULL));

export topLevel():bool := when loadprint("-",StopIfError) is Error do false else true;
topLevel(e:Expr):Expr := toExpr(topLevel());
setupfun("topLevel",topLevel);

value(e:Expr):Expr := (
     when e
     is q:SymbolClosure do q.frame.values.(q.symbol.frameindex)
     is s:string do (
	  r := readeval(stringTokenFile("a string", s+newline));
	  when r 
	  is err:Error do (
	       if err.position == dummyPosition
	       || int(err.position.LoadDepth) < ErrorDepth
	       then r
	       else buildErrorPacket("--backtrace--"))
	  else r)
     else WrongArg(1,"a string or a symbol"));
setupfun("value",value);

export process():void := (
     laststmtno = -1;			  -- might have done dumpdata()
     localFrame = globalFrame;
     stdin .inisatty  =   0 != isatty(0) ;
     stdin.echo       = !(0 != isatty(0));
     stdout.outisatty =   0 != isatty(1) ;
     stderr.outisatty =   0 != isatty(2) ;
     StopIfError = false;				    -- this is usually true after loaddata()
     ret := (
	  when readeval(stringTokenFile("--startupString1--/layout.m2",startupString1))
	  is Error do (
	       if StopIfError
	       then 1					    -- probably can't happen, because layout.m2 doesn't set StopIfError
	       else (
		    if topLevel() 			    -- give a prompt for debugging
		    then 0 else 1))
	  else 
	  when readeval(stringTokenFile("--startupString2--/startup.m2",startupString2)) -- startup.m2 calls topLevel and eventually returns
	  is Error do (
	       if StopIfError 
	       then 1
	       else (
		    if topLevel() 			    -- give a prompt for debugging
		    then 0 else 1))
	  else 0);
     value(Expr("exit " + tostring(ret)));		    -- try to exit the user's way
     exit(if ret == 0 then 1 else ret);			    -- if that doesn't work, try harder and indicate an error
     );
