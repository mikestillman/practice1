/* Copyright 2002 by Michael E. Stillman */

#ifndef _engine_h_
#define _engine_h_

/**
   \mainpage Hi, this is my main documentation page.
 */
#include "../d/M2types.h"

#if defined(__cplusplus)
class Monomial;
class Monoid;
class Ring;
class FreeModule;
class MonomialIdeal;
class Matrix;
class MutableMatrix;
class RingElement;
class RingMap;
class Computation;

typedef struct MonomialOrdering MonomialOrdering;
#else
/* Define the externally visible types here */
typedef struct Monomial Monomial;
typedef struct Monoid Monoid;
typedef struct Ring Ring;
typedef struct RingElement RingElement;
typedef struct FreeModule FreeModule;
typedef struct Matrix Matrix;
typedef struct MutableMatrix MutableMatrix;
typedef struct RingMap RingMap;
typedef struct Computation Computation;

typedef struct MonomialOrdering MonomialOrdering;
typedef struct MonomialIdeal MonomialIdeal;
typedef struct Monomial_pair Monomial_pair;
typedef struct RingElement_pair RingElement_pair;
typedef struct Matrix_pair Matrix_pair;
typedef struct Matrix_int_pair Matrix_int_pair;
typedef struct M2_Integer_pair M2_Integer_pair;
#endif

#ifdef __cplusplus
#define BASECLASS : public our_new_delete
#include "newdelete.hpp"
#else
#define BASECLASS
#endif

struct Monomial_pair BASECLASS { Monomial *a; Monomial *b; };
struct RingElement_pair BASECLASS { RingElement *a; RingElement *b; };
struct M2_Integer_pair BASECLASS { M2_Integer a; M2_Integer b; };
struct Matrix_pair BASECLASS { const Matrix *a; const Matrix *b; };
struct Matrix_int_pair BASECLASS { const Matrix *a; int b; };

typedef struct M2_Integer_array {
  unsigned int len;
  M2_Integer array[1];
} M2_Integer_array;

typedef struct M2_MonomialOrdering_struct {
     unsigned int len;
     MonomialOrdering  *array[1];
     } *MonomialOrdering_array;

typedef struct Monomial_array {
  unsigned int len;
  const Monomial *array[1];
} Monomial_array;

typedef struct RingElement_array {
  unsigned int len;
  const RingElement *array[1];
} RingElement_array;

typedef struct ArrayPair {
  Monomial_array *monoms;
  RingElement_array *coeffs;
} *ArrayPairOrNull;  

typedef struct Matrix_array {
  unsigned int len;
  const Matrix *array[1];
} Matrix_array;

typedef M2_Integer M2_IntegerOrNull;
typedef M2_Rational M2_RationalOrNull;
typedef M2_CC M2_CCOrNull;
typedef Monomial MonomialOrNull;
typedef Monoid MonoidOrNull;
typedef Ring RingOrNull;
typedef RingElement RingElementOrNull;
typedef FreeModule FreeModuleOrNull;
typedef Matrix MatrixOrNull;
typedef MutableMatrix MutableMatrixOrNull;
typedef MonomialIdeal MonomialIdealOrNull;
typedef RingMap RingMapOrNull;
typedef Computation ComputationOrNull;

typedef Matrix_pair Matrix_pair_OrNull;
typedef Matrix_int_pair Matrix_int_pair_OrNull;
typedef M2_Integer_pair M2_Integer_pair_OrNull;
typedef M2_arrayint M2_arrayint_OrNull;
typedef Matrix_array Matrix_array_OrNull;
typedef RingElement_array RingElement_array_OrNull;

#if defined(__cplusplus)
extern "C" {
#endif
  void IM2_initialize(void); /* drg: connected */
  M2_string IM2_last_error_message(void); /* drg: connected */

  /* Other routine groups still to add:
     computations
       LLL
       Hermite
       Smith?
       GB (several versions)
       res (several versions)
       gb kernel 
       determinants
       pfaffians
       Hilbert functions?

     rings
       QQ, ...
  */
  /**************************************************/
  /**** Random numbers ******************************/
  /**************************************************/

  void IM2_random_seed(unsigned long seed); /* TODO */

  unsigned long IM2_random(); /* TODO */

  /* Other random number routines of interest:
     ringelement
     matrix of scalars
     combinations of a given matrix
     large integer random numbers */

  /**************************************************/
  /**** Monomial routines ***************************/
  /**************************************************/
  /* Monomials in the engine: are not associated with a monoid, and may have negative
   * exponents.  Monomials are immutable objects.
   */

  const MonomialOrNull *rawVarMonomial(int v, int e); /* drg: connected 'rawVarMonomial' */
  /* return the monomial v^e */

  const MonomialOrNull *rawMakeMonomial(const M2_arrayint m); /* drg: connected rawMakeMonomial */
    /* Takes an array of the form [n, v1, e1, v2, e2, ..., vn, en]
       and returns the monomial v1^e1*v2^e2*...vn^en.
       ASSUMPTION: v1 > v2 > ... > vn >= 0, each en is not 0. */

  M2_bool IM2_Monomial_is_equal(const Monomial *a, const Monomial *b); /* drg: connected to === */

  M2_bool rawMonomialIsOne(const Monomial *a); /* drg: connected to x == 1 */

  int rawCompareMonomial(const Monoid *M, const Monomial *a, const Monomial *b); /* drg: connected to rawCompareMonomial */
  /* returns -1, 0, or 1, if a < b, a==b, a > b respectively, in the monomial ordering
   * in the monoid M 
   */

  M2_bool rawMonomialDivides(const Monoid *M, const Monomial *a, const Monomial *b); /* drg : connected to rawMonomialDivides */
  /* returns true if in the monoid M, there is a monomial c such that a*c === b */

  MonomialOrNull *rawMonomialDivide(const Monoid *M, const Monomial *a, const Monomial *b); /* drg : connected to rawMonomialDivide */
  /* returns a/b if b divides a in the monoid M, and null otherwise. */

  const MonomialOrNull *IM2_Monomial_mult(const Monomial *a, 
					  const Monomial *b); /* drg: connected * */

  const Monomial *rawColonMonomial(const Monomial *a, 
				     const Monomial *b); /* drg: connected rawColon */
  /* returns the monomial whose i th exponent is max(ai-bi,0) */

  const MonomialOrNull *IM2_Monomial_power(const Monomial *a, int n); /* drg: connected ^ */
  /* return a^n, n is any integer, null is returned on overflow. */

  const Monomial *rawLCM(const Monomial *a, 
				   const Monomial *b); /* drg: connected rawLCM*/
  /* return the monomial whose i th exponent is max(ai,bi) */

  const Monomial *rawGCD(const Monomial *a, 
				   const Monomial *b); /* drg: connected rawGCD */
  /* return the monomial whose i th exponent is min(ai,bi) */

  const Monomial *rawSaturateMonomial(const Monomial *a, 
				   const Monomial *b); /* drg: connected rawSaturate */
  /* return the monomial whose i th exponent is ai if bi is 0, 0 if bi != 0 */

  const Monomial *rawRadicalMonomial(const Monomial *a); /* drg: connected rawRadical */
  /* return the monomial whose i th exponent is 1 if ai != 0 */

  const Monomial_pair *rawSyzygy(const Monomial *a, 
					const Monomial *b); /* drg: connected rawSyzygy */
  /* */

  unsigned long IM2_Monomial_hash(const Monomial *a); /* drg: connected hash */

  const M2_arrayint rawSparseListFormMonomial(const Monomial *a); /* drg: connected rawSparseListFormMonomial */

  const M2_string IM2_Monomial_to_string(const Monomial *a); /* drg: connected intrinsic */

  /**************************************************/
  /**** MonomialOrdering routines *******************/
  /**************************************************/
  MonomialOrdering *rawLexMonomialOrdering(int nvars, int packing); /* drg: connected rawMonomialOrdering*/
    /* Lex, LexSmall, LexTiny */

  MonomialOrdering *rawGRevLexMonomialOrdering(M2_arrayint degs, int packing); /* drg: connected rawMonomialOrdering*/
    /* GRevLex, GrevLexSmall, GRevLexTiny */

  MonomialOrdering *rawRevLexMonomialOrdering(int nvars); /* drg: connected rawMonomialOrdering*/
    /* RevLex => n */

  MonomialOrdering *rawWeightsMonomialOrdering(M2_arrayint wts); /* drg: connected rawMonomialOrdering*/
    /* Weights => {...} */

  MonomialOrdering *rawGroupLexMonomialOrdering(int nvars); /* drg: connected rawMonomialOrdering*/
    /* GroupLex => n */

  MonomialOrdering *rawGroupRevLexMonomialOrdering(int nvars);
    /* GroupRevLex => n */

  MonomialOrdering *rawNClexMonomialOrdering(int nvars); /* drg: connected rawMonomialOrdering*/
    /* NCLex => n */

  MonomialOrdering *rawPositionMonomialOrdering(M2_bool up_or_down); /* drg: connected rawMonomialOrdering */
    /* argument of true:  Position => Up, (should be the default)
     * argument of false: Position => Down
     */

  MonomialOrdering *rawProductMonomialOrdering(MonomialOrdering_array mo); /* drg: connected rawMonomialOrdering*/
    /* for tensor products */

  MonomialOrdering *rawJoinMonomialOrdering(MonomialOrdering_array mo); /* drg: connected rawMonomialOrdering*/
    /* default, when making monoids and polynomial rings */

  int rawNumberOfVariables(const MonomialOrdering *mo); /* drg: connected rawNumberOfVariables*/

  int rawNumberOfInvertibleVariables(const MonomialOrdering *mo); /* drg: connected rawNumberOfInvertibleVariables*/

  M2_arrayint rawNonTermOrderVariables(const MonomialOrdering *mo); /* Dan: PLEASE CONNECT */
  /* Returns an array of the indices of those variables which are less than 1 in the
     monomial ordering.  If this number is > 0, then the monomial ordering is not a term
     order, and local (tangent cone) algorithms must be used for GB's */

  M2_string IM2_MonomialOrdering_to_string(const MonomialOrdering *mo); /* drg: connected */

  unsigned long IM2_MonomialOrdering_hash(MonomialOrdering *mo); /* drg: connected hash */
    /* Assigned sequentially */

  /**************************************************/
  /**** Monoid routines *****************************/
  /**************************************************/
  Monoid *IM2_Monoid_trivial();  /* drg: connected rawMonoid*/
    /* Always returns the same object */

  MonoidOrNull *IM2_Monoid_make(MonomialOrdering *mo,
				M2_stringarray names,
				Ring *DegreeRing,
				M2_arrayint degs); /* drg: connected rawMonoid*/
    /* This function will return NULL if the monomial order cannot be handled
       currently, if the first components for each degree are not all
       positive, or under various other "internal" error conditions */
  
  unsigned long IM2_Monoid_hash(Monoid *M);  /* drg: connected hash */
    /* Assigned sequentially */

  M2_string IM2_Monoid_to_string(const Monoid *M);  /* drg: connected */
    /* For debugging purposes */

  /**************************************************/
  /**** Ring routines *******************************/
  /**************************************************/
  unsigned long IM2_Ring_hash(const Ring *R);  /* drg: connected hash */
    /* assigned sequentially */

  M2_string IM2_Ring_to_string(const Ring *M); /* drg: connected */

  const Ring *IM2_Ring_ZZ(void);  /* drg: connected rawZZ*/
    /* always returns the same object */

  const Ring *IM2_Ring_QQ(void);  /* drg: connected rawQQ */
    /* always returns the same object */

  const RingOrNull *IM2_Ring_ZZp(int p); /* drg: connected rawZZp*/
    /* Expects a prime number p in range 2 <= p <= 32749 */

  const RingOrNull *rawGaloisField(const RingElement *f); /* drg: connected to rawGaloisField */
    /* f should be a primitive element in a ring
       R = ZZ/p[x]/(g(x)), some p, some variable x, g irreducible, 
       monic, deg >= 2.
       However, currently, NONE of this is checked...
    */

  const RingOrNull *IM2_Ring_RR(double precision); /* drg: connected rawRR */

  const RingOrNull *IM2_Ring_CC(double precision); /* drg: connected rawCC */

  const RingOrNull *IM2_Ring_RRR(void); /* drg: connected rawBigRR */

  const RingOrNull *IM2_Ring_CCC(void); /* drg: connected rawBigCC */

  const RingOrNull *IM2_Ring_polyring(const Ring *K, 
				      const Monoid *M); /* drg: connected rawPolynomialRing(,) */
  /* K can be either commutative or not. */

  const Ring *IM2_Ring_trivial_polyring(); /* drg: connected rawPolynomialRing() */
  /* This returns the polynomial ring ZZ[], whose degree ring is itself */

  const RingOrNull *IM2_Ring_skew_polyring(const Ring *R,
					   M2_arrayint skewvars); /* drg: reconnected rawSkewPolynomialRing */
  /* R should be a polynomial ring K[M], where K is commutative.
     skewvars should be a list of variable indices (for the monoid M) indicating which ones
     are skew-commutative with each other. Further quotients (using IM2_Ring_quotient) are allowed */

  const RingOrNull *IM2_Ring_weyl_algebra(const Ring *R,
					  M2_arrayint comm_vars,
					  M2_arrayint diff_vars,
					  int homog_var); /* drg: reconnected rawWeylAlgebra*/
  /* R should be a polynomial ring K[M], where K is commutative.
     comm_vars and diff_vars should be arrays of the same length.  diff_vars[i] is the differential
     operator corresponding to comm_vars[i].  If homog_var is non-negative, then the multiplication
     is taken to be the homogeneous Weyl algebra (that is Dx*x = x*Dx + h^2, where h is the 
     variable of R with index 'homog_var').
     Further quotients (using IM2_Ring_quotient) are allowed, however, one must make sure that
     the quotient is only by elements in the center of the ring.
  */

  const RingOrNull *IM2_Ring_solvable_algebra(const Ring *R,
					      const Matrix *Q); /* drg: connected rawSolvableAlgebra */
  /* R should be a polynomial ring K[M], where K is commutative.
     Q is a square matrix of size #gens(M).  If the variables are X_1, ..., X_r,
     then multiplication is defined to be X_j X_i = Q_(j,i), for j > i.
     The initial term of Q_(j,i) MUST be c * X_i * X_j, for some constant
     c (depending on i and j), or at least less than that in the monomial ordering.
  */

  const RingOrNull *IM2_Ring_frac(const Ring *R); /* drg: connected rawFractionRing*/

  const RingOrNull *IM2_Ring_localization(const Ring *R, 
					  const Matrix *P); /* drg: connected rawLocalRing */
  /* Create the localization of R.
     R should be a COMMUTATIVE ring.  P should be a one row matrix
     whose entries generate a prime ideal of R.
  */

  const RingOrNull * IM2_Ring_quotient(const Ring *R, 
				       const Matrix *I); /*drg: connected rawQuotientRing */

  const RingOrNull * IM2_Ring_quotient1(const Ring *R, 
					const Ring *B); /*drg: connected rawQuotientRing */
  /* if R is a polynomial ring of the form A[x]/J, and B = A/I (where A is a poly ring)
     then form the quotient ring B[x]/J. */

  const RingOrNull *IM2_Ring_schur(const Ring *R); /* drg: reconnected rawSchurRing */

  M2_bool IM2_Ring_is_field(const Ring *K); /* drg: connected rawIsField*/
    /* Returns true if K is a field, or has been declared to be one.
       In the latter case, if an operation shows that K cannot be a field,
       then this function will thereafter return false, and 
       IM2_Ring_get_zero_divisor(K) can be used to obtain a non-unit, if one
       has been found. */
  
  void IM2_Ring_declare_field(const Ring *K); /* drg: connected rawDeclareField*/
    /* Declare that K is a field.  The ring K can then be used as the coefficient
       ring for computing Groebner bases,etc.  */

  const RingElement * IM2_Ring_get_zero_divisor(const Ring *K); /* drg: connected rawGetZeroDivisor*/
    /* Return a non-unit for the ring K, if one has been found, or the zero
       element, if not. Perhaps we should name this 'get_non_unit'.  This
       function currently never seems to return a non-zero value, but I plan 
       on fixing that (MES, June 2002). */

  const RingOrNull *rawAmbientRing(const Ring *R); /* drg: connected rawAmbientRing */
  /* If R is a quotient of a polynomial ring, or is a fraction ring, return the
     polynomial ring over a basic ring of which this is a quotient (or fraction ring) of.
     For example, if R = frac(ZZ[s,t]/(s^2-1))[x,y,z]/(s*x+t*y+z^2), then the returned
     ring is ZZ[s,t][x,y,z]. This routine is provided only for debugging the engine. */

  const RingOrNull *rawDenominatorRing(const Ring *R); /* drg: connected rawDenominatorRing */
  /* If elements of R may have denominators, then this routine returns true, and 
     the ambient ring for denominators returned. Otherwise, NULL
     is returned, which is not to be considered an error.  This routine is provided only for debugging the engine. */

  /**************************************************/
  /**** Ring element routines ***********************/
  /**************************************************/
  const RingElement *IM2_RingElement_from_Integer(const Ring *R, 
						  const M2_Integer d);  /* drg: connected rawFromNumber*/

  const RingElementOrNull *IM2_RingElement_from_rational(const Ring *R, 
							 const M2_Rational r); /* rawFromNumber*/

  const RingElement *IM2_RingElement_from_double(const Ring *R, 
						 double d); /* drg: connected rawFromNumber*/

  const RingElement *IM2_RingElement_from_complex(const Ring *R, 
						  M2_CC z); /* drg: connected rawFromNumber*/

  const M2_IntegerOrNull IM2_RingElement_to_Integer(const RingElement *a); /* drg: connected rawToInteger*/
    /* If the ring of a is ZZ, or ZZ/p, this returns the underlying representation.
       Otherwise, NULL is returned, and an error is given */

  const M2_Rational IM2_RingElement_to_rational(const RingElement *a); /* connected: rawToRational */
    /* If the ring of a is ZZ, or ZZ/p, this returns the underlying representation.
       Otherwise, NULL is returned, and an error is given */

  double IM2_RingElement_to_double(const RingElement *a); /* rawToReal */
    /* If the ring of a is RR, this returns the underlying representation of 'a'.
       Otherwise 0.0 is returned. */

  M2_CC IM2_RingElement_to_complex(const RingElement *a); /* rawToComplex */
    /* If the ring of a is RR, this returns the underlying representation of 'a'.
       Otherwise 0.0 is returned. */

  const RingElement *rawRRRFromString(const M2_string *s);

  const RingElementOrNull *IM2_RingElement_from_BigReal(const Ring *R, 
							const M2_BigReal d); /* drg: waiting, rawFromNumber*/
    /* TODO */

  M2_BigReal IM2_RingElement_to_BigReal(const RingElement *a); /* TODO */
    /* If the ring of a is BigRR, this returns the underlying representation of 'a'.
       Otherwise NULL is returned. */


  const RingElementOrNull *IM2_RingElement_make_var(const Ring *R, 
						    int v, 
						    int e); /* drg: connected rawRingVar*/

  M2_bool IM2_RingElement_is_zero(const RingElement *a); /* drg: connected rawIsZero*/

  M2_bool IM2_RingElement_is_equal(const RingElement *a,
				   const RingElement *b); /* drg: connected === */

  const RingElement *IM2_RingElement_negate(const RingElement *a); /* drg: connected */

  const RingElementOrNull *IM2_RingElement_add(const RingElement *a, 
					       const RingElement *b); /* drg: connected */

  const RingElementOrNull *IM2_RingElement_subtract(const RingElement *a, 
						    const RingElement *b); /* drg: connected */

  const RingElementOrNull *IM2_RingElement_mult(const RingElement *a, 
						const RingElement *b); /* drg: connected */

  const RingElementOrNull *IM2_RingElement_power(const RingElement *a, 
						 const M2_Integer n); /* drg: connected */
  
  const RingElementOrNull *IM2_RingElement_invert(const RingElement *a);/* TODO */

  const RingElementOrNull *IM2_RingElement_div(const RingElement *a, 
					       const RingElement *b); /* drg: connected // */

  const RingElementOrNull *IM2_RingElement_mod(const RingElement *a, 
					       const RingElement *b); /* drg: connected % */

  const RingElement_pair *IM2_RingElement_divmod(const RingElement *a, 
						 const RingElement *b); /* drg: connected rawDivMod*/


  const M2_string IM2_RingElement_to_string(const RingElement *a); /* drg: connected */

  unsigned long IM2_RingElement_hash(const RingElement *a);/* TODO */

  const Ring * IM2_RingElement_ring(const RingElement *a); /* drg: connected rawRing*/

  /**************************************************/
  /**** polynomial ring element routines ************/
  /**************************************************/

  M2_bool IM2_RingElement_is_graded(const RingElement *a); /* drg: connected rawIsHomogeneous*/

  M2_arrayint_OrNull IM2_RingElement_multidegree(const RingElement *a); /* drg: connected rawMultiDegree*/

  M2_Integer_pair_OrNull *IM2_RingElement_degree(const RingElement *a, 
						 const M2_arrayint wts); /* drg: connected rawDegree*/
    /* The first component of the degree is used, unless the degree monoid is trivial,
       in which case the degree of each variable is taken to be 1. 
       Returns lo,hi degree.  If the ring is not a graded ring or a polynomial ring
       then (0,0) is returned.
    */

  const RingElementOrNull *IM2_RingElement_homogenize_to_degree(
            const RingElement *a,
	    int v,
	    int deg,
	    const M2_arrayint wts); /* drg: connected rawHomogenize*/

  const RingElementOrNull *IM2_RingElement_homogenize(
            const RingElement *a,
	    int v,
	    const M2_arrayint wts); /* drg: connected rawHomogenize*/
						
  const RingElementOrNull *IM2_RingElement_term(
            const Ring *R,
	    const RingElement *a,
	    const Monomial *m); /* drg: connected rawTerm*/
    /* R must be a polynomial ring, and 'a' an element of the
       coefficient ring of R.  Returns a*m, if this is a valid
       element of R.  Returns NULL if not (with an error message). 
    */

  const RingElement *IM2_RingElement_get_terms(
            const RingElement *a,
	    int lo, int hi); /* drg: connected rawGetTerms*/
    /* Returns the sum of some monomials of 'a', starting at 'lo',
       going up to 'hi'.  If either of these are negative, they are indices 
       from the back of the polynomial.
       'a' should be an element of a polynomial ring. 
    */

  const RingElementOrNull *IM2_RingElement_get_coeff(
            const RingElement *a,
	    const Monomial *m); /* drg: connected rawCoefficient*/
    /* Return (as an element of the coefficient ring) the coeff
       of the monomial 'm'. 
    */

  const RingElementOrNull *IM2_RingElement_lead_coeff(const RingElement *a); /* drg: connected rawLeadCoefficient*/

  const MonomialOrNull *IM2_RingElement_lead_monomial(const RingElement *a); /* drg: connected rawLeadMonomial*/

  int IM2_RingElement_n_terms(const RingElement *a); /* drg: connected rawTermCount*/

  ArrayPairOrNull IM2_RingElement_list_form(const RingElement *f); /* drg: connected rawPairs */

  int IM2_RingElement_index_if_var(const RingElement *f); /* drg: connected rawIndexIfVariable */
  /* if f is a variable of its ring, then the index of that variable is returned.
     If f is not a variable, then -1 is returned. */

  M2_arrayint IM2_RingElement_indices(const RingElement *f); /* drg: connected rawIndices */
  /* The list of indices of variables which occur in f is returned. */

  /**************************************************/
  /**** fraction field ring element routines ********/
  /**************************************************/

  /** The numerator of a fraction.
   * \param a An element of a fraction ring frac(R).
   * \return The numerator of a, as an element of the ring R.
   *
   * Connected as rawNumerator.
   *
   */
  const RingElement *IM2_RingElement_numerator(const RingElement *a); /* drg: connected rawNumerator*/

  /** The denominator of a fraction.
   * \param a An element of a fraction ring frac(R).
   * \return The denominator of a, as an element of the ring R.
   *
   * Connected as rawDenominator.
   *
   */
  const RingElement *IM2_RingElement_denominator(const RingElement *a); /* drg: connected rawDenominator*/

  const RingElementOrNull *IM2_RingElement_fraction(const Ring *R,
						    const RingElement *a,
						    const RingElement *b); /* drg: connected rawFraction*/

  const M2_IntegerOrNull rawSchurDimension(const RingElement *f); /* Dan: please connect */
  /* f should be a polynomial whose base ring was created using rawSchurRing
     (otherwise NULL is returned).  If so, the dimension of the corresponding
     (virtual) GL(n) representation is returned. */

  const RingElement *rawRealPart(const RingElement *f);

  const RingElement *rawImaginaryPart(const RingElement *f);

  const RingElement *rawMakeComplex(const RingElement *re, const RingElement *im);

  /**************************************************/
  /**** FreeModule routines *************************/
  /**************************************************/
  /* A FreeModule in the engine is always over a specific
     ring, and is graded using the degree monoid of the ring.
     Monomials in a free module are ordered, either in a way
     determined by the ordering in the ring, or using an induced
     (Schreyer) monomial ordering (in the case when the ring
     is a polynomial ring of some sort) */
  /* General notes: these are immutable obects, at least once
     they are returned by the engine */
  /* BUGS/TODO: the Schreyer orders produced by sum,tensor,
     symm,exterior, and submodule, ignore the current tie
     breaker values.
     Also: I might keep all freemodules for a ring unique.
     This is not currently done. */
  
  const Ring *
  IM2_FreeModule_ring(
          const FreeModule *F); /* drg: connected rawRing*/

  int
  IM2_FreeModule_rank(
          const FreeModule *F); /* drg: connected rawRank*/

  const M2_string 
  IM2_FreeModule_to_string(
          const FreeModule *F); /* drg: connected */

  const unsigned long int 
  IM2_FreeModule_hash(
          const FreeModule *F); /* TODO */ /* drg: waiting, returning 0 */

  const FreeModuleOrNull *
  IM2_FreeModule_make(
          const Ring *R, 
	  int rank); /* drg: connected rawFreeModule*/

  const FreeModuleOrNull *
  IM2_FreeModule_make_degs(
          const Ring *R, 
	  M2_arrayint degs); /* drg: connected rawFreeModule*/
    /* Make a graded free module over R.  'degs' should be of length 
     * divisible by the length 'd' of a degree vector, and the 
     * i th degree will be degs[i*d]..degs[i*d+d-1], starting at
     * i = 0. 
     */
  
  const FreeModuleOrNull *
  IM2_FreeModule_make_schreyer(
          const Matrix *m); /* drg: connected rawFreeModule*/
    /* Returns G, (a copy of) the source free module of 'm', modified to
     * use the induced order via m: compare two monomials of G via
     * x^A e_i > x^B e_j iff either 
     * leadmonomial((in m)(x^A e_i)) > leadmonomial((in m)(x^B e_j))
     * or these are the same monomial, and i > j. 
     * The case where the target of 'm' has a Schreyer order is 
     * handled efficiently. 
     */

  const M2_arrayint 
  IM2_FreeModule_get_degrees(
          const FreeModule *F); /* drg: connected rawMultiDegree*/

  const Matrix * 
  IM2_FreeModule_get_schreyer(
          const FreeModule *F); /* drg: connected rawGetSchreyer*/

  const M2_bool 
  IM2_FreeModule_is_equal(
          const FreeModule *F, 
	  const FreeModule *G); /* drg: connected === */
    /* Determines if F and G are the same graded module.  If one has a
     * Schreyer order and one does not, but their ranks and degrees are the
     * same, then they are considered equal by this routine. 
     */

  const FreeModuleOrNull *
  IM2_FreeModule_sum(
          const FreeModule *F,
	  const FreeModule *G); /* drg: connected rawDirectSum */
    /* The direct sum of two free modules over the same ring, or NULL.
     * If F or G has a Schreyer order, then so does their direct sum 
     */

  const FreeModuleOrNull * 
  IM2_FreeModule_tensor(
          const FreeModule *F,
	  const FreeModule *G); /* drg: connected rawTensor*/
    /* The tensor product of two free modules over the same ring, or NULL.
     * If F has (ordered basis {f_1,...,f_r}, and 
     * G has (ordered) basis {g_1, ..., g_s}, then
     * the result has (ordered) basis 
     *    {f_1 ** g_1, f_1 ** g_2, ..., f_1 ** g_s,
     *     f_2 ** g_1, f_2 ** g_2, ..., f_2 ** g_s,
     *     ...
     *     f_r ** g_1, ...              f_r ** g_s}.
     *  If F or G has a Schreyer order, what about their tensor product?
     *  At the moment, the answer is almost yes...
     */
  
  const FreeModule * 
  IM2_FreeModule_dual(
	  const FreeModule *F); /* drg: connected rawDual*/
    /* Returns the graded dual F^* of F: if F has basis {f_1,...,f_r},
     * with degrees {d_1, ..., d_r}, then F^* has rank r, with
     * degrees {-d_1, ..., -d_r}.  The result does not have a 
     * Schreyer order (even if F does). 
     */

  const FreeModule * 
  IM2_FreeModule_symm(
          int n, 
	  const FreeModule *F); /* drg: connected rawSymmetricPower*/
    /* Returns the n th symmetric power G of F.
     * If F has basis {f_1,...,f_r}, then G has basis
     * the monomials of f_1, ..., f_r of degree exactly n, in
     * descending lexicographic order.
     * If F has a Schreyer order, then G is set to have one as well. 
     */


  const FreeModule * 
  IM2_FreeModule_exterior(
          int n, 
	  const FreeModule *F); /* drg: connected rawExteriorPower*/
    /* Returns the n th exterior power G of F.
     * If F has basis {f_1,...,f_r}, then G has basis
     * the squarefree monomials of f_1, ..., f_r of degree exactly n, in
     * descending reverse lexicographic order.
     * If F has a Schreyer order, then G is set to have one as well. 
     */

  const FreeModuleOrNull * 
  IM2_FreeModule_submodule(
	  const FreeModule *F, 
	  M2_arrayint selection); /* drg: connected rawSubmodule*/
    /* Returns a free module obtained by choosing basis elements of F:
     * if F has basis {f_0, ..., f_(r-1)} with degrees {d_0, ..,d_(r-1)},
     * and selection = [i_1, ..., i_s], where 0 <= i_j < r for all j,
     * then return a free module of rank s, having degrees 
     * d_(i_1), ..., d_(i_s).  'selection' may include duplicate values.
     * If F has a Schreyer order, the result has one as well. 
     */
  
  /**************************************************/
  /**** Matrix routines *****************************/
  /**************************************************/

  const FreeModule * IM2_Matrix_get_target(const Matrix *M); /* drg: connected rawTarget*/

  const FreeModule * IM2_Matrix_get_source(const Matrix *M); /* drg: connected rawSource, used in rawMatrixColumns*/

  int IM2_Matrix_n_rows(const Matrix *M); /* drg: connected rawNumberOfRows*/

  int IM2_Matrix_n_cols(const Matrix *M); /* drg: connected rawNumberOfColumns*/

  const M2_arrayint IM2_Matrix_get_degree(const Matrix *M); /* drg: connected rawMultiDegree*/

  const M2_string IM2_Matrix_to_string(const Matrix *M); /* drg: connected */

  int IM2_Matrix_hash(const Matrix *M); /* drg: connected to "hash", but it always returns 0, sigh */

  const RingElementOrNull * IM2_Matrix_get_entry(const Matrix *M, int r, int c); /* drg: connected rawMatrixEntry, OK*/

  /*******************************************************************************/
  const Matrix * IM2_Matrix_identity(const FreeModule *F,
				     int preference
				     ); /* drg: connected rawIdentity, OK*/

  const MatrixOrNull * IM2_Matrix_zero(const FreeModule *F,
				       const FreeModule *G,
				       int preference
				       ); /* drg: connected rawZero, OK */

  const MatrixOrNull * IM2_Matrix_make1(const FreeModule *target,
					int ncols,
					const RingElement_array *M,
					int preference); /* drg: connected rawMatrix1, OK */

  const MatrixOrNull * IM2_Matrix_make2(const FreeModule *target,
					const FreeModule *source,
					const M2_arrayint deg,
					const RingElement_array *M,
					int preference); /* drg: connected rawMatrix2, OK */

  const MatrixOrNull * IM2_Matrix_make_sparse1(const FreeModule *target,
					       int ncols,
					       const M2_arrayint rows,
					       const M2_arrayint cols,
					       const RingElement_array *entries,
					       int preference); /* drg: connected rawSparseMatrix1, OK */
  
  const MatrixOrNull * IM2_Matrix_make_sparse2(const FreeModule *target,
					       const FreeModule *source,
					       const M2_arrayint deg,
					       const M2_arrayint rows,
					       const M2_arrayint cols,
					       const RingElement_array *entries,
					       int preference); /* drg: connected rawSparseMatrix2, OK */

  M2_bool IM2_Matrix_is_implemented_as_dense(const Matrix *M); /* connected to rawIsDense */
  /* Is the matrix M implemented in the engine as a dense matrix? */ 

  const MatrixOrNull * IM2_Matrix_remake1(const FreeModule *target,
					  const Matrix *M,
					  int preference
					  ); /* drg: connected rawMatrixRemake1, OK  */
  /* Create a new matrix (mutable or immutable), from M, with new target,
     and/or mutable-ness. The target free module must have the expected rank.
     The source free module is computed heuristically from the the target and the
     columns of the matrix.
  */

  const MatrixOrNull * IM2_Matrix_remake2(const FreeModule *target,
					  const FreeModule *source,
					  const M2_arrayint deg,
					  const Matrix *M,
					  int preference
					  ); /* drg: connected rawMatrixRemake2, OK */
  /* Create a new matrix (mutable or immutable), from M, with new target,
     source, deg and/or mutable-ness. The new free modules must have 
     the expected rank. 
  */

  MatrixOrNull *IM2_Matrix_random(const Ring *R, 
				  int r, int c, 
				  double fraction_non_zero, 
				  int special_type, /* 0: general, 1:upper triangular, others? */
				  int preference); /* connected to rawMatrixRandom, OK */

  /**********************************************************************************/

  const M2_bool IM2_Matrix_is_zero(const Matrix *M); /* drg: connected rawIsZero*/

  const M2_bool IM2_Matrix_is_equal(const Matrix *M, 
				    const Matrix *N); /* drg: connected === and to rawIsEqual for use with == */
    /* This checks that the entries of M,N are the same, as well as
       that the source and target are the same (as graded free modules).
       Therefore, it can happen that M-N == 0, but M != N.
    */

  const M2_bool IM2_Matrix_is_graded(const Matrix *M); /* drg: connected rawIsHomogeneous*/

  const MatrixOrNull * IM2_Matrix_add(const Matrix *M, const Matrix *N); /* drg: connected + */
    /* If the sizes do not match, then NULL is returned.  If they do match,
       the addition is performed.  If the targets are not equal, the target 
       of the result is set to have each degree zero.  Similarly with the
       source, and also with the degree of the matrix. */

  const MatrixOrNull * IM2_Matrix_subtract(const Matrix *M, const Matrix *N); /* drg: connected - */
    /* If the sizes do not match, then NULL is returned.  If they do match,
       the addition is performed.  If the targets are not equal, the target 
       of the result is set to have each degree zero.  Similarly with the
       source, and also with the degree of the matrix. */

  const Matrix * IM2_Matrix_negate(const Matrix *M); /* drg: connected - */

  const MatrixOrNull * IM2_Matrix_mult(const Matrix *M, 
				       const Matrix *N, 
				       M2_bool opposite_mult); /* drg: connected * */
    /* If the sizes do not match, then NULL is returned.  If they do match,
       the multiplication is performed, and the source and target are taken from N,M
       respectively.  The degree of the result is the sum of the two degrees */

  const MatrixOrNull * IM2_Matrix_scalar_mult(const RingElement *f,
					      const Matrix *M, 
					      M2_bool opposite_mult); /* drg: connected * */

  const MatrixOrNull * IM2_Matrix_concat(const Matrix_array *Ms); /* drg: connected rawConcat*/

  const MatrixOrNull * IM2_Matrix_direct_sum(const Matrix_array *Ms); /* drg: connected rawDirectSum*/

  const MatrixOrNull * IM2_Matrix_tensor(const Matrix *M,
					 const Matrix *N); /* drg: connected rawTensor*/

  const Matrix * IM2_Matrix_transpose(const Matrix *M); /* drg: connected rawDual*/

  const MatrixOrNull * IM2_Matrix_reshape(const Matrix *M,
					  const FreeModule *F,
					  const FreeModule *G); /* drg: connected rawReshape*/

  const MatrixOrNull * IM2_Matrix_flip(const FreeModule *F,
				       const FreeModule *G); /* drg: connected rawFlip*/

  const MatrixOrNull * rawWedgeProduct(int p,
				       int q,
				       const FreeModule *F); /* drg: connected rawWedgeProduct */
  /* Constructs the map
     exterior(p,F) ** exterior(q,F) --> exterior(p+q,F)
  */

  const MatrixOrNull * IM2_Matrix_submatrix(const Matrix *M,
					    const M2_arrayint rows,
					    const M2_arrayint cols); /* drg: connected rawSubmatrix*/

  const MatrixOrNull * IM2_Matrix_submatrix1(const Matrix *M,
					     const M2_arrayint cols); /* drg: connected rawSubmatrix*/


  const MatrixOrNull * IM2_Matrix_koszul(int p, const Matrix *M); /* drg: connected rawKoszul*/

  const MatrixOrNull * 
  IM2_Matrix_koszul_monoms(const Matrix *M,
			   const Matrix *N);
  /* M and N should each have one row, and the base ring should be a
     polynomial ring.  The (i,j) th entry of the resulting matrix is
     1 or -1 times N_j/M_i (if non-zero). */

  const MatrixOrNull * IM2_Matrix_symm(int p, const Matrix *M); /* drg: connected rawSymmetricPower*/

  const Matrix * IM2_Matrix_exterior(int p, const Matrix *M, int strategy); /* drg: connected rawExteriorPower*/

  const M2_arrayint IM2_Matrix_sort_columns(const Matrix *M, 
					    int deg_order, 
					    int mon_order); /* drg: connected rawSortColumns*/


  const Matrix * IM2_Matrix_minors(int p, const Matrix *M, int strategy); /* drg: connected rawMinors*/
  /* can this really not return null ? */

  const MatrixOrNull * IM2_Matrix_pfaffians(int p, const Matrix *M); /* drg: connected rawPfaffians*/

  const Matrix * rawMatrixCompress(const Matrix *M); /* connected rawMatrixCompress */

  const MatrixOrNull * IM2_Matrix_uniquify(const Matrix *M); /* TODO */
  /* if any two columns are the same up to a scalar multiple, then keep only
     one of the columns.  Remove any zero columns too.
     The definition of "same up to a scalar" is this:
     if K is the base field or ring (i.e. QQ in QQ(x,y,z)[s,t]),
     and if c and d are the lead scalar coeff's of vecs v,w, resp, then
     v and w are scalar multiplies iff d*v == c*w.  
     Warning: Over non-domains, this might not be the intended effect.
  */

  const Matrix * rawRemoveMonomialFactors(const Matrix *m, M2_bool make_squarefree_only);
  /* Dan: please connect */

  const MatrixOrNull * IM2_Matrix_remove_content(const Matrix *M); 
      /* drg: tried to connect to rawRemoveContent*/
      /* drg: but where is it??? */
      /* drg: TODO, I guess */

  /* Routines for use when the base ring is a polynomial ring of some sort */

  const MatrixOrNull * IM2_Matrix_diff(const Matrix *M,
				       const Matrix *N); /* drg: connected rawMatrixDiff*/

  const MatrixOrNull * IM2_Matrix_contract(const Matrix *M,
					   const Matrix *N); /* drg: connected rawMatrixContract*/

  const MatrixOrNull * IM2_Matrix_homogenize(const Matrix *M,
					     int var,
					     M2_arrayint wts); /* drg: connected rawHomogenize*/

  const Matrix_pair_OrNull * IM2_Matrix_coeffs(const Matrix *M, M2_arrayint vars) ;/* TODO */


  const MatrixOrNull * rawCoefficients(const M2_arrayint vars,
				       const Matrix *monoms,
				       const Matrix *M); /* drg: connected as rawCoefficients*/
  /* Given:
   *  vars : a list of variable indices in the (common) ring R of monoms and M 
   *  monoms : a map R^b --> R^a such that each column has exactly one monomial
   *      which is only in the variables in 'vars'.
   *  M : a map R^c --> R^a such that every (module) monomial of each column of M
   *      matches one of the columns of 'monoms', in the variables 'vars'.
   *
   * Returns: a matrix C : R^c --> R^b such that 
   *      (i) the entries of C do not involve the variables in 'vars', and
   *      (ii) monoms * C == M
   *
   * Assumptions on rings: if R is non-commutative, then the variables in 'vars'
   *      should commute with the variables outside of 'vars'.
   * 
   * If each column of monoms has more than one monomial, or if variables other than
   *      those in 'vars' occur, then only the first monomial is used, and the other
   *      variables are ignored.
   * If a monomial occurs twice, then one of them will be used (which one is left undefined)
   */

  const MatrixOrNull * IM2_Matrix_monomials(const M2_arrayint vars, const Matrix *M); /* drg: connected rawMonomials*/

  const Matrix * IM2_Matrix_initial(int nparts, const Matrix *M); /* drg: connected rawInitial*/

  const M2_arrayint IM2_Matrix_elim_vars(int nparts, const Matrix *M); /* drg: connected rawEliminateVariables*/

  const M2_arrayint IM2_Matrix_keep_vars(int nparts, const Matrix *M); /* drg: connected rawKeepVariables*/

  Matrix_int_pair * IM2_Matrix_divide_by_var(const Matrix *M, int var, int maxdegree); /* drg: connected rawDivideByVariable*/
  /* If M = [v1, ..., vn], and x = 'var'th variable in the ring, 
     return the matrix [w1,...,wn], where wi * x^(ai) = vi,
     and wi is not divisible by x, or ai = maxdegree, 
     and the integer which is the maximum of the ai's.
     QUESTION: what rings should this work over?
  */

  Matrix_pair_OrNull * rawTopCoefficients(const Matrix *M); /* connected to rawTopCoefficients */
  /* Returns a pair of matrices: the first is a list of monomials (of form var^exp),
     and the second has the same row space as M.  For each column, find the smallest 
     index variable, var,  which occurs, and exp, the largest degree to which it occurs
     in that column.  Place var^exp in the first matrix.
     Place the coeff of var^exp (a vector) into the second matrix.
     If the ring is not a polynomial ring, an error is given, and Null is returned.
  */

  M2_arrayint IM2_Matrix_min_leadterms(const Matrix *M, M2_arrayint vars); /* TODO */

  const MatrixOrNull * IM2_Matrix_auto_reduce(const Matrix *M); /* TODO */
  
  const MatrixOrNull * IM2_Matrix_reduce(const Matrix *M, const Matrix *N); /* TODO */

  const MatrixOrNull * IM2_Matrix_reduce_by_ideal(const Matrix *M, const Matrix *N); /* TODO */

  /* Routines when considering matrices as modules of some sort */

  const MatrixOrNull * rawModuleTensor(const Matrix *M,
				       const Matrix *N); /* connected rawModuleTensor */

  const MatrixOrNull * rawBasis(const Matrix *M,
				M2_arrayint lo_degree, /* possibly length 0 */
				M2_arrayint hi_degree,
				M2_arrayint wt,
				M2_arrayint vars,
				M2_bool do_truncation,
				int limit); /* connect to rawBasis */
  /* Yields a monomial basis of part of the graded R-module cokernel(M).
   * Returns a matrix of monomials which maps to the target of M, such that
   *  (i) The image spans the sum of M_i, for lo_degree <= i <= hi_degree 
   *       where M_i is the degree i piece of M.
   * Notes:
   *  -- 'vars' is a list of variables.  The entries of the result will only involve
   *     these variables.
   *  -- 'wt' should be a list of integers of length <= number of degrees,
   *     with the property that each variable in 'vars' has (its degree) dot wt > 0.
   *  -- if lo_degree has length 0, then it is assumed to be -infinity
   *  -- if hi_degree has length 0, then it is assumed to be infinity
   *  -- in either of these cases, or if lo_degree is not equal to hi_degree, then
   *     the degree ring of R must have one variable.
   *  -- if limit >= 0, then only the first 'limit' monomials are placed into the result.
   *  -- if do_truncation is set, then monomials of degree higher than hi_degree will be
   *     placed into .
   * 
   * If R is a quotient ring, then the monomial order had better be a product order
   * such that the first block (or blocks) consists of the variables in 'vars'.
   * 
   */

  int IM2_Matrix_dimension(const Matrix *M); /* TODO */

  const RingElementOrNull * IM2_Matrix_Hilbert(const Matrix *M); /* drg: connected rawHilbert*/
  /* This routine computes the numerator of the Hilbert series
     for coker leadterms(M), using the degrees of the rows of M. 
     NULL is returned if the ring is not appropriate for
     computing Hilbert series, or the computation was interrupted. */

  /**************************************************/
  /**** RingMap routines ****************************/
  /**************************************************/
  /* My plan, Dan, is to make changes to how ring maps are
     constructed, in the case when we have rings over polynomials
     rings (including galois field bases) */

  const Ring * IM2_RingMap_target(const RingMap *F); /* drg: connected rawTarget*/

  const M2_string IM2_RingMap_to_string(const RingMap *F); /* drg: connected */

  const unsigned long int IM2_RingMap_hash(const RingMap *F); /* TODO */ /* drg: waiting, returning 0 */

  const M2_bool IM2_RingMap_is_equal(const RingMap*, const RingMap*); /* drg: connected === */

  const RingMap * IM2_RingMap_make(const Matrix *M, const Ring *base); /* TODO */

  const RingMap * IM2_RingMap_make1(const Matrix *M); /* drg: connected rawRingMap */
  /* WARNING: I want to change the interface to this routine */

  const RingElementOrNull * IM2_RingMap_eval_ringelem(const RingMap *F, 
						      const RingElement *a); /* drg: connected rawRingMapEval*/

  const MatrixOrNull * IM2_RingMap_eval_matrix(const RingMap *F, 
					       const FreeModule *newTarget,
					       const Matrix *M); /* drg: connected rawRingMapEval*/

  const RingElement *IM2_RingElement_promote(const Ring *S, const RingElement *f); /* drg: connected rawPromote*/

  const RingElement *IM2_RingElement_lift(const Ring *S, const RingElement *f); /* drg: connected rawLift*/

    /* Is this documentation correct for promote and lift?
       We have several ways of moving from one ring to the next:
       R ---> R[x1..xn]
       R ---> R/I
       R ---> frac R
       Z/p[x]/F(x) ---> GF(p,n)
       R ---> local(R,I)    (much later...)
       
       Both of the following routines assume that S ---> 'this'
       is one of these construction steps.  Promote takes an element of
       S, and maps it into 'this', while lift goes the other way.
    */

  /**************************************************/
  /**** MutableMatrix routines **********************/
  /**************************************************/

  MutableMatrix * IM2_MutableMatrix_identity(const Ring *R, 
					     int nrows, 
					     M2_bool prefer_dense); /* drg: connected rawMutableIdentity, OK*/

  MutableMatrix * IM2_MutableMatrix_make(const Ring *R,
					 int nrows,
					 int ncols,
					 M2_bool prefer_dense); /* drg: connected rawMutableMatrix, OK */

  MutableMatrix * IM2_MutableMatrix_from_matrix(const Matrix *M, M2_bool prefer_dense); /* drg: connected rawMutableMatrix, OK*/


  const Matrix * IM2_MutableMatrix_to_matrix(const MutableMatrix *M); /* drg: connected rawMatrix, OK*/

  const M2_string IM2_MutableMatrix_to_string(const MutableMatrix *M); /* drg: connected toString, OK */

  unsigned long  IM2_MutableMatrix_hash(const MutableMatrix *M); /* TODO */ /* drg: connected to "hash", returning 0 */

  int IM2_MutableMatrix_n_rows(const MutableMatrix *M);	/* drg: connected rawNumberOfRows, OK */

  int IM2_MutableMatrix_n_cols(const MutableMatrix *M);	/* drg: connected rawNumberOfColumns, OK */

  MutableMatrixOrNull * IM2_MutableMatrix_get_row_change(MutableMatrix *M); /* drg: connected rawRowChange, OK*/

  MutableMatrixOrNull * IM2_MutableMatrix_get_col_change(MutableMatrix *M); /* drg: connected rawColumnChange, OK*/

  M2_bool IM2_MutableMatrix_set_row_change(MutableMatrix *M,
					   MutableMatrix *rowChange); /* drg: connected rawRowChange, OK*/
  /* Returns false, if rowChange is not suitable 
     (i.e. has the wrong ring, or wrong number of columns) */

  M2_bool IM2_MutableMatrix_set_col_change(MutableMatrix *M,
					   MutableMatrix *colChange); /* drg: connected rawColumnChange, OK*/
  /* Returns false, if rowChange is not suitable 
     (i.e. has the wrong ring, or wrong number of columns) */


  const RingElementOrNull * IM2_MutableMatrix_get_entry(const MutableMatrix *M, 
							int r, int c); /* drg: connected rawMatrixEntry, OK*/

  /* Each of these routines returns false if there was an error. */

  M2_bool IM2_MutableMatrix_set_entry(MutableMatrix *M, int r, int c, const RingElement *a); /* drg: connected rawMatrixEntry, OK */

  M2_bool IM2_MutableMatrix_row_swap(MutableMatrix *M, int i, int j); /* drg: connected rawMatrixRowSwap, OK */

  M2_bool IM2_MutableMatrix_column_swap(MutableMatrix *M, int i, int j); /* drg: connected rawMatrixColSwap, OK*/

  M2_bool IM2_MutableMatrix_row_operation(MutableMatrix *M, 
					  int i, 
					  const RingElement *r, 
					  int j,
					  M2_bool opposite_mult); /* drg: connected rawMatrixRowChange, OK */
  /* row(i) <- row(i) + r * row(j), returns false if matrix is 
     immutable, or rows are out of bounds */

  M2_bool IM2_MutableMatrix_column_operation(MutableMatrix *M, 
					     int i, 
					     const RingElement *r, 
					     int j,
					     M2_bool opposite_mult); /* drg: connected rawMatrixColChange, OK*/
  /* column(i) <- column(i) + r * column(j), returns false if matrix is 
     immutable, or columns are out of bounds */

  M2_bool IM2_MutableMatrix_row_scale(MutableMatrix *M, 
				      const RingElement *r, 
				      int i, 
				      M2_bool opposite_mult); /* drg: connected rawMatrixRowScale, OK*/
  /* row(i) <- r * row(i), returns false if matrix is immutable
     or row is out of bounds */

  M2_bool IM2_MutableMatrix_column_scale(MutableMatrix *M, 
					 const RingElement *r, 
					 int i, 
					 M2_bool opposite_mult); /* drg: connected rawMatrixColumnScale, OK */
  /* column(i) <- r * column(i), returns false if matrix is immutable
     or row is out of bounds */

  M2_bool IM2_MutableMatrix_insert_columns(MutableMatrix *M, int i, int n_to_add); /* connected to rawInsertColumns, OK */
  /* Insert n_to_add columns directly BEFORE column i. */

  M2_bool IM2_MutableMatrix_insert_rows(MutableMatrix *M, int i, int n_to_add); /* connected to rawInsertRows, OK */
  /* Insert n_to_add rows directly BEFORE row i. */

  M2_bool IM2_MutableMatrix_delete_columns(MutableMatrix *M, int i, int j); /* connected to rawDeleteColumns, OK */
  /* Delete columns i .. j from M */

  M2_bool IM2_MutableMatrix_delete_rows(MutableMatrix *M, int i, int j); /* connected to rawDeleteRows, OK  */
  /* Delete rows i .. j from M */


  M2_bool IM2_MutableMatrix_column_2by2(MutableMatrix *M,
					int c1, int c2, 
					const RingElement *a1, const RingElement *a2,
					const RingElement *b1, const RingElement *b2,
					M2_bool opposite_mult);	/* connected to rawMatrixColumnOperation2, OK */
  /* column(c1) <- a1 * column(c1) + a2 * column(c2)
     column(c2) <- b1 * column(c1) + b2 * column(c2)
  */

  M2_bool IM2_MutableMatrix_row_2by2(MutableMatrix *M,
				     int r1, int r2, 
				     const RingElement *a1, const RingElement *a2,
				     const RingElement *b1, const RingElement *b2,
				     M2_bool opposite_mult); /* connected to rawMatrixRowOperation2, OK */
  /* row(r1) <- a1 * row(r1) + a2 * row(r2)
     row(r2) <- b1 * row(r1) + b2 * row(r2)
  */

  M2_bool IM2_MutableMatrix_sort_columns(MutableMatrix *M, int lo, int hi); /* connected to rawSortColumns2, OK */
  /* Returns false if M is not mutable, or lo, or hi are out of range */

  M2_bool IM2_MutableMatrix_row_permute(MutableMatrix *M,
					int start, 
					M2_arrayint perm); /* connected to rawPermuteRows, OK */
  /* if perm = [p0 .. pr], then row(start + i) --> row(start + pi), and
     all other rows are unchanged.  p0 .. pr should be a permutation of 0..r */

  M2_bool IM2_MutableMatrix_column_permute(MutableMatrix *M,
					   int start, 
					   M2_arrayint perm); /* connected to rawPermuteColumns, OK */
  /* if perm = [p0 .. pr], then column(start + i) --> column(start + pi), and
     all other rows are unchanged.  p0 .. pr should be a permutation of 0..r */

  const RingElement * IM2_Matrix_dot_product(const MutableMatrix *M, int c1, int c2); /* connect to rawColumnDotProduct, OK */
  /* Return the dot product of columns c1 and c2 of the matrix M.  If either c1 or c2 is
     out of range, 0 is returned. */

  const M2_bool IM2_MutableMatrix_is_zero(const MutableMatrix *M); /* drg: connected rawIsZero, OK */

  const M2_bool IM2_MutableMatrix_is_equal(const MutableMatrix *M, 
					   const MutableMatrix *N); /* drg: connected to rawIsEqual for use with ==, not connected to '===', OK */
  /* This checks that the entries of M,N are the same */

  MutableMatrix * IM2_MutableMatrix_copy(MutableMatrix *M, M2_bool prefer_dense); /* connected to rawMutableMatrix, OK */

     /* DAN DAN DAN : routines below here for mutable matrices not really connected yet */

  M2_bool IM2_MutableMatrix_set_values(MutableMatrix *M, 
				       M2_arrayint rows,
				       M2_arrayint cols,
				       RingElement_array *values);
  /* Given three arrays of the same length, 'rows', 'cols', 'values', set the
     corresponding values of M.  If any elements are out of range, ignore those
     triples.  If the type of ring element is not valid, or the sizes of the
     matrices do not match, return false. */

  MutableMatrixOrNull * IM2_MutableMatrix_add(const MutableMatrix *M, const MutableMatrix *N); /* drg: connected + */
    /* If the sizes do not match, then NULL is returned.  If they do match,
       the addition is performed.  If the targets are not equal, the target 
       of the result is set to have each degree zero.  Similarly with the
       source, and also with the degree of the matrix. */

  MutableMatrixOrNull * IM2_MutableMatrix_subtract(const MutableMatrix *M, const MutableMatrix *N); /* drg: connected - */
    /* If the sizes do not match, then NULL is returned.  If they do match,
       the addition is performed.  If the targets are not equal, the target 
       of the result is set to have each degree zero.  Similarly with the
       source, and also with the degree of the matrix. */

  MutableMatrix * IM2_MutableMatrix_negate(const MutableMatrix *M); /* drg: connected - */

  MutableMatrixOrNull * IM2_MutableMatrix_mult(const MutableMatrix *M, 
					       const MutableMatrix *N, 
					       M2_bool opposite_mult); /* drg: connected * */
    /* If the sizes do not match, then NULL is returned.  If they do match,
       the multiplication is performed, and the source and target are taken from N,M
       respectively.  The degree of the result is the sum of the two degrees */

  MutableMatrixOrNull * IM2_MutableMatrix_scalar_mult(const RingElement *f,
						      const MutableMatrix *M, 
						      M2_bool opposite_mult); /* drg: connected * */

  MutableMatrixOrNull * IM2_MutableMatrix_submatrix(const MutableMatrix *M,
						    const M2_arrayint rows,
						    const M2_arrayint cols); /* drg: connected rawSubmatrix*/
  
  MutableMatrixOrNull * IM2_MutableMatrix_submatrix1(const MutableMatrix *M,
						     const M2_arrayint cols); /* drg: connected rawSubmatrix*/


  /***************************************************
   ***** Lapack routines for dense mutable matrices **
   ***************************************************/

  /* Each of the following routines accepts honest MutableMatrix arguments,
     and returns false if there is an error.  The return values are placed into
     some of the (already existing) parameters of the routine */

  M2_bool rawSolve(MutableMatrix *A,
		   MutableMatrix *b,
		   MutableMatrix *x);

  M2_bool rawLU(MutableMatrix *A,
		MutableMatrix *L,
		MutableMatrix *U,
		MutableMatrix *P);
  /*
   */

  M2_bool rawEigenvalues(MutableMatrix *A,
			 MutableMatrix *eigenvalues,
			 M2_bool is_symm_or_hermitian);
  /*
   */

  M2_bool rawEigenvectors(MutableMatrix *A,
			  MutableMatrix *eigenvalues,
			  MutableMatrix *eigenvectors,
			  M2_bool is_symm_or_hermitian);
  /*
   */

  M2_bool rawSVD(MutableMatrix *A,
		 MutableMatrix *Sigma,
		 MutableMatrix *U,
		 MutableMatrix *VT,
		 M2_bool use_divide_and_conquer);
  /* 
   */

  M2_bool rawLeastSquares(MutableMatrix *A, 
			  MutableMatrix *b, 
			  MutableMatrix *x, /* return value: argument modified */
			  M2_bool assume_full_rank);
  /* Case 1: A is a dense matrix over RR.  Then so are b,x.
     Case 2: A is a dense matrix over CC.  Then so are b,x. */



  /**************************************************/
  /**** Monomial ideal routines *********************/
  /**************************************************/

  /* A MonomialIdeal is an immutable object, having a base ring.
     The base ring should be a polynomial ring or quotient of one.
     In case a quotient is given, the monomial ideal is considered
     to be in the commutative quotient ring whose quotient elements
     are the lead terms of the quotient polynomials.
     Each monomial ideal is represented by its minimal generators only */

  const MonomialIdealOrNull *IM2_MonomialIdeal_make(const Matrix *m, int n); /* drg: connected rawMonomialIdeal*/
  /* Given a matrix 'm' over an allowed base ring (as above), create the
     monomial ideal consisting of all of the lead monomials of the columns
     of 'm' which have their lead term in row 'n'.  If 'n' is out of range,
     or the ring is not allowed, NULL is returned. */

  const Matrix *IM2_MonomialIdeal_to_matrix(const MonomialIdeal *I); /* drg: connected rawMonomialIdealToMatrix */
  /* Return a one row matrix over the base ring of I consisting
     of the monomials in I */

  M2_string IM2_MonomialIdeal_to_string(const MonomialIdeal *I); /* TODO */

  unsigned long IM2_MonomialIdeal_hash(const MonomialIdeal *I);/* TODO */

  M2_bool IM2_MonomialIdeal_is_equal(const MonomialIdeal *I1, 
				     const MonomialIdeal *I2); /* drg: connected === */

  int IM2_MonomialIdeal_n_gens(const MonomialIdeal *I); /* drg: connected rawNumgens*/
  /* Returns the number of minimal generators of I */

  
  const MonomialIdeal *rawRadicalMonomialIdeal(const MonomialIdeal *I); /* drg: connected rawRadicalMonomialIdeal*/
  /* The radical of the monomial ideal, that is, the monomial ideal 
     generated by the square-free parts of the each monomial of I. */

  const MonomialIdealOrNull *IM2_MonomialIdeal_add(const MonomialIdeal *I, 
						   const MonomialIdeal *J); /* drg: connected + */

  const MonomialIdealOrNull *IM2_MonomialIdeal_product(const MonomialIdeal *I, 
						       const MonomialIdeal *J); /* drg: connected * */

  const MonomialIdealOrNull *IM2_MonomialIdeal_intersect(const MonomialIdeal *I, 
							 const MonomialIdeal *J); /* drg: connected rawIntersect*/
  
  const MonomialIdeal *rawColonMonomialIdeal1(const MonomialIdeal *I, 
						   const Monomial *a); /* drg: connected rawColon*/
  /* If I = (m1, ..., mr),
     Form the monomial ideal (I : a) = (m1:a, ..., mr:a) */

  const MonomialIdealOrNull *rawColonMonomialIdeal2(const MonomialIdeal *I, 
						      const MonomialIdeal *J); /* drg: connected rawColon*/
  /* Form the monomial ideal (I : J) = intersect(I:m1, ..., I:mr),
     where J = (m1,...,mr) */

  const MonomialIdeal *rawSaturateMonomialIdeal1(const MonomialIdeal *I, 
					      const Monomial *a); /* drg: connected rawSaturateMonomialIdeal*/
  /* Form I:a^\infty.  IE, set every variable which occurs in 'a' to '1' in
     every generator of I. */

  const MonomialIdealOrNull *rawSaturateMonomialIdeal2(const MonomialIdeal *I,
						   const MonomialIdeal *J); /* drg: connected rawSaturateMonomialIdeal*/
  /* Form (I : J^\infty) = intersect(I:m1^\infty, ..., I:mr^\infty),
     where J = (m1,...,mr). */

  const MonomialIdeal *IM2_MonomialIdeal_borel(const MonomialIdeal *I); /* drg: connected rawStronglyStableClosure*/
  /* This should really be named: ..._strongly_stable.
     Form the smallest monomial ideal J containing I which is strongly stable,
     that is that:
     If m is in J, then p_ij(m) is in J,
     where p_ij(m) = x_j * (m/x_i), for j <= i, s.t. x_i | m. (Here the
     variables in the ring are x1, ..., xn */

  M2_bool IM2_MonomialIdeal_is_borel(const MonomialIdeal *I); /* drg: connected rawIsStronglyStable*/
  /* This should really be named: ..._is_strongly_stable.
     Determine if I is strongly stable (see IM2_MonomialIdeal_borel for the
     definition of strongly stable */

  int IM2_MonomialIdeal_codim(const MonomialIdeal *I); /* drg: connected rawCodimension*/
  /* Return the codimension of I IN THE AMBIENT POLYNOMIAL RING. */

  const MonomialIdeal *IM2_MonomialIdeal_assprimes(const MonomialIdeal *I); /* drg: connected rawAssociatedPrimes*/
  /* RENAME THIS ROUTINE */
  /* Return a monomial ideal whose generators correspond to the 
     minimal primes of I of maximal dimension.  If a minimal prime
     of I has the form (x_i1, ..., x_ir), then the corresponding monomial
     is x1 ... xn /(x_i1 ... x_ir), i.e. the complement of the support of
     the monomial generates the monomial minimal prime. */

  const RingElementOrNull * IM2_MonomialIdeal_Hilbert(const MonomialIdeal *I); /* Dan: please connect */
  /* This routine computes the numerator of the Hilbert series
     for coker I.  NULL is returned if the ring is not appropriate for
     computing Hilbert series, or the computation was interrupted. */

#if 0
  Monomial *IM2_MonomialIdeal_remove(MonomialIdeal *I); /* CAN WE REMOVE THIS?? */
  MonomialIdeal *IM2_MonomialIdeal_copy(MonomialIdeal *I); /* and THIS? */
#endif

  /**************************************************/
  /**** Groebner basis objects **********************/
  /**************************************************/
#if 0
  GroebnerBasis *rawGBMake(const FreeModule *F, const FreeModule *Fsyz);

  void rawGBAdd(GroebnerBasis *G, const Matrix *M, const Matrix *Msyz);
  /* Add the columns of (M,Msyz) to the GB. */

  GroebnerBasis *rawGBMinimalize(GroebnerBasis *G);
  

  rawGBRemainder(const GroebnerBasis *G, const Matrix *M);


  const MatrixOrNull *rawGBGetMatrix(GroebnerBasis *G);
  /* Get the matrix whose columns are the GB elements being used */

  const MatrixOrNull *rawGBGetLeadTerms(GroebnerBasis *G, int nparts);
  /* Get the matrix of lead terms of rawGBGetMatrix(G) */

  const MatrixOrNull *rawGBChangeOfBasis(GroebnerBasis *G);
  /* Yields the change of basis matrix from the Groebner basis to
     the original generators, at least if n_rows_to_keep was set
     when creating the GB computation.  This matrix, after the 
     computation has run to completion, should satisfy:
     (original matrix) = (GB matrix) * (change of basis matrix). */

  const MatrixOrNull *rawGBMatrixRemainder(GroebnerBasis *G, 
					   const Matrix *m); /* drg: connected rawGBMatrixRemainder */

  void rawGBMatrixLift(GroebnerBasis *G,
		       const Matrix *m,
		       MatrixOrNull **result_remainder,
		       MatrixOrNull **result_quotient
		       ); /* Dan: please connect */

  int rawGBMatrixContains(GroebnerBasis *G,
			  const Matrix *m); /* Dan: please connect */
  /* Returns the index of the first column of m which does not reduce to 0, if any,
     otherwise -1 is returned. */
#endif

  /**************************************************/
  /**** Groebner basis and resolution routines ******/
  /**************************************************/

enum ComputationStatusCode
{
#include "statuscodes.h"
};

enum StrategyValues
  {
    STRATEGY_LONGPOLYNOMIALS = 1,
    STRATEGY_SORT = 2,
    STRATEGY_USE_HILB = 4
  };

enum Algorithms
  {
    GB_polyring_field = 1, /* The main GB algorithm to use */
    GB_polyring_field_homog = 2
  };

enum gbTraceValues
  {
    /* The printlevel flags */
    PRINT_SPAIR_TRACKING=1024
  };
  
  ComputationOrNull* IM2_Computation_set_stop(Computation *G,          
				     M2_bool always_stop,       /* 1 */
				     M2_bool stop_after_degree, /* 2 */
				     M2_arrayint degree_limit,
				     int basis_element_limit,   /* 3 */
				     int syzygy_limit,          /* 4 */
				     int pair_limit,            /* 5 */
				     int codim_limit,           /* 6 */
				     int subring_limit,         /* 7 */
				     M2_bool just_min_gens,     /* 8 */
				     M2_arrayint length_limit   /* 9 */  /* not for GB */
				     ); /* drg: connected rawGBSetStop */

  /* Each of these routines can return NULL, because of errors */

  void rawStartComputation(Computation *G);
  /* start or continue the computation */

  enum ComputationStatusCode rawStatus1(Computation *C);

  int rawStatus2(Computation *C);
  /* The computation is complete up to and including this degree.
     The exact meaning of 'degree' is computation specific */

  const M2_string IM2_GB_to_string(Computation *C); /* drg: connected, in actors4.d */

  int IM2_GB_hash(const Computation *C); /* drg: connected, in basic.d */

  extern int gbTrace;

  /*******************************************
   * Computation routines for Groebner bases *
   *******************************************/

  ComputationOrNull *IM2_GB_make(const Matrix *m,
				 M2_bool collect_syz,
				 int n_rows_to_keep,
				 M2_arrayint gb_weights,
				 M2_bool use_max_degree,
				 int max_degree,
				 int algorithm,
				 int strategy); /* drg: connected rawGB */

  ComputationOrNull *IM2_GB_force(const Matrix *m,
				  const Matrix *gb,
				  const Matrix *change); /* drg: connected rawGBForce */
  
  ComputationOrNull *IM2_GB_set_hilbert_function(Computation *G,
						 const RingElement *h); /* drg: connected rawGBSetHilbertFunction */


  const MatrixOrNull *rawGBGetMatrix(Computation *C);
  /* Get the minimal, auto-reduced GB of a GB computation.
     Each call to this will produce a different raw matrix */

  const MatrixOrNull *rawGBGetLeadTerms(Computation *G, int nparts);

  const MatrixOrNull *rawGBMinimalGenerators(Computation *C);
  /* Yields a matrix whose columns form a minimal generating set
     for the ideal or submodule, as computed so far.  In the
     inhomogeneous case, this yields a generating set which is
     sometimes smaller than the entire Groebner basis. */

  const MatrixOrNull *rawGBChangeOfBasis(Computation *C);
  /* Yields the change of basis matrix from the Groebner basis to
     the original generators, at least if n_rows_to_keep was set
     when creating the GB computation.  This matrix, after the 
     computation has run to completion, should satisfy:
     (original matrix) = (GB matrix) * (change of basis matrix). */

  const MatrixOrNull *rawGBSyzygies(Computation *C);  
  /* Yields a matrix containing the syzygies computed so far
     via the GB computation C, assuming that 'collect_syz' was
     set when the computation was created.  If 'n_rows_to_keep' was
     set to a non-negative integer, then only that many rows of each
     syzygy are kept. */

  const MatrixOrNull *rawGBMatrixRemainder(Computation *G, 
						const Matrix *m); /* drg: connected rawGBMatrixRemainder */

  void IM2_GB_matrix_lift(Computation *G,
			  const Matrix *m,
			  MatrixOrNull **result_remainder,
			  MatrixOrNull **result_quotient
			  ); /* drg: connected rawGBMatrixLift */

  int IM2_GB_contains(Computation *G, 
		      const Matrix *m); /* drg: connected rawGBContains */


  /*******************************************
   * Computation routines for Resolutions ****
   *******************************************/

  /* LongPolynomial, Sort, Primary, Inhomogeneous, Homogeneous */
  /* Res: SortStrategy, 0, 1, 2, 3 ?? */

  ComputationOrNull *IM2_res_make(const Matrix *m,
				  M2_bool resolve_cokernel,
				  int max_level,
				  M2_bool use_max_slanted_degree,
				  int max_slanted_degree,
				  int algorithm,
				  int strategy /* drg: connected rawResolution */
				  );

  const MatrixOrNull *rawResolutionGetMatrix(Computation *G,int level); 
  /* rawResolutionGetMatrix */

  const FreeModuleOrNull *rawResolutionGetFree(Computation *G, int level);
    /*drg: connected rawResolutionGetFree*/

  const M2_arrayint rawResolutionBetti(Computation *G,
				 int type); /* drg: connected rawGBBetti */
  /* type:
	 0: minimal betti numbers,
	 1: non-minimal betti numbers (skeleton size, or size of GB's).
	 2: number of S-pairs remaining to consider
	 3: number of monomials in polynomials at this slot
     Not all of these may be accessible with all algorithms.  If not available, 
     A betti diagram with all -1's is displayed.
  */

  /* I don't know what this is supposed to do (mike) */
  int IM2_Resolution_status(Computation *G,
		    int * complete_up_through_this_degree,
		    int * complete_up_through_this_level); /* drg: TODO */
  /* -1: error condition, and the error message is set.
     0: not made, and in fact it won't ever be done...
     1: not started,
     2: started, 
     3: stopped because of a stopping condition
     4: finished the computation completely
  */

  enum ComputationStatusCode IM2_Resolution_status_level(Computation *G, 
							 int level, 
							 M2_bool minimize,
							 int * complete_up_through_this_degree); 
  /* WARNING: 'minimize' is completely ignored, and should be removed from the interface */
  /* drg: connected rawResolutionStatusLevel */
  
  /**************************************************/
  /**** Fraction free LU decomposition **************/
  /**************************************************/

  M2_arrayint_OrNull IM2_FF_LU(MutableMatrix *M); /* Dan, please connect */
  /* Replace M by a column echelon form.  No fractions are generated, but the
     base ring should be a domain.
     If M has a column change of basis matrix attached, it will be modified accordingly. 
  */

  /**************************************************/
  /**** LLL bases ***********************************/
  /**************************************************/
  
  M2_bool IM2_LLL(MutableMatrix *M, const RingElement *threshold); /* Dan, please connect */
  /* Given a mutable matrix M over ZZ, and a rational number threshold, 1/4 < threshold <= 1,
     modify M so that it forms a Lenstra-Lenstra-Lovasz
     basis of the image of (the original) M.  ASSUMPTION: the columns of M are already a a basis for the 
     lattice.  The algorithm used is that in Cohen's book on computational algebraic number
     theory, BUT: beware of the typos in the algorithm!
     If there is any error (interupted, M or threshold not the correct kind), then false
     is returned, and LLL is set to 0.
     If M has a column change of basis matrix attached, it will be modified accordingly. 
  */

  M2_bool IM2_SmithNormalForm(MutableMatrix *M);
  /* Given a mutable matrix over ZZ, compute the Smith normal form for M. (replaces
     M with this normal form.
     Currently the algorithm used makes computing the change of basis matrices
     difficult (we use mod D arithmetic, where D is the determinant). 
     If there is an error, then an error is flagged and false is returned.
  */

  M2_bool IM2_HermiteNormalForm(MutableMatrix *M);
  /* Given a mutable matrix over ZZ, compute the Hermite normal form for M. (replaces
     M with this normal form.
     Currently the algorithm used makes computing the change of basis matrices
     difficult (we use mod D arithmetic, where D is the determinant). 
     If there is an error, then an error is flagged and false is returned.
  */

  /**************************************************/
  /**** Factory and libfac routines *****************/
  /**************************************************/

  const RingElementOrNull *rawGCDRingElement(const RingElement *f, const RingElement *g); /* connected to rawGCD */
  const RingElementOrNull *rawPseudoRemainder(const RingElement *f, const RingElement *g); /* connected to rawPseudoRemainder */
  void rawFactor(const RingElement *f, 
		 RingElement_array_OrNull **result_factors, 
		 M2_arrayint_OrNull *result_powers); /* connected to rawFactor  */
  M2_arrayint_OrNull rawIdealReorder(const Matrix *M);/* connected to rawIdealReorder */
  Matrix_array_OrNull * rawCharSeries(const Matrix *M);/* connected to rawCharSeries */

#if defined(__cplusplus)
}
#endif

#endif

/*
// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e "
// End:
*/
