// Copyright 2008  Michael E. Stillman

#ifndef _complex_h_
#define _complex_h_

/* The interface is similar to mpfr:
   Every gmp_CC struct needs to be initialized with init or init_set.
   All rounding is GMP_RNDN.
   Resulting values are the first argument
*/

#if !defined(SAFEC_EXPORTS)
#include <engine-exports.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

  void mpfc_init(gmp_CC result, long precision);
  void mpfc_clear(gmp_CC result);
  void mpfc_init_set(gmp_CC result, gmp_CC a);
  void mpfc_set_si(gmp_CC result, long re);
  void mpfc_set(gmp_CC result, gmp_CC a);
  int mpfc_is_zero(gmp_CC a);
  void mpfc_add(gmp_CC result, gmp_CC a, gmp_CC b);
  void mpfc_sub(gmp_CC result, gmp_CC a, gmp_CC b);
  void mpfc_mul(gmp_CC result, gmp_CC a, gmp_CC b);
  void mpfc_invert(gmp_CC result, gmp_CC v);

  void mpfc_sub_mult(gmp_CC result, gmp_CC a, gmp_CC b);
  /* result -= a*b */

  void mpfc_div(gmp_CC result, gmp_CC a, gmp_CC b);
  void mpfc_abs(gmp_RR result, gmp_CC a);
  void mpfc_sqrt(gmp_CC result, gmp_CC a);
  void mpfc_conj(gmp_CC result, gmp_CC a);

#if defined(__cplusplus)
}
#endif

#endif

// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e "
// indent-tabs-mode: nil
// End:
