#include "engine.h"
#include "ringmap.hpp"

const Ring * IM2_RingMap_target(const RingMap *F)
{
  return F->get_ring();
}

const M2_string IM2_RingMap_to_string(const RingMap *F)
{
  buffer o;
  F->text_out(o);
  return o.to_string();
}

const unsigned long int IM2_RingMap_hash(const RingMap *F); /* TODO */

const M2_bool IM2_RingMap_is_equal(const RingMap *f, const RingMap *g)
{
  return f->is_equal(g);
}

const RingMap * IM2_RingMap_make(const Matrix *M, const Ring *base); /* TODO */

const RingMap * 
IM2_RingMap_make1(const Matrix *M)
{
  return RingMap::make(M);
}

const RingElementOrNull * 
IM2_RingMap_eval_ringelem(const RingMap *F, 
			  const RingElement *a)
{
  return F->eval(a);
}

const VectorOrNull * 
IM2_RingMap_eval_vector(const RingMap *F, 
			const FreeModule *newTarget,
			const Vector *v)
{
  return F->eval(newTarget,v);
}

const MatrixOrNull * 
IM2_RingMap_eval_matrix(const RingMap *F, 
			const FreeModule *newTarget,
			const Matrix *M)
{
  return F->eval(newTarget,M);
}


// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e"
// End:
