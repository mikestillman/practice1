------------------------------------------------------------
-- Code to test, at a low level, the new engine code
------------------------------------------------------------
clone = method()
targ = method()
src = method()
symm = method()
exterior = method()
submod = method()
inducedOrder = method()
degreeWeights = method()
isGraded = method()
getTerms = method()
exteriorProduct = method()
modtensor = method()

if ECoefficientRing === quote ECoefficientRing then
  ECoefficientRing = new Type of MutableHashTable
if MonOrder === quote MonOrder then
  MonOrder = new Type of MutableHashTable
if EMonoid === quote EMonoid then
  EMonoid = new Type of MutableHashTable
if ERing === quote ERing then
  ERing = new Type of MutableHashTable
if EFreeModule === quote EFreeModule then
  EFreeModule = new Type of MutableHashTable
if EVector === quote EVector then
  EVector = new Type of MutableHashTable
if EMatrix === quote EMatrix then
  EMatrix = new Type of MutableHashTable
if ERingMap === quote ERingMap then
  ERingMap = new Type of MutableHashTable

if ERingElement === quote ERingElement then
  ERingElement = new Type of EVector

------------------------------------------------------------
BeforePrint ECoefficientRing := (R) -> sendgg(ggPush R, ggsee)
BeforePrint MonOrder := (R) -> sendgg(ggPush R, ggsee)
BeforePrint EMonoid := (R) -> sendgg(ggPush R, ggsee)
BeforePrint ERing := (R) -> sendgg(ggPush R, ggsee)
BeforePrint EFreeModule := (R) -> sendgg(ggPush R, ggsee)
BeforePrint ERingElement := (R) -> sendgg(ggPush R, ggsee)
BeforePrint EVector := (R) -> sendgg(ggPush R, ggsee)
BeforePrint EMatrix := (R) -> sendgg(ggPush R, ggsee)
BeforePrint ERingMap := (R) -> sendgg(ggPush R, ggsee)
------------------------------------------------------------
newECoefficientRing = () -> (
     v := new ECoefficientRing;
     v.handle = newHandle();
     v)

newMonOrder = () -> (
     v := new MonOrder;
     v.handle = newHandle();
     v)
     
newEMonoid = () -> (
     v := new EMonoid;
     v.handle = newHandle();
     v)

newERing = () -> (
     v := new ERing;
     v.handle = newHandle();
     v)

newEFreeModule = () -> (
     v := new EFreeModule;
     v.handle = newHandle();
     v)

newERingElement = () -> (
     v := new ERingElement;
     v.handle = newHandle();
     v)

newEVector = () -> (
     v := new EVector;
     v.handle = newHandle();
     v)

newEMatrix = () -> (
     v := new EMatrix;
     v.handle = newHandle();
     v)

newERingMap = () -> (
     v := new ERingMap;
     v.handle = newHandle();
     v)

engine = args -> (
     a := args_{0..#args-2};
     resulttype := args_(-1);
     callgg a;
     if resulttype === ZZ then
       eePopInt()
     else if resulttype === Boolean then
       eePopBool()
     else if resulttype === Intarray then
       eePopIntarray()
     else (
	  v := new resulttype;
	  v.handle = newHandle();
	  v))
------------------------------------------------------------
ZmodP = (p) -> engine(ggEcharp, p, ERing)
EZ = engine(ggEZZ, ERing)

degreeRing = (ndegs) -> (
    local names;
    if ndegs == 1 then names = "t"
	else 
      names = concatenate apply(ndegs, i -> " t" | i);
	D := emonoid(monomialOrder(GroupLex=>ndegs), toList(0..ndegs-1), names);
    polyring(ZmodP 32003, D, EZ, {}))

------------------------------------------------------------
monomialOrder = args -> (
     sendgg(ggMOinit);
     mo := newMonOrder();
     hasComponent := false;
     if class args =!= Sequence then
         args = sequence args;
     scan(args, val -> (
	 if class val === ZZ then (
	      sendgg(ggPush mo, ggPush val, ggMOrevlex)
	      )
	 else if val === Component then (
	      if hasComponent then
	      	   error "At most one 'Component' allowed in monomial order"
	      else (
		   sendgg(ggPush mo,ggMOcomponent);
		   hasComponent = true;))
	 else if class val === Option then (
	      if val#0 === Lex then (
		   if class val#1 === ZZ then 
		       sendgg(ggPush mo,ggPush val#1, ggMOlex)
		   else if class val#1 === List and all(val#1,i->class i === ZZ) then
		       sendgg(ggPush mo,ggPush val#1,ggMOlex)
		   else
		       error "Expected Lex argument to be an integer or list of integers")
	      else if val#0 === GroupLex then (
		   if class val#1 === ZZ then 
		       sendgg(ggPush mo,ggPush val#1, ggPush 1, ggMOlex)
		   else if class val#1 === List and all(val#1,i->class i === ZZ) then
		       sendgg(ggPush mo,ggPush val#1, ggPush 1, ggMOlex)
		   else
		       error "Expected Lex argument to be an integer or list of integers")
	      else if val#0 === RevLex then (
		   if class val#1 === ZZ then 
		       sendgg(ggPush mo,ggPush val#1, ggMOrevlex)
		   else if class val#1 === List and all(val#1,i->class i === ZZ) then
		       sendgg(ggPush mo,ggPush val#1,ggMOrevlex)
		   else
		       error "Expected RevLex argument to be an integer or list of integers")
	      else if val#0 === NCLex then (
		   if class val#1 === ZZ then 
		       sendgg(ggPush mo,ggPush val#1, ggMONClex)
		   else
		       error "Expected NCLex argument to be an integer")
	      else if val#0 === Weights then (
		   if class val#1 === List and all(val#1,i->class i === ZZ) then
		       sendgg(ggPush mo,ggPush val#1,ggMOwtfcn)
		   else
		       error "Expected 'Weights' argument to be a list of integers"))));
    if not hasComponent then sendgg(ggPush mo, ggMOcomponent);
    mo)
     

clone MonOrder := (mo) -> engine(ggMOclone, mo, EMonOrder)

MonOrder ** MonOrder := (mo,mo2) -> (
     result := clone mo;
     sendgg(ggPush result,ggPush mo2,ggMOproduct);
     result)
------------------------------------------------------------
emonoid = (mo,printorder,names) -> engine(ggmonoid, mo, printorder, names, EMonoid)

stats EMonoid := (M) -> sendgg(ggPush M, ggstats)
------------------------------------------------------------
polyring = (K,M,ZD,degs) -> engine(ggpolyring, ZD, degs, K, M, ERing)

weyl = (K,M,diffs,comms,ZD,degs) -> engine(ggweylalgebra, ZD, degs, K, M, diffs, comms, -1, ERing)

weylhom = (K,M,diffs,comms,homvar,ZD,degs) -> 
    engine(ggweylalgebra, ZD, degs, K, M, diffs, comms, homvar, ERing)

skewpolyring = (K,M,skews,ZD,degs) -> engine(ggskewpolyring, ZD, degs, K, M, skews, ERing)

-----------------
-- EFreeModule --
-----------------

ERing ^ ZZ := (R,n) -> engine(ggfree, R, n, EFreeModule)
ERing ^ List := (R,a) -> engine(ggfree, R, a, EFreeModule)
ERing ^ EMatrix := (R,m) -> engine(ggfree, R, m, EFreeModule)

rank EFreeModule := (F) -> engine(ggrank, F, ZZ)
ring EFreeModule := (F) -> engine(gggetring, F, ERing)
degrees EFreeModule := (F) -> engine(ggdegree, F, Intarray)
inducedOrder EFreeModule := (F) -> engine(gggetcols, F, EMatrix)
EFreeModule == EFreeModule := (F,G) -> engine(ggisequal, F, G, Boolean)
EFreeModule ++ EFreeModule := (F,G) -> (
    sendgg(ggPush F, ggPush G, ggadd);
	newEFreeModule())
EFreeModule ** EFreeModule := (F,G) -> (
    sendgg(ggPush F, ggPush G, ggmult);
	newEFreeModule())
dual EFreeModule := (F) -> (
    sendgg(ggPush F, ggtranspose);
	newEFreeModule())
submod(EFreeModule,List) := (F,a) -> (
    -- check: a is a list of integers
	sendgg(ggPush F, ggPush a, ggsubmodule);
	newEFreeModule())
symm (ZZ, EFreeModule) := (p,F) -> (
    sendgg(ggPush F, ggPush p, ggsymm);
	newEFreeModule())
exterior (ZZ, EFreeModule) := (p,F) -> (
    sendgg(ggPush F, ggPush p, ggexterior);
	newEFreeModule())
	
EFreeModule _ ZZ := (F,x) -> (
     sendgg(ggPush F, ggPush x, ggbasisElement);
     newEVector())

EFreeModule _ Sequence := (F,arg) -> (
     if #arg =!= 2 then error "expected (Integer Array, Integer)";
     if class arg#0 =!= List and class arg#0 =!= Sequence
       then error "expected (Integer Array, Integer)";
     if not all(arg#0, i -> class i === ZZ)
       then error "expected (Integer Array, Integer)";
     if class arg#1 =!= ZZ 
       then error "expected (Integer Array, Integer)";
     sendgg(ggPush F, ggPush toList(arg#0), ggPush arg#1, ggterm);
     newEVector())

-------------------
-- Ring Elements --
-------------------

ZZ _ ERing := (n,R) -> (
     sendgg(ggPush R, ggPush n, ggfromint);
     newERingElement())

ERing _ Sequence := (R,a) -> (
     if class (a#1) === ZZ then
       callgg(ggvar,a#0,a#1,R)
     else
       callgg(ggterm,R,a#0,a#1);
     newERingElement())

ERing _ List := (R,a) -> (
     -- check: a is a list of integers of even length.
     sendgg(ggPush R, ggPush 1_EZ, ggPush a, ggterm);
     newERingElement())

-------------
-- EVector --
-------------
ambient EVector := (v) -> (
    sendgg(ggPush v, gggetFreeModule);
	newEFreeModule())

EVector == EVector := (v,w) -> (
     sendgg(ggPush v, ggPush w, ggisequal);
     eePopBool())
EVector == ZZ := (v,n) -> (
     if n =!= 0 then error "cannot compare vector to non-zero integer";
     sendgg(ggPush v, ggiszero);
     eePopBool())
- EVector := (v) -> (
     sendgg(ggPush v, ggnegate);
     newEVector())
EVector + EVector := (v,w) -> (
     sendgg(ggPush v, ggPush w, ggadd);
     newEVector())
EVector - EVector := (v,w) -> (
     sendgg(ggPush v, ggPush w, ggsubtract);
     newEVector())
ZZ * EVector := (n,v) -> (
     sendgg(ggPush n, ggPush v, ggmult);
     newEVector())
EVector * EVector := (v,w) -> (
     sendgg(ggPush v, ggPush w, ggmult);
     newEVector())
rightMultiply = (v,w) -> (     
     sendgg(ggPush v, ggPush w, ggrightmult);
     newEVector())
EVector ^ ZZ := (v,n) -> (callgg(ggpower,v,n); newEVector())
EVector _ ZZ := (v,n) -> (
    sendgg(ggPush v, ggPush n, ggelem);
    newEVector())
getTerms (EVector, ZZ, ZZ) := (v,lo,hi) -> (
    sendgg(ggPush v, ggPush lo, ggPush hi, gggetterms);
    newEVector())
evector = (F,elems) -> (
    n := rank F;
	scan(n, i -> sendgg ggPush (elems#i));
	sendgg(ggPush F, ggvector);
	newEVector())
esparseVector = (F,elems) -> (
    n := #elems;
	scan(n, i -> sendgg ggPush (elems#i#0));
	r := apply(elems, i -> i#1);
	sendgg(ggPush F, ggPush r, ggsparsevector);
	newEVector())
size EVector := (v) -> (
    sendgg(ggPush v, gglength);
	eePopInt())
degree EVector := (v) -> (
    sendgg(ggPush v, ggdegree);
	eePopIntarray())
degreeWeights(EVector,List) := (v,wts) -> (
    -- check: wts is a vector of length = #vars in the ring.
    sendgg(ggPush v, ggPush wts, ggdegree);
    hi := eePopInt();
    lo := eePopInt();
    {lo, hi})
isGraded EVector := (v) -> (
    sendgg(ggPush v, ggishomogeneous);
    eePopBool())
leadComponent EVector := (v) -> (
    sendgg(ggPush v, ggleadcomp);
    eePopInt())
leadCoefficient EVector := (v) -> (
    sendgg(ggPush v, ggleadcoeff);
    eePopInt())
leadMonomial EVector := (v) -> (
     sendgg(ggPush v, ggleadmonom);
     eePopIntarray())
leadTerm(EVector) := (v) -> (
    sendgg(ggPush v, ggPush (-1), ggPush 1, ggleadterm);
    newEVector())
leadTerm(EVector,ZZ) := (v,n) -> (
    sendgg(ggPush v, ggPush n, ggPush 1, ggleadterm);
    newEVector())
leadTerm(EVector,ZZ,ZZ) := (v,n,samecomp) -> (
    sendgg(ggPush v, ggPush n, ggPush samecomp, ggleadterm);
    newEVector())
homogenize(EVector,ZZ,List) := (v,var,wts) -> (
    -- check: var is in range
    -- check: wts is of length #vars in ring
    sendgg(ggPush v, ggPush var, ggPush wts, gghomogenize);
    newEVector())
homogenize(EVector,List,List) := (v,var'd,wts) -> (
    -- check: var is in range
    -- check: wts is of length #vars in ring
    sendgg(ggPush v, ggPush var'd#0, ggPush var'd#1, ggPush wts, gghomogenize);
    newEVector())
    
-------------
-- EMatrix --
-------------
ematrix = (R,elems) -> (
    rank := #elems;
	elems = transpose elems;
	F := R^rank;
	vecs := apply(elems, v -> evector(F,v));
	scan(vecs, v -> sendgg ggPush v);
	sendgg(ggPush F, ggPush (#elems), ggPush {}, ggmatrix);
	newEMatrix())

targ EMatrix := (m) -> (sendgg(ggPush m, gggetrows); newEFreeModule())
src EMatrix := (m) -> (sendgg(ggPush m, gggetcols); newEFreeModule())

EMatrix _ ZZ := (m,c) -> (sendgg(ggPush m, ggPush c, ggelem); newEVector())
EMatrix _ Sequence := (m,x) -> (
    sendgg(ggPush m, ggPush x#0, ggPush x#1, ggelem);
	newEVector())

EMatrix == EMatrix := (m,n) -> (
    sendgg(ggPush m, ggPush n, ggisequal);
	eePopBool())
EMatrix == ZZ := (m,n) -> (
    if n =!= 0 then error "cannot compare matrix to integer";
    sendgg(ggPush m, ggiszero);
	eePopBool())

EMatrix + EMatrix := (m,n) -> (
    sendgg(ggPush m, ggPush n, ggadd);
	newEMatrix())
- EMatrix := (m) -> (
    sendgg(ggPush m, ggnegate);
	newEMatrix())
EMatrix - EMatrix := (m,n) -> (
    sendgg(ggPush m, ggPush n, ggsubtract);
	newEMatrix())
EMatrix * EMatrix := (m,n) -> (
    sendgg(ggPush m, ggPush n, ggmult);
	newEMatrix())
ZZ * EMatrix := (a,m) -> (
    sendgg(ggPush a, ggPush m, ggmult);
	newEMatrix())
transpose EMatrix := (m) -> (
    sendgg(ggPush m, ggtranspose);
	newEMatrix())

ZZ _ EFreeModule := (i,F) -> (
    if i === 1 then (
	    sendgg(ggPush F, ggiden);
		newEMatrix())
	else if i === 0 then (
	    sendgg(ggPush F, ggPush F, ggzeromat);
		newEMatrix()))

EMatrix ** EMatrix := (m,n) -> (
    sendgg(ggPush m, ggPush n, ggtensor);
	newEMatrix())
EMatrix ++ EMatrix := (m,n) -> (
    sendgg(ggPush m, ggPush n, ggdirectsum);
	newEMatrix())

submatrix(EMatrix,List) := (m,a) -> (
    -- test: a is an array of integers, all within the range 0..c-1, where
	-- c is the number of columns of m.
	sendgg(ggPush m, ggPush a, ggsubmatrix);
	newEMatrix())
submatrix(EMatrix,List,List) := (m,rows,cols) -> (
    -- test: rows is an array of integers, all within the range 0..r-1, where
	-- r is the number of rows, and cols is an integer array  all within the
	-- rank 0..c-1, where c is the number of columns of m.
	sendgg(ggPush m, ggPush rows, ggPush cols, ggsubmatrix);
	newEMatrix())

isGraded EMatrix := (m) -> (
    sendgg(ggPush m, ggishomogeneous);
	eePopBool())
EMatrix * EVector := (m,v) -> (
    sendgg(ggPush m, ggPush v, ggmult);
	newEVector())
reshape(EMatrix,EFreeModule,EFreeModule) := (m,F,G) -> (
    sendgg(ggPush m, ggPush F, ggPush G, ggreshape);
	newEMatrix())
flip(EFreeModule,EFreeModule) := (F,G) -> (
    sendgg(ggPush F, ggPush G, ggflip);
	newEMatrix())
koszul(ZZ,EMatrix) := (p,m) -> (
    sendgg(ggPush m, ggPush p, ggkoszul);
	newEMatrix())
koszul(EMatrix,EMatrix) := (m,n) -> (
    sendgg(ggPush m, ggPush n, ggkoszul);
	newEMatrix())
leadTerm(EMatrix) := (m) -> leadTerm(m,-1,true)
leadTerm(EMatrix,ZZ,Boolean) := (m,n,samecomp) -> (
    samecomp = if samecomp then 1 else 0;
    sendgg(ggPush m, ggPush n, ggPush samecomp, ggleadterm);
	newEMatrix())
random(ERing,ZZ,ZZ) := (R,r,c) -> (
    sendgg(ggPush R, ggPush r, ggPush c, ggrandom);
	newEMatrix())
modtensor(EMatrix,EMatrix) := (m,n) -> (
    sendgg(ggPush m, ggPush n, ggmontensor);
	newEMatrix())
exteriorProduct(ZZ,ZZ,EFreeModule) := (p,q,F) -> (
    sendgg(ggPush p, ggPush q, ggPush F, ggexteriorproduct);
	newEMatrix())
EMatrix | EMatrix := (m,n) -> (
    sendgg(ggPush m, ggPush n, ggPush 2, ggconcat);
	newEMatrix())
