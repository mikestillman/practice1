#ifndef __interface_h_
#define __interface_h_

#include "Monomials.h"
#include "MonomialSet.h"
#include "../freemod.hpp"
#include <vector>

class Matrix;

typedef int COEFF_TYPE;

struct poly : public our_new_delete {
  int len;
  COEFF_TYPE *coeffs;
  monomial *monoms; // points into either S, or into a set Sd
  
  // One must be careful about adding two polynomials, since we
  // need to have an order on the monomials in this case
};

enum gbelem_type { 
  ELEM_IN_RING,  // These are ring elements
  ELEM_POSSIBLE_MINGEN,   // These are min GB elements which might also be min gens
  // In the graded case, they ARE minimal generators
  ELEM_MIN_GB,    // These are elements which are minimal GB elements
  ELEM_NON_MIN_GB // These are elements which are not minimal GB elements
};

struct gbelem : public our_new_delete {
  poly f;
  int deg;
  int alpha; // the homogenizing degree
  unsigned char is_minimal;
  gbelem_type minlevel;
};

typedef std::vector<gbelem *, gc_allocator<gbelem *> > gb_array;

void from_M2_vec(MonomialSet *H,
		 const FreeModule *F, 
		 vec v, 
		 int &degree,
		 int &alpha,
		 poly &result);

void from_M2_matrix(const Matrix *m, 
		    MonomialSet *H,
		    gb_array &result_polys);

vec to_M2_vec(poly &f,
	      const FreeModule *F);

Matrix *to_M2_matrix(gb_array &polys, const FreeModule *F);

void spair_testing(MonomialSet *H,
		   gb_array &result_polys);

#endif

// Local Variables:
//  compile-command: "make -C $M2BUILDDIR/Macaulay2/e/linalgGB "
//  End:
