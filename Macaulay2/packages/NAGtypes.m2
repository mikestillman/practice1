-- -*- coding: utf-8 -*-
-- licensed under GPL v2 or any later version
newPackage(
     "NAGtypes",
     Version => "1.4",
     Date => "March, 2011",
     Headline => "Common types used in Numerical Algebraic Geometry",
     HomePage => "http://people.math.gatech.edu/~aleykin3/NAG4M2",
     Authors => {
	  {Name => "Anton Leykin", Email => "leykin@math.gatech.edu"}
	  },
     -- DebuggingMode should be true while developing a package, 
     --   but false after it is done
     DebuggingMode => true 
     )

export {
     Norm, MaxConditionNumber, -- options
     -- service functions
     generalEquations, addSlackVariables,
     -- witness set
     "WitnessSet", "witnessSet", "equations", "slice", "points", 
     "Equations", "Slice", "Points", "sliceEquations", "IsIrreducible",
     -- point (solution)
     "Point", "point", "coordinates",
     isRealPoint, realPoints, plugIn, residual, relativeErrorEstimate, classifyPoint,
     "Tolerance", "sortSolutions", "areEqual", "isGEQ",
     "Coordinates", "SolutionStatus", "LastT", "ConditionNumber", "Multiplicity", 
     "NumberOfSteps", "ErrorBoundEstimate",
     "MaxPrecision", "WindingNumber", "DeflationNumber",
     "Regular", "Singular", "Infinity", "MinStepFailure", "NumericalRankFailure"
     }

Point = new Type of MutableHashTable 
WitnessSet = new Type of MutableHashTable 

-----------------------------------------------------------------------
-- POINT = {
--   Coordinates => List of CC,
--   NumberOfSteps => ZZ, -- number of steps made while tracking the path
--   SolutionStatus => {Regular, Singular, Infinity, MinStepFailure, null}
--   LastT => RR in [0,1]
--   ConditionNumber => condition number of the Jacobian
--   ErrorBoundEstimate => absolute error bound estimate (from Newton's method)
--   Multiplicity => the multiplicity (sometimes: the number of paths converging to the point) 
--   MaxPrecision => max precision used during the homotopy tracking
--   WindingNumber => used in the end-games
--   DeflationNumber => number of first-order deflations 
--   }
Point.synonym = "point"
point = method()
point List := s -> new Point from {Coordinates=>first s} | drop(s,1)
net Point := p -> (
     if not p.?SolutionStatus or p.SolutionStatus === Regular then net p.Coordinates 
     else if p.SolutionStatus === Singular then net toSequence p.Coordinates
     else if p.SolutionStatus === MinStepFailure then net "[M,t=" | net p.LastT | net "]"
     else if p.SolutionStatus === Infinity then net "[I,t=" | net p.LastT | net "]"
     else if p.SolutionStatus === NumericalRankFailure then net "[N]"
     else error "the point is corrupted"
     ) 
coordinates = method()
coordinates Point := p -> p.Coordinates
status Point := o -> p -> if p.?SolutionStatus then p.SolutionStatus else null
matrix Point := o -> p -> matrix {coordinates p}

-- plug a point p into the system S (expects S to be a list of polynomials)
plugIn = method()
plugIn (List, List) := (S,p) -> flatten entries sub(matrix{S}, matrix{ p / toCC })
plugIn (List, Point) := (S,p) -> plugIn(S,coordinates p)

norm (Thing, Point) := (no,p) -> norm(no, coordinates p)
norm (Thing, List) := (no,p) -> (
     if instance(no,InfiniteNumber) and no === infinity then return max(p/abs);
     assert((instance(no, ZZ) or instance(no, QQ) or instance(no, RR)) and no>0);
     (sum(p, c->abs(c)^no))^(1/no)
     )

residual = method(Options=>{Norm=>2})
residual (List,Point) := o->(S,p)->norm(o.Norm,plugIn(S,p))

relativeErrorEstimate = method(Options=>{Norm=>2})
relativeErrorEstimate(Point) := o->p->p.ErrorBoundEstimate/norm(o.Norm,p) 

isRealPoint = method(Options=>{Tolerance=>1e-6})
isRealPoint Point := o -> p -> norm (coordinates p / imaginaryPart) < o.Tolerance

realPoints = method(Options=>{Tolerance=>1e-6})
realPoints List := o -> pp -> select(pp, isRealPoint)

classifyPoint = method(Options=>{MaxConditionNumber=>1e6})
classifyPoint(Point) := o -> p -> if status p === null and p.?ConditionNumber then (
     p.SolutionStatus = 
     if p.ConditionNumber < o.MaxConditionNumber 
     then Regular  
     else Singular
     )  

areEqual = method(TypicalValue=>Boolean, Options=>{Tolerance=>1e-6, Projective=>false})
areEqual (List,List) := o -> (a,b) -> (
     if class first a === List 
     or class first a === Point 
     then (
	  #a == #b and all(#a, i->areEqual(a#i,b#i,o))
	  ) else (
     	  #a == #b and ( if o.Projective 
	       then (1 - abs sum(a,b,(x,y)->x*conjugate y))/((norm(2,a)) * (norm(2,b))) < o.Tolerance  -- projective distance is too rough in practice
	       else norm(2,(a-b)) < o.Tolerance * norm(2,a)
	       )
	  )
     ) 
