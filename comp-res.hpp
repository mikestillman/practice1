// Copyright 2004 Michael E. Stillman.

#ifndef _comp_res_hpp_
#define _comp_res_hpp_

#include "comp.hpp"
class buffer;

class ResolutionComputation : public Computation
// This is the base type for all resolution computations
{
protected:
  ResolutionComputation();

  virtual bool stop_conditions_ok() = 0;
  // If the stop conditions in stop_ are inappropriate,
  // return false, and use ERROR(...) to provide an error message.

  void betti_init(int lo, int hi, int len, int *&bettis) const;
  M2_arrayint betti_make(int lo, int hi, int len, int *bettis) const;

  void betti_display(buffer &o, M2_arrayint a) const;
public:
  virtual ResolutionComputation * cast_to_ResolutionComputation() { return this;} 

  virtual ~ResolutionComputation();
  virtual void remove_res();

  static ResolutionComputation *choose_res(const Matrix *m,
					   M2_bool resolve_cokernel,
					   int max_level,
					   M2_bool use_max_slanted_degree,
					   int max_slanted_degree,
					   int algorithm,
					   int strategy
					   );
  // Values for algorithm and strategy are documented in engine.h

  virtual void start_computation() = 0;

  //  virtual ComputationStatusCode compute(const StopConditions &stop_, long &complete_thru_this_degree);
  virtual int complete_thru_degree() const = 0;
  // The computation is complete up through this slanted degree.

  ////////////////////////////////
  // Results of the computation //
  ////////////////////////////////
  virtual const MatrixOrNull *get_matrix(int level) = 0;

  virtual const FreeModuleOrNull *get_free(int level) = 0;

  virtual M2_arrayint get_betti(int type) const = 0;
  // type is documented under rawResolutionBetti, in engine.h

  //////////////////////////////////////
  // Statistics and spair information //
  //////////////////////////////////////

  virtual void text_out(buffer &o) const = 0;
  // This displays statistical information, and depends on the
  // gbTrace value.

};

class EngineResolutionComputation : public EngineComputation
// This is the base type for all resolution computations
{
protected:
  ResolutionComputation *C;
protected:
  EngineResolutionComputation(ResolutionComputation *C);
  virtual ~EngineResolutionComputation();
public:
  virtual EngineResolutionComputation * cast_to_EngineResolutionComputation() { return this;} 

  static EngineResolutionComputation * create(ResolutionComputation *C0);

  virtual void destroy();
  
  virtual void start_computation();

  virtual long complete_thru_degree() const;
  // The computation is complete up through this slanted degree.

  ResolutionComputation *get_ResolutionComputation() { return C; }
};


extern "C" void remove_res(void *p, void *cd);
void intern_res(ResolutionComputation *G);


#endif

// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e "
// End:
