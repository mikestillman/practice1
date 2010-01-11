--		Copyright 2000 by Daniel R. Grayson

use C;
use system;
use strings;
use stdio;
use varstrin;
use ctype;
use getline;
use tokens;
use objects;
use binding;
use convertr;
use evaluate;
use common;
use struct;

TeXmacsEvaluate := makeProtectedSymbolClosure("TeXmacsEvaluate");

export topLevelTeXmacs(localInterpState:threadLocalInterp):int := (
     unsetprompt(stdin);
     while true do (
	  stdin.bol = false;				    -- sigh, prevent a possible prompt
     	  stdin.echo = false;
	  r := getLine(stdin);
	  when r is e:errmsg do (
	       flush(stdout);
     	       endLine(stderr);
	       stderr << "can't get line : " << e.message << endl << Flush;
	       exit(1);
	       )
	  is item:string do (
	       if stdin.eof then return 0;
	       method := lookup(stringClass,TeXmacsEvaluate);
	       if method == nullE 
	       then (
		    flush(stdout);
     		    endLine(stderr);
		    stderr << "no method for TeXmacsEvaluate" << endl;
		    )
	       else (
		    applyEE(localInterpState,method,Expr(item));
		    )
	       )));
topLevelTeXmacs(localInterpState:threadLocalInterp,e:Expr):Expr := toExpr(topLevelTeXmacs(localInterpState));
setupfun("topLevelTeXmacs",topLevelTeXmacs);

-- Local Variables:
-- compile-command: "make -C $M2BUILDDIR/Macaulay2/d "
-- End:
