// Copyright 1995 Michael E. Stillman

#ifndef _polyring_hh_
#define _polyring_hh_

#include "ring.hpp"
#include "ringelem.hpp"
#include "skew.hpp"

///// Ring Hierarchy ///////////////////////////////////

class TermIdeal;
class Matrix;
class GBRing;
class GBRingSkew;
class GBComputation;

#include "poly.hpp"

class PolyRing : public PolyRingFlat
{
  friend class GBRingSkew;
  friend class FreeModule;

  void initialize_poly_ring(const Ring *K, const Monoid *M, 
			    const PolynomialRing *deg_ring);
  // Only to be called from initialize_poly_ring and make_trivial_ZZ_poly_ring

protected:
  void initialize_poly_ring(const Ring *K, const Monoid *M);
  // Called by subclasses (e.g. skew comm, Weyl, solvable.
  // After this, set the information for the special multiplication.
  // Then set the gb_ring

  virtual ~PolyRing();
  PolyRing() {}

  static PolyRing *trivial_poly_ring;
  static void make_trivial_ZZ_poly_ring();
public:
  static const PolyRing *create(const Ring *K, const Monoid *M);

protected:

  int *_EXP1, *_EXP2, *_EXP3;
public:

  static const PolyRing *get_trivial_poly_ring();

  virtual const PolyRing * cast_to_PolyRing()  const { return this; }
  virtual       PolyRing * cast_to_PolyRing()        { return this; }


  virtual void text_out(buffer &o) const;

  /////////////////////////
  // Arithmetic ///////////
  /////////////////////////

  virtual ring_elem from_double(double n) const;
  virtual ring_elem from_int(int n) const;
  virtual ring_elem from_int(mpz_ptr n) const;
  virtual ring_elem from_rational(mpq_ptr q) const;

  virtual ring_elem var(int v) const;

  virtual int index_of_var(const ring_elem a) const;
  virtual M2_arrayint support(const ring_elem a) const;

  virtual bool promote(const Ring *R, const ring_elem f, ring_elem &result) const;
  virtual bool lift(const Ring *R, const ring_elem f, ring_elem &result) const;

  virtual ring_elem preferred_associate(ring_elem f) const;

  virtual bool is_unit(const ring_elem f) const;
  virtual bool is_zero(const ring_elem f) const;
  virtual bool is_equal(const ring_elem f, const ring_elem g) const;

  virtual bool is_homogeneous(const ring_elem f) const;
  virtual void degree(const ring_elem f, int *d) const;
  virtual bool multi_degree(const ring_elem f, int *d) const;
  virtual int primary_degree(const ring_elem f) const;
  virtual void degree_weights(const ring_elem f, const M2_arrayint wts, 
			      int &lo, int &hi) const;
  virtual ring_elem homogenize(const ring_elem f, int v, int deg, 
			       const M2_arrayint wts) const;
  virtual ring_elem homogenize(const ring_elem f, int v, const M2_arrayint wts) const;

  virtual ring_elem copy(const ring_elem f) const;
  virtual void remove(ring_elem &f) const;

  void internal_negate_to(ring_elem &f) const;
  void internal_add_to(ring_elem &f, ring_elem &g) const;
  void internal_subtract_to(ring_elem &f, ring_elem &g) const;

  virtual ring_elem negate(const ring_elem f) const;
  virtual ring_elem add(const ring_elem f, const ring_elem g) const;
  virtual ring_elem subtract(const ring_elem f, const ring_elem g) const;
  virtual ring_elem mult(const ring_elem f, const ring_elem g) const;
  virtual ring_elem power(const ring_elem f, mpz_t n) const;
  virtual ring_elem power(const ring_elem f, int n) const;
  virtual ring_elem invert(const ring_elem f) const;
  virtual ring_elem divide(const ring_elem f, const ring_elem g) const;
  virtual ring_elem gcd(const ring_elem f, const ring_elem g) const;
  virtual ring_elem gcd_extended(const ring_elem f, const ring_elem g, 
				  ring_elem &u, ring_elem &v) const;

protected:
  void minimal_monomial(ring_elem f, int *&monom) const;
  Nterm *division_algorithm(Nterm *f, Nterm *g, Nterm *&quot) const;
  Nterm *division_algorithm(Nterm *f, Nterm *g) const;
  Nterm *powerseries_division_algorithm(Nterm *f, Nterm *g, Nterm *&quot) const;

public:
  virtual ring_elem remainder(const ring_elem f, const ring_elem g) const;
  virtual ring_elem quotient(const ring_elem f, const ring_elem g) const;
  virtual ring_elem remainderAndQuotient(const ring_elem f, const ring_elem g, 
					 ring_elem &quot) const;

  virtual void syzygy(const ring_elem a, const ring_elem b,
		      ring_elem &x, ring_elem &y) const;

  virtual ring_elem random() const;

  virtual void elem_text_out(buffer &o, const ring_elem f) const;

  virtual ring_elem eval(const RingMap *map, const ring_elem f) const;

  virtual ring_elem mult_by_term(const ring_elem f, 
				  const ring_elem c, const int *m) const;

  virtual int n_flat_terms(const ring_elem f) const;
  virtual int n_logical_terms(int nvars0,const ring_elem f) const;

  virtual ring_elem get_coeff(const Ring *coeffR,const ring_elem f, const int *vp) const;
  virtual ring_elem get_terms(int nvars0, const ring_elem f, int lo, int hi) const;

  virtual ring_elem make_flat_term(const ring_elem a, const int *m) const;
  virtual ring_elem make_logical_term(const Ring *coeffR, const ring_elem a, const int *exp) const;

  virtual ring_elem lead_flat_coeff(const ring_elem f) const;
  virtual ring_elem lead_logical_coeff(const Ring *coeffR, const ring_elem f) const;

  virtual const int * lead_flat_monomial(const ring_elem f) const;
  virtual void lead_logical_exponents(int nvars0, const ring_elem f, int * result_exp) const;

  ring_elem lead_term(const ring_elem f) const; // copies the lead term
  int compare(const ring_elem f, const ring_elem g) const; // compares the lead terms

  virtual ArrayPairOrNull list_form(const Ring *coeffR, const ring_elem f) const;

  virtual void mult_coeff_to(ring_elem a, ring_elem &f) const;

  virtual ring_elem lead_term(int nparts, const ring_elem f) const;

public:
  virtual vec vec_lead_term(int nparts, const FreeModule *F, vec v) const;

  virtual vec vec_top_coefficient(const vec v, int &var, int &exp) const;

  const vecterm * vec_locate_lead_term(const FreeModule *F, vec v) const;
  // Returns a pointer to the lead vector of v.
  // This works if F has a Schreyer order, or an up/down order.

protected:
  vec vec_coefficient_of_var(vec v, int var, int exp) const;


  ring_elem diff_term(const int *m, const int *n, 
		      int *resultmon,
		      int use_coeff) const;


  ring_elem get_logical_coeff(const Ring *coeffR, const Nterm *&f) const;
  // Given an Nterm f, return the coeff of its logical monomial, in the
  // polynomial ring coeffR.  f is modified, in that it is replaced by
  // the pointer to the first term of f not used (possibly 0).

public:
  virtual void monomial_divisor(const ring_elem a, int *exp) const;

  virtual ring_elem diff(ring_elem a, ring_elem b, int use_coeff) const;
  virtual bool in_subring(int nslots, const ring_elem a) const;
  virtual void degree_of_var(int n, const ring_elem a, int &lo, int &hi) const;
  virtual ring_elem divide_by_var(int n, int d, const ring_elem a) const;
  virtual ring_elem divide_by_expvector(const int *exp, const ring_elem a) const;

  // Routines special to polynomial rings
  // possibly others?
  // Rideal, exterior_vars.
  // nbits
  // heap merge of elements...?

  // Routines special to PID's
  // these include: gcd, gcd_extended.

  // Routines special to fields (anything else?)
protected:
  Nterm *new_term() const;
  Nterm *copy_term(const Nterm *t) const;

  bool imp_attempt_to_cancel_lead_term(ring_elem &f, 
				      ring_elem g, 
				      ring_elem &coeff, 
				      int *monom) const;

protected:
  ring_elem imp_skew_mult_by_term(const ring_elem f, 
				  const ring_elem c, const int *m) const;
  void imp_subtract_multiple_to(ring_elem &f, 
				ring_elem a, const int *m, const ring_elem g) const;
public:
  void sort(Nterm *&f) const;

  ///////////////////////////////////////////////////////
  // Used in gbvector <--> vector/ringelem translation //
  ///////////////////////////////////////////////////////
  // These are only meant to be called by Ring's.
public:
  void determine_common_denominator_QQ(ring_elem f,
					       mpz_ptr denom_so_far) const;

  ring_elem get_denominator_QQ(ring_elem f) const;

  ring_elem vec_get_denominator_QQ(vec f) const;

  gbvector * translate_gbvector_from_vec_QQ(const FreeModule *F, 
					    const vec v, 
					    ring_elem &result_denominator) const;

  vec translate_gbvector_to_vec_QQ(const FreeModule *F, 
				   const gbvector *v,
				   const ring_elem denom) const;

  gbvector *translate_gbvector_from_ringelem_QQ(ring_elem coeff) const;

  gbvector *translate_gbvector_from_ringelem(ring_elem coeff) const;
  
  gbvector * translate_gbvector_from_vec(const FreeModule *F, 
					 const vec v, 
					 ring_elem &result_denominator) const;
  
  vec translate_gbvector_to_vec(const FreeModule *F, const gbvector *v) const;
  
  vec translate_gbvector_to_vec_denom(const FreeModule *F, 
				      const gbvector *v,
				      const ring_elem denom) const;
  // Translate v/denom to a vector in F.  denom does not need to be positive,
  // although it had better be non-zero.
};

#endif

// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e "
// End:
