--		Copyright 1994 by Daniel R. Grayson
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

dummyMultaryFun(c:CodeSequence):Expr := (
     error("dummy multary function called");
     nullE);
dummyForFun(c:forCode):Expr := (
     error("dummy for function called");
     nullE);

export AdjacentFun := dummyBinaryFun;	-- filled in later
export AssignElemFun := dummyTernaryFun;	-- filled
export AssignQuotedElemFun := dummyTernaryFun;	-- filled
export TryElseFun := dummyBinaryFun; 	-- filled in later
export TryFun := dummyUnaryFun; 	-- filled in later
export IfThenFun := dummyBinaryFun;	-- filled in later
export IfThenElseFun := dummyTernaryFun;-- filled in later
export ForFun := dummyForFun;      -- filled in later
export WhileDoFun := dummyBinaryFun;      -- filled in later
export WhileListFun := dummyBinaryFun;      -- filled in later
export WhileListDoFun := dummyTernaryFun;      -- filled in later
export NewFun := dummyUnaryFun;	  -- filled in later
export NewFromFun := dummyBinaryFun;	  -- filled in later
export NewOfFun := dummyBinaryFun;	  -- filled in later
export NewOfFromFun := dummyTernaryFun;	  -- filled in later

export AssignNewFun := dummyBinaryFun;
export AssignNewOfFun := dummyTernaryFun;
export AssignNewFromFun := dummyTernaryFun;
export AssignNewOfFromFun := dummyMultaryFun;

export InstallMethodFun := dummyMultaryFun;
export UnaryInstallMethodFun := dummyTernaryFun;

export InstallValueFun := dummyMultaryFun;
export UnaryInstallValueFun := dummyTernaryFun;

export convert(e:ParseTree):Code;
CodeSequenceLength(e:ParseTree):int := (
     i := 0;
     while true do (
     	  when e
     	  is b:Binary do (
	       if b.operator.word == commaW
	       then ( i = i + CodeSequenceLength(b.lhs); e = b.rhs )
	       else return(i+1))
	  is u:Unary do (
	       if u.operator.word == commaW
	       then ( i = i + 1; e = u.rhs )
	       else return(i+1))
	  else return(i+1)));
fillCodeSequence(e:ParseTree,v:CodeSequence,m:int):int := (
     -- starts filling v at position m, returns the next available position
     while true do (
     	  when e
     	  is b:Binary do (
	       if b.operator.word == commaW
	       then ( m = fillCodeSequence(b.lhs,v,m); e = b.rhs )
	       else ( v.m = convert(e); return(m+1)))
	  is u:Unary do (
	       if u.operator.word == commaW
	       then ( 
		    v.m = Code(nullCode());
		    m = m + 1; 
		    e = u.rhs )
	       else ( v.m = convert(e); return(m+1)))
	  is p:EmptyParentheses do (
	       (v.m = convert(e); return(m+1)))
	  is dummy do (
	       v.m = Code(nullCode());
	       return(m+1);
	       )
	  is p:Parentheses do (
	       ( v.m = convert(e); return(m+1)))
	  else ( v.m = convert(e); return(m+1))));
makeCodeSequence(e:ParseTree):CodeSequence := (
     v := new CodeSequence len CodeSequenceLength(e) do provide dummyCode;
     fillCodeSequence(e,v,0);
     v);
SymbolSequenceLength(e:ParseTree):int := (
     i := 0;
     while true do (
     	  when e
	  is p:Parentheses do e = p.contents
     	  is b:Binary do (
	       i = i+1;
	       e = b.lhs;
	       )
	  else (					    -- should be the first token
	       i = i+1;
	       return(i);
	       )
	  )
     );
makeSymbolSequence(e:ParseTree):SymbolSequence := (	    -- but replace local symbols by dummySymbol
     m := SymbolSequenceLength(e);
     v := new SymbolSequence len m do provide dummySymbol;
     while true do (
     	  when e
	  is p:Parentheses do e = p.contents
     	  is b:Binary do (
	       when b.rhs is t:Token do (
		    m = m-1;
		    v.m = t.entry;
		    )
	       else nothing;				    -- shouldn't happen
	       e = b.lhs;
	       )
	  is t:Token do (
	       m = m-1;
	       v.m = t.entry;
	       break;
	       )
	  else break;					    -- shouldn't happen
	  );
     v);

nestingDepth(frameID:int,d:Dictionary):int := (
     if frameID == 0 then return(-1);
     n := 0;
     while d.frameID != frameID do (
	  if !d.transient || d.framesize != 0 then n = n+1; -- empty transient frames will not appear at runtime
	  if d == d.outerDictionary then (
	       error("internal error during conversion: frameID " + tostring(frameID) + " not found");
	       break;
	       );
	  d = d.outerDictionary;
	  );
     n);

tokenAssignment(e:ParseTree,b:Binary,t:Token):Code := (
     if t.entry.frameID == 0
     then Code(globalAssignmentCode(t.entry,convert(b.rhs),treePosition(e)))
     else Code(localAssignmentCode(nestingDepth(t.entry.frameID,t.dictionary),t.entry.frameindex,convert(b.rhs),treePosition(e)))
     );

parallelAssignment(e:ParseTree,b:Binary,p:Parentheses):Code := (
     symbols := makeSymbolSequence(b.lhs);
     n := length(symbols);
     nd := new array(int) len n do foreach x in symbols do provide nestingDepth(x.frameID,b.operator.dictionary);
     fr := new array(int) len n do foreach x in symbols do provide x.frameindex;
     foreach x in symbols do if x.frameID != 0 then x = dummySymbol;
     Code(parallelAssignmentCode(
	       nd,
	       fr,
	       symbols,
	       convert(b.rhs),
	       treePosition(e)
	       ))
     );

export convert(e:ParseTree):Code := (
     when e
     is s:StartDictionary do (
	  if s.dictionary.framesize != 0
	  then Code(newLocalFrameCode(s.dictionary.frameID,s.dictionary.framesize,convert(s.body)))
	  else convert(s.body)
	  )
     is w:For do Code(
	  forCode(
	       convert(w.fromClause), convert(w.toClause),
	       convert(w.whenClause), convert(w.listClause), 
	       convert(w.doClause),
	       w.dictionary.frameID,
	       w.dictionary.framesize,
	       treePosition(e)))
     is w:WhileDo do Code(
	  binaryCode(WhileDoFun,convert(w.predicate),convert(w.doClause),
	       treePosition(e)))
     is w:WhileList do Code(
	  binaryCode(WhileListFun,convert(w.predicate),convert(w.listClause),
	       treePosition(e)))
     is w:WhileListDo do Code(
	  ternaryCode(WhileListDoFun,convert(w.predicate),convert(w.listClause),convert(w.doClause),
	       treePosition(e)))
     is n:New do (
	  if n.newparent == dummyTree
	  then if n.newinitializer == dummyTree
	       then Code(unaryCode(NewFun,convert(n.newclass),
		    treePosition(e)))
	       else Code(binaryCode(NewFromFun,convert(n.newclass),convert(n.newinitializer),
		    treePosition(e)))
	  else if n.newinitializer == dummyTree
	       then Code(binaryCode(NewOfFun,convert(n.newclass),convert(n.newparent),
		    treePosition(e)))
	       else Code(ternaryCode(NewOfFromFun,
		    convert(n.newclass),convert(n.newparent),convert(n.newinitializer),
		    treePosition(e))))
     is i:IfThen do Code(
	  binaryCode(IfThenFun,
	       convert(i.predicate),convert(i.thenclause),
	       treePosition(e)))
     is i:IfThenElse do Code(
	  ternaryCode(IfThenElseFun,
	       convert(i.predicate),
	       convert(i.thenclause),convert(i.elseClause),
	       treePosition(e)))
     is token:Token do (
	  var := token.entry;
	  wrd := token.word;
	  pos := token.position;
	  if wrd.typecode == TCdouble
	  then Code(realCode(parseDouble(wrd.name),pos))
	  else if wrd.typecode == TCint
	  then Code(integerCode(parseInt(wrd.name),pos))
 	  else if wrd.typecode == TCstring
	  then (
	       s := parseString(wrd.name);
	       Code(stringCode(s, hash(s), pos))
	       )
	  else (
	       if var.frameID == 0
	       then Code(globalMemoryReferenceCode(var.frameindex,pos))
	       else Code(localMemoryReferenceCode(nestingDepth(var.frameID,token.dictionary),var.frameindex,pos))
	       )
	  )
     is a:Adjacent do Code(
	  binaryCode(AdjacentFun, convert(a.lhs),convert(a.rhs),
	       treePosition(e)))
     is p:EmptyParentheses do (
	  if p.left.word == leftparen then Code(sequenceCode(CodeSequence(),treePosition(e)))
	  else if p.left.word == leftbrace then Code(listCode(CodeSequence(),treePosition(e)))
	  else if p.left.word == leftbracket then Code(arrayCode(CodeSequence(),treePosition(e)))
	  else dummyCode			  -- should not happen
	  )
     is p:Parentheses do (
	  if p.left.word == leftparen then convert(p.contents)
	  else if p.left.word == leftbrace 
	  then Code(listCode(makeCodeSequence(p.contents),treePosition(e)))
	  else 
	  if p.left.word == leftbracket 
	  then Code(arrayCode(makeCodeSequence(p.contents),treePosition(e)))
	  else 
	  dummyCode			  -- should not happen
	  )
     is b:Binary do (
	  if b.operator.entry == DotS.symbol
	  || b.operator.entry == DotQuestionS.symbol
	  then (
	       when b.rhs
	       is token:Token do (
	  	    wrd := token.word;
		    var := token.entry;
		    if wrd.typecode == TCid
		    then (
	       		 Code(binaryCode(
			 	   b.operator.entry.binary,
			 	   convert(b.lhs),
	       	    	 	   Code(globalSymbolClosureCode(var,treePosition(b.rhs))),
			 	   treePosition(e)
				   )
			      )
			 )
		    else dummyCode	  -- should not occur
		    )
	       else dummyCode		  -- should not occur
	       )
	  else if b.operator.word == commaW
	  then Code(sequenceCode(makeCodeSequence(e),treePosition(e)))
	  else if b.operator.word == EqualW
	  then (
	       when b.lhs
	       is a:Adjacent do (
		    Code(
			 multaryCode(
			      InstallValueFun,
			      CodeSequence(
			      	   Code(globalSymbolClosureCode(AdjacentS.symbol,dummyPosition)),
			      	   convert(a.lhs),
			      	   convert(a.rhs),
			      	   convert(b.rhs)),
			      treePosition(e))))
	       is u:Unary do Code(
		    ternaryCode(
			 UnaryInstallValueFun,
			 Code(globalSymbolClosureCode(u.operator.entry,u.operator.position)),
			 convert(u.rhs),
			 convert(b.rhs),
			 treePosition(e)))
	       is u:Postfix do Code(
		    ternaryCode(
			 UnaryInstallValueFun,
			 Code(globalSymbolClosureCode(u.operator.entry,u.operator.position)),
			 convert(u.lhs),
			 convert(b.rhs),
			 treePosition(e)))
	       is c:Binary do (
		    if c.operator.entry == SharpS.symbol
		    then Code(ternaryCode( AssignElemFun, convert(c.lhs),
			      convert(c.rhs), convert(b.rhs), treePosition(e)))
		    else if c.operator.entry == DotS.symbol
		    then (
			 when c.rhs
			 is crhs:Token do
			 Code(ternaryCode(
				   AssignElemFun,
				   convert(c.lhs),
			 	   Code(globalSymbolClosureCode(crhs.entry,crhs.position)),
				   convert(b.rhs),
				   treePosition(e)))
			 else dummyCode --should not happen
			 )
		    else Code(multaryCode(
			      InstallValueFun,
			      CodeSequence(
				   Code(globalSymbolClosureCode(c.operator.entry,c.operator.position)), 
				   convert(c.lhs),
				   convert(c.rhs),
				   convert(b.rhs)),
			      treePosition(e))))
	       is t:Token do tokenAssignment(e,b,t)
	       is p:Parentheses do parallelAssignment(e,b,p)
	       else dummyCode		  -- should not happen
	       )
	  else if b.operator.word == ColonEqualW
	  then (
	       when b.lhs
	       is n:New do (
		    if n.newparent == dummyTree 
		    then if n.newinitializer == dummyTree 
		    then Code(binaryCode(
			      AssignNewFun,
			      convert(n.newclass),
			      convert(b.rhs), 
			      treePosition(e)))
		    else Code(ternaryCode(
			      AssignNewFromFun,
			      convert(n.newclass),
			      convert(n.newinitializer),
			      convert(b.rhs),
			      treePosition(e)))
     	       	    else if n.newinitializer == dummyTree 
		    then Code(ternaryCode(
			      AssignNewOfFun,
			      convert(n.newclass),
			      convert(n.newparent),
			      convert(b.rhs),
			      treePosition(e)))
		    else Code(multaryCode(
			      AssignNewOfFromFun,
			      CodeSequence(
				   convert(n.newclass),
				   convert(n.newparent),
				   convert(n.newinitializer),
				   convert(b.rhs)),
			      treePosition(e))))
	       is a:Adjacent do (
		    Code(multaryCode(
			      InstallMethodFun,
			      CodeSequence(
			      	   Code(globalSymbolClosureCode(AdjacentS.symbol,dummyPosition)),
			      	   convert(a.lhs),
			      	   convert(a.rhs),
			      	   convert(b.rhs)),
			      treePosition(e))))
	       is u:Unary do Code(ternaryCode(
			 UnaryInstallMethodFun,
			 Code(globalSymbolClosureCode(u.operator.entry,u.operator.position)),
			 convert(u.rhs), convert(b.rhs), treePosition(e)))
	       is u:Postfix do Code(ternaryCode(
			 UnaryInstallMethodFun,
			 Code(globalSymbolClosureCode(u.operator.entry,u.operator.position)),
			 convert(u.lhs), convert(b.rhs), treePosition(e)))
	       is c:Binary do (
		    if c.operator.entry == SharpS.symbol
		    then Code(ternaryCode( AssignElemFun, convert(c.lhs),
			      convert(c.rhs), convert(b.rhs), treePosition(e)))
		    else if c.operator.entry == UnderscoreS.symbol
		    then Code(multaryCode(
			      InstallMethodFun,
			      CodeSequence( 
			      	   Code(globalSymbolClosureCode(UnderscoreS.symbol,dummyPosition)),
				   convert(c.lhs),
				   convert(c.rhs),
			      	   convert(b.rhs)),
			      treePosition(e)))
		    else if c.operator.entry == DotS.symbol
		    then (
			 when c.rhs
			 is crhs:Token do
			 Code(ternaryCode(
				   AssignElemFun,
				   convert(c.lhs),
			 	   Code(globalSymbolClosureCode(crhs.entry,crhs.position)),
				   convert(b.rhs),
				   treePosition(e)))
			 else dummyCode --should not happen
			 )
		    else Code(multaryCode(
			      InstallMethodFun,
			      CodeSequence(
			 	   Code(globalSymbolClosureCode(c.operator.entry,c.operator.position)),
				   convert(c.lhs),
				   convert(c.rhs),
				   convert(b.rhs)),
			      treePosition(e))))
	       is t:Token do tokenAssignment(e,b,t)
	       is p:Parentheses do parallelAssignment(e,b,p)
	       else dummyCode		  -- should not happen
	       )
	  else Code(binaryCode(b.operator.entry.binary,convert(b.lhs),
	       	    convert(b.rhs),treePosition(e)))
	  )
     is a:Arrow do Code(functionCode(
	       convert(a.lhs),		  -- just for display purposes!
	       convert(a.rhs),a.desc
	       ))
     is u:Unary do (
	  if u.operator.word == commaW
	  then Code(sequenceCode(makeCodeSequence(e),treePosition(e)))
	  else Code(unaryCode(u.operator.entry.unary,convert(u.rhs),treePosition(e))))
     is q:Quote do (
	  token := q.rhs;
	  sym := token.entry;
	  pos := treePosition(e);
	  if sym.frameID == 0
	  then Code(globalSymbolClosureCode(sym,pos))
	  else Code(localSymbolClosureCode(nestingDepth(sym.frameID,token.dictionary),sym,pos)))
     is q:GlobalQuote do (
	  token := q.rhs;
	  sym := token.entry;
	  pos := treePosition(e);
     	  Code(globalSymbolClosureCode(sym,pos)))
     is q:LocalQuote do (
	  token := q.rhs;
	  sym := token.entry;
	  pos := treePosition(e);
	  nd := nestingDepth(sym.frameID,token.dictionary);
	  Code(localSymbolClosureCode(nd,sym,pos)))
     is i:TryElse do Code(
	  binaryCode(TryElseFun,
	       convert(i.primary),convert(i.alternate),
	       treePosition(e)))
     is i:Try do Code(
	  unaryCode(TryFun,
	       convert(i.primary),
	       treePosition(e)))
     is u:Postfix do Code(
	  unaryCode(u.operator.entry.postfix,convert(u.lhs),treePosition(e)))
     is d:dummy do dummyCode
     );

export tostring(c:Code):string := (
     when c
     is x:arrayCode do concatenate(array(string)( "(array ", between(" ",new array(string) len length(x.z) do foreach s in x.z do provide tostring(s)), ")"))
     is x:binaryCode do concatenate(array(string)("(2-OP ",tostring(x.lhs)," ",tostring(x.rhs),")"))
     is x:forCode do concatenate(array(string)(
	       "(for from: ",tostring(x.fromClause),
	       " to: ",tostring(x.toClause),
	       " when: ",tostring(x.whenClause),
	       " list: ",tostring(x.listClause),
	       " do: ",tostring(x.doClause),
	       ")"))
     is x:functionCode do concatenate(array(string)(
	       "(function restargs: ",tostring(x.desc.restargs),
	       " numparms: ",tostring(x.desc.numparms),
	       " framesize: ",tostring(x.desc.framesize),
	       " frameID: ",tostring(x.desc.frameID),
	       " ",tostring(x.body),")"))
     is x:globalAssignmentCode do concatenate(array(string)("(= ",x.lhs.word.name," ",tostring(x.rhs),")"))
     is x:globalMemoryReferenceCode do concatenate(array(string)("(fetch ",tostring(x.frameindex),")"))
     is x:globalSymbolClosureCode  do x.symbol.word.name
     is x:integerCode do tostring(x.x)
     is x:listCode do concatenate(array(string)( "(list ", between(" ",new array(string) len length(x.y) do foreach s in x.y do provide tostring(s)), ")"))
     is x:localAssignmentCode do concatenate(array(string)("(store ",tostring(x.frameindex)," ",tostring(x.nestingDepth)," ",tostring(x.rhs),")"))
     is x:localMemoryReferenceCode do concatenate(array(string)("(fetch ",tostring(x.frameindex)," ",tostring(x.nestingDepth),")"))
     is x:localSymbolClosureCode do concatenate(array(string)("(local ",x.symbol.word.name," nestingDepth: ",tostring(x.nestingDepth),")"))
     is x:multaryCode do concatenate(array(string)( "(OP ", between(" ",new array(string) len length(x.args) do foreach c in x.args do provide tostring(c)), ")" ))
     is x:newLocalFrameCode do concatenate(array(string)())
     is x:nullCode do "NULL"
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
			 else provide x.lhs.i.word.name),
		    ") ", tostring(x.rhs), ")" ) ) )
     is x:realCode do tostring(x.x)
     is x:sequenceCode do (
	  concatenate(array(string)(
		    "(sequence ",
		    between(" ",new array(string) len length(x.x) do foreach s in x.x do provide tostring(s)),
     	       	    ")")))
     is x:stringCode do concatenate(array(string)("\"",present(x.x),"\""))
     is x:ternaryCode do concatenate(array(string)("(3-OP ",tostring(x.arg1)," ",tostring(x.arg2)," ",tostring(x.arg3),")"))
     is x:unaryCode do concatenate(array(string)("(1-OP ",tostring(x.rhs),")"))
     );