areEqual (Number,Number) := o -> (a,b) -> areEqual(toCC a, toCC b, o)
areEqual (CC,CC) := o -> (a,b) -> (
     abs(a-b) < o.Tolerance
     ) 
areEqual (Matrix,Matrix) := o -> (a,b) -> (
     areEqual(flatten entries a, flatten entries b, o)
     ) 
areEqual (Point,Point) := o -> (a,b) -> areEqual(a.Coordinates, b.Coordinates, o) 

isGEQ = method(TypicalValue=>Boolean, Options=>{Tolerance=>1e-6})
isGEQ(List,List) := o->(t,s)-> (
     n := #t;
     for i from 0 to n-1 do ( 
	  if not areEqual(t#i,s#i, Tolerance=>o.Tolerance) 
	  then 
	  if abs(realPart t#i - realPart s#i) < o.Tolerance then 
	  return imaginaryPart t#i > imaginaryPart s#i
	  else return realPart t#i > realPart s#i
	  ); 
     true -- if approx. equal 
     )

sortSolutions = method(TypicalValue=>List, Options=>{Tolerance=>1e-6})
sortSolutions List := o -> sols -> (
-- sorts numerical solutions     
     if #sols == 0 then sols
     else (
	  sorted := {0};
	  get'coordinates := sol -> if class sol === Point then coordinates sol else sol;
	  scan(#sols-1, s->(
		    -- find the first element that is "larger";
		    -- "larger" means the first coord that is not (approx.) equal 
		    -- has (significantly) larger realPart, if tie then larger imaginaryPart
		    --l := position(sorted, t->isGEQ(first t, first s));
     	       	    s = s + 1;
		    t := get'coordinates sols#s;
		    l := 0; r := #sorted-1;
		    if isGEQ(t, get'coordinates sols#(sorted#r)) then  sorted = sorted | {s}
		    else if isGEQ(get'coordinates sols#(sorted#l),t) then  sorted = {s} | sorted 
		    else (
		    	 while r-l>0 do (
			      m := (l+r)//2;
			      if isGEQ(get'coordinates sols#(sorted#m), t) then r=m
			      else l=m+1; 
			      );
		    	 sorted = take(sorted,r) | {s} | drop(sorted,r);
		    	 )
		    ));      
	  );
     apply(sorted, i->sols#i)
     )


-----------------------------------------------------------------------
-- WITNESS SET = {
--   Equations,            -- an ideal  
--   Slice,                -- a list of linear equations OR a matrix (of their coefficients)
--   Points,	           -- a list of points (in the format of the output of solveSystem/track) 
--   IsIrreducible         -- true, false, or null
--   }
-- caveat: we assume that #Equations = dim(Slice)   
WitnessSet.synonym = "witness set"
protect Tolerance
dim WitnessSet := W -> ( if class W.Slice === List then #W.Slice 
     else if class W.Slice === Matrix then numrows W.Slice 
     else error "ill-formed slice in WitnessSet" )
codim WitnessSet := W -> numgens ring W - dim W
ring WitnessSet := W -> ring W.Equations
degree WitnessSet := W -> #W.Points
ideal WitnessSet := W -> W.Equations
net WitnessSet := W -> "(dim=" | net dim W |",deg="| net degree W | ")" 

witnessSet = method(TypicalValue=>WitnessSet)
witnessSet (Ideal,Ideal,List) := (I,S,P) -> 
  new WitnessSet from { Equations => I, Slice => S_*, Points => VerticalList P, IsIrreducible=>null }
witnessSet (Ideal,Matrix,List) := (I,S,P) -> 
  new WitnessSet from { Equations => I, Slice => S, Points => VerticalList P, IsIrreducible=>null}

points = method() -- strips all info except coordinates, returns a doubly-nested list
points WitnessSet := (W) -> apply(W.Points, coordinates)

equations = method() -- returns list of equations
equations WitnessSet := (W) -> (W.Equations)_*

slice = method() -- returns linear equations for the slice (in both cases)   
slice WitnessSet := (W) -> ( if class W.Slice === List then W.Slice
     else if class W.Slice === Matrix then sliceEquations(W.Slice, ring W)
     else error "ill-formed slice in WitnessSet" )

sliceEquations = method(TypicalValue=>List) 
sliceEquations (Matrix,Ring) := (S,R) -> (
-- make slicing plane equations
     apply(numrows S, i->(sub(S^{i},R) * transpose(vars R | matrix{{1_R}}))_(0,0)) 
     )

{**********************************************************************
A NEW PROPOSED DATA TYPE: 

  NumericalVariety = {
     0 => list of (irreducible) witness sets
     1 => list of (irreducible) witness sets
     ...
     dim => list of (irreducible) witness sets
     ...
     }

SERVICE FUNCTIONS:
  dim
  degree
  isReduced
  NumericalVariety union NumericalVariety (binary)
  
*}

generalEquations = method()

-- make k random linear combinations of gens I
generalEquations (ZZ,Ideal) := (k,I) -> (
     R := ring I;
     ngens := numgens I;
     ideal ( (gens I) * random(R^ngens, R^k) )
     )
generalEquations (ZZ,List) := (k,F) -> (generalEquations(k,ideal F))_*;


-- change the equations to be general change of vars, if not a CI
-- the output is a new witness set, with the same points and slice.
generalEquations WitnessSet := (W) -> (
     R := ring W;
     n := numgens R;
     d := dim W;
     ngens := numgens ideal W;
     if ngens === n-d then W
     else (
	  -- take random combinations of the equations
	  neweqns := (generators ideal W) * random(R^ngens, R^(n-d));
	  witnessSet(ideal neweqns, ideal W.Slice, W.Points))
     )

addSlackVariables = method()
addSlackVariables WitnessSet := (W) -> (
     -- creates a new system of polynomials, in variables:
     -- old set of variables, and zz1, ..., zzd, where
     -- d is the dimension of W.
     R := ring W;
     n := numgens R;
     d := dim W; -- this will be the number of slack variables to add
     W1 := generalEquations W;
     -- Add in new variables zz1, ..., zzd,
     --  this changes the equations, the slice, and the points
     slackvars := apply(d, i->getSymbol("zz"|toString (i+1)));
     newR := (coefficientRing R)[gens R, slackvars];
     newvars := (vars newR)_{n..n+d-1};
     -- new slice:
     newSlice := apply(d, i -> sub(W1.Slice#i,newR) + newR_(n + i));
     -- add a linear matrix 
     A := random(newR^(d),newR^(n-d));
     AZ := transpose newvars * A;
     newEqns := (sub(gens ideal W1, newR) + AZ) | newvars;
     -- new points
     zeros := toList apply(d, i -> 0_(coefficientRing R));
     newPoints := apply(W1.Points, pt -> join(pt,zeros));
     witnessSet(ideal newEqns, ideal newSlice, newPoints)
     )

beginDocumentation()

undocumented {(generalEquations,WitnessSet), generalEquations}

document {
     Key => NAGtypes,
     Headline => "Common types used in Numerical Algebraic Geometry",
     PARA{
     	  "The package defines types used by the package ", TO "NumericalAlgebraicGeometry::NumericalAlgebraicGeometry", 
     	  " as well as other numerical algebraic geometry packages: e.g., an interface package ", 
     	  TO "PHCpack::PHCpack", "."
	  },  
     "Main datatypes: ",
     UL{
	  {"Point", " -- numerical approximation of a point in a complex space (and related methods)"},
	  {"WitnessSet", " -- a witness set representing (possibly positive-dimensional) solution components"}
	  },
     "Other service functions: ",
     UL{
	  {"areEqual", ""},
	  {"sortSolutions", ""},
	  {"addSlackVariables", ""},
	  {"generalEquations", ""}
	  }
     }
document {
     Key => {Point, coordinates, (coordinates,Point), (status,Point), (matrix,Point), 
	  Regular, Singular, Infinity, MinStepFailure, (net, Point),
	  Coordinates, SolutionStatus, LastT, ConditionNumber, NumberOfSteps, ErrorBoundEstimate},
     Headline => "a type used to store a point in complex space",
     "This type is used to store a solution to a polynomial system obtained by such fuctions as ", 
     TO "solveSystem", ", ", TO "track",". The following methods can be used to access a ", 
     TO "Point", ":",
     UL{
	  {"coordinates", " -- get the coordinates (returns a list)"},
	  {"status", " -- get the type of solution (e.g., Regular)"},
	  {"matrix", " -- get the coordinates (returns a matrix)"}
	  },
     "Possible types of Points (accessed by ", TO "status", "): ",
     UL { {"Regular", " -- the jacobian of the polynomial system is regular at the point"}, 
	  {"Singular", " -- the jacobian of the polynomial system is (near)singular at the point"}, 
	  {"Infinity", " -- the solution path has been deemed divergent"},
	  {"MinStepFailure", " -- the tracker failed to stay above the minimal step increment threshold"},
	  {null, " -- the point has not been classified"}
	  },
     "Only coordinates are displayed (by ", TO "net", "); to see the rest use ", 
     TO "peek", ".  Different algorithms attach different information describing the point. For example, the
     solveSystem function with default options produces the following.",
     PARA{},
     EXAMPLE lines ///
       loadPackage "NumericalAlgebraicGeometry";
       R = CC[x,y];
       sols = solveSystem{x^2+y^2-3, x^3-y^3-7}
       pt = first sols
       peek pt
       coordinates pt
       status pt
     ///,
     PARA{"For example, one may see the condition number of the Jacobian of the polynomial system, evaluated at this point
      (the smaller the value, the better) as follows."},
     EXAMPLE lines ///
       pt.ConditionNumber
     ///,
     PARA{"The other keys that may be attached include "}, 
     UL{
	  {TO NumberOfSteps, " -- the number of steps in made by the continuation procedure"}, 
     	  {TO LastT, " -- the last value of the continuation parameter produced during tracking (equals 1 for a regular solution)"},
	  {TO ErrorBoundEstimate, " -- an estimate of the distance from the approximation to the actual solution"},
	  {TT "Tracker", " -- reserved for developers"}
	  }
     }
document {
	Key => {(point,List), point},
	Headline => "construct a Point",
	Usage => "p = point c",
	Inputs => { 
	     "c"=> {"contains elements in the form {{list of complex coordinates}, other data}"}
	     },
	Outputs => {"p"=>Point},
	PARA{"Used to construct a ", TO2{Point, "point"}, " from the old format of output."},
        EXAMPLE lines ///
p = point {{1+0.2*ii, 0.5}, SolutionStatus=>Regular, LastT=>1., NumberOfSteps=>10, ConditionNumber=>2.3}
peek p 
     	///
	}
document {
     Key => {WitnessSet,equations,(equations,WitnessSet),slice,(slice,WitnessSet),
	  points,(points,WitnessSet),(ideal,WitnessSet),Equations,Slice,Points,
     	  (codim,WitnessSet),(degree,WitnessSet),(dim,WitnessSet),(ring,WitnessSet),(net,WitnessSet) 
     	  },
Headline => "a witness set",
     "This type stores a witness set of an equidimensional solution component. ", 
     "The following methods can be used to access a ", 
     TT "WitnessSet", ":",
     UL{
     	  {"ideal", " -- get the defining ideal of the algebraic superset"},
	  {"equations", " -- get the list of defining polynomials of the algebraic superset"},
	  {"slice", " -- get linear functions defining the slicing plane"},
	  {"points", " -- get the list of witness points (which are zeroes of all above)"}
	  },
     "Also one may determine",
     UL {
	  {"dim", " -- the dimension"},
	  {"codim", " -- the codimension"},
	  {"deg", " -- the degree (the number of witness points)"},
	  {"ring", " -- the ring of the defining polynomials"}
	  }, 
     "Only dimension and degree are displayed (by ", TO "net", "); to see the rest use ", 
     TO "peek", "."
     }
document {
	Key => {witnessSet,(witnessSet,Ideal,Ideal,List),(witnessSet,Ideal,Matrix,List)},
	Headline => "construct a WitnessSet",
	Usage => "w = witnessSet(E,S,P)",
	Inputs => { 
	     "E" => Ideal => {"in a polynomial ring over ", TO CC },
	     "S" => {ofClass Ideal, " generated by linear polynomials or ", ofClass Matrix, " with complex coefficients of these generators"},
	     "P" => List => {"contains witness points (of type ", TO "Point", ")"}
	     },
	Outputs => {"w"=> WitnessSet},
	PARA {"Used to construct a witness set of the variety ", TT "V(E)", ". It is expected that ", TT "codim E == dim S", 
	     " and that ", TT "P", " is a subset of the intersection of ", TT "V(E)", " and ", TT "V(S)", "."},
        EXAMPLE lines ///
R = CC[x,y]	
w = witnessSet( ideal(x^2+y^2+2), ideal(x-y), {point {{0.999999*ii,0.999999*ii}}, point {{-1.000001*ii,-1.000001*ii}}} )
peek w
     	///
	}

document {
	Key => {(sliceEquations,Matrix,Ring),sliceEquations},
	Headline => "slicing linear functions",
	Usage => "S = sliceEquations(M,R)",
	Inputs => { 
	     "M"=> Matrix => " contains the coefficients of the slicing linear polynomials",
	     "R"=> Ring => " where the output polynomials belong"
	     },
	Outputs => {"S"=>List=>"contains linear polynomials"},
        PARA {"A service function used  in ", TO "NumericalAlgebraicGeometry::NumericalAlgebraicGeometry", "."},
	EXAMPLE lines ///
R = CC[x,y]	
sliceEquations(matrix{{1,2,3},{4,5,6*ii}}, R)
     	///
	}

document {
	Key => {(sortSolutions,List), sortSolutions},
	Headline => "sort the list of solutions",
	Usage => "t = sortSolutions s",
	Inputs => { 
	     "s"=>{"contains solutions (represented either by lists of coordinates or ", TO2{Point,"points"}, ")"}
	     },
	Outputs => {"t"=> "contains solutions sorted as described below"},
	"The sorting is done lexicographically regarding each complex n-vector as real 2n-vector. ",
	"The output format of ", TO "track", " and ", TO "solveSystem", " is respected.", BR{}, 
	"For the corresponding coordinates a and b (of two real 2n-vectors) a < b if b-a is larger than ", 
	TO Tolerance, ". ", 
     	PARA {},
        EXAMPLE lines ///
needsPackage "NumericalAlgebraicGeometry"
R = CC[x,y];
s = solveSystem {x^2+y^2-1, x*y}
sortSolutions s
     	///,
	SeeAlso => {"solveSystem", "track", areEqual}
	}

document { Key => {Tolerance, [sortSolutions,Tolerance], [areEqual,Tolerance], [isGEQ,Tolerance], [isRealPoint,Tolerance], [realPoints,Tolerance]},
     Headline => "specifies the tolerance of a numerical computation" 
     }

document {
	Key => {isGEQ, (isGEQ,List,List)},
	Headline => "compare two points",
	Usage => "b = isGEQ(x,y)",
	Inputs => {
	     "x" => "complex (floating point) numbers",
	     "y" => "complex (floating point) numbers"
	     },
	Outputs => {"b"=>{"tells if ", TT "x", " is (approximately) greater or equal than ", TT "y"}},
	PARA {"The inputs are lists of complex numbers, the order is (approximately) lexicographic: regard each complex n-vector as real 2n-vector, 
	      for the corresponding coordinates a and b (of two real 2n-vectors) a < b if b-a is larger than ", TO Tolerance, ". "},
	EXAMPLE lines ///
isGEQ({1,1,1},{1,0,2})
isGEQ({1,1e-7},{1, 0})
     	///,
	SeeAlso => {"areEqual"}
	}
document {
	Key => {areEqual, (areEqual,CC,CC), (areEqual,Number,Number), (areEqual,List,List), (areEqual,Matrix,Matrix), (areEqual,Point,Point), 
	     [areEqual,Projective]},
	Headline => "determine if solutions are equal",
	Usage => "b = areEqual(x,y)",
	Inputs => {
	     "x" => "a solution or a list of solutions",
	     "y" => "a solution or a list of solutions",
	     Projective=>{"if ", TO true, " then solutions are considered as representatives of points 
		  in the projective space"}
	     },
	Outputs => {"b"=>{"tells if ", TT "x", " and ", TT "y", " are approximately equal"}},
	PARA {"The inputs can be complex numbers, ", TO2{Point, "points"}, ", ", " or lists of points (presented as ", TO2{Point, "points"}, " or lists of coordinates)."},
	"The function returns false if the distance between ", TT "x", " and ", TT "y", " exceeds ", TO Tolerance, " and true, otherwise.",
	PARA {"If ", TT "Projective=>true", " then ", TEX "1-\\cos\\alpha", " is compared with the ", TO Tolerance, ", where ",
	     TEX "\\alpha", " is the angle between ", TT "x", " and ", TT "y", "." },
	EXAMPLE lines ///
areEqual({{-1,1e-7},{1e-7*ii,-1}}, {{-1, 0}, {0, -1}})
areEqual({3*ii,2*ii,1+ii}, {-6,-4,-2+2*ii}, Projective=>true)  
     	///,
	SeeAlso => {"solveSystem", "track", sortSolutions}
	}


document {
     Key => {isRealPoint, (isRealPoint,Point)},
     Headline => "determine whether a point is real",
     Usage => "b = isRealPoint p",
     Inputs => {
	     "p"
	     },
     Outputs => {"b"=>{"tells if ", TT "p", " is within ", TO Tolerance, " to a real point"}},
     PARA{},
     EXAMPLE lines ///
     needsPackage "NumericalAlgebraicGeometry"
     R = CC[x,y];
     sols = solveSystem{x^3-y^2, x-y-2}
     sols / isRealPoint
     ///
     }
document {
     Key => {realPoints, (realPoints,List)},
     Headline => "determine whether a point is real",
     Usage => "R = isRealPoint L",
     Inputs => {
	     "L" => {TO2{Point,"points"}}
	     },
     Outputs => {"R"=>{TO2{Point,"points"}, " that are real (up to ", TO Tolerance, ")"}},
     PARA{},
     EXAMPLE lines ///
     needsPackage "NumericalAlgebraicGeometry"
     R = CC[x,y];
     sols = solveSystem{x^6-y^4, x-y-2}
     realPoints sols
     ///
     }
document {
     Key => {(norm,Thing,Point)},
     Headline => "p-norm of the point",
     Usage => "a = norm(p,pt)",
     Inputs => {
	     "p"=>{"a positive real number or ", TO infinity},
	     "pt"
	     },
     Outputs => {"a"=>{"the ", TT "p", "-norm of the point ", TT "pt"}},
     PARA{},
     EXAMPLE lines ///
     needsPackage "NumericalAlgebraicGeometry"
     R = CC[x,y];
     sols = solveSystem{x^2+y^2-3, x^3-y^3-7}
     norm(infinity, first sols)
     norm(2.5, last sols) 
     ///
     }


TEST ///
CC[x,y]
S = {x^2+y^2-6, 2*x^2-y}
p = point({{1.0,2.3}, ConditionNumber=>1000, ErrorBoundEstimate =>0.01});
assert ( (100*plugIn(S,p)/round) == {29, -30} )
assert (round (1000*norm(4.5,p)) == 2312)
assert isRealPoint p
classifyPoint p
assert(round (10000*residual(S,p)) == 4173)
p2 =  point {{1.001,2.3+ii}}
p3 =  point {{.999,2.3+ii}}
assert areEqual(sortSolutions {p,p2,p3}, {p3,p,p2})
///
endPackage "NAGtypes" 

end

restart
loadPackage "NAGtypes"
uninstallPackage "NAGtypes"
installPackage "NAGtypes"
-- install docs with no absolute links
uninstallPackage "Style"
installPackage("Style", AbsoluteLinks=>false)
installPackage("NAGtypes", AbsoluteLinks=>false)

installPackage ("NAGtypes", MakeDocumentation=>false)
check "NAGtypes"
