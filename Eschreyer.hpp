// Copyright 1999  Michael E. Stillman
#ifndef _Eschreyer_hpp_
#define _Eschreyer_hpp

#include "polyring.hpp"
#include "gbring.hpp"
#include "schorder.hpp"
#include "matrix.hpp"

struct GBMatrix : public our_new_delete {
  const FreeModule *F; // target
  std::vector<gbvector *, gc_allocator<gbvector *> > elems;

  GBMatrix(const Matrix *m);
  GBMatrix(const FreeModule *F);
  void append(gbvector *f); // grabs f
  const FreeModule * get_free_module() const { return F; }
  Matrix *to_matrix();
};

class GBKernelComputation : public object
{
  const PolynomialRing *R;
  const Ring *K;
  GBRing *GR;
  const Monoid *M;
  const SchreyerOrder *SF; // order for F.
  const SchreyerOrder *SG; // order for G.
  const FreeModule *F;  // This is where the action is...
  const FreeModule *G;  // This is where the resulting syzygies live.
        // This MUST be a Schreyer free module compatible with the input!

  array<MonomialIdeal *> mi;  // Used in reduction.
  array<gbvector *> gb;            // This is the "stripped" GB.
  array<gbvector *> syzygies;     // This is basically the result.

  int *PAIRS_mon;   // A monomial in M, used only in new_pairs.
  int *REDUCE_mon;  // A monomial in M, used only in reduce.
  int *REDUCE_exp;  // An exponent vector, used only in reduce.
  int n_ones;
  int n_unique;
  int n_others;
  int total_reduce_count;

  void new_pairs(int i);
  void strip_gb(const std::vector<gbvector *, gc_allocator<gbvector *> > &m);
  void strip_gb(const GBMatrix *m);

  gbvector * make_syz_term(ring_elem c, const int *monom, int comp) const;
  // This routine grabs 'c', and 'monom' should be the total monomial.

  bool find_ring_divisor(const int *exponents, const gbvector *&result);
  int find_divisor(const MonomialIdeal * mi, const int *exponents, int &result);
  // Returns the index of the least element in the monomial order which divides.

  void GBKernelComputation::wipe_unneeded_terms(gbvector * & f);
  // removes every term of f which is not a lead term of some element of gb.

  gbvector * s_pair(gbvector * syz);
  void reduce(gbvector * &g, gbvector * &gsyz);  // Reduces g to zero.  gsyz is real result.
  void geo_reduce(gbvector * &g, gbvector * &gsyz);  // Reduces g to zero.  gsyz is real result.
public:
  GBKernelComputation(const GBMatrix *m);

  virtual ~GBKernelComputation();

  int calc();

  GBMatrix *get_syzygies();

public:
  GBKernelComputation * cast_to_GBKernelComputation() { return this; }
  const GBKernelComputation * cast_to_GBKernelComputation() const { return this; }
};


#endif

// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e "
// End:
