// Copyright 1995 Michael E. Stillman.

// need to be careful that exponent does not overflow, according to GMP

#ifndef _CCC_hh_
#define _CCC_hh_

#include "ring.hpp"

class CCC : public Ring
{
  int precision;
  int _elem_size;

  M2_CCC _zero_elem;

  M2_CCC new_elem() const;
  void remove_elem(M2_CCC f) const;

protected:
  CCC() {}
  virtual ~CCC() {}
  bool initialize_CCC(int prec);
public:
  static CCC * create(int prec);

  CCC * cast_to_CCC() { return this; }
  const CCC * cast_to_CCC() const { return this; }

  // should there be a complex conjugation function?

  ring_elem from_doubles(double r, double s) const;
  ring_elem from_BigReals(mpfr_ptr a, mpfr_ptr b) const;

  virtual mpfr_ptr to_BigReal(ring_elem f) const;

  bool is_real(const ring_elem f) const;  // see if |f| is purely real
  ring_elem absolute(const ring_elem f) const;  // norm |f| of f

  // The following are all the routines required by 'ring'
  virtual bool is_CCC() const         { return true; }

  virtual void text_out(buffer &o) const;

  virtual ring_elem from_int(int n) const;
  virtual ring_elem from_int(mpz_ptr n) const;
  virtual ring_elem from_double(double r) const;
  virtual ring_elem from_rational(mpq_ptr r) const;
  virtual ring_elem from_complex(M2_CC z) const;
  virtual ring_elem from_BigReal(mpfr_ptr r) const;
  virtual ring_elem from_BigComplex(M2_CCC z) const;
  virtual bool promote(const Ring *R, const ring_elem f, ring_elem &result) const;
  virtual bool lift(const Ring *R, const ring_elem f, ring_elem &result) const;

  // TO DO: MAKE IT SAME AS CC
  virtual ring_elem preferred_associate(ring_elem f) const;

  virtual bool is_unit(const ring_elem f) const;
  virtual bool is_zero(const ring_elem f) const;
  virtual bool is_equal(const ring_elem f, const ring_elem g) const;
  virtual int compare_elems(const ring_elem a, const ring_elem b) const;

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
