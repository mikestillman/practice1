#include "engine.h"
#include "exceptions.hpp"

#include "relem.hpp"
#include "aring-glue.hpp"
#include "aring-zzp.hpp"
#include "aring-gf.hpp"
#include "aring-m2-gf.hpp"
#include "aring-ffpack.hpp"

#include "polyring.hpp"

const Ring /* or null */ *rawARingZZp(int p)
  /* p must be a prime number <= 32767 */
{
  if (p <= 1 || p >= 32750)
    {
      ERROR("ZZP: expected a prime number p in range 2 <= p <= 32749");
      return 0;
    }
  M2::ARingZZp *A = new M2::ARingZZp(p);
  return M2::ConcreteRing<M2::ARingZZp>::create(A);
}

const Ring /* or null */ *rawARingGaloisField1(const RingElement *f)
{
  // Check that the ring R of f is a polynomial ring in one var over a ZZ/p
  // Check that f has degree >= 2
  // Check that f is monic
  // If any of these fail, then return 0.
  const PolynomialRing *R = f->get_ring()->cast_to_PolynomialRing();
  if (R == 0)
    {
      ERROR("expected poly ring of the form ZZ/p[x]/(f)");
      return 0;
    }
  if (R->n_vars() != 1)
    {
      ERROR("expected poly ring of the form ZZ/p[x]/(f)");
      return 0;
    }
  if (R->n_quotients() != 1)
    {
      ERROR("expected poly ring of the form ZZ/p[x]/(f)");
      return 0;
    }
  if (R->charac() == 0)
    {
      ERROR("expected poly ring of the form ZZ/p[x]/(f)");
      return 0;
    }
  try {
    M2::ARingGFM2 *A = new M2::ARingGFM2(*R,f->get_value());
    return M2::ConcreteRing<M2::ARingGFM2>::create(A);
  }
  catch (exc::engine_error e) {
    ERROR(e.what());
    return NULL;
  }
}


/// @todo why parameters are ints and not longs or mpz's? is an overflow possible?
/// @todo check prime for primality (probably at top of Macaulay2)
/// @todo ARingGF uses tables and may consume a huge amount of memory -
///        pass therefore a 'MaxMemoryConsumption' parameter and if the value is overstepped by ARingGF, create polynomial representation?
/// @todo  the check if in general polynomial representation is needed cost some additional work, similar to linbox/field/givaro-gfq.h. Use GivaroGfq instead of Givaro::GFqDom in ARingGF?
///@todo: return Macaulay Galois field in some cases.

const Ring /* or null */ *rawARingGaloisField(int prime, int dimension)
{
        if (dimension < 0  )
        {
            ERROR(" givaroGF/FFPACK: help, dimension is negative ! ");
            return 0;
        }
     try {
#if defined(HAVE_FFLAS_FFPACK) && defined(HAVE_GIVARO)

        if (prime <= 1  )
        {
            ERROR("givaroGF/FFPACK: expected a prime number p ");
            return 0;
        }

        //-- NEED_POLYNOMIAL_REPRESENTATION is not usable(namespace problems) and is not correct, because the answer depends on the template used for Givaro::GFqDom.
        /*if (Givaro::NEED_POLYNOMIAL_REPRESENTATION(prime,dimension) )  
        {
            ERROR("givaro Galois Field: polynomial representation is needed  - todo ");
            return 0;
        }*/
        if (dimension==1 && M2::ARingZZpFFPACK::getMaxModulus()> prime) 
        {
	  std::cout << "maximum modulus = " << M2::ARingZZpFFPACK::getMaxModulus() << std::endl;
          M2::ARingZZpFFPACK *A = new M2::ARingZZpFFPACK(prime);
          return M2::ConcreteRing<M2::ARingZZpFFPACK>::create(A);
        }
        M2::ARingGF *A = new M2::ARingGF(prime,dimension);
        return M2::ConcreteRing<M2::ARingGF>::create(A);
#else
       ERROR("add --enable-fflas-ffpack --enable-givaro when building M2");
       return 0;
#endif
     }
     catch (exc::engine_error e) {
          ERROR(e.what());
          return NULL;
     }
}

M2_arrayintOrNull rawARingGFPolynomial(const Ring *R)
{
  const M2::ConcreteRing<M2::ARingGF> *RGF = dynamic_cast<const M2::ConcreteRing<M2::ARingGF> *>(R);
  if (RGF == 0)
    {
      ERROR("expected a GaloisField");
      return 0;
    }
  const M2::ARingGF &A = RGF->ring();
  return A.getModPolynomialCoeffs();
}

M2_arrayintOrNull rawARingGFCoefficients(const RingElement *f)
{
  const M2::ConcreteRing<M2::ARingGF> *RGF = dynamic_cast<const M2::ConcreteRing<M2::ARingGF> *>(f->get_ring());
  if (RGF == 0)
  {
      ERROR("expected a GaloisField");
      return 0;
  }
  const M2::ARingGF &A = RGF->ring();
  M2::ARingGF::ElementType a;
  A.from_ring_elem(a, f->get_value());
  return A.fieldElementToM2Array(a);
}
