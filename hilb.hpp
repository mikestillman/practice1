// Copyright 1996 Michael E. Stillman

#ifndef _hilb_hh_
#define _hilb_hh_

// Computation of Hilbert functions via Bigatti's (et al) algorithm.

#include "comp.hpp"
#include "monideal.hpp"
#include "matrix.hpp"
#include "polyring.hpp"

class RingElement;
class Matrix;
class MonomialIdeal;

class partition_table
    // Partition a monomial ideal into several such that
    // the graph of variables occuring in each is connected.
    // Implemented using a union-find algorithm.
{
  int n_vars;
  int n_sets;
  intarray adad;
  intarray aoccurs;
  int *dad;
  int *occurs;

  int merge_in(int x, int y);
  void merge_in(const int *m);
  int representative(int x);

public:
  partition_table(int nvars);
  ~partition_table() {}

  void reset(int nvars);
  void partition(MonomialIdeal * &I, array<MonomialIdeal *> &result);	// Consumes I.
};

struct hilb_step
{
  hilb_step *up;
  hilb_step *down;
  int i;			// Which monomial ideal is next
  ring_elem h0;			// Hilbert function so far computed 'to the left'
  ring_elem h1;
  int first_sum;		// First monomial ideal which corresponds to the 'sum' part
  array<MonomialIdeal *> monids;	// The (partitoned) array of monomial ideals 
};

class hilb_comp : public mutable_object
{
  const PolynomialRing *S;		// This is the base ring of the monomial ideal
  const PolynomialRing *R;		// This is the output degree ring.
  const Monoid *M;		// S->Nmonoms()
  const Monoid *D;		// R->Nmonoms() == S->degree_monoid()

  // Collected values from the matrix
  const Matrix *input_mat;             // The input matrix
  ring_elem result_poincare;	// The result poincare polynomial from components
				// 0..this_comp-1
  int this_comp;

  // The recursion stack is the following:
  hilb_step *current;

  // Statistics
  int nsteps;
  int depth;
  int maxdepth;
  int nideal;
  int nrecurse;

  // Some useful precomputed values
  ring_elem one;
  ring_elem minus_one;

  // Local variables that are allocated once and for all
  // extreme care is needed for their use!
  int *LOCAL_deg1; // An element of the degree monoid
  intarray LOCAL_vp;
  partition_table part_table;

  int step();			// Returns 0 when done
  void recurse(MonomialIdeal * &I, const int *pivot_vp);
  void do_ideal(MonomialIdeal * I);
  
public:
  hilb_comp(const PolynomialRing *R, const Matrix * M);
  ~hilb_comp();

  void reset();
  void next_monideal();
  int calc(int nsteps);
  int is_done() const;
  RingElement *value();
  void stats() const;

  // static routines
  static int coeff_of(const RingElement *h, int deg);

#if 0
  static int hilbertSeries(const Matrix *M, RingElement * &result);
  // A return of 0 means that the result can be used.  A non-zero return
  // value means that the computation was interrupted, and so control should
  // return to the user.
#endif

  static RingElement *hilbertNumerator(const Matrix *M);
  /* This routine computes the numerator of the Hilbert series
     for coker leadterms(M), using the degrees of the rows of M. 
     NULL is returned if the ring is not appropriate for
     computing Hilbert series, or the computation was interrupted. */
};

#endif

// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e "
// End:
