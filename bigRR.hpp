// Copyright 1995 Michael E. Stillman.

// need to be careful that exponent does not overflow, according to GMP

#ifndef _bigRR_hh_
#define _bigRR_hh_

#include "ring.hpp"
#include <gmp.h>

#define MPF_VAL(f) (reinterpret_cast<mpf_ptr>((f).poly_val))
#define MPF_RINGELEM(a) (ring_elem(reinterpret_cast<Nterm *>(a)))

#if 0
#define MPF_VAL(f) (mpf_ptr ((f).poly_val))
#define MPF_RINGELEM(a) ((ring_elem) ((Nterm *) (a)))
#endif

class bigRR : public Ring
{
  int _elem_size;
  mpf_ptr _zero_elem;

  mpf_ptr new_elem() const;
  void remove_elem(mpf_ptr f) const;

  static mpf_ptr _epsilon;  // Elements less than this are considered zero.

protected:
  bigRR() {}
  virtual ~bigRR() {}
  bool initialize_bigRR();
public:
  static bigRR * create();

  bigRR * cast_to_bigRR() { return this; }
  const bigRR * cast_to_bigRR() const { return this; }

  static void set_epsilon(mpf_ptr epsilon);
  static mpf_ptr get_epsilon();

  void zeroize_tiny_lead_components(vec &v, mpf_ptr epsilon) const; 
  // zeroizes coeffs until lead coeff no longer less than epsilon in abs value

  bool is_greater(const ring_elem a, const ring_elem b) const;
  bool is_less(const ring_elem a, const ring_elem b) const;
  ring_elem absolute(const ring_elem f) const;
  ring_elem sqrt(const ring_elem f) const;

  virtual mpf_ptr to_BigReal(ring_elem f) const;

// The following are all the routines required by 'ring'
  virtual bool is_bigRR() const         { return true; }

  virtual bool is_pid() const       { return 1; }
  virtual bool has_gcd() const      { return 1; }

  virtual void text_out(buffer &o) const;

  virtual ring_elem from_int(int n) const;
  virtual ring_elem from_int(mpz_ptr n) const;
  virtual ring_elem from_double(double r) const;
  virtual ring_elem from_rational(mpq_ptr r) const;
  virtual ring_elem from_BigReal(mpf_ptr r) const;
  virtual ring_elem var(int v, int n) const;
  virtual bool promote(const Ring *R, const ring_elem f, ring_elem &result) const;
  virtual bool lift(const Ring *R, const ring_elem f, ring_elem &result) const;

  virtual ring_elem preferred_associate(ring_elem f) const;

  int is_positive(const ring_elem a) const;

  virtual bool is_unit(const ring_elem f) const;
  virtual bool is_zero(const ring_elem f) const;
  virtual bool is_equal(const ring_elem f, const ring_elem g) const;

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
  virtual ring_elem divide(const ring_elem f, const ring_elem g, ring_elem &rem) const;

  virtual ring_elem remainder(const ring_elem f, const ring_elem g) const;
  virtual ring_elem quotient(const ring_elem f, const ring_elem g) const;
  virtual ring_elem remainderAndQuotient(const ring_elem f, const ring_elem g, 
					 ring_elem &quot) const;

  virtual ring_elem gcd(const ring_elem f, const ring_elem g) const;
  virtual ring_elem gcd_extended(const ring_elem f, const ring_elem g, 
				  ring_elem &u, ring_elem &v) const;

  virtual void syzygy(const ring_elem a, const ring_elem b,
		      ring_elem &x, ring_elem &y) const;

  virtual ring_elem random() const;

  virtual void elem_text_out(buffer &o, const ring_elem f) const;

  virtual ring_elem eval(const RingMap *map, const ring_elem f) const;

};

#endif

// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e "
// End:
