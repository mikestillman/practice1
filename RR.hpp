// Copyright 2001 Michael E. Stillman.

#ifndef _RR_hh_
#define _RR_hh_

#include "ring.hpp"

class CoefficientRingRR;

class RingRR : public Ring
// Elements of this ring are real numbers: 'double's
{
  friend class CoefficientRingRR;
  struct RRelem_rec {
    double val;
  };
  typedef RRelem_rec *RRelem;

  double _epsilon;  // Elements closer than this are considered identical.

  RRelem new_elem() const;
  void remove_elem(RRelem f) const;

  bool is_zero_RR(double a) const;
  int compare_RR(double a, double b) const;

  CoefficientRingRR *coeffR;
protected:
  RingRR() {}
  virtual ~RingRR();
  bool initialize_RR(double epsilon);
public:
  static RingRR * create(double epsilon);

  RingRR * cast_to_RingRR() { return this; }
  const RingRR * cast_to_RingRR() const { return this; }

  CoefficientRingRR *get_CoeffRing() const { return coeffR; }

  double to_double(ring_elem a);

// The following are all the routines required by 'ring'
  virtual bool is_RR() const { return true; }

  virtual bool is_pid() const       { return 1; }
  virtual bool has_gcd() const      { return 1; }

  virtual void text_out(buffer &o) const;

  virtual ring_elem from_int(int n) const;
  virtual ring_elem from_int(mpz_ptr n) const;
  virtual ring_elem from_double(double r) const;
  virtual ring_elem from_rational(mpq_ptr r) const;
  virtual ring_elem from_BigReal(mpf_ptr a) const;
  virtual bool promote(const Ring *R, const ring_elem f, ring_elem &result) const;
  virtual bool lift(const Ring *R, const ring_elem f, ring_elem &result) const;

  virtual ring_elem preferred_associate(ring_elem f) const;

  int compare(const ring_elem a, const ring_elem b) const;
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
  virtual ring_elem gcd(const ring_elem f, const ring_elem g) const;
  virtual ring_elem gcd_extended(const ring_elem f, const ring_elem g, 
				  ring_elem &u, ring_elem &v) const;

  virtual ring_elem remainder(const ring_elem f, const ring_elem g) const;
  virtual ring_elem quotient(const ring_elem f, const ring_elem g) const;
  virtual ring_elem remainderAndQuotient(const ring_elem f, const ring_elem g, 
					 ring_elem &quot) const;

  virtual void syzygy(const ring_elem a, const ring_elem b,
		      ring_elem &x, ring_elem &y) const;

  virtual ring_elem random() const;

  virtual void elem_text_out(buffer &o, const ring_elem f) const;

  virtual ring_elem eval(const RingMap *map, const ring_elem f, int first_var) const;
};

#endif

// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e "
// End:
