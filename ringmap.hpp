// Copyright 1996  Michael E. Stillman
#ifndef _ringmap_hh_
#define _ringmap_hh_

#include "ring.hpp"

class RingElement;
class Matrix;

class RingMap : public immutable_object
{
  struct var {
    bool is_zero;		// Does this variable map to 0?

    bool coeff_is_one;
    bool monom_is_one;
    bool bigelem_is_one;

    ring_elem coeff;		// this variable maps to coeff*monom*bigelem
				// where coeff is 1 if isone is true.
				// and   monom is 1 if mon_isone is true,
				// and   bigelem is 1 if bigone_isone is true.
				// coeff is an element of type K.
    int *monom;			// This is an exponent vector in R.
    ring_elem bigelem;
  };

  const Ring *R;		// This is the target ring.
  const Ring *K;
  const Monoid *M;

  bool is_monomial;		// True, if each term maps to a term in the
				// target ring.

  int nvars;			// Number of variables in the source ring.
  var *_elem;			// elem[i] is the structure representing the image of
				// the i th variable.
  RingMap(const Matrix *m);
public:
  ~RingMap();

  static const RingMap *make(const Matrix *m);

  const Ring *get_ring() const { return R; }
  const ring_elem elem(int i) const { 
    assert(i < nvars);
    return _elem[i].bigelem; 
  }

  bool is_equal(const RingMap *phi) const;

  ring_elem eval_term(const Ring *coeff_ring, const ring_elem coeff, 
		      const int *vp) const;

  RingElementOrNull *eval(const RingElement *r) const;
  MatrixOrNull *eval(const FreeModule *newrows, const Matrix *m) const;

  void text_out(buffer &o) const;
};

#endif

// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e "
// End:
