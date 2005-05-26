// Copyright 2005, Michael Stillman

// This file contains routines which often conflict with our names

#include <cstdio>
#include <gc.h>
#include <gmp.h>
#include <NTL/mat_ZZ.h>
#include "ntl_interface.hpp"

#define ALLOC(p) (((long *) (p))[0])
#define SIZE(p) (((long *) (p))[1])
#define DATA(p) ((mp_limb_t *) (((long *) (p)) + 2))

void ntl_ZZ_to_mpz(mpz_t result, const ZZ &a)
{
  if (a == 0)
    {
      mpz_set_ui(result,0);
      return;
    }
  long size = SIZE(a.rep);
  if (size < 0) size = -size;
  mpz_import(result, 
	     size,
	     -1, /* most sig first */
	     sizeof(long), 
	     0, /* native endian-ness */
	     0, /* NAILS stuff */
	     DATA(a.rep));
  if (SIZE(a.rep) < 0)
    mpz_neg(result,result);
}

ZZ ntl_ZZ_from_mpz(mpz_t a)
{
  // Make sure this handles -1, 0, 1 correctly too!
  if (mpz_sgn(a) == 0)
    {
      return ZZ::zero();
    }
  long size = mpz_size(a);
  ZZ result(INIT_SIZE, size);
  long *p = static_cast<long *>(result.rep);
  for (int i=0; i<size; i++)
    p[2+i] = a->_mp_d[i];
  p[1] = a->_mp_size;
  return result;
}

void removeNTL_mat_ZZ(void *p, void *cd)
{
  mat_ZZ *A = static_cast<mat_ZZ *>(p);
  fprintf(stderr, "removing mat_ZZ\n");
  delete A;
}

mat_ZZ *makeNTLMatrixZZ(int nrows, int ncols)
{
  GC_finalization_proc notused1;
  void *notused2;
  mat_ZZ *X = new mat_ZZ;	// this uses builtin new, so is probably a memory leak.
  X->SetDims(nrows,ncols);
  GC_register_finalizer(X, removeNTL_mat_ZZ,
			0,
			&notused1, &notused2);
  return X;
}

void mat_ZZ_set_entry(mat_ZZ *A, long i, long j, mpz_t a)
{
  ZZ b = ntl_ZZ_from_mpz(a);
  (*A)(i+1,j+1) = b;
}

void mat_ZZ_get_entry(mat_ZZ *A, long i, long j, mpz_t result)
{
  ZZ t = (*A)(i+1,j+1);
  ntl_ZZ_to_mpz(result, t);
}


// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e "
// End:
