#ifndef _minimalgb_hpp_
#define _minimalgb_hpp_

#include <vector>
#include "gbring.hpp"
#include "montable.hpp"
#include "montableZZ.hpp"

class MinimalGB : public our_new_delete
{
protected:
  GBRing *R;
  const PolynomialRing *originalR;
  const FreeModule *F;
  const FreeModule *Fsyz;
  vector<POLY, gc_alloc> polys;
public:
  MinimalGB(GBRing *R0,
	    const PolynomialRing *originalR0,
	    const FreeModule *F0,
	    const FreeModule *Fsyz0) : R(R0), originalR(originalR0), F(F0), Fsyz(Fsyz0) {}

  virtual ~MinimalGB();

  virtual void set_gb(vector<POLY, gc_alloc> &polys0) = 0;

  virtual void minimalize(const vector<POLY, gc_alloc> &polys0) = 0;
  // I have to decide: does this ADD to the existing set?

  // Choose a minimal set of generators of the lead terms.
  // sort the resulting elements
  // auto reduce them
  // This class will be subclassed by:
  //   base is a field
  //   base is ZZ, strong GB
  //   base is ZZ, weak GB
  //   base is a frac field, # frac vars is given.
  //   ring has a local term order: reduction can not be complete...

  const vector<POLY, gc_alloc> &get() const { return polys; }

  virtual void remainder(POLY &f, bool use_denom, ring_elem &denom) = 0;
  // WARNING: this should only be used with term orders!
  // REALLY??

  virtual void remainder(gbvector *&f, bool use_denom, ring_elem &denom) = 0;
};	     

class MinimalGB_Field : public MinimalGB
{
  MonomialTable *T;
  const MonomialIdeal *Rideal;
public:
  MinimalGB_Field(GBRing *R0,
		  const PolynomialRing *originalR0,
		  const FreeModule *F0,
		  const FreeModule *Fsyz0);

  virtual ~MinimalGB_Field();

  virtual void set_gb(vector<POLY, gc_alloc> &polys0);

  virtual void minimalize(const vector<POLY, gc_alloc> &polys0);
  // I have to decide: does this ADD to the existing set?

  virtual void remainder(POLY &f, bool use_denom, ring_elem &denom);
  // WARNING: this should only be used with term orders!
  // REALLY??
  virtual void remainder(gbvector *&f, bool use_denom, ring_elem &denom);

};	     

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
