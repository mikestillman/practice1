// Copyright 2005, Michael E. Stillman

#ifndef _minimalgb_ZZ_hpp_
#define _minimalgb_ZZ_hpp_

#include "minimalgb.hpp"

class MinimalGB_ZZ : public MinimalGB
{
  enum divisor_type { DIVISOR_NONE, DIVISOR_RING, DIVISOR_MODULE};

  MonomialTableZZ *T;
  const MonomialTableZZ *ringtableZZ;

  enum divisor_type find_divisor(exponents exp, int comp, int &result_loc);

public:

  MinimalGB_ZZ(GBRing *R0,
	       const PolynomialRing *originalR0,
	       const FreeModule *F0,
	       const FreeModule *Fsyz0);

  virtual ~MinimalGB_ZZ();

  virtual void set_gb(vector<POLY, gc_alloc> &polys0);

  virtual void minimalize(const vector<POLY, gc_alloc> &polys0);
  // I have to decide: does this ADD to the existing set?

  virtual void remainder(POLY &f, bool use_denom, ring_elem &denom);
  // WARNING: this should only be used with term orders!
  // REALLY??
  virtual void remainder(gbvector *&f, bool use_denom, ring_elem &denom);

};	     

#endif

// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e "
// End:
