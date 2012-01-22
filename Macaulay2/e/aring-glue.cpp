// Copyright 2011 Michael E. Stillman

#include "aring-glue.hpp"
#include "aring-zzp.hpp"
#include "aring-gf.hpp"
#include "aring-ffpack.hpp"
namespace M2 {



  template<class RingType>
  ConcreteRing<RingType> * ConcreteRing<RingType>::create(const RingType *R)
  {
    ConcreteRing<RingType> *result = new ConcreteRing<RingType>(R);
    result->initialize_ring(R->characteristic());
    result->declare_field();

    //    zeroV = from_int(0);
    //    oneV = from_int(1);
    //    minus_oneV = from_int(-1);

    return result;
  }


  //explicit instantiation
 template class ConcreteRing< ARingZZp >;

#if defined(HAVE_FFLAS_FFPACK) && defined(HAVE_GIVARO)

  //explicit instantiation
 template class ConcreteRing< ARingGF >;
 
#endif
#if defined(HAVE_FFLAS_FFPACK)  

  //explicit instantiation
 
 template class ConcreteRing< ARingFFPACK >;
#endif


};


// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e  "
// indent-tabs-mode: nil
// End:
