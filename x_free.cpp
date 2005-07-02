// Copyright 1995 Michael E. Stillman

#include "engine.h"

#include "freemod.hpp"
#include "matrix.hpp"

const Ring *IM2_FreeModule_ring(const FreeModule *F)
{
  return F->get_ring();
}

int IM2_FreeModule_rank(const FreeModule *F)
{
  return F->rank();
}

const M2_string IM2_FreeModule_to_string(const FreeModule *F)
{
  buffer o;
  F->text_out(o);
  return o.to_string();
}

unsigned long int IM2_FreeModule_hash(const FreeModule *F); /* TODO */

const FreeModuleOrNull *IM2_FreeModule_make(const Ring *R, int rank)
{
  if (rank < 0)
    {
      ERROR("freemodule rank must be non-negative");
      return 0;
    }
  return R->make_FreeModule(rank);
}

const FreeModuleOrNull *IM2_FreeModule_make_degs(const Ring *R, 
						 M2_arrayint degs)
{
  const Monoid *D = R->degree_monoid();
  unsigned int eachdeg = D->n_vars();
  unsigned int rank = degs->len / eachdeg;
  if (rank * eachdeg != degs->len)
    {
      ERROR("inappropriate number of degrees");
      return 0;
    }
  int *deg = D->make_one();
  FreeModule *F = R->make_FreeModule();
  for (unsigned int i=0; i<rank; i++)
    {
      D->from_expvector(degs->array + i*eachdeg, deg);
      F->append(deg);
    }
  return F;
}

const FreeModuleOrNull *IM2_FreeModule_make_schreyer(const Matrix *m)
{
  return FreeModule::make_schreyer(m);
}

const M2_arrayint IM2_FreeModule_get_degrees(const FreeModule *F)
{
  const Ring *R = F->get_ring();
  const Monoid *D = R->degree_monoid();
  M2_arrayint result = makearrayint(F->rank() * D->n_vars());
  int next = 0;
  int *exp = newarray(int,D->n_vars());
  for (int i=0; i<F->rank(); i++)
    {
      D->to_expvector(F->degree(i), exp);
      for (int j=0; j<D->n_vars(); j++)
	result->array[next++] = exp[j];
    }
  deletearray(exp);
  return result;
}

const Matrix * IM2_FreeModule_get_schreyer(const FreeModule *F)
{
  return F->get_induced_order();
}

const M2_bool IM2_FreeModule_is_equal(const FreeModule *F, 
				      const FreeModule *G)
/* Determines if F and G are the same graded module.  If one has a
   Schreyer order and one does not, but their ranks and degrees are the
   same, then they are considered equal by this routine. */
{
  return F->is_equal(G);
}


const FreeModuleOrNull * IM2_FreeModule_sum(const FreeModule *F,
					    const FreeModule *G)
{
  return F->direct_sum(G);
}

const FreeModuleOrNull * IM2_FreeModule_tensor(const FreeModule *F,
					       const FreeModule *G)
{
  return F->tensor(G);
}

const FreeModule * IM2_FreeModule_dual(const FreeModule *F)
{
  return F->transpose();
}

const FreeModule * IM2_FreeModule_symm(int n, const FreeModule *F)
{
  return F->symm(n);
}

const FreeModule * IM2_FreeModule_exterior(int n, const FreeModule *F)
{
  return F->exterior(n);
}

const FreeModule * IM2_FreeModule_submodule(const FreeModule *F, 
					    M2_arrayint selection)
{
  return F->sub_space(selection);
}

// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e "
// End:
