// Copyright 1995 Michael E. Stillman

#ifndef _ring_hh_
#define _ring_hh_

#include "ringelem.hpp"
#include "monoid.hpp"

///// Ring Hierarchy ///////////////////////////////////

class ZZ;
class QQ;
class RR;
class Z_mod;
class GF;
class FractionField;
class PolynomialRing;
class SkewPolynomialRing;
class SchurRing;
class WeylAlgebra;
class SolvableAlgebra;

class FreeModule;
class RingMap;

class gbvectorHeap;
class gbvector;

class Ring : public mutable_object
{
protected:
  int P;
  int _nvars;
  int _totalvars;		// The total number of variables, including all base rings
                                // BUT: not including basic fields.
  const PolynomialRing *degree_ring;

  ring_elem _zero_divisor;
  bool _isfield;		// true means yes, or declared yes.
				// If a zero divisor is found, isfield is set to false.

  ring_elem zeroV;              // Three generally useful values in a ring.
  ring_elem oneV;
  ring_elem minus_oneV;

  void initialize_ring(int charac, 
		       int nvars, 
		       int totalvars, 
		       const PolynomialRing *DR = 0);
  Ring() {}
public:
  virtual ~Ring();

  ////////////////////////
  // Ring informational //
  ////////////////////////

  int charac() const { return P; }
  int n_vars() const { return _nvars; } // This will be 0 except for frac fields and poly rings.
  int total_n_vars() const { return _totalvars; }
  virtual int n_fraction_vars() const { return 0; }
  // The ultimate number of fraction field variables.

  const Monoid * degree_monoid() const;
  const PolynomialRing *get_degree_ring() const { return degree_ring; }

  virtual bool is_basic_ring() const { return true; } // The default is to be a basic ring.
  virtual bool is_ZZ() const { return false; }
  virtual bool is_QQ() const { return false; }
  virtual bool is_fraction_field() const { return false; }

  virtual bool is_fraction_poly_ring() const { return false; }
  // returns true if this ring has fractions.  This includes
  // polynomial rings over QQ, polynomial rings over fraction fields,
  // fraction rings, and localizations.
  // If this returns true, then 'get_denominator_ring()' returns non-NULL value.
  // 

  virtual bool is_poly_ring() const { return false; }
  // Returns true if this is a polynomial ring, possibly with fractions
  // and possibly with quotient ideal, and possibly with non-commutative
  // multiplication.  Equivalent to (cast_to_PolynomialRing() != 0).

  virtual bool is_commutative_ring() const { return true; }
  // Returns true iff this is a commutative ring.

  virtual bool is_quotient_ring() const { return false; }
  // Returns true if this is a polynomial ring, (possibly with fractions),
  // with a quotient ideal.  This could be a non-commutative ring
  // with skew-commutative, Weyl algebra, or other multiplication.

  virtual bool is_weyl_algebra() const { return false; }
  // Returns true if this is a polynomial ring (possibly with quotient)
  // (possibly with ZZ fractions, or other commutative fractions)
  // but with Weyl algebra multiplication on some of the variables.

  virtual bool is_skew_commutative_ring() const { return false; }
  // Returns true if this is a polynomial ring (possibly with quotient)
  // (possibly with ZZ fractions, or other commutative fractions)
  // but with some variables anti-commuting.

  virtual bool is_solvable_algebra() const { return false; }

  virtual bool is_pid() const = 0;
  virtual bool has_gcd() const = 0;

  virtual bool is_graded() const { return true; }
  // Is this ring graded, with the given grading?
  // ZZ, QQ, ZZ/p, GF, RR, ... are all graded.
  // polynomial rings are graded
  // Weyl algebras can be graded or not
  // quotient polynomial rings can be graded or not.

  bool is_field() const;
  void declare_field();
  ring_elem get_zero_divisor() const;

  typedef enum {COEFF_ZZ, COEFF_QQ, COEFF_BASIC} CoefficientType;
  virtual  CoefficientType coefficient_type() const { return COEFF_BASIC; }
  // What the ultimate coefficient type is.  ZZ, QQ, finite fields return these 
  // three values.  Fraction fields return their ultimate value, as do poly rings.

  virtual const Ring *get_ambient_ring() const { return this; }
  virtual const Ring *get_denominator_ring() const { return 0; }

  ///////////////////////////////////
  // Casting up the ring hierarchy //
  ///////////////////////////////////
  virtual const ZZ * cast_to_ZZ() const         { return 0; }
  virtual       ZZ * cast_to_ZZ()               { return 0; }
  virtual const QQ * cast_to_QQ() const         { return 0; }
  virtual       QQ * cast_to_QQ()               { return 0; }
  virtual const RR * cast_to_RR() const         { return 0; }
  virtual       RR * cast_to_RR()               { return 0; }
  virtual const Z_mod * cast_to_Z_mod() const         { return 0; }
  virtual       Z_mod * cast_to_Z_mod()               { return 0; }
  virtual const GF * cast_to_GF() const         { return 0; }
  virtual       GF * cast_to_GF()               { return 0; }
  virtual const PolynomialRing * cast_to_PolynomialRing()  const      { return 0; }
  virtual       PolynomialRing * cast_to_PolynomialRing()             { return 0; }
  virtual const FractionField * cast_to_FractionField() const    { return 0; }
  virtual       FractionField * cast_to_FractionField()          { return 0; }
  virtual const SchurRing * cast_to_SchurRing() const { return 0; }
  virtual       SchurRing * cast_to_SchurRing()       { return 0; }
  virtual const SkewPolynomialRing * cast_to_SkewPolynomialRing()  const      { return 0; }
  virtual       SkewPolynomialRing * cast_to_SkewPolynomialRing()             { return 0; }
  virtual const SolvableAlgebra * cast_to_SolvableAlgebra()  const      { return 0; }
  virtual       SolvableAlgebra * cast_to_SolvableAlgebra()             { return 0; }
  virtual const WeylAlgebra *cast_to_WeylAlgebra() const { return 0; }

  virtual FreeModule *make_FreeModule() const;
  virtual FreeModule *make_Schreyer_FreeModule() const;
  virtual FreeModule *make_FreeModule(int n) const;

  virtual void text_out(buffer &o) const = 0;

  //////////////////////
  // Ring arithmetic ///
  //////////////////////
  virtual int coerce_to_int(ring_elem a) const;

  ring_elem one() const { return oneV; }
  ring_elem minus_one() const { return minus_oneV; }
  ring_elem zero() const { return zeroV; }

  virtual ring_elem from_int(int n) const = 0;
  virtual ring_elem from_int(mpz_ptr n) const = 0;
  virtual ring_elem from_double(double a) const;  // The default version converts to a mpz_ptr,
				// then uses from_int.
  virtual ring_elem from_rational(mpq_ptr q) const;  
  // The default version calls from_int(0). Change it?
  virtual ring_elem from_complex(M2_CC z) const;
  // The default version calls from_int(0). Change it?
  virtual ring_elem from_BigReal(mpf_ptr a) const;  
  // The default version calls from_int(0). Change it?
  virtual ring_elem from_BigComplex(M2_BigComplex z) const;  
  // The default version calls from_int(0). Change it?

  virtual ring_elem var(int v, int n) const = 0;

  virtual ring_elem preferred_associate(ring_elem f) const;
  // Returns an invertible element c of the same ring such that c*f is the
  // preferred associate of the element f.
  // WARNING: The default implementation is for a field.

  virtual bool promote(const Ring *R, const ring_elem f, ring_elem &result) const = 0;
  virtual bool lift(const Ring *R, const ring_elem f, ring_elem &result) const = 0;

  virtual bool is_unit(const ring_elem f) const = 0;
  virtual bool is_zero(const ring_elem f) const = 0;
  virtual bool is_equal(const ring_elem f, const ring_elem g) const = 0;

  virtual ring_elem copy(const ring_elem f) const = 0;
  virtual void remove(ring_elem &f) const = 0;
  void remove_vector(vec &v) const;

          void negate_to(ring_elem &f) const;
          void add_to(ring_elem &f, ring_elem &g) const;
          void subtract_to(ring_elem &f, ring_elem &g) const;
          void mult_to(ring_elem &f, const ring_elem g) const;
  virtual ring_elem negate(const ring_elem f) const = 0;
  virtual ring_elem add(const ring_elem f, const ring_elem g) const = 0;
  virtual ring_elem subtract(const ring_elem f, const ring_elem g) const = 0;
  virtual ring_elem mult(const ring_elem f, const ring_elem g) const = 0;
  virtual ring_elem power(const ring_elem f, mpz_t n) const = 0;
  virtual ring_elem power(const ring_elem f, int n) const = 0;
  virtual ring_elem invert(const ring_elem f) const = 0;

  virtual ring_elem divide(const ring_elem f, const ring_elem g) const = 0;
  virtual ring_elem divide(const ring_elem f, const ring_elem g, ring_elem &rem) const = 0;

  virtual ring_elem remainder(const ring_elem f, const ring_elem g) const = 0;
  virtual ring_elem quotient(const ring_elem f, const ring_elem g) const = 0;
  virtual ring_elem remainderAndQuotient(const ring_elem f, const ring_elem g, 
					 ring_elem &quot) const = 0;
  // These three routines: remainder, quotient and remainderAndQuotient
  // satisfy these properties:
  // If r = remainder(f,g), q = quotient(f,g), then
  // (1) f = q*g + r
  // (2) If f is in ideal(g), then r = 0.
  // (3) If g is invertible, then r = 0, and q = f * g^(-1).
  // (4) If the ring is ZZ, then the remainder is "balanced": -[g/2] < r <= [g/2]
  // remainderAndQuotient combines remainder and quotient into one routine.

  virtual ring_elem gcd(const ring_elem f, const ring_elem g) const = 0;
  virtual ring_elem gcd_extended(const ring_elem f, const ring_elem g, 
				  ring_elem &u, ring_elem &v) const = 0;

  virtual void syzygy(const ring_elem a, const ring_elem b,
		      ring_elem &x, ring_elem &y) const = 0;
  // Constructs elements x and y in the ring s.t. ax + by = 0.  This syzygy is
  // chosen as simply as possible.  For example, over QQ, x is chosen 
  // to be positive.  The routine must handle the case when a=0, but can
  // ignore the case when b=0... (Really?)

  virtual ring_elem random() const;
  virtual ring_elem random(int homog, const int *deg) const;

  virtual void elem_text_out(buffer &o, const ring_elem f) const = 0;

  virtual ring_elem eval(const RingMap *map, const ring_elem f) const = 0;

  // Polynomial routines
  // The default implementation is for non-polynomial rings
  virtual int index_of_var(const ring_elem a) const;
  virtual M2_arrayint support(const ring_elem a) const;

  virtual ring_elem diff(ring_elem a, ring_elem b, int use_coeff) const;
  virtual bool in_subring(int n, const ring_elem a) const;
  virtual void degree_of_var(int n, const ring_elem a, int &lo, int &hi) const;
  virtual ring_elem divide_by_var(int n, int d, const ring_elem a) const;
  virtual ring_elem divide_by_expvector(const int *exp, const ring_elem a) const;

  virtual int n_terms(const ring_elem f) const;
  virtual ring_elem term(const ring_elem a, const int *m) const;
  virtual ring_elem lead_coeff(const ring_elem f) const;
  virtual ring_elem get_coeff(const ring_elem f, const int *m) const;
  virtual ring_elem get_terms(const ring_elem f, int lo, int hi) const;

  virtual ring_elem homogenize(const ring_elem f, int v, int deg, 
			       const M2_arrayint wts) const;
  virtual ring_elem homogenize(const ring_elem f, int v, const M2_arrayint wts) const;

