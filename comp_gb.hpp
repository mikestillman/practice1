// Copyright 2004 Michael E. Stillman.

#ifndef _comp_gb_hpp_
#define _comp_gb_hpp_

#include "computation.hpp"
class buffer;

// The following are the return values from s_pair_step,
// These are used in GB_comp, GBinhom_comp
const int SPAIR_DONE   = 0;
const int SPAIR_GB     = 1;
const int SPAIR_SYZ    = 2;
const int SPAIR_ZERO   = 3;
const int SPAIR_MINGEN = 4;
const int SPAIR_GEN    = 5;
const int SPAIR_PAIR   = 6;
const int SPAIR_RING   = 7;
const int SPAIR_REMOVED = 8;
const int SPAIR_DEFERRED = 9;


class GBComputation : public Computation
// This is the base type for all Groebner basis and syzygy computations
{
protected:
  GBComputation();
  virtual ~GBComputation();

  virtual bool stop_conditions_ok() = 0;
  // If the stop conditions in _Stop are inappropriate,
  // return false, and use ERROR(...) to provide an error message.

public:
  static ComputationOrNull *choose_gb(const Matrix *m,
				      M2_bool collect_syz,
				      int n_rows_to_keep,
				      M2_arrayint gb_degrees,
				      M2_bool use_max_degree,
				      int max_degree,
				      int algorithm,
				      int strategy);
  // Values for algorithm and strategy are documented in engine.h

  virtual ComputationOrNull *set_hilbert_function(const RingElement *h);
  // The default version returns an error saying that Hilbert functions cannot be used.

  static ComputationOrNull * force(const Matrix *m,
				   const Matrix *gb,
				   const Matrix *change);

  virtual void start_computation() = 0;

  virtual enum ComputationStatusCode gb_status(int *degree); 
  // The computation is complete up through this degree.

  ////////////////////////////////
  // Results of the computation //
  ////////////////////////////////
  virtual const MatrixOrNull *get_gb();

  virtual const MatrixOrNull *get_mingens();

  virtual const MatrixOrNull *get_change();

  virtual const MatrixOrNull *get_syzygies();

  virtual const MatrixOrNull *get_initial(int nparts);

  ////////////////////////////////
  // Normal forms and lifting ////
  ////////////////////////////////

  virtual const MatrixOrNull *matrix_remainder(const Matrix *m) = 0;

  virtual void matrix_lift(const Matrix *m,
			   MatrixOrNull **result_remainder,
			   MatrixOrNull **result_quotient) = 0;

  virtual int contains(const Matrix *m) = 0;

  //////////////////////////////////////
  // Statistics and spair information //
  //////////////////////////////////////

  virtual void text_out(buffer &o);
  // This displays statistical information, and depends on the
  // gbTrace value.

};


#endif

// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e "
// End:
