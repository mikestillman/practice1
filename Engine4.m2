needs "Engine.m2"
-----------------------
-- Vector operations --
-----------------------

-- Over a simple base ring.
R = makeRing(monomialOrder(RevLex=>6))
F = R^3
v = evector(F,{a,b,c})
w = esparseVector(F,{{a,0},{b^2-c,1}})
(a+1_R)*v
assert(size v == 3)
assert(size(v-v) == 0)
assert(v+v == 2*v)
assert((v+v)+v == 3*v)
assert(-v == evector(F,{-a,-b,-c}))
assert(v+w == evector(F,{2*a, b + b^2 - c, c}))
assert(v-w == evector(F,{0_R, b - b^2 + c, c}))
assert(a*v == evector(F,{a^2,a*b,a*c}))
assert(101*v == 0)

-- BELOW THIS NOT SET UP YET
M = emonoid(monomialOrder 4, toList(0..3), "a b c d")
R = polyring(ZmodP 101, M, degreeRing 1, {1,1,1,1})
R1 = R^1
a = R1_((1,0,0,0),0)
b = R1_((0,1,0,0),0)
c = R1_((0,0,1,0),0)
d = R1_((0,0,0,1),0)

v = esparseVector(R^10,{{a*b,4},{c^5,8}})  -- is the order correct?
assert(v_4 == a*b)
assert(v_3 == 0)
assert(v_0 == 0)
assert(v_10 == 0)
assert(v_(-1) == 0)
assert(leadComponent v === 4)
assert(leadCoefficient v === 1)
leadTerm(v,1)  -- INCORRECT since compare routine is not written yet...
v = esparseVector(R^5, {{2*a*b-c^2,3}, {a*d-b*c,4}})
assert(degree v == {2})
assert(degreeWeights(v,{1,1,1,1}) == {2,2})
assert(degreeWeights(v,{0,0,-1,0}) == {-2,0})
assert(degreeWeights(v,{0,0,1,0}) == {0,2})
assert(degreeWeights(v,{0,1,1,1}) == {1,2})
assert(size v == 4)
assert((getTerms(v,0,0))_3 == 2*a*b)
assert(v == getTerms(v,0,0) + getTerms(v,1,-1))
assert(getTerms(v,-1,-1) == -c^2 * (R^5)_3)
assert(getTerms(v,4,4) == 0)
assert(getTerms(v,4,-1) == 0)

M = emonoid(monomialOrder(Weights=>{1,1,0,0},4), toList(0..3), "a b c d")
R = polyring(ZmodP 101, M, degreeRing 1, {1,1,1,1})
R1 = R^1
a = R1_((1,0,0,0),0)
b = R1_((0,1,0,0),0)
c = R1_((0,0,1,0),0)
d = R1_((0,0,0,1),0)

F = R^3
v = a*b*F_0 + a^2*b*c*F_1 + b^3*F_2 + F_0 + a^2*b*c*F_0 + a^2*b*d*F_1
  -- a2bc<1>+a2bc<0>+a2bd<1>+b3<2>+ab<0>+<0>
assert(leadTerm v == a^2*b*c*F_1)
assert(leadTerm(v,1) == a^2*b*c*F_1 + a^2*b*d*F_1)
assert(leadTerm(v,2) == a^2*b*c*F_1)
assert(leadTerm(v,2,0) == a^2*b*c*F_1 + a^2*b*c*F_0)
assert(leadTerm(v,1,1) == a^2*b*c*F_1 + a^2*b*d*F_1)
assert(leadTerm(v,1,0) == a^2*b*c*F_1 + a^2*b*c*F_0 + a^2*b*d*F_1 + 
                          b^3*F_2)

----------------------------------------------------------
----------------------------------------------------------
-- Basic arithmetic

-- Test A. Base ring is a coefficient ring

-- Test B. Base is a polynomial ring
makeRing monomialOrder(RevLex=>6)  
m1 = ematrix(R,{{a,b},{c,d}})
m2 = ematrix(R,{{a^2,0_R,1_R},{d^3+b^2,a*b*c-2*a,0_R}})
m3 = random(R,3,5)

src m1
targ m1
degree m1  -- FAILS: what name is this?

assert(not(m1 == m2))
m1 + m1 == 2*m1 -- FAILS
assert(m1 - m1 == 0)
m1_(0,0)
m1_0  -- INCORRECT: base shoul be R^2, but display is as in R^1
m1 + m2  -- MY PROBLEM
- m1
m1 - m2 -- MY PROBLEM
m1 * m1
f = a+b
f*m1  -- NOT DONE
2*m1  -- NOT DONE
transpose m1
m1 * transpose m2  -- CRASHES
submatrix(m1,{0,1,2})
submatrix(m1,{0,2},{1,0,2})
m1 ** m2
m1 ++ m2
m1 | m2
m * (src m)_2

m = ematrix(R,{{a,b,c,d}})
koszul(0,m)
koszul(1,m)
koszul(2,m)
koszul(3,m)
koszul(4,m)
koszul(5,m)

-- MISSING: zero, identity

-- MISSING: degree routines, isGraded
isGraded m
homogenize(m,f,{1,2,3,4,5,1})

-- MISSING: kbasis, truncate, symm, wedge, 

-- MISSING: flip, reshape, other koszul, exteriorProduct

-- MISSING: moduleTensor, selectInSubring, divideByVariable

-- MISSING: leadTerm

-- NOT DONE: diff, contract, coeffs, simplify, sort,
-- auto_reduce, minimal_lead_terms


-- Test set A for low level engine functionality
needs "Engine.m2"


mo = monomialOrder(RevLex=>3,Weights=>{1,2,3,4},Lex=>2,Component,NCLex=>3)
M = emonoid(mo,toList(0..13),"a b c d x1 x2 x3 y1 y2 z1 z2 z3 z4 z5")
M = emonoid(monomialOrder(3,4),toList(0..6),"a b c d e f g")
see M


R = polyring(ZmodP 5, M, EZ, {})
R = skewpolyring(ZmodP 5, M, {0,1,2}); see R
R = weyl(ZmodP 32003, emonoid(monomialOrder 6, toList(0..5), "a b c Da Db Dc"), {3,4,5}, {0,1,2})
R = weyl(ZmodP 32003, emonoid(monomialOrder 6, toList(0..5), "a b c Da Db Dc"), {-1,4,5}, {0,1,2})
R = weylhom(ZmodP 32003, emonoid(monomialOrder(6,1), toList(0..6), "a b c Da Db Dc h"), {3,4,5}, {0,1,2},6)
R = polyring(ZmodP 32003, emonoid(monomialOrder(NCLex=>4),toList(0..3),"a b c d")); see R

R^3



-- Test of vector creation:
M = emonoid(monomialOrder 4, toList(0..3), "a b c d")
R = polyring(ZmodP 101, M, EZ, {})
R1 = R^1
a = R1_((1,0,0,0),0)
b = R1_((0,1,0,0),0)
c = R1_((0,0,1,0),0)
d = R1_((0,0,0,1),0)
(a+b+c)*(a+b+c)
stats M
F = R^4
a*b*F_2
v = F_2
a*v
v*a
w = F_3
v+w
v-w
v-v
2*v+142*w
w = F_10  -- incorrect: should produce 0.
v = F_((1,1,1,1),2)
w = F_((1,2,3,4),1)
v+w
v-w
(v+w)+(v-w)

evector(R^3,{a,b,c})
F = R^2
v1 = evector(F,{a,c})
v2 = evector(F,{b,d})
sendgg(ggPush v1, ggPush v2, ggPush F, ggPush 2, ggPush {}, ggmatrix)
m = newEMatrix()
m = ematrix(R,{{a,b},{c,d}})
-- Test of NC arithmetic:
M = emonoid(monomialOrder (NCLex=>4), toList(0..3), "a b c d")
R = polyring(ZmodP 101, M, EZ, {})
R1 = R^1
a = R1_((1,0,0,0),0)
b = R1_((0,1,0,0),0)
c = R1_((0,0,1,0),0)
d = R1_((0,0,0,1),0)
g = c+d
time (g^4)
time (g^4);
time (g^8);
time (g^8);
time (g^10);  -- 1.04 sec  revised: .04 sec
time (g^12);  -- 19.49 sec revised: .3 sec
time (g^16); --           revised: 53.9 sec: very bad hash function. (Although better than none!)
f = a*b + b*a
f1 = b*a + a*b
assert(f == f1)
f^4
stats M
f = a*a-a*b
g1 = a*f
g2 = rightMultiply(f,a)
h = g1-g2
assert(h == a*b*a-a*a*b)
-- Test of mixed NC and Comm arithmetic
R = polyring(ZmodP 101, emonoid(monomialOrder (2, NCLex=>2), toList(0..3), "a b c d"))
R1 = R^1
a = R1_((1,0,0,0),0)
b = R1_((0,1,0,0),0)
c = R1_((0,0,1,0),0) -- BUG
d = R1_((0,0,0,1),0) -- BUG
assert(a*d == d*a)
assert(a*c == c*a)
assert(a*b == b*a)
assert(c*d != d*c)

-- Test of Weyl arithmetic
R = weyl(ZmodP 32003, emonoid(monomialOrder 6, toList(0..5), "a b c Da Db Dc"), {3,4,5}, {0,1,2})
R1 = R^1
oneR = R1_({0,0,0,0,0,0},0)
a = R1_((1,0,0,0,0,0),0)
b = R1_((0,1,0,0,0,0),0)
c = R1_((0,0,1,0,0,0),0)
Da = R1_((0,0,0,1,0,0),0)
Db = R1_((0,0,0,0,1,0),0)
Dc = R1_((0,0,0,0,0,1),0)
theta = a*Da
a*Da
Da*Da*a

-- Test of Homogeneous Weyl arithmetic
M = emonoid(monomialOrder(6,1), toList(0..6), "a b c Da Db Dc h")
R = weylhom(ZmodP 32003, M, {3,4,5}, {0,1,2},6)
R1 = R^1
oneR = R1_({0,0,0,0,0,0,0},0)
a = R1_((1,0,0,0,0,0,0),0)
b = R1_((0,1,0,0,0,0,0),0)
c = R1_((0,0,1,0,0,0,0),0)
Da = R1_((0,0,0,1,0,0,0),0)
Db = R1_((0,0,0,0,1,0,0),0)
Dc = R1_((0,0,0,0,0,1,0),0)
h = R1_((0,0,0,0,0,0,1),0)
Da*a  -- BUG
Db*b
-- Test of Skew arithmetic
R = skewpolyring(ZmodP 5, emonoid(monomialOrder 4, toList(0..3), "a b c d"), {0,1,2})
R1 = R^1
a = R1_((1,0,0,0),0)
b = R1_((0,1,0,0),0)
c = R1_((0,0,1,0),0)
d = R1_((0,0,0,1),0)
b*a
d*a
b*b*a
b*d*b

------------------------------
-- Test of vector routines ---
------------------------------

------------------------------
-- Test of matrix routines ---
------------------------------
-- Part A: Straightforward commutative polynomial ring.
M = emonoid(monomialOrder 4, toList(0..3), "a b c d")
R = polyring(ZmodP 101, M, EZ, {})
R1 = R^1
a = R1_((1,0,0,0),0)
b = R1_((0,1,0,0),0)
c = R1_((0,0,1,0),0)
d = R1_((0,0,0,1),0)
m = ematrix(R,{{a,b},{c,d}})
m1 = ematrix(R,{{a^5,b^5},{c^5,d^5}})

assert(m_(0,0) == a)
assert(m_0 == evector(targ m, {a,c}))

m2 = m*m
assert(m2_(0,0) == a^2+b*c)
assert(m2_(1,0) == a*c + c*d)

m = random(R,3,5)  -- a '1' doesn't display...
submatrix(m,{0,1,2})
submatrix(m,{1,0},{0,1,2})
transpose m  -- CRASHED
m = ematrix(R,{{a,b,c,d}})
koszul(0,m)
koszul(1,m)
koszul(2,m)
koszul(3,m)
koszul(4,m)
koszul(5,m)
m ** m
m ++ m
m | m  -- FAILS
m * (R^4)_2

///
------------
-- To Do ---
------------
- Schreyer order:
  addTo
  inSubring, leadTerm
  testing this
- unique objects
  at front end: handle of ringelem,vector,freemodule,matrix,ringmap should be
  unique.
  - need to rewrite the hashtable code for handles.
  - incorporate matrices and ring maps
- rings: during 'make': what is grabbed, and what is not?
  Also: what is checked?
- check deletion logic
  in particular, when a vector is created or destroyed, the free module
  needs to be bumped up, down?
- symm of a free module needs to be written.
- a few vector operations remain:
  divideBy*
  inSubring
  various coefficientOf routines...
- matrix operations
- special ops:
   evaluation
   det
   pfaff
   basis
   symm
- BIG THING: how to deal with ring elements and coefficient rings?
- Quotient rings
- hash functions for:
  monomials
  vectors
  free modules
  matrices
  ring maps
- DONE first n parts of the order
  DONE Add information in the monoid so that it is immediate how many slots to 
      check
  DONE Change the code so that keeping the same component or all such components
      is another parameter. (in leadTerm, not in inSubring).
  
///

-- Local Variables:
-- compile-command: "make -C $M2BUILDDIR/Macaulay2/e"
-- End:
