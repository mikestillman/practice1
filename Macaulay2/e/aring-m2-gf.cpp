// Copyright 2012 Michael E. Stillman

#include <vector>
#include <ostream>

#include "relem.hpp"
#include "polyring.hpp"
#include "aring-m2-gf.hpp"

#include "../system/supervisorinterface.h"
#define SYSTEM_INTERRUPTED test_Field(THREADLOCAL(interrupts_interruptedFlag,struct atomic_field))

namespace M2 {

  GaloisFieldTable::GaloisFieldTable(const PolynomialRing& R,
                                     const ring_elem prim):
    mCharac(R.charac()),
    mOriginalRing(R),
    mPrimitiveElement(prim)
  {
    ASSERT(mOriginalRing.n_quotients() == 1);

    ring_elem f = mOriginalRing.quotient_element(0);
    Nterm *t = f;
    mDimension = mOriginalRing.getMonoid()->primary_degree(t->monom);
    mOrder = mCharac;
    for (int i=1; i<mDimension; i++) mOrder *= mCharac;
    mOne = mOrder - 1; // representation for the number 1: p^n - 1.
    mOrderMinusOne = mOne; // p^n - 1
    mMinusOne = (mCharac == 2 ? mOne : mOne/2);
    
    // Get ready to create mOneTable.
    std::vector<ring_elem> polys;
    polys.push_back(mOriginalRing.from_int(0));
    polys.push_back(mOriginalRing.copy(mPrimitiveElement));
    
    ring_elem oneR = mOriginalRing.from_int(1);
    
    mGeneratorExponent = static_cast<size_t>(-1);
    ring_elem x = mOriginalRing.var(0);
    if (mOriginalRing.is_equal(mPrimitiveElement, x))
      mGeneratorExponent = 1;
    for (size_t i=2; i<mOne; i++)
      {
	ring_elem g = mOriginalRing.mult(polys[i-1], mPrimitiveElement);
	polys.push_back(g);
	if (mOriginalRing.is_equal(g, oneR)) break;
	if (mOriginalRing.is_equal(g, x))
	  mGeneratorExponent = i;
      }

    ASSERT(polys.size() == mOrderMinusOne);
    ASSERT(mGeneratorExponent != static_cast<size_t>(-1));
    
    // Set 'one_table'.
    mOneTable = newarray_atomic(size_t,mOrder);
    mOneTable[0] = mOrderMinusOne;

    for (size_t i=1; i<=mOrderMinusOne; i++)
      {
        if (SYSTEM_INTERRUPTED) 
          {
            // first clean up?
            return;
          }
	ring_elem f1 = mOriginalRing.add(polys[i], oneR);
        size_t j;
	for (j=1; j<=mOrderMinusOne; j++)
          if (mOriginalRing.is_equal(f1, polys[j]))
            break;
        mOneTable[i] = j;
      }
    
    // Create the ZZ/P ---> GF(Q) inclusion map
    mFromIntTable = newarray_atomic(size_t,mCharac);
    size_t a = mOne;;
    mFromIntTable[0] = 0;
    for (size_t i=1; i<mCharac; i++)
      {
        mFromIntTable[i] = a;
        a = mOneTable[a];
      }
  }

  void GaloisFieldTable::display(std::ostream &o) const
  {
    o << "GF(" << mCharac << "^" << mDimension << ")" << std::endl;
    o << " order = " << mOrder << std::endl;
    o << " 1     = " << mOne << std::endl;
    o << " -1    = " << mMinusOne << std::endl;
    o << " fromZZ: " << std::endl << "    ";
    for (size_t i = 0; i<mCharac; i++)
      {
        if ((i+1) % 10 == 0) o << std::endl << "    ";
        o << mFromIntTable[i] << " ";
      }
    o << std::endl << " oneTable: " << std::endl << "    ";
    for (size_t i = 0; i<mOrder; i++)
      {
        if ((i+1) % 10 == 0) o << std::endl << "    ";
        o << mOneTable[i] << " ";
      }
    o << std::endl;
  }

  void ARingGFM2::text_out(buffer &o) const
  {
    o << "GF(" << mGF.characteristic() << "^" << mGF.dimension() << ",M2)";
  }

  void ARingGFM2::elem_text_out(buffer &o,
                                elem a,
                                bool p_one,
                                bool p_plus,
                                bool p_parens) const
  {
    if (a == 0)
      {
        o << "0";
        return;
      }
    ring_elem h = mGF.ring().power(mGF.primitiveElement(), a);
    mGF.ring().elem_text_out(o, h, p_one, p_plus, p_parens);
    mGF.ring().remove(h);
  }
  
};

// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e "
// indent-tabs-mode: nil
// End:
