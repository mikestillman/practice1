newPackage(
    "BooleanGB",
    Version => "1.0", 
    Date => "May 9, 2011",
    Authors => {{Name => "Franziska Hinkelmann", 
    Email => "fhinkel@vt.edu", 
    HomePage => "http://www.math.vt.edu/people/fhinkel/"}, 
    {Name => "Mike Stillman"},
    {Name => "Elizabeth Arnold"}},
    Headline => "Groebner Bases for Ideals in Boolean Polynomial Quotient Ring",
    DebuggingMode => true
    )

exportFrom_Core {"gbBoolean"}

-- Code here


beginDocumentation()

doc ///
Key
  BooleanGB
Headline
  Groebner Bases for Ideals in Boolean Polynomial Quotient Ring
Description
  Text
    BooleanGB is a package to compute Groebner Bases in lexicographic order for polynomial ideals in the
    quotient ring $\mathbb F_2[x_1, \ldots, x_n]/J$, where J is the ideal
    generated by field polynomials $x_i^2 - x_i $ for $i \in \{ 1, \ldots,
    n\}$. The algorithm is implemented bitwise rather than symbolically, which
    reduces the computational complexity.
  Example
    n = 3;
    R = ZZ/2[vars(0)..vars(n-1)];
    J = apply( gens R, x -> x^2 + x); 
    QR = R/J;
    I = ideal(a+b,b);
    gbBoolean I
    gens gb I
Caveat
  BooleanGB always assumes that the ideal is in the Boolean quotient ring, i.e., $\mathbb
  F_2[x_1, \ldots, x_n] / <x_1^2-x_1, \ldots, x_n^2-x_n >$, regardless of the ring in which the
  ideal was generated. Thus, any ideal in the base ring is 
  promoted to the quotient ring automatically, even if the quotient ring has not been
  defined. 
SeeAlso
///

doc ///
Key
  (gbBoolean, Ideal)
  gbBoolean
Headline
  Compute Groebner Basis for Ideals in Boolean Polynomial Quotient Ring
Usage
  gbBoolean I
Inputs
  I:Ideal
Outputs
  J:Ideal
    the reduced Groebner basis of I
Consequences
Description
  Text
    gbBoolean computes $J$, a reduced Groebner basis in lexicographic order
    for the ideal $I$ in the Boolean quotient ring, i.e., $\mathbb F_2[x_1,
    \ldots, x_n] / <x_1^2-x_1, \ldots, x_n^2-x_n >$. The algorithm is
    implemented bitwise rather than symbolic, which reduces the computational
    complexity.
  Example
    n = 3
    R = ZZ/2[vars(0)..vars(n-1)]
    J = apply( gens R, x -> x^2 + x) 
    QR = R/J
    I = ideal(a+b,b)
    gbBoolean I
    gens gb I
Caveat
  gbBoolean always assumes that the ideal is in the Boolean quotient ring, i.e., $\mathbb
  F_2[x_1, \ldots, x_n] / <x_1^2-x_1, \ldots, x_n^2-x_n >$, regardless of the ring in which the
  ideal was generated. Thus, gbBoolean promotes an ideal in the base ring
  to the quotient ring automatically, even if the quotient ring has not been
  defined. 
SeeAlso
  gb
///


-- These tests check the generators for equivalence


TEST ///
R = ZZ/2[vars(0..14), MonomialOrder=>Lex]
l = apply(gens R, x-> x^2+x);
QR = R/l;
I = ideal(b*k+a+o+1,a*k+b,a*c*i+c*d*i+a*i*o+c*d+1,h*i*j*l+c*h*j+i*l+d+l,b*c*d*f*n+c*d*f+b*d*n+b*c+b*f+d*f+b*n+b+c+d+e,f,a*b*g*j*n+b*g*n+a*b+b*g+b+g,e*i*m*o+e*h*i+e*i*o,d*f*g+c*f*o+f+i,f*g*j+h*m+h+j,b*d*i+d*f*j+f*i*j+k,e*o+o,d*i*k+d*i+m,d*e*k*o+d*e*k+d*g*o+e*g*o+e*k*o+d*e+e*g+g*o+n+1,a*d*e+a*e*j+a*d*m+a*e*m+d*j*m+a*m+j+o+1)
C = gb I;
B = gbBoolean I;
assert( sort gens B - sort gens C == 0 ) 
///

TEST ///
R = ZZ/2[a..t, MonomialOrder=>Lex]
l = apply(gens R, x-> x^2+x);
QR = R/l;
I = ideal {
  b*c+1,
  a*b*c*d*f*g*h*t + i*o*p*q*r*s*t + r + s, 
  a*c*e*i*q + d*m*o*q + f*g
};
C = gb I;
B = gbBoolean I;
assert( sort gens B - sort gens C == 0 ) 
///

TEST ///
R=ZZ/2[vars(0..31), MonomialOrder=>Lex]
l =  apply( gens R, x -> x^2+x);
RQ = R/l
I = ideal(a);
C = gb I;
B = gbBoolean I;
assert( sort gens B - sort gens C == 0 )
///

TEST ///
R = ZZ/2[a..t, MonomialOrder=>Lex]
l = apply(gens R, x-> x^2+x);
QR = R/l;
I = ideal {
  b*c+1,
  a*b*c*d*f*g*h*t + i*o*p*q*r*s*t + r + s, 
  b*c*l*o*r*s + b*s + i + m*n*q, 
  a*c*e*i*q + d*m*o*q + f*g, 
  i + l*m*n + q*r + q +1
};
C = gb I;
B = gbBoolean I;
assert( sort gens B - sort gens C == 0 ) 
///

TEST ///
R = ZZ/2[a..t, MonomialOrder=>Lex]
l = apply(gens R, x-> x^2+x);
QR = R/l;
I = ideal {
  b*c,
  a*b*c*d*f*g*h*t + i*o*p*q*r*s*t + r + s, 
  b*c*l*o*r*s + b*s + i + m*n*q, 
  a*c*e*i*q + d*m*o*q + f*g, 
  i + l*m*n + q*r + q
};
C = gb I;
B = gbBoolean I;
assert( sort gens B - sort gens C == 0 ) 
///

TEST ///
R=ZZ/2[vars(0..31), MonomialOrder=>Lex]
l =  apply( gens R, x -> x^2+x);
RQ = R/l
I = ideal(a,b, a*c+d);
C = gb I;
B = gbBoolean I;
assert( sort gens B - sort gens C == 0 )
///

TEST ///
  R = ZZ/2[x,y,z, MonomialOrder=>Lex]
  QR = R / ideal apply(gens R, x -> x^2 + x)
  I = ideal(x+y,x)
  correct = sort flatten entries gens gb I
  G = sort flatten entries gens gbBoolean I
  assert(correct === G ) 
///

TEST ///
  R = ZZ/2[x,y,z, MonomialOrder=>Lex]
  QR = R / ideal apply(gens R, x -> x^2 + x)
  I = ideal(x*y+z)
  correct = sort flatten entries gens gb I
  G = sort flatten entries gens gbBoolean I
  assert(correct === G ) 
///



 TEST ///
   R = ZZ/2[ vars(1..20), MonomialOrder=>Lex]
   QR = R / ideal apply(gens R, x -> x^2 + x)
   II3 = ideal (c*k*r + 1, b*d*h*i*n + b*h*i*n + b*d*h + b*d*i + b*i*n + d*n + b, g*h*l*o*r + g*o, j*l*m + d*m*t + l*m*t + l*t, e*k*t*u + g*k*t*u + e*g*k + e*g*u + g*k + u, m*n*q*r + k*n + n*q + m*r + 1, b*e*g*o + e*g*o*s + b*g*o + e*g*o + b*o*s + e*o*s + e*g, e*g*k*q + g*k*q*t + g*k*q + g*t + k*t, j*m*t*u + f*j*t, o*q*t*u + o*t*u, p*s*u + q*r + r*s + q + u, b*s, b*f*n*s + f*n*s + n*s*t + f*n + f, d*p + d*t, g*l*q*t + q*t, c*d*e*p*q, d*q*r*t + o*q*r + d*q + o*r + r*t + o, d*h*m*n*p + h*m*n*p, f*k*o*s*t + f*k*o*s + f*o*s*t + k*o*s*t + f*k*o + f*k*s + f*k*t + f*k + o*t + f, k*q*t + h*q + h + 1)
   correctSolution = sort flatten entries gens gb( II3, Algorithm=>Sugarless)
   G = sort flatten entries gens gbBoolean II3
   assert( G == correctSolution )
 ///


 TEST ///
   R = ZZ/2[ vars(1..20)]
   QR = R / ideal apply(gens R, x -> x^2 + x)
   II5 = ideal(l,c*g*h*k*q+c*h*k*q+c*k*q+h*k*q+g*k+h*q+k+1,b*f*k*q*t+b*f*k*t+b*t+q,c*d*j*k+c*d*j*t+c*d*k*t+d*j*k+j*k*t+d*j+j*k+j,e*j*p*q*u+e*j*q*u+e*j+e*q+p*q+q,c*k*m*s*u+c*k*m*u+c*k*m+k*m*s+k*m*u+k*u+m*u+m,b*f*g*r+e*f*g*r+e*f*r+f*g+e*r,f*k*l*u+i*k*l*u+f*l*u+k*l*u+i*k+i*l+k*l+k,f*l*o+f*n+l*o+n*o+l*u,d*e*g*n+d*n+g,d*j*m*o+d*e*o+d*o+m*o+1,f*g*h*i+f*g*h+f*g*i+f*g*q+h*i+1,d*p*r*s+d*p*s+f*r+f*s+f,b*j*k*q*r+b*j*k*q+j*k*q*r+b*q,d*f*g*n+d*f*p+f*n*p+g*n*p+d*g+g*p+1,k*p*q,h*l*o+h*n*r+h*o*r+l*o*r+n*o*r+l+o,f*p*u+c*f+p*u+1,d*h+d,b*g*h+h)
   correct = sort flatten entries gens gb II5      -- used 0.0001 seconds
   G = sort flatten entries gens gbBoolean II5
   assert( correct == G ) 
 ///

 TEST ///
   R = ZZ/2[ vars(1..20), MonomialOrder=>Lex]
   QR = R / ideal apply(gens R, x -> x^2 + x)
   II6 = ideal(b*c*e*j+b*c*j*n+b*e*j*n+c*e+c*n+c,g*k+g,d*e*f*o+d*f*o*r+d*f*o+e*f*o+d*e*r+e+1,f*s+n*s,d*e*j*o+d*e*j*q+d*j*o*q+e*j*o+d*j*q+e*o+d+o,f*i*n+f*n,f*j*l*p+f*j+j*l,e*k*n*s+e*g*s+e*n*s+g*s+g,c*p*s*t+c*j*t+s,c*k+f,b*e*f+b*e*o+b*o*t+e*o*t+b*o+f*o,b*g+f*q+q,i*m+b*t+k,e*i*l+e*i*m+h*i+h*m+e+1,r*t+1,d*m,d*f*p+e*p*q+f*p*q+d*f+d*p,e*i*m+e*i*p+i*m*p+e*m+f*p+f+i+p,e*g*h*i*u+g*h*i*u+g*h+h*i,c*q+i*q)
   G = sort flatten entries gens gbBoolean II6
   correctSolution = sort flatten entries gens gb (II6, Algorithm=>Sugarless)
   assert(G == correctSolution )
 ///

 TEST ///
   R = ZZ/2[ vars(1..20)]
   QR = R / ideal apply(gens R, x -> x^2 + x)
   II4 = ideal (d*h*j + h*j*o + d*k*o + j*k*o + d*k + d + h, e*f*o*q + e*g*o + f*g*o + e*g + f*q + g*q + g + 1, h*l*n + j*n*r, q*u + f + q + 1, b*j + h*n, l*m*o + l*q, f*h*o*q + f*h*o + f*o*t + h*q*t + h + 1, g*h*p*r + g*h*r + g*p*r + h*p*r + g*r + h*r + p*r + h + m, d*k + d*r + f*r + k, b*j*o*p*s + b*j*p*s + b*j*p + b*o + b + p + 1, g*r, e*j*r*s + o*r*s + o*r + r*s + e + j + s, m*u + n*u, i*j*p*q + h*i + h*p + j + q, e*l*t*u + d*e*l + e*l*t + d*l*u + e*l*u + d, e*l*m*r + e*l*m*s + l*m*r + l*r*s + m*r*s + l*m, j*m*q*r*t + j*m*q + j*m*t, b*d*r*u + d*p*r*u + d*p*u + b*p + d*p + b*r + p, c*e*m*s, d*e*q*u + e*u + q*u + q)
   GG = sort flatten entries gens gbBoolean II4
   assert( GG == {1})
 ///

end
--------- here is the big end ----------
---------------------------------------


check "BooleanGB"

restart
load "BooleanGB.m2"
installPackage "BooleanGB"
viewHelp BooleanGB 