  // Routines expecting a grading.  The default implementation
  // is that the only degree is 0.
  virtual bool is_homogeneous(const ring_elem f) const;
  virtual void degree(const ring_elem f, int *d) const;
  virtual bool multi_degree(const ring_elem f, int *d) const;
    // returns true iff f is homogeneous
  virtual int primary_degree(const ring_elem f) const;
  virtual void degree_weights(const ring_elem f, const M2_arrayint wts, int &lo, int &hi) const;

  ///////////////////////////////////////////////////////
  // Used in gbvector <--> vector/ringelem translation //
  ///////////////////////////////////////////////////////
  // Default values are provided for base rings (ZZ, ZZ/p, GF)
  // All others should redefine these routines

public:
  typedef enum { BASE, FRAC_QQ, FRAC, POLY } trans_tag;
  virtual ring_elem trans_to_ringelem(ring_elem coeff, 
				      const int *exp) const;
  virtual ring_elem trans_to_ringelem_denom(ring_elem coeff, 
					    ring_elem denom, 
					    int *exp) const;
  virtual void trans_from_ringelem(gbvectorHeap &H, 
				   ring_elem coeff, 
				   int comp, 
				   int *exp,
				   int firstvar) const;
  
  virtual trans_tag trans_type() const;

  ////////////////////////////////////////////////////////  


  //////////////////////////////////////////
  /// vector operations ////////////////////
  //////////////////////////////////////////
  // These routines all act on linked lists
  // of vecterm's, sorted by descending component.
  // We always assume that ringelem's are immutable:
  // The same value might be shared in several vecterms.
  //
  // These routines are implemented in ring-vec.cpp
  //////////////////////////////////////////
protected:
  vec new_vec() const;
  void remove_vec_node(vec n) const;

  void sort(vecterm *&f) const;
public:
  vec make_vec(int r, ring_elem a) const;
  vec copy_vec(const vecterm * v) const;
  void remove_vec(vec v) const;

  bool is_equal(const vecterm * a, const vecterm * b) const;
  bool get_entry(const vecterm * v, int r, ring_elem &result) const;
  vec sub_vector(const vecterm * v, const M2_arrayint r) const;
  int n_nonzero_terms(const vecterm * v) const;
  void elem_text_out(buffer &o, const vecterm * v) const;

  void set_entry(vec &v, int i, ring_elem r) const;
  void mult_vec_to(vec &v, const ring_elem r, bool opposite_mult) const; // multiplies v <- r * v or v * r
  void mult_row(vec &v, const ring_elem r, int i, bool opposite_mult) const;
  void add_vec_to(vec &v, vec &w) const; // v <- v+w, w is set to 0.
  void interchange_rows(vec &v, int i, int j) const;
  void vec_row_op(vec &v, int i, ring_elem r, int j, bool opposite_mult) const;

  vec mult_vec_matrix(const Matrix *m,
		      vec v,
		      bool opposite_mult) const;

  vec component_shift(int n, vec v) const;

  vec tensor_shift(int n, int m, vec v) const;

  vec tensor(const FreeModule *F, vec v, 
	     const FreeModule *G, vec w) const;


  void row2by2(vec &, int r1, int r2,
	       ring_elem a1, ring_elem a2,
	       ring_elem b1, ring_elem b2) const;

  void divide_vec_to(vec &v, const ring_elem a) const;
  void divide_row(vec &v, int r, const ring_elem a) const;
  ring_elem dot_product(const vecterm *v, const vecterm *w) const;


  // Other operations desired:
  // assemble a vector from ring elements
  // get the entries, non-zero entries.

  // negate, subtract, _to routines as well
  // mult/divide on the right
  // component shift (all components > a are incremented by b)

  // diff

  // given degrees of the components:
  //   degree

  // polynomial routines (vectors of polynomials)
  //   degree_weights
  //   is_homogeneous
  //   homogenize (2 forms).

};

#define ZERO_RINGELEM (ring_elem(reinterpret_cast<Nterm *>(0)))

#include "ZZ.hpp"
extern ZZ *globalZZ;
extern QQ *globalQQ;
extern RR *globalRR;
#endif

// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e "
// End:
