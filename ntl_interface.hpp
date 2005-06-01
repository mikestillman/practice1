#ifndef _ntl_interface_hpp_
#define _ntl_interface_hpp_

#include <gmp.h>
class ZZ;
class mat_ZZ;
class MutableMatrixXXX;
extern mat_ZZ *makeNTLMatrixZZ(int nrows, int ncols);
void mat_ZZ_set_entry(mat_ZZ *A, long i, long j, mpz_t a);
void mat_ZZ_get_entry(const mat_ZZ *A, long i, long j, mpz_t result);

extern void ntl_ZZ_to_mpz(mpz_t result, const ZZ &a);
extern ZZ ntl_ZZ_from_mpz(mpz_t a);

void dntl_matZZ(const mat_ZZ *A);
void dntl_ZZ(const ZZ *f);
mat_ZZ *mutableMatrix_to_NTL_mat_ZZ(const MutableMatrixXXX *M);
bool ntl_LLL(MutableMatrixXXX *M, long a, long b, int strategy);
  // the threshold is a/b.  strategy is explained for rawLLL in engine.h
#endif

// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e "
// End:





