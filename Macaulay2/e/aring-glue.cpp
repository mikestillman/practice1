// Copyright 2011 Michael E. Stillman

#include "aring-promoter.hpp"

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

  template<typename RingType>
  bool ConcreteRing<RingType>::promote(const Ring *S, 
                                       const ring_elem fR, 
                                       ring_elem &resultS) const
  {
    const Ring *R = this;
    fprintf(stderr, "calling promote\n");
    typedef RingPromoter RP;
    if (R == globalZZ)
      {
        resultS = S->from_int(fR.get_mpz());
        return true;
      }
    if (R == S)
      {
        resultS = copy(fR);
        return true;
      }
    switch (R->ringID()) {
    case M2::ring_ZZp:
      switch (S->ringID()) {
      case M2::ring_ZZp: return false;
      case M2::ring_GF: return RP::promoter<ARingZZp,ARingGF>(R,S,fR,resultS);
      case M2::ring_FFPACK: return RP::promoter<ARingZZp,ARingZZpFFPACK>(R,S,fR,resultS);
      default: return false;
      }
      break;
    case M2::ring_GF:
      switch (S->ringID()) {
      case M2::ring_ZZp: return RP::promoter<ARingGF,ARingZZp>(R,S,fR,resultS);
      case M2::ring_GF: return RP::promoter<ARingGF,ARingGF>(R,S,fR,resultS);
      case M2::ring_FFPACK: return RP::promoter<ARingGF,ARingZZpFFPACK>(R,S,fR,resultS);
      default: return false;
      }
    case M2::ring_FFPACK:
      switch (S->ringID()) {
      case M2::ring_ZZp: return RP::promoter<ARingZZpFFPACK,ARingZZp>(R,S,fR,resultS);
      case M2::ring_GF: return RP::promoter<ARingZZpFFPACK,ARingGF>(R,S,fR,resultS);
      case M2::ring_FFPACK: return RP::promoter<ARingZZpFFPACK,ARingZZpFFPACK>(R,S,fR,resultS);
      default: return false;
      }
    default:
      break;
    };
    return false;
  }

  //explicit instantiation
 template class ConcreteRing< ARingZZp >;

  template class ConcreteRing< ARingGFM2 >;

#if defined(HAVE_FFLAS_FFPACK) && defined(HAVE_GIVARO)

  //explicit instantiation
 template class ConcreteRing< ARingGF >;
 
#endif
#if defined(HAVE_FFLAS_FFPACK)  

  //explicit instantiation
 
 template class ConcreteRing< ARingZZpFFPACK >;
#endif


};


// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e  "
// indent-tabs-mode: nil
// End:
