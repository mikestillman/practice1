// Copyright 2004 Michael E. Stillman

#ifndef _poly_hpp_
#define _poly_hpp_

#include "ringelem.hpp"
#include "engine.h"
#include "skew.hpp"
#include <vector>

class buffer;
class Monoid;
class Ring;
class MonomialIdeal;
class MonomialTable;
class MonomialTableZZ;

class PolynomialRing;
class PolyRing;
class PolyRingSkew;
class PolyRingWeyl;
class PolyFrac;
class PolyRingNC;
class PolyQuotient;

class GBRing;
class GBRingSkew;
class GBComputation;

#include "ring.hpp"

class PolynomialRing : public Ring
{
  bool is_graded_;
  vector<Nterm *, gc_alloc> quotient_ideal_;
  vector<gbvector *, gc_alloc> quotient_gbvectors_;
protected:
  void appendQuotientElement(Nterm *f, gbvector *g);
  void setIsGraded(bool new_val) { is_graded_ = new_val; }

  virtual ~PolynomialRing();
  PolynomialRing() {}

public:
  static PolynomialRing *create(const Ring *K, const Monoid *MF);

  static PolynomialRing *create_quotient_ring(const Matrix *M);

  const Ring *  Ncoeffs() const { return getCoefficients(); }
  const Monoid * Nmonoms() const { return getMonoid(); }
  // MY BAD: sometimes means flat coeffs, sometimes logical coeffs
  // Both Ncoeffs and Nmonoms need to be totally removed.

  virtual const Ring *getLogicalCoefficients() const = 0;
  // The logical coefficient ring of 'this'.  
  // This is either a non polynomial ring, or it is a PolyRing.

  virtual const Ring *getCoefficients() const = 0;
  // The implementation coeff ring of 'this'.  This is either a basic ring (field, ZZ), or
  // is another PolyRing.

  virtual const Monoid *getLogicalMonoid() const = 0;
  // The logical monoid of this polynomial ring.

  virtual const Monoid *getMonoid() const = 0;
  // The implementation monoid of this ring.

  virtual const PolyRing * getAmbientRing() const = 0;
  // Yields the ambient PolyRing corresponding to this polynomial ring
  // This ring has no quotients, no fractions (not even QQ), but may have
  // skew, weyl, or solvable multiplication, or even (later) be an associative
  // free algebra.

  virtual const RingOrNull *getDenominatorRing() const = 0;
  // If this ring has no denominators, NULL is returned.  Otherwise the ring which
  // implements denominators is returned.  When one asks for a denominator for elements of
  // 'this', the result value is its ring.

  virtual GBRing *get_gb_ring() const = 0;

  virtual const PolynomialRing * cast_to_PolynomialRing()  const { return this; }
  virtual       PolynomialRing * cast_to_PolynomialRing()        { return this; }

  virtual bool is_basic_ring() const { return false; }

  // Quotient ring information
  virtual bool        is_quotient_ring() const { return false; }
  virtual const MonomialTable * get_quotient_MonomialTable() const { return 0; }
  virtual const MonomialIdeal *  get_quotient_monomials() const { return 0; }
  virtual const MonomialTableZZ * get_quotient_MonomialTableZZ() const { return 0; }
  int n_quotients() const { return quotient_ideal_.size(); }
  Nterm * quotient_element(int i) const { return quotient_ideal_[i]; }
  const gbvector * quotient_gbvector(int i) const { return quotient_gbvectors_[i]; }
  
  // skew commutativity 
  virtual bool is_skew_commutative() const { return false; }
  virtual int n_skew_commutative_vars() const { return 0; }
  virtual int skew_variable(int i) const { return -1; }
  virtual bool is_skew_var(int v) const { return false; }

  virtual bool is_pid() const = 0;
  virtual bool has_gcd() const = 0;
  virtual bool is_graded() const { return is_graded_; }
  virtual bool is_poly_ring() const { return true; }
  virtual bool is_quotient_poly_ring() const { return false; }

  virtual CoefficientType coefficient_type() const
  { return Ncoeffs()->coefficient_type(); }

  virtual int n_fraction_vars() const
  { return Ncoeffs()->n_fraction_vars(); }

  virtual void text_out(buffer &o) const = 0;

  ////////////////////////
  // Arithmetic //////////
  ////////////////////////

  virtual ring_elem from_double(double n) const = 0;
  virtual ring_elem from_int(int n) const = 0;
  virtual ring_elem from_int(mpz_ptr n) const = 0;
  virtual ring_elem var(int v, int n) const = 0;

  virtual bool promote(const Ring *R, const ring_elem f, ring_elem &result) const = 0;
  virtual bool lift(const Ring *R, const ring_elem f, ring_elem &result) const = 0;

  virtual ring_elem preferred_associate(ring_elem f) const = 0;

  virtual bool is_unit(const ring_elem f) const = 0;
  virtual bool is_zero(const ring_elem f) const = 0;
  virtual bool is_equal(const ring_elem f, const ring_elem g) const = 0;

  virtual ring_elem copy(const ring_elem f) const = 0;
  virtual void remove(ring_elem &f) const = 0;

  virtual ring_elem negate(const ring_elem f) const = 0;
  virtual ring_elem add(const ring_elem f, const ring_elem g) const = 0;
  virtual ring_elem subtract(const ring_elem f, const ring_elem g) const = 0;
  virtual ring_elem mult(const ring_elem f, const ring_elem g) const = 0;
  virtual ring_elem power(const ring_elem f, mpz_t n) const = 0;
  virtual ring_elem power(const ring_elem f, int n) const = 0;
  virtual ring_elem invert(const ring_elem f) const = 0;
  virtual ring_elem divide(const ring_elem f, const ring_elem g) const = 0;
  virtual ring_elem gcd(const ring_elem f, const ring_elem g) const = 0;
  virtual ring_elem gcd_extended(const ring_elem f, const ring_elem g, 
				  ring_elem &u, ring_elem &v) const = 0;

  virtual ring_elem remainder(const ring_elem f, const ring_elem g) const = 0;
  virtual ring_elem quotient(const ring_elem f, const ring_elem g) const = 0;
  virtual ring_elem remainderAndQuotient(const ring_elem f, const ring_elem g, 
					 ring_elem &quot) const = 0;

  virtual void syzygy(const ring_elem a, const ring_elem b,
		      ring_elem &x, ring_elem &y) const = 0;

  virtual ring_elem random() const = 0;
  virtual ring_elem random(int homog, const int *deg) const = 0;

  virtual void elem_text_out(buffer &o, const ring_elem f) const = 0;

  virtual ring_elem eval(const RingMap *map, const ring_elem f) const = 0;

  /////////////////////////
  // Polynomial routines //
  /////////////////////////
  virtual int index_of_var(const ring_elem a) const = 0;
  virtual M2_arrayint support(const ring_elem a) const = 0;

  virtual bool is_homogeneous(const ring_elem f) const = 0;
  virtual void degree(const ring_elem f, int *d) const = 0;
  virtual bool multi_degree(const ring_elem f, int *d) const = 0;
  virtual int primary_degree(const ring_elem f) const = 0;
  virtual void degree_weights(const ring_elem f, const M2_arrayint wts, 
			      int &lo, int &hi) const = 0;
  virtual ring_elem homogenize(const ring_elem f, int v, int deg, 
			       const M2_arrayint wts) const = 0;
  virtual ring_elem homogenize(const ring_elem f, int v, const M2_arrayint wts) const = 0;


  virtual ring_elem mult_by_term(const ring_elem f, 
				  const ring_elem c, const int *m) const = 0;

  virtual int n_flat_terms(const ring_elem f) const = 0;
  virtual int n_logical_terms(const ring_elem f) const = 0;

  virtual ArrayPairOrNull list_form(const ring_elem f) const = 0;

  int n_terms(const ring_elem f) const { return n_flat_terms(f); }
  // This is here mainly because geopoly requires n_terms.

  virtual ring_elem make_flat_term(const ring_elem a, const int *m) const = 0;
  virtual ring_elem make_logical_term(const ring_elem a, const int *m) const = 0;
  //  virtual ring_elem term(const ring_elem a, const int *m) const = 0;

  virtual ring_elem lead_flat_coeff(const ring_elem f) const = 0;
  virtual ring_elem lead_logical_coeff(const ring_elem f) const = 0;

  virtual ring_elem get_coeff(const ring_elem f, const int *vp) const = 0;
  // vp is a varpower monomial, in the logical monoid.
  // The result will be an element in the logical coefficient ring.

  virtual ring_elem get_terms(const ring_elem f, int lo, int hi) const = 0;
  // get the (logical) terms from lo to hi in f.  A negative value means count from
  // the end.  get_terms(f,0,0) is the logical lead term of f.

  virtual const int * lead_flat_monomial(const ring_elem f) const = 0;
  virtual const int * lead_logical_monomial(const ring_elem f) const = 0;

  virtual void mult_coeff_to(ring_elem a, ring_elem &f) const = 0;

  virtual ring_elem diff(ring_elem a, ring_elem b, int use_coeff) const = 0;
  virtual bool in_subring(int n, const ring_elem a) const = 0;
  virtual void degree_of_var(int n, const ring_elem a, int &lo, int &hi) const = 0;
  virtual ring_elem divide_by_var(int n, int d, const ring_elem a) const = 0;
  virtual ring_elem divide_by_expvector(const int *exp, const ring_elem a) const = 0;

  void sort(Nterm *&f) const;

  virtual gbvector * translate_gbvector_from_ringelem(ring_elem coeff) const = 0;

  virtual gbvector * translate_gbvector_from_vec(const FreeModule *F, 
						 const vec v, 
						 ring_elem &result_denominator) const = 0;
  // result/denom == v.
  // result_denom will be an element in getDenominatorRing() (if non-NULL).
  
  virtual vec translate_gbvector_to_vec(const FreeModule *F, const gbvector *v) const = 0;
  
  virtual vec translate_gbvector_to_vec_denom(const FreeModule *F, 
					      const gbvector *v,
					      const ring_elem denom) const = 0;
  // Translate v/denom to a vector in F.  denom does not need to be positive,
  // although it had better be non-zero.
  // denom should be an element of getDenominatorRing() (if non-NULL, otherwise 'denom'
  // is ignored).
};








///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
#if 0
class RRing
{
public:
  virtual const PPolynomialRing *cast_to_PPolynomialRing() const { return 0; }
  virtual const PolyRing *cast_to_PolyRing() const { return 0; }
};

class PPolynomialRing : public RRing
{
  int element_size_; // Size of ring_elem for ring elements for this ring.

  bool is_graded_;   // Is this a graded ring?

protected:
  int getElementSize();
  int setElementSize();

  void setGraded(bool isgraded);
public:
  const PolyRing *getAmbientRing() const;
  // Every polynomial ring has an ambient ring.
  // R is an ambient ring if R == R->getAmbientRing().
  // An ambient ring has no quotient, no fractions, not even QQ.

  const RRing * getDenominatorRing() const;

  void text_out(buffer &o) const; // Display the ring in debugging form

  ////////////////
  // casting up //
  ////////////////
  virtual const PolyRing     *cast_to_PolyRing() const { return 0; }
  virtual const PolyRingSkew *cast_to_PolyRingSkew() const { return 0; }
  virtual const PolyRingWeyl *cast_to_PolyRingWeyl() const { return 0; }

  virtual const PolyQuotient *cast_to_PolyQuotient() const { return 0; }

  virtual const PolyFrac     *cast_to_PolyFrac() const { return 0; }

  virtual const PolyRingNC   *cast_to_PolyRingNC() const { return 0; }

  ////////////////////////
  // Ring informational //
  ////////////////////////
  bool isGraded() const;

  bool isSkewCommutative() const;
  bool isWeylAlgebra() const;
  bool isGradedWeylAlgebra() const;

  //////////////////////////////
  // Ring creation interface ///
  //////////////////////////////

  static PPolynomialRing *createAmbientRing(const Ring *K, const Monoid *M);

  const PPolynomialRing *create_Quotient(const Matrix *quotients) const;
  // Creates a clone of this, but mod out by the GB in quotients.
  // 'this' may already be a quotient ring.
  
  const PPolynomialRing *create_Quotient(const PPolynomialRing *B) const;
  // Creates a clone of this, if this = A[M], then
  // and B = A/I, then create A[M]/I.
  // One possible restriction is that A is an ambient poly ring.
  // Assumption: A is the ambient polynomial ring of B.  If not, NULL
  // is returned, and an error is flagged.

  const PPolynomialRing *create_Fractions(const Matrix *Prime) const;
  // Create a fraction ring, where fractions can be any element not in P.
  // P should be a GB (in the ambient ring)  of a prime ideal.
  // P can be a list of length 0, in which case this is a fraction field.

  const PPolynomialRing *create_Fractions(const Ring *A) const;
  // Create a polynomial ring over a fraction ring.  The ring A
  // should have its ambient ring == a coefficient ring of the ambient ring
  // of A, and A should be a fraction ring.

  const RRing *findCoefficientRing(const RRing *A) const;
  // If A is a coefficient ring of this, or could be, return the coefficient
  // ring of this which corresponds.  NULL means no such coefficient ring
  // could be found.

  ////////////////////////
  // Arithmetic //////////
  ////////////////////////

  ///////////////////////////
  // Parts of a polynomial //
  ///////////////////////////
};

///////////////////////////////////////////////////
///////////////////////////////////////////////////
class PolyRing : public PPolynomialRing
{
  // The class of polynomial rings, possibly non-commutative
  // with no fractions (including no QQ coefficients).
  // HOWEVER: they may have quotient ideals.
protected:
  static M2_arrayint addScalar(M2_arrayint a, int n); // helper routine

  void initialize(const RRing *coeff_ring,
		  const Monoid *monoid,
		  const RRing *flat_coeff_ring,
		  const Monoid *flat_monoid);

  virtual const PolyRing *createPolyRing(const Monoid *monoid) const;
  // Create a polynomial ring over this, of the same type (e.g. weyl, skew, comm) 
  // as this.
public:
  static const PolyRing * getTrivialPolyRing();

  static const PolyRing * create(const RRing *coeff_ring,
				  const Monoid *monoid);
  // Creates an ambient poly ring.  If coeff_ring is
  // skew or weyl, so is the result.

  virtual const PolyRing *cast_to_PolyRing() const { return this; }

  const RRing * getCoefficients() const;
  const RRing * getFlatCoefficients() const;
  const Monoid * getMonoid() const; // What about NC polys??
  const Monoid * getFlatMonoid() const;
};
///////////////////////////////////////////////////
///////////////////////////////////////////////////
class PolyRingSkew : public PolyRing
{
  bool is_skew_;
  SkewMultiplication skew_;

  void setSkewInfo(const M2_arrayint skew_vars);

  virtual const PolyRing *createPolyRing(const Monoid *monoid) const;
  // Create a polynomial ring over this, of the same type (e.g. weyl, skew, comm) 
  // as this.

public:
  virtual const PolyRingSkew *cast_to_PolyRingSkew() const { return this; }

  virtual ~PolyRingSkew();
  void getSkewInfo(M2_arrayint &result_skewvars);
  // result_skewvarsw is a CONSTANT return value.  DO NOT
  // change its value!!

  static const PolyRingSkew *create(const PolyRing *P, M2_arrayint skewvars);
  // Creates a clone of P, with skew symmetric multiplication.
};
///////////////////////////////////////////////////
///////////////////////////////////////////////////
class PolyRingWeyl : public PolyRing
{
  void setWeylInfo(M2_arrayint result_comm, 
		   M2_arrayint result_deriv, 
		   int homog_var);

  virtual const PolyRing *createPolyRing(const Monoid *monoid) const;
  // Create a polynomial ring over this, of the same type (e.g. weyl, skew, comm) 
  // as this.
public:
  virtual const PolyRingWeyl *cast_to_PolyRingWeyl() const { return this; }

  virtual ~PolyRingWeyl();
  void getWeylInfo(M2_arrayint &result_comm, 
		   M2_arrayint &result_deriv, 
		   int &homog_var) const;
  // result_comm and result_deriv are CONSTANT return values.  DO NOT
  // change their values!!

  const PolyRingWeyl *create(const PolyRing *P,
			     M2_arrayint derivatives,
			     M2_arrayint commutatives,
			     int homog_var);
  // Creates a clone of P, with Weyl algebra multiplication.
};
///////////////////////////////////////////////////
///////////////////////////////////////////////////
class PolyQuotient : public PPolynomialRing
{
  // This class handles quotients of poly rings, skew comm, and Weyl algebras
  // If there are fractions (even QQ), this is the wrong class to use?

  std::vector<ring_elem,gc_alloc> quotients_;

  MonomialIdeal *Rideal_; // contains the lead monomials (in flatMonoid)
  MonomialTable *quotient_table_;
  MonomialTableZZ *quotient_table_ZZ_;
  bool is_ZZ_quotient_;
  ring_elem ZZ_quotient_value_;

  void setQuotientInfo(std::vector<ring_elem,gc_alloc> &quotients);

  // Passes off most routines to its ambient ring, which is a PolyRing.
  // exceptions: promote, (maybe lift), mult, term.  What else?
  // from_int, from_double, var,
  // promote, is_unit, homogenize
  // mult_by_term, term, 
public:
  virtual ~PolyQuotient();

  static const PolyQuotient * create(const Matrix *m);
};
///////////////////////////////////////////////////
///////////////////////////////////////////////////
class PolyRingNC : public PolyRing
{
public:
  virtual ~PolyRingNC();
};
///////////////////////////////////////////////////
///////////////////////////////////////////////////
class PolyFrac : public PPolynomialRing
{
public:
  virtual ~PolyFrac();
};
///////////////////////////////////////////////////
///////////////////////////////////////////////////
class PolyFracQuotient : public PolyFrac
{
  PolyFrac *R; // A fraction ring
  PolyQuotient *S; // This is a quotient 
  // This ring is R/(ideal(S))
public:
  virtual ~PolyFracQuotient();

  static const PolyFracQuotient * create(const Matrix *m);
};

///////////////////////////////////////////////////
///////////////////////////////////////////////////
///// Fraction Info ///////////////////////////////
///////////////////////////////////////////////////
///////////////////////////////////////////////////

struct MultClosedSet
{
  MultClosedSet *next;

  virtual bool is_invertible(Nterm *f);
};
struct LocalSet : public MultClosedSet
{
  GBComputation *G;

  bool is_invertible(Nterm *f)
  {
    if (in_subring(nvars, f))
      {
	G->reduce(f);
	return f != 0;
      }
  }
};

class Fractions
{
  const PolyRing *R;

public:
  bool is_invertible(Nterm *f);
};

class MultClosedSet_local
{
  const PolyRing *R;
};

class MultClosedSet_semilocal
{
  const PolyRing *R;
};

class MultClosedSet_powers
{
  const PolyRing *R;
};

class MultClosedSet_powers
{
  const PolyRing *R;
};
///////////////////////////////////////////////////
///////////////////////////////////////////////////
///// Quotient Info ///////////////////////////////
///////////////////////////////////////////////////
///////////////////////////////////////////////////

class Quotient
{
public:
  virtual void normal_form(Nterm *&f, ring_elem &g);
  virtual void normal_form(Nterm *&f); // Throws away any denominator used.

  // Obtaining quotient elements
  int n_quotients() const;
  ring_elem get_quotient_element(int i) const; // elements in the PolyRing R.

  // Obtaining monomial ideal
  const MonomialIdeal * get_quotient_monomials() const;
  const MonomialTable * get_quotient_montable() const;

  // Obtaining monomial ideal
  const MonomialTableZZ * get_quotient_montableZZ() const;
};

class QuotientBasic : public Quotient
{
  const PolyRing *R; // This is the ambient ring of the quotient
  
public:
  QuotientBasic(const Matrix *m);
  // The ring of m should be a PolyRing, and m should be a monic GB
  // of this ring.

  QuotientBasic(const Matrix *m, const Matrix *n);
  // The rings of m and n should be the same, a PolyRing, and the union
  // of the columns should be a (possibly non-minimal) GB, where every 
  // column of n is minimal.

  ~QuotientBasic();

  void normal_form(Nterm *&f);

  void normal_form(const FreeModule *F, gbvector *&f);
};

class QuotientZZ : public Quotient
{
  const PolyRing *R; // This is the ambient ring of the quotient
  // whose flat coeff ring is ZZ.
  
public:
  QuotientZZ(const Matrix *m);
  // The ring of m should be a PolyRing, and m should be a monic GB
  // of this ring.

  QuotientZZ(const Matrix *m, const Matrix *n);
  // The rings of m and n should be the same, a PolyRing, and the union
  // of the columns should be a (possibly non-minimal) GB, where every 
  // column of n is minimal.

  ~QuotientZZ();

  void normal_form(Nterm *&f);

  void normal_form(const FreeModule *F, gbvector *&f);
};

class QuotientFrac : public Quotient
{
  const PolyRing *R; // This is the ambient ring of the quotient

  const RRing *denomR; // These are the allowed denominators
  // This will be globalZZ, if the ring is QQ[...].
public:
  QuotientFrac(const RRing *denomR, const Matrix *m);
  // The ring of m should be a PolyRing, and m should be a monic GB
  // of this ring.

  QuotientFrac(const RRing *denomR, const Matrix *m, const Matrix *n);
  // The rings of m and n should be the same, a PolyRing, and the union
  // of the columns should be a (possibly non-minimal) GB, where every 
  // column of n is minimal.

  ~QuotientFrac();

  void normal_form(Nterm *&f, ring_elem &denom);

  void normal_form(const FreeModule *F, gbvector *&f, ring_elem &denom);

};
#endif
#endif

// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e "
// End:
