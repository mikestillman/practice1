// Copyright 1997  Michael E. Stillman

#include "gb2.hpp"
#include "hilb.hpp"
#include "text_io.hpp"
#include "vector.hpp"
#include "matrixcon.hpp"

extern ring_elem hilb(const Matrix &M, const Ring *RR);
extern int compare_type;

gb_emitter::gb_emitter(const Matrix *m)
  : gens(m), g(NULL), n_left(m->n_cols()), n_i(0), n_in_degree(-1)
{
  originalR = m->get_ring()->cast_to_PolynomialRing();
  assert(originalR != 0);
  GR = originalR->get_gb_ring();
  this_degree = m->cols()->lowest_primary_degree() - 1;
  n_gens = 0;			// Also needs to be set at that time.
  these = newarray(int,m->n_cols());
}

gb_emitter::~gb_emitter()
{
  deletearray(these);
}

enum ComputationStatusCode gb_emitter::hilbertNumerator(RingElement *&)
{
  assert(0); // This routine should NEVER be called
  return COMP_DONE;
}

enum ComputationStatusCode gb_emitter::hilbertNumeratorCoefficient(int, int &)
{
  assert(0); // This routine should NEVER be called
  return COMP_DONE;
}

enum ComputationStatusCode gb_emitter::calc_gb(int degree, const intarray & /*stop*/)
{
  // This is when we ship off the elements in this degree.  This should NEVER
  // be called if elements of lower degree have not been sent.
  if (this_degree != degree)
    {
      start_degree(degree);
    }
  for (;;)
    {
      if (system_interruptedFlag)
	return COMP_INTERRUPTED;
      if (n_i >= n_gens) return COMP_DONE;
      if (g != NULL)
	{
	  ring_elem denom;
	  gbvector *v = originalR->translate_gbvector_from_vec(gens->rows(),
					      (*gens)[these[n_i]],
					      denom);
	  g->receive_generator(v, these[n_i], denom);
	}
      n_i++;
      n_left--;
    }
}
enum ComputationStatusCode gb_emitter::calc_gens(int degree, const intarray &stop)
{
  return calc_gb(degree, stop);
}

int gb_emitter::start_degree(int deg)
{
  this_degree = deg;
  n_gens = 0;
  n_i = 0;
  for (int i=0; i<gens->n_cols(); i++)
    {
      if (gens->cols()->primary_degree(i) == this_degree)
	these[n_gens++] = i;
    }
  return n_gens;
}

void gb_emitter::flush()
{
  n_left -= (n_gens - n_i);
  n_gens = 0;
  n_i = 0;
}

bool gb_emitter::is_done()
{
  return (n_left == 0);
}
void gb_emitter::stats() const
{
}

void gb_emitter::text_out(buffer &o) const
{
}

typedef gb_node *gb_node_ptr;

void gbres_comp::setup(const Matrix *m, 
		     int length,
		     int origsyz,
		     int strategy)
{
  int i;
  originalR = m->get_ring()->cast_to_PolynomialRing();
  if (originalR == NULL) assert(0);
  GR = originalR->get_gb_ring();

#warning "FreeModules should be over what ring?"
  FreeModule *Fsyz = originalR->make_Schreyer_FreeModule();
  if (length <= 0)
    {
      ERROR("resolution length must be at least 1");
      length = 1;
    }

  // If origsyz, and length>1, create Fsyz as a Schreyer free
  // if origsyz is smaller, truncate this module...

  if (length > 1 && origsyz > 0)
    {
      if (origsyz > m->n_cols())
	origsyz = m->n_cols();
      int *one = originalR->Nmonoms()->make_one();
      const int *mon;
      for (i=0; i<origsyz; i++)
	{
	  if ((*m)[i] == NULL)
	    mon = one;
	  else
	    {
	      Nterm *t = (*m)[i]->coeff;
	      mon = t->monom;
	    }
	  Fsyz->append_schreyer(m->cols()->degree(i), mon, i);
	}
      originalR->Nmonoms()->remove(one);
    }

  lo_degree = m->cols()->lowest_primary_degree();
  last_completed_degree = lo_degree-1;

  n_nodes = length + 1;
  nodes = newarray(gb_node_ptr,n_nodes);

  nodes[0] = new gb_emitter(m);
  nodes[1] = new gb2_comp(Fsyz,nodes[0],lo_degree,origsyz,1,strategy);
  nodes[0]->set_output(nodes[1]);
  if (n_nodes == 2)
    {
      // Don't compute syzygies at all.
      nodes[1]->set_output(NULL);
    }
  else if (n_nodes >= 3)
    {
      // Compute a resolution to length 'length', with last being
      // a gb node.
      int deg = lo_degree+1;
      if (origsyz > 0) deg--;
      for (i=2; i<n_nodes-1; i++)
	{
	  FreeModule *F = originalR->make_Schreyer_FreeModule();
	  nodes[i] = new gb2_comp(F,nodes[i-1],deg++,-1,i,strategy);
	  nodes[i-1]->set_output(nodes[i]);
	}
      FreeModule *F = originalR->make_Schreyer_FreeModule();
      nodes[n_nodes-1] = new gb2_comp(F,nodes[n_nodes-2],deg++,0,n_nodes-1,strategy);
      nodes[n_nodes-1]->set_output(NULL);      
    }
  strategy_flags = strategy;
}

gbres_comp::gbres_comp(const Matrix *m, int length, int origsyz, int strategy)
{
  setup(m,length,origsyz,strategy);
}

gbres_comp::gbres_comp(const Matrix *m, int length, int origsyz,
		 const RingElement */*hf*/, int strategy)
{
  // MES: check homogeniety
  setup(m, length, origsyz, strategy);
  //nodes[0]->set_HF(hf);
}

gbres_comp::~gbres_comp()
{
  for (int i=0; i<n_nodes; i++)
    {
      nodes[i]->set_output(NULL);
      deleteitem(nodes[i]);
    }
}


//---- state machine (roughly) for the computation ----

bool gbres_comp::stop_conditions_ok()
{
  if (stop_.length_limit <= 0)
    {
      ERROR("length limit out of range");
      return false;
    }

  if (stop_.length_limit != 0 && stop_.length_limit->len > 0)
    {
      ERROR("cannot change length of resolution using this algorithm");
      return false;
    }

  if (stop_.stop_after_degree && stop_.degree_limit != 0)
    {
      ERROR("expected resolution stop degree");
      return false;
    }
  return true;
}

void gbres_comp::start_computation()
{
  intarray stop;
  int old_compare_type = compare_type;
  compare_type = (strategy_flags >> 10);
  if (gbTrace >= 4) 
    {
      buffer o;
      o << "compare=" << compare_type << newline;
      emit(o.str());
    }
  for (int i=lo_degree; !is_done(); i++)
    {
      if (stop_.stop_after_degree && stop_.degree_limit->array[0] < i)
	{
	  set_status(COMP_DONE_DEGREE_LIMIT);
	  compare_type = old_compare_type;
	  return;
	}
      if (gbTrace >= 1)	
	{
	  buffer o;
	  o << "{" << i << "}";
	  emit(o.str());
	}
      enum ComputationStatusCode ret = nodes[n_nodes-1]->calc_gens(i+n_nodes-3, stop);
      if (ret != COMP_DONE)
	{
	  set_status(ret);
	  compare_type = old_compare_type;
	  return;
	}
      last_completed_degree = i;
    }
  compare_type = old_compare_type;
  set_status(COMP_DONE);
}

bool gbres_comp::is_done()
{
  for (int i=0; i<n_nodes; i++)
    if (!nodes[i]->is_done())
      return false;
  return true;
}

int gbres_comp::complete_thru_degree() const
{
  return last_completed_degree;
}

//--- Reduction --------------------------


Matrix *gbres_comp::reduce(const Matrix *m, Matrix *&lift)
{
  const FreeModule *F = nodes[0]->output_free_module();
  if (m->n_rows() != F->rank()) {
       ERROR("expected matrices to have same number of rows");
       return 0;
  }
  MatrixConstructor mat_red(m->rows(), m->cols(), false, m->degree_shift());
  MatrixConstructor mat_lift(nodes[1]->output_free_module(), m->cols(), false);

  for (int i=0; i<m->n_cols(); i++)
    {
      const FreeModule *Fsyz = nodes[1]->output_free_module();

      ring_elem denom;
      gbvector * f = originalR->translate_gbvector_from_vec(F, (*m)[i], denom);
      gbvector * fsyz = GR->gbvector_zero();

      nodes[1]->reduce(f, fsyz);

      vec fv = originalR->translate_gbvector_to_vec_denom(F, f, denom);
      GR->get_flattened_coefficients()->negate_to(denom);
      vec fsyzv = originalR->translate_gbvector_to_vec_denom(Fsyz,fsyz, denom);
      mat_red.set_column(i, fv);
      mat_lift.set_column(i,fsyzv);
    }
  lift = mat_lift.to_matrix();
  return mat_red.to_matrix();
}


//////////////////////////////////
// Obtaining matrices as output //
//////////////////////////////////

const FreeModule *gbres_comp::free_module(int level) const
{
  if (level >= 0 && level <= n_nodes-1)
    return const_cast<FreeModule *>(nodes[level]->output_free_module());
  return nodes[0]->output_free_module()->get_ring()->make_FreeModule();

}
const Matrix *gbres_comp::min_gens_matrix(int level)
{
  if (level <= 0 || level >= n_nodes)
    return Matrix::zero(free_module(level-1), free_module(level), false);
  return nodes[level]->min_gens_matrix();
}
const Matrix *gbres_comp::get_matrix(int level)
{
#warning "if the matrix could change in the future, we MUST copy it"
  if (level <= 0 || level >= n_nodes)
    return Matrix::zero(free_module(level-1), free_module(level), false);
  return nodes[level]->get_matrix();
}

const Matrix *gbres_comp::initial_matrix(int n, int level)
{
  if (level <= 0 || level >= n_nodes)
    return Matrix::zero(free_module(level-1), free_module(level), false);
  return nodes[level]->initial_matrix(n);
}

const Matrix *gbres_comp::gb_matrix(int level)
{
  if (level <= 0 || level >= n_nodes)
    return Matrix::zero(free_module(level-1), free_module(level), false);
  return nodes[level]->gb_matrix();
}

const Matrix *gbres_comp::change_matrix(int level)
{
  if (level <= 0 || level >= n_nodes)
    return Matrix::zero(free_module(level-1), free_module(level), false);
  return nodes[level]->change_matrix();
}

void gbres_comp::stats() const
{
  emit_line("  #gb #pair #comp     m     z     o     u #hilb  #gcd #mons  #chg");
  for (int i=1; i<n_nodes; i++)
    nodes[i]->stats();
}
void gbres_comp::text_out(buffer &o) const
{
  o << "  #gb #pair #comp     m     z     o     u #hilb  #gcd #mons  #chg";
  for (int i=1; i<n_nodes; i++)
    nodes[i]->text_out(o);
}

M2_arrayint gbres_comp::betti_minimal() const
    // Negative numbers represent upper bounds
{
  int lev, i, j, d, k;
  int lo = nodes[0]->output_free_module()->lowest_primary_degree();
  if (n_nodes >= 2)
    {
      int lo1 = nodes[1]->output_free_module()->lowest_primary_degree()-1;
      if (lo1 < lo) lo=lo1;
    }
  if (n_nodes >= 3)
    {
      int lo2 = nodes[2]->output_free_module()->lowest_primary_degree()-2;
      if (lo2 < lo) lo=lo2;
    }
  int hi = lo;
  int len = 1;
  
  intarray *degs = newarray(intarray,n_nodes);
  
  for (lev=0; lev<n_nodes; lev++)
    {
      const FreeModule *F = free_module(lev);
      if (F->rank() > 0)
	len = lev;

      for (i=0; i<F->rank(); i++)
	{
	  d = F->primary_degree(i) - lev;
	  if (d > hi) hi = d;
	  if (d-lo >= degs[lev].length())
	    {
	      for (k=degs[lev].length(); k<=d-lo; k++)
		degs[lev].append(0);
	    }
	  degs[lev][d-lo] += 1;
	}
    }
  for (lev=0; lev<=len; lev++)
    for (j=degs[lev].length(); j<=hi-lo; j++)
      degs[lev].append(0);

  M2_arrayint result = makearrayint(3 + (hi-lo+1)*(len+1));

  result->array[0] = lo;
  result->array[1] = hi;
  result->array[2] = len;

  int next = 3;
  for (d=lo; d<=hi; d++)
    for (lev=0; lev<=len; lev++)
      result->array[next++] = degs[lev][d-lo];

  deletearray(degs);
  return result;
}

const M2_arrayint gbres_comp::get_betti(int type) const
// Only type = 0 (minimal) is supported by this type.
// Should we do the other types as well?
  // type is documented under rawResolutionBetti, in engine.h
{
  if (type == 0)
    return betti_minimal();

  ERROR("received unsupported betti type for this algorithm");
  return 0;
}
/////////////////////////////////////////
// Commands for using this computation //
/////////////////////////////////////////

#if 0
void cmd_gbres_calc(object &op, object &odeg, object &oargs)
{
  gbres_comp *p = op->cast_to_gbres_comp();
  intarray *deg = odeg->intarray_of();
  intarray *args = oargs->intarray_of();
  if (args->length() != 6)
    {
      gError << "res: expected 5 elements in parameter array";
      return;
    }
  int *d;
  if (deg->length() == 0)
    d = NULL;
  else 
    d = deg->raw();

  //args[0] = LengthLimit
  //args[1] = SyzygyLimit
  //args[2] = PairLimit
  //args[3..5] = SyzygyLimit(nSyz, Level, Degree)
  gStack.insert(make_object_int(p->calc(d, *args)));
}
void cmd_gbres_stats(object &op)
{
  gbres_comp *p = op->cast_to_gbres_comp();
  p->stats();
}
void cmd_betti2_pairs(object & /*op*/)
{
#if 0
  gbres_comp *p = op->cast_to_gbres_comp();
  object_intarray *result = new object_intarray;
  //  p->betti_skeleton(*result->intarray_of());
  gStack.insert(result);
#endif
}
void cmd_betti2_remaining(object & /*op*/)
{
#if 0
  gbres_comp *p = op->cast_to_gbres_comp();
  object_intarray *result = new object_intarray;
  //  p->betti_remaining(*result->intarray_of());
  gStack.insert(result);
#endif
}
void cmd_betti2_minimal(object &op)
{
  gbres_comp *p = op->cast_to_gbres_comp();
  object_intarray *result = new object_intarray;
  p->betti_minimal(*result->intarray_of());
  gStack.insert(result);
}
void cmd_betti2_nmonoms(object & /*op*/)
{
#if 0
  gbres_comp *p = op->cast_to_gbres_comp();
  object_intarray *result = new object_intarray;
  //  p->betti_nmonoms(*result->intarray_of());
  gStack.insert(result);
#endif
}

void cmd_gbres_Nmap(object &op, object &on)
{
  gbres_comp *p = op->cast_to_gbres_comp();
  int n = on->int_of();
  gStack.insert(p->get_matrix(n));
}

void cmd_gbres_map(object &op, object &on)
{
  gbres_comp *p = op->cast_to_gbres_comp();
  int n = on->int_of();
  gStack.insert(p->get_matrix(n));
}

void cmd_gbres_Nmodule(object &op, object &on)
{
  gbres_comp *p = op->cast_to_gbres_comp();
  int n = on->int_of();
  gStack.insert(p->free_module(n));
}

void cmd_gbres_module(object &op, object &on)
{
  gbres_comp *p = op->cast_to_gbres_comp();
  int n = on->int_of();
  gStack.insert(p->free_module(n));
}
void cmd_gbres_mingens(object &op, object &on)
{
  gbres_comp *p = op->cast_to_gbres_comp();
  int n = on->int_of();
  gStack.insert(p->min_gens_matrix(n));
}
void cmd_gbres_getgb(object &op, object &on)
{
  gbres_comp *p = op->cast_to_gbres_comp();
  int n = on->int_of();
  gStack.insert(p->gb_matrix(n));
}
void cmd_gbres_change(object &op, object &on)
{
  gbres_comp *p = op->cast_to_gbres_comp();
  int n = on->int_of();
  gStack.insert(p->change_matrix(n));
}
void cmd_gbres_initial(object &op, object &on, object &olevel)
{
  gbres_comp *p = op->cast_to_gbres_comp();
  int n = on->int_of();
  int level = olevel->int_of();
  gStack.insert(p->initial_matrix(n,level));
}

int i_gbres_cmds()
{
  // Resolutions
  //install(ggres, cmd_gbres, TY_MATRIX, TY_INT, TY_INT);
  install(ggcalc, cmd_gbres_calc, TY_GBRES_COMP, TY_INTARRAY, TY_INTARRAY);

  install(ggstats, cmd_gbres_stats, TY_GBRES_COMP);
  install(ggpairs, cmd_betti2_pairs, TY_GBRES_COMP);
  install(ggremaining, cmd_betti2_remaining, TY_GBRES_COMP);
  install(ggbetti, cmd_betti2_minimal, TY_GBRES_COMP);
  install(ggnmonoms, cmd_betti2_nmonoms, TY_GBRES_COMP);

  install(ggresmap, cmd_gbres_map, TY_GBRES_COMP, TY_INT);
  install(ggresNmap, cmd_gbres_Nmap, TY_GBRES_COMP, TY_INT);
  install(ggresmodule, cmd_gbres_module, TY_GBRES_COMP, TY_INT);
  install(ggresNmodule, cmd_gbres_Nmodule, TY_GBRES_COMP, TY_INT);

  install(gggetmingens, cmd_gbres_mingens, TY_GBRES_COMP, TY_INT);
  install(gggetgb, cmd_gbres_getgb, TY_GBRES_COMP, TY_INT);
  install(gggetchange, cmd_gbres_change, TY_GBRES_COMP, TY_INT);
  install(gginitial, cmd_gbres_initial, TY_GBRES_COMP, TY_INT, TY_INT);
  return 1;
}
#endif

// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e "
// End:
