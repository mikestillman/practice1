// Copyright 1996.  Michael E. Stillman

#include "respoly2.hpp"
#include "res2.hpp"
#include "geores.hpp"

stash *res2_pair::mystash;
stash *res2_comp::mystash;
stash *auto_reduce_node::mystash;

extern char system_interrupted;
extern int comp_printlevel;

static int n_ones = 0;
static int n_unique = 0;
static int n_others = 0;

#include "res_aux2.cpp"

int res2_comp::skeleton(int level)
  // Compute the skeleton of the resolution
  // returns COMP_DONE or COMP_INTERRUPTED
{
  res2_pair *p;
  if (resn[level]->state != RES_SKELETON) return COMP_DONE;
  // If we are new here, next_pairs will be null, so we should sort
  if (resn[level]->next_pair == NULL)
    {
      sort_skeleton(resn[level]->pairs);
      int n = 0;
      for (p = resn[level]->pairs; p != NULL; p = p->next)
	{
	  p->me = n++;
	  p->pair_num = p->me;
	}
      resn[level]->next_pair = resn[level]->pairs;
    }


  // Now compute the pairs at the next level
  for (;;)
    {
      p = resn[level]->next_pair;
      if (p == NULL) break;
      resn[level]->next_pair = p->next;
      // The following will only insert pairs of degree > topdegree
      // so this routine may be used to increase the degree bound
      // note: also need to redo monomial ideals...
      new_pairs(p);
      system_spincursor();	  
      if (system_interrupted) return COMP_INTERRUPTED;	  
    }
  resn[level]->state = RES_MONORDER;
  return COMP_DONE;
}

void res2_comp::increase_level(int newmax)
{
  for (int i=resn.length(); i<=newmax+2; i++)
    {
      res2_level *p = new res2_level;
      p->pairs = NULL;
      p->next_pair = NULL;
      p->state = RES_SKELETON;
      p->npairs = 0;
      p->nleft = 0;
      p->nminimal = 0;
      p->nthrown = 0;
      resn.append(p);
    }
  length_limit = newmax;
}

static int PairLimit;
static int SyzygyLimit;

int res2_comp::do_all_pairs(int level, int degree)
{
  // First compute all pairs necessary for doing this (level,degree)
  // and then actually compute the ones in that degree.
  if (level <= 0 || level > length_limit+1) return COMP_COMPUTING;
  //  if (degree <= 0 || degree > hidegree) return COMP_COMPUTING;
  if (level < resn.length() && 
      (resn[level]->next_pair == NULL ||
       degree < resn[level]->next_pair->degree))
    return COMP_COMPUTING;
  int ret = do_all_pairs(level, degree-1);
  if (ret != COMP_COMPUTING) return ret;
  ret = do_all_pairs(level+1, degree-1);
  if (ret != COMP_COMPUTING) return ret;
  ret = do_pairs(level,degree);
  return ret;
}

int res2_comp::do_pairs(int level, int degree)
{
  // Find the first pair in this degree, and count the number of
  // pairs to compute in this degree.
  // If this number is 0, continue
  res2_pair *p;

  if (comp_printlevel >= 2)
    {
      p = resn[level]->next_pair;
      int nelems = 0;
      while (p != NULL && p->degree == degree)
	{
	  if (p->syz_type == SYZ2_S_PAIR ||
	      p->syz_type == SYZ2_MAYBE_MINIMAL)
	    nelems++;
	  p = p->next;
	}
      if (nelems > 0)
	{
	  buffer o;
	  o << "[" << degree << " " << level << " " << nelems << "]";
	  emit(o.str());
	}
    }
  for (p = resn[level]->next_pair; p != NULL; p = p->next)
    {
      if (p->degree != degree) break;
      resn[level]->next_pair = p->next;
      if (p->syz_type == SYZ2_S_PAIR
	  || p->syz_type == SYZ2_MAYBE_MINIMAL)
	{
	  handle_pair(p);
	  if (--PairLimit == 0) return COMP_DONE_PAIR_LIMIT;
	  if (SyzygyLimit >= 0 && nminimal >= SyzygyLimit)
	    return COMP_DONE_SYZ_LIMIT;
	}
      system_spincursor();
      if (system_interrupted) return COMP_INTERRUPTED;
    }
  return COMP_COMPUTING;
}

int res2_comp::do_pairs_by_level(int level)
{
  // The pairs should be sorted in ascending monomial order
  
  res2_pair *p;

  if (comp_printlevel >= 2)
    {
      int nelems = resn[level]->nleft;
      if (nelems > 0)
	{
	  buffer o;
	  o << "[lev " << nelems << ']';
	  emit(o.str());
	}
    }
  for (p = resn[level]->next_pair; p != NULL; p = p->next)
    {
      resn[level]->next_pair = p->next;
      handle_pair_by_level(p);
      if (--PairLimit == 0) return COMP_DONE_PAIR_LIMIT;
      system_spincursor();
      if (system_interrupted) return COMP_INTERRUPTED;
    }

  if (do_by_level == 2) 
    {
      // The following is experimental
      // Remove any term that cannot appear as a lead term.
      // This destroys the resolution as we go, so in general, we would have
      // to place this information elsewhere.
      int nmonoms = 0;
      int nkilled = 0;
      for (p = resn[level]->pairs; p != NULL; p = p->next)
	{
	  res2term *f = p->syz;
	  res2term head;
	  res2term *g = &head;
	  for (f = p->syz; f != NULL; f = f->next)
	    {
	      if (f->comp->mi.length() > 0)
		{
		  g->next = R->new_term(K->copy(f->coeff),
					f->monom,
					f->comp);
		  g = g->next;
		  nmonoms++;
		}
	      else
		nkilled++;
	    }
	  g->next = NULL;
	  p->pivot_term = head.next;
	}
      if (comp_printlevel >= 3)
	{
	  buffer o;
	  o << "[kept " << nmonoms << " killed " << nkilled << "]";
	  emit(o.str());
	}
    }
  return COMP_DONE;
}

int res2_comp::do_pairs_by_degree(int level, int degree)
{
  // Find the first pair in this degree, and count the number of
  // pairs to compute in this degree.
  // If this number is 0, continue
  res2_pair *p;

  if (comp_printlevel >= 2 && level > 1)
    {
      p = resn[level]->next_pair;
      int nelems = 0;
      while (p != NULL && p->degree == degree)
	{
	  nelems++;
	  p = p->next;
	}
      if (nelems > 0)
	{
	  buffer o;
	  o << '[' << nelems << ']';
	  emit(o.str());
	}
    }
  for (p = resn[level]->next_pair; p != NULL; p = p->next)
    {
      if (p->degree != degree) break;
      resn[level]->next_pair = p->next;
      if (p->syz_type == SYZ2_S_PAIR || p->syz_type == SYZ2_NOT_NEEDED
	  || (p->syz_type == SYZ2_MAYBE_MINIMAL && level < length_limit+1))
	{
	  handle_pair_by_degree(p);
	  if (--PairLimit == 0) return COMP_DONE_PAIR_LIMIT;
	  if (SyzygyLimit >= 0 && nminimal >= SyzygyLimit)
	    return COMP_DONE_SYZ_LIMIT;
	}
      system_spincursor();
      if (system_interrupted) return COMP_INTERRUPTED;
    }
  return COMP_COMPUTING;
}

int res2_comp::calc(const int *DegreeLimit, 
		   int LengthLimit, 
		   int ArgSyzygyLimit,
		   int ArgPairLimit,
		   int /*SyzLimitValue*/,
		   int /*SyzLimitLevel*/,
		   int /*SyzLimitDegree*/)
{
  int n, level;
  res2_pair *p;

  if (ArgPairLimit == 0)
    return COMP_DONE_PAIR_LIMIT;
  if (ArgSyzygyLimit == 0)
    return COMP_DONE_SYZ_LIMIT;

  PairLimit = ArgPairLimit;
  SyzygyLimit = ArgSyzygyLimit;

  if (LengthLimit <= 0)
    {
      gError << "length limit out of range";
      return COMP_ERROR;
    }
  if (LengthLimit > length_limit)
    {
      increase_level(LengthLimit);
      
    }
  length_limit = LengthLimit;
  int compute_level = COMPUTE_RES;

  for (level=1; level<=length_limit+1; level++)
    {
      int ret = skeleton(level);
      if (ret != COMP_DONE) return ret;
    }      

  if (comp_printlevel >= 3)
    {
      buffer o;
      o << "--- The total number of pairs in each level/slanted degree -----" << newline;
      intarray a;
      betti_skeleton(a);
      betti_display(o, a);
      emit(o.str());
    }

  // The skeleton routine sets the following:
    // hidegree: highest slanted degree found so far
    // a flag for each level as to whether the level has new elements
    //    since the last time the pairs were sorted for reduction


  if (compute_level <= COMPUTE_SKELETON) return COMP_DONE;

  // Set the monomial order for each level, and then prepare for
  // reductions
  for (level=1; level<=length_limit+1; level++)
    {
      if (resn[level]->state != RES_MONORDER) continue;
      // The sort will use compare_num's from syz->comp->compare_num
      // and will use these numbers to break ties:
      for (n=0, p = resn[level]->pairs; p != NULL; p=p->next, n++)
	p->compare_num = n;
      sort_monorder(resn[level]->pairs);
      for (n=0, p = resn[level]->pairs; p != NULL; p=p->next, n++)
	p->compare_num = n;
      sort_reduction(resn[level]->pairs);
      resn[level]->state = RES_REDUCTIONS;
      resn[level]->next_pair = resn[level]->pairs;
    }

  for (level=1; level<=length_limit+1; level++)
    {
      resn[level]->next_pair = resn[level]->pairs;
    }

  // Here: possibly compute the resolution of the monomial ideal first.

  if (compute_level <= COMPUTE_MONOMIAL_RES) return COMP_DONE;

  if (do_by_level != 0)
    {
      for (level=1; level<=length_limit; level++)
	{
	  int ret = do_pairs_by_level(level);
	  if (ret != COMP_DONE) return ret;
	}
      return COMP_DONE;
    }
  
  if (do_by_degree)
    {
      for (int deg=0; deg<=hidegree; deg++)
	{
	  if (DegreeLimit != NULL && deg > *DegreeLimit - lodegree)
	    return COMP_DONE_DEGREE_LIMIT;
	  if (comp_printlevel >= 1)
	    {
	      buffer o;
	      o << '{' << deg+lodegree << '}';
	      emit(o.str());
	    }
	  for (level=1; level<=length_limit; level++)
	    {
	      int ret = do_pairs_by_degree(level, deg);
	      if (ret != COMP_COMPUTING) return ret;
	    }
	}
      return COMP_DONE;
    }

  // Now do all of the reductions
  for (int deg=0; deg<=hidegree; deg++)
    {
      if (DegreeLimit != NULL && deg > *DegreeLimit - lodegree)
	return COMP_DONE_DEGREE_LIMIT;
      if (comp_printlevel >= 1)
	{
	  buffer o;
	  o << '{' << deg+lodegree << '}';
	  emit(o.str());
	}
      for (level=1; level<=length_limit+1; level++)
	{
	  int ret = do_all_pairs(level, deg);
	  if (ret != COMP_COMPUTING) return ret;
	  if (level > projdim) break;
	}
    }
#if 0
  //This is the older, working version...
  // Now do all of the reductions
  for (int deg=0; deg<=hidegree; deg++)
    {
      if (DegreeLimit != NULL && deg > *DegreeLimit - lodegree)
	return COMP_DONE_DEGREE_LIMIT;
      if (comp_printlevel >= 1)
	{
	  buffer o;
	  o << '{' << deg+lodegree << '}';
	  emit(o.str());
	}
      for (level=1; level<=length_limit+1; level++)
	{
	  int ret = do_pairs(level, deg);
	  if (ret != COMP_COMPUTING) return ret;
	}
    }
#endif
  return COMP_DONE;
}

//////////////////////////////////////////////
//  Initialization of a computation  /////////
//////////////////////////////////////////////

void res2_comp::initialize(Matrix mat, 
			   int LengthLimit,
			   bool UseDegreeLimit,
			   int SlantedDegreeLimit,
			   int SortStrategy)
{
  int i;

  P = mat.get_ring()->cast_to_PolynomialRing();
  assert(P != NULL);
  R = new res2_poly((PolynomialRing *)P);
  M = P->Nmonoms();
  K = P->Ncoeffs();
  bump_up(P);
  bump_up(K);
  generator_matrix = mat;

  length_limit = -3;  // The resolution is always kept at least in range
                      // 0 .. length_limit + 2.
  increase_level(LengthLimit);

  projdim = 0;
  max_mon_degree = M->max_degree();

  if (mat.n_rows() > 0)
    {
      lodegree = mat.rows()->primary_degree(0);
      for (i=1; i<mat.n_rows(); i++)
	if (lodegree > mat.rows()->primary_degree(i))
	  lodegree = mat.rows()->primary_degree(i);
    }
  else
    lodegree = 0;

  // This can't actually set lodegree, can it?
  for (i=0; i<mat.n_cols(); i++)
    if (lodegree > mat.cols()->primary_degree(i) - 1)
      lodegree = mat.cols()->primary_degree(i) - 1;

  hidegree = 0;  // hidegree is an offset from 'lodegree'.

  have_degree_limit = UseDegreeLimit;
  hard_degree_limit = SlantedDegreeLimit;

  nleft = 0;
  npairs = 0;
  nminimal = 0;
  total_reduce_count = 0;

  n_ones = 0;
  n_unique = 0;
  n_others = 0;

  // Do level 0
  next_component = 0;
  for (i=0; i<mat.n_rows(); i++)
    {
      res2_pair *p = new_base_res2_pair(i);
      base_components.append(p);
    }
  for (i=base_components.length()-1; i>=0; i--)
    {
      res2_pair *p = base_components[i];
      insert_pair(p);
    }

  // Do level 1
  for (i=0; i<generator_matrix.n_cols(); i++)
    if (generator_matrix[i] != NULL)
      {
	res2_pair *p = new_res2_pair(i); // Makes a generator 'pair'
	insert_pair(p);
      }

  REDUCE_exp = new int[P->n_vars()];
  REDUCE_mon = M->make_one();
  PAIRS_mon = M->make_one();
  MINIMAL_mon = M->make_one();

  skeleton_sort = SortStrategy & 63;
  reduction_sort = (SortStrategy >> 6) & 63;
  reduction_sort |= FLAGS_DEGREE;      // ALWAYS do by increasing degree.
  do_by_level = (SortStrategy & FLAGS_LEVEL ? 1 : 0);
  if (SortStrategy & FLAGS_LEVEL_STRIP) do_by_level = 2;
  do_by_degree = (SortStrategy & FLAGS_DEGREELEVEL ? 1 : 0);
  auto_reduce = (SortStrategy & FLAGS_AUTO) >> SHIFT_AUTO;
  use_respolyHeaps = (SortStrategy & FLAGS_GEO ? 1 : 0);

  if (do_by_degree) do_by_level = 0;
  if (comp_printlevel >= 3)
    {
      buffer o;
      o << "auto-reduce level = " << auto_reduce << newline;
      if (do_by_level)
	{
	  o << "computing resolution level by level" << newline;
	  if (do_by_level == 2)
	    o << "with strip optimization" << newline;
	}
      if (do_by_degree)
	o << "computing resolution degree by slanted degree" << newline;
      if (use_respolyHeaps)
	o << "using heap based reduction" << newline;
      o << "skeleton order = ";
      display_order(o, skeleton_sort);
      o << "reduction sort = ";
      display_order(o, reduction_sort);
      emit(o.str());
    }
}

void res2_comp::display_order(buffer &o, int sortval) const
{
  o << "[";
  switch (sortval & FLAGS_SORT) {
  case COMPARE_LEX:
    o << "LEX";
    break;
  case COMPARE_LEX_EXTENDED:
    o << "LEX1";
    break;
  case COMPARE_LEX_EXTENDED2:
    o << "LEX2";
    break;
  case COMPARE_ORDER:
    o << "ORDER";
    break;
  default:
    o << "bad order";
    break;
  }
  if (sortval & FLAGS_DEGREE)
    o << " (ascending degree first)";
  if (sortval & FLAGS_DESCEND)
    o << " descend";
  else
    o << " ascend";
  if ((sortval & FLAGS_REVERSE) && ((sortval & FLAGS_SORT) != COMPARE_ORDER))
    o << " reverse";
  o << "]" << newline;
}

res2_comp::res2_comp(Matrix m, 
		     int LengthLimit, 
		     bool UseDegreeLimit,
		     int SlantedDegreeLimit,
		     int SortStrategy)
{
  initialize(m, 
	     LengthLimit, 
	     UseDegreeLimit,
	     SlantedDegreeLimit,
	     SortStrategy);
}

void res2_comp::remove_res2_pair(res2_pair *p)
{
  if (p == NULL) return;
  R->remove(p->syz);
  delete p;
}

void res2_comp::remove_res2_level(res2_level *lev)
{
  if (lev == NULL) return;
  while (lev->pairs != NULL)
    {
      res2_pair *p = lev->pairs;
      lev->pairs = p->next;
      remove_res2_pair(p);
    }
  delete lev;
}

res2_comp::~res2_comp()
{
  int i;
  for (i=0; i<resn.length(); i++)
    remove_res2_level(resn[i]);

  delete [] REDUCE_exp;
  M->remove(REDUCE_mon);
  M->remove(PAIRS_mon);
  M->remove(MINIMAL_mon);

  bump_down(P);
  bump_down(K);
  delete R;
}
//////////////////////////////////////////////
//  Data structure insertion and access  /////
//////////////////////////////////////////////

res2_pair *res2_comp::new_res2_pair(res2_pair *first, 
				 res2_pair * /* second */,
				 const int *basemon)
{
  res2_pair *p = new res2_pair;
  p->next = NULL;
  p->me = next_component++;
  p->pair_num = p->me;
  p->syz_type = SYZ2_S_PAIR;
  p->level = first->level + 1;
  p->degree = M->primary_degree(basemon) + first->degree
    - M->primary_degree(first->syz->monom) - 1;
  p->compare_num = 0;		// Will be set after pairs are done
  p->syz = R->new_term(K->from_int(1), basemon, first);
#if 0
  if (second != NULL)
    p->syz->next = R->new_term(K->from_int(-1), basemon, second);
#endif
  p->mi = MonomialIdeal(P);
  p->pivot_term = NULL;

  return p;
}

res2_pair *res2_comp::new_base_res2_pair(int i)
{
  res2_pair *p = new res2_pair;
  p->next = NULL;
  p->me = next_component++;
  p->pair_num = p->me;
  p->syz_type = SYZ2_MINIMAL;
  p->level = 0;
  p->degree = generator_matrix.rows()->primary_degree(i) - lodegree;
  p->compare_num = i;
  int *m = M->make_one();
  p->syz = R->new_term(K->from_int(1), m, p); // circular link...
  M->remove(m);
  p->mi = MonomialIdeal(P);
  p->pivot_term = NULL;
  return p;
}

res2_pair *res2_comp::new_res2_pair(int i)
{
  res2_pair *p = new res2_pair;
  p->next = NULL;
  p->me = next_component++;
  p->pair_num = p->me;
  p->syz_type = SYZ2_S_PAIR;
  p->level = 1;
  p->degree = generator_matrix.cols()->primary_degree(i) - 1 - lodegree;
  p->compare_num = 0;
  p->syz = R->from_vector(base_components, generator_matrix[i]);
  p->mi = MonomialIdeal(P);
  p->pivot_term = NULL;
  return p;
}

void res2_comp::insert_pair(res2_pair *q)
{
  // This is where we insert the pair:
  // If the degree is not in the range lodegree..hard_degree_limit+1
  // then don't insert it.
  if (q->level >= 1)
    {
      if (have_degree_limit)
	{
	  if (q->degree + lodegree > hard_degree_limit+1)
	    {
	      resn[q->level]->nthrown++;
	      remove_res2_pair(q);
	      return;
	    }
	  if (q->degree <= hard_degree_limit && q->degree > hidegree)
	    hidegree = q->degree;
	}
      else if (q->degree > hidegree)
	hidegree = q->degree;
    }
    
  q->next = resn[q->level]->pairs;
  resn[q->level]->pairs = q;
  npairs++;
  resn[q->level]->npairs++;
  if (q->syz_type == SYZ2_S_PAIR)
    {
      nleft++;
      resn[q->level]->nleft++;
    }
  else
    {
      nminimal++;
      resn[q->level]->nminimal++;
    }
}

void res2_comp::multi_degree(const res2_pair *p, int *deg) const
{
  const res2_pair *q;
  M->multi_degree(p->syz->monom, deg);
  for (q = p; q->level != 0; q = q->syz->comp);
  degree_monoid()->mult(deg, generator_matrix.rows()->degree(q->me), deg);
}

//////////////////////////////////////////////
//  Sorting //////////////////////////////////
//////////////////////////////////////////////

static int EXP1[1000], EXP2[1000], EXP3[1000], EXP4[1000];

static int compare_type = COMPARE_LEX;
static int compare_use_degree = 0;
static int compare_use_descending = 1;
static int compare_use_reverse = 0;

int res2_comp::compare_res2_pairs(res2_pair *f, res2_pair *g) const
{
  // Lots of different orders appear here, controlled by the above
  // static variables.
  // if compare(f,g) returns -1, this says place g BEFORE f on the list.
  int cmp, df, dg;

  if (compare_use_degree)
    {
      df = f->degree;
      dg = g->degree;
      if (df > dg) return -1;
      if (df < dg) return 1;
    }

  switch (compare_type) {
  case COMPARE_MONORDER:
    cmp = f->syz->comp->compare_num - g->syz->comp->compare_num;
    if (cmp < 0) return 1;
    if (cmp > 0) return -1; // Lower compare num's should be on list earlier
    cmp = f->compare_num - g->compare_num;
    if (cmp < 0) return 1;
    if (cmp > 0) return -1;
    return 0;
  case COMPARE_LEX:
    M->to_expvector(f->syz->monom, EXP1);
    M->to_expvector(g->syz->monom, EXP2);
    if (compare_use_reverse)
      {
	for (int i=M->n_vars()-1; i>=0; i--)
	  {
	    if (EXP1[i] < EXP2[i]) return -compare_use_descending;
	    if (EXP1[i] > EXP2[i]) return compare_use_descending;
	  }
      }
    else 
      {
	for (int i=0; i<M->n_vars(); i++)
	  {
	    if (EXP1[i] < EXP2[i]) return -compare_use_descending;
	    if (EXP1[i] > EXP2[i]) return compare_use_descending;
	  }
      }
    // Fall through to COMPARE_ORDER
  case COMPARE_ORDER:
    cmp = M->compare(f->syz->monom, g->syz->monom);
    if (cmp != 0) return compare_use_descending*cmp;
    cmp = f->syz->comp->compare_num - g->syz->comp->compare_num;
    if (cmp < 0) return -compare_use_descending;
    if (cmp > 0) return compare_use_descending;
    return 0;
  case COMPARE_LEX_EXTENDED:
    while (f->level >= 1)
      {
	M->to_expvector(f->syz->monom, EXP1);
	M->to_expvector(g->syz->monom, EXP2);
	if (compare_use_reverse)
	  {
	    for (int i=M->n_vars()-1; i>=0; i--)
	      {
		if (EXP1[i] < EXP2[i]) return -compare_use_descending;
		if (EXP1[i] > EXP2[i]) return compare_use_descending;
	      }
	  }
	else 
	  {
	    for (int i=0; i<M->n_vars(); i++)
	      {
		if (EXP1[i] < EXP2[i]) return -compare_use_descending;
		if (EXP1[i] > EXP2[i]) return compare_use_descending;
	      }
	  }
	// Go down one level
	f = f->syz->comp;
	g = g->syz->comp;
      }
    return 0;
  case COMPARE_LEX_EXTENDED2:
    M->to_expvector(f->syz->monom, EXP1);
    M->to_expvector(g->syz->monom, EXP2);
    if (compare_use_reverse)
      {
	for (int i=M->n_vars()-1; i>=0; i--)
	  {
	    if (EXP1[i] < EXP2[i]) return -compare_use_descending;
	    if (EXP1[i] > EXP2[i]) return compare_use_descending;
	  }
      }
    else 
      {
	for (int i=0; i<M->n_vars(); i++)
	  {
	    if (EXP1[i] < EXP2[i]) return -compare_use_descending;
	    if (EXP1[i] > EXP2[i]) return compare_use_descending;
	  }
      }
    while (f->level >= 1)
      {
	// Go down one level
	M->to_expvector(f->syz->monom, EXP1);
	M->to_expvector(g->syz->monom, EXP2);
	f = f->syz->comp;
	g = g->syz->comp;
	M->to_expvector(f->syz->monom, EXP3);
	M->to_expvector(g->syz->monom, EXP4);
	if (compare_use_reverse)
	  {
	    for (int i=M->n_vars()-1; i>=0; i--)
	      {
		if (EXP1[i]-EXP3[i] < EXP2[i]-EXP4[i]) return -compare_use_descending;
		if (EXP1[i]-EXP3[i] > EXP2[i]-EXP4[i]) return compare_use_descending;
	      }
	  }
	else 
	  {
	    for (int i=0; i<M->n_vars(); i++)
	      {
		if (EXP1[i]-EXP3[i] < EXP2[i]-EXP4[i]) return -compare_use_descending;
		if (EXP1[i]-EXP3[i] > EXP2[i]-EXP4[i]) return compare_use_descending;
	      }
	  }
      }
    return 0;
  default:
    return 0;
  }
}

res2_pair *res2_comp::merge_res2_pairs(res2_pair *f, res2_pair *g) const
{
  if (g == NULL) return f;
  if (f == NULL) return g;
  res2_pair head;
  res2_pair *result = &head;
  while (1)
    switch (compare_res2_pairs(f, g))
      {
      case 0:
      case -1:
	result->next = g;
	result = result->next;
	g = g->next;
	if (g == NULL) 
	  {
	    result->next = f;
	    return head.next;
	  }
	break;
      case 1:
	result->next = f;
	result = result->next;
	f = f->next;
	if (f == NULL) 
	  {
	    result->next = g; 
	    return head.next;
	  }
	break;
	//      case 0:
	//	assert(0);
      }
}

void res2_comp::sort_res2_pairs(res2_pair *& p) const
{
  // These elements are sorted in ascending 'me' values
  if (p == NULL || p->next == NULL) return;
  res2_pair *p1 = NULL;
  res2_pair *p2 = NULL;
  while (p != NULL)
    {
      res2_pair *tmp = p;
      p = p->next;
      tmp->next = p1;
      p1 = tmp;

      if (p == NULL) break;
      tmp = p;
      p = p->next;
      tmp->next = p2;
      p2 = tmp;
    }

  sort_res2_pairs(p1);
  sort_res2_pairs(p2);
  p = merge_res2_pairs(p1, p2);
}

void res2_comp::sort_skeleton(res2_pair *&p) const
{
  compare_type = (skeleton_sort & FLAGS_SORT);
  compare_use_descending = (skeleton_sort & FLAGS_DESCEND ? 1 : -1);
  compare_use_reverse = (skeleton_sort & FLAGS_REVERSE);
  compare_use_degree = (skeleton_sort & FLAGS_DEGREE);
  sort_res2_pairs(p);
}
void res2_comp::sort_monorder(res2_pair *&p) const
{
  // Rest of the compare_* don't matter in this case, except degree
  compare_type = COMPARE_MONORDER;
  compare_use_degree = 0;
  sort_res2_pairs(p);
}
void res2_comp::sort_reduction(res2_pair *&p) const
{
  compare_type = (reduction_sort & FLAGS_SORT);
  compare_use_descending = (reduction_sort & FLAGS_DESCEND ? 1 : -1);
  compare_use_reverse = (reduction_sort & FLAGS_REVERSE);
  compare_use_degree = 1;
  sort_res2_pairs(p);
}

int res2_comp::sort_value(res2_pair *p, const int *sort_order) const
{
  M->to_expvector(p->syz->monom, REDUCE_exp);
  int result = 0;
  for (int i=0; i<P->n_vars(); i++)
    result += REDUCE_exp[i] * sort_order[i];
  return result;
}

//////////////////////////////////////////////
//  Creation of new pairs ////////////////////
//////////////////////////////////////////////

void res2_comp::new_pairs(res2_pair *p)
    // Create and insert all of the pairs which will have lead term 'p'.
    // This also places 'p' into the appropriate monomial ideal
{
  Index<MonomialIdeal> j;
  queue<Bag *> elems;
  intarray vp;			// This is 'p'.
  intarray thisvp;

  if (comp_printlevel >= 10)
    {
      buffer o;
      o << "Computing pairs with first = " << p->pair_num << newline;
      emit(o.str());
    }
  M->divide(p->syz->monom, p->syz->comp->syz->monom, PAIRS_mon);
  M->to_varpower(PAIRS_mon, vp);

  // First add in syzygies arising from exterior variables
  // At the moment, there are none of this sort.

  if (M->is_skew())
    {
      intarray vplcm;
      intarray find_pairs_vp;

      int *skewvars = new int[M->n_vars()];
      varpower::to_ntuple(M->n_vars(), vp.raw(), find_pairs_vp);
      int nskew = M->exp_skew_vars(find_pairs_vp.raw(), skewvars);
      
      // Add in syzygies arising from exterior variables
      for (int v=0; v < nskew; v++)
	{
	  int w = skewvars[v];

	  thisvp.shrink(0);
	  varpower::var(w,1,thisvp);
	  Bag *b = new Bag(0, thisvp);
	  elems.insert(b);
	}
      // Remove the local variables
      delete [] skewvars;
    }

  // Second, add in syzygies arising from the base ring, if any
  // The baggage of each of these is NULL
  if (P->is_quotient_ring())
    {
      const MonomialIdeal &Rideal = P->get_quotient_monomials();
      for (j = Rideal.first(); j.valid(); j++)
	{
	  // Compute (P->quotient_ideal->monom : p->monom)
	  // and place this into a varpower and Bag, placing
	  // that into 'elems'
	  thisvp.shrink(0);
	  varpower::divide(Rideal[j]->monom().raw(), vp.raw(), thisvp);
	  if (varpower::is_equal(Rideal[j]->monom().raw(), thisvp.raw()))
	    continue;
	  Bag *b = new Bag(0, thisvp);
	  elems.insert(b);
	}
    }
  
  // Third, add in syzygies arising from previous elements of this same level
  // The baggage of each of these is their corresponding res2_pair

  MonomialIdeal &mi_orig = p->syz->comp->mi;
  for (j = mi_orig.first(); j.valid(); j++)
    {
      Bag *b = new Bag(mi_orig[j]->basis_ptr());
      varpower::divide(mi_orig[j]->monom().raw(), vp.raw(), b->monom());
      elems.insert(b);
    }

  // Make this monomial ideal, and then run through each minimal generator
  // and insert into the proper degree. (Notice that sorting does not
  // need to be done yet: only once that degree is about to begin.

  mi_orig.insert_minimal(new Bag(p, vp));

  queue<Bag *> rejects;
  Bag *b;
  MonomialIdeal mi(P, elems, rejects);
  while (rejects.remove(b))
    delete b;

  if (comp_printlevel>= 11) mi.debug_out(1);

  int *m = M->make_one();
  for (j = mi.first(); j.valid(); j++)
    {
      res2_pair *second = (res2_pair *) mi[j]->basis_ptr();
      M->from_varpower(mi[j]->monom().raw(), m);
      M->mult(m, p->syz->monom, m);
      
      res2_pair *q = new_res2_pair(p, second, m);
      insert_pair(q);
    }
}
//////////////////////////////////////////////
//  S-pairs and reduction ////////////////////
//////////////////////////////////////////////

int res2_comp::find_ring_divisor(const int *exp, ring_elem &result) const
     // If 'exp' is divisible by a ring lead term, then 1 is returned,
     // and result is set to be that ring element.
     // Otherwise 0 is returned.
{
  if (!P->is_quotient_ring()) return 0;
  Bag *b;
  if (!P->get_quotient_monomials().search_expvector(exp, b))
    return 0;
  result = (Nterm *) b->basis_ptr();
  return 1;
}

int res2_comp::find_divisor(const MonomialIdeal &mi, 
				   const int *exp,
				   res2_pair *&result) const
{
  // Find all the posible matches, use some criterion for finding the best...
  res2_pair *p;
  array<Bag *> bb;
  mi.find_all_divisors(exp, bb);
  if (bb.length() == 0) return 0;
  result = (res2_pair *) (bb[0]->basis_ptr());
  // Now search through, and find the best one.  If only one, just return it.
  if (comp_printlevel >= 5)
    if (mi.length() > 1)
      {
	buffer o;
	o << ":" << mi.length() << "." << bb.length() << ":";
	emit(o.str());
      }
  if (bb.length() == 1)
    {
      if (mi.length() == 1)
	n_ones++;
      else
	n_unique++;
      return 1;
    }
  n_others++;
  //  int n = R->n_terms(result->syz);

  unsigned int lowest = result->pair_num;
  for (int i=1; i<bb.length(); i++)
    {
      p = (res2_pair *) (bb[i]->basis_ptr());      
      if (p->pair_num < lowest)
	{
	  lowest = p->pair_num;
	  result = p;
	}
#if 0
      int nt = R->n_terms(p->syz);
      if (nt < n) 
	{
	  n = nt;
	  result = p;
	}
#endif
    }
  return 1;
}

res2term *res2_comp::s_pair(res2term *f) const
{
  res2term *result = NULL;
  int *si = M->make_one();
  while (f != NULL)
    {
      M->divide(f->monom, f->comp->syz->monom, si);
      res2term *h = R->mult_by_term(f->comp->syz, f->coeff, si);
      R->add_to(result, h);
      //      R->subtract_multiple_to(result, f->coeff, si, f->comp->syz);
      f = f->next;
    }
  M->remove(si);
  return result;
}

res2_pair *res2_comp::reduce(res2term * &f, res2term * &fsyz, res2term * &pivot, res2_pair *)
     // Reduce f, placing the reduction history in fsyz.
     // Returns NULL if f reduces to 0, otherwise 
     // returns the res2_pair at the previous level (f will "fill in" that pair), and
     // place a pointer to the corresponding term in "pivot".
{
  // 'lastterm' is used to append the next monomial to fsyz->syz
  res2term *lastterm = (fsyz->next == NULL ? fsyz : fsyz->next);

  res2_pair *q;
  ring_elem rg;
  //  Bag *b;

  int count = 0;
  if (comp_printlevel >= 4)
    emit_wrapped(",");
  
  while (f != NULL)
    {
      M->divide(f->monom, f->comp->syz->monom, REDUCE_mon);
      M->to_expvector(REDUCE_mon, REDUCE_exp);
      if (find_ring_divisor(REDUCE_exp, rg))
	{
	  // Subtract off f, leave fsyz alone
	  Nterm *r = rg;
	  M->divide(f->monom, r->monom, REDUCE_mon);
	  R->ring_subtract_multiple_to(f, f->coeff, REDUCE_mon, f->comp, rg);
	  total_reduce_count++;
	  count++;
	}
      //      else if (f->comp->mi.search_expvector(REDUCE_exp, b))
      else if (find_divisor(f->comp->mi, REDUCE_exp, q))
	{
	  //q = (res2_pair *) (b->basis_ptr());
	  lastterm->next = R->new_term(K->negate(f->coeff), f->monom, q);
	  lastterm = lastterm->next;
	  pivot = lastterm;
	  if (q->syz_type == SYZ2_S_PAIR || q->syz_type == SYZ2_MAYBE_MINIMAL) 
	    {
	      if (comp_printlevel >= 4)
		{
		  buffer o;
		  o << count;
		  emit_wrapped(o.str());
		}
	      return q; // i.e. not computed yet
	    }
	  M->divide(f->monom, q->syz->monom, REDUCE_mon);
	  R->subtract_multiple_to(f, f->coeff, REDUCE_mon, q->syz);
	  total_reduce_count++;
	  count++;
	}
      else
	{
	  res2term *tmp = f;
	  f = f->next;
	  tmp->next = NULL;
	  R->remove(tmp);
	}
    }
  if (comp_printlevel >= 4)
    {
      buffer o;
      o << count;
      emit_wrapped(o.str());
    }
  return NULL;
}

res2_pair *res2_comp::reduce2(res2term * &f, res2term * &fsyz, res2term * &pivot, res2_pair *)
     // Reduce f, placing the reduction history in fsyz.
     // Returns NULL if f reduces to 0, otherwise 
     // returns the res2_pair at the previous level (f will "fill in" that pair), and
     // place a pointer to the corresponding term in "pivot".
     // 'p' is just here for auto-reduction...
{
  // 'lastterm' is used to append the next monomial to fsyz->syz
  res2term *lastterm = (fsyz->next == NULL ? fsyz : fsyz->next);
  res2term head;
  res2term *red = &head;
  res2_pair *result = NULL;
  res2_pair *q;
  ring_elem rg;

  int count = 0;
  if (comp_printlevel >= 4)
    emit_wrapped(",");

  while (f != NULL)
    {
      M->divide(f->monom, f->comp->syz->monom, REDUCE_mon);
      M->to_expvector(REDUCE_mon, REDUCE_exp);
      if (find_ring_divisor(REDUCE_exp, rg))
	{
	  // Subtract off f, leave fsyz alone
	  Nterm *r = rg;
	  M->divide(f->monom, r->monom, REDUCE_mon);
	  R->ring_subtract_multiple_to(f, f->coeff, REDUCE_mon, f->comp, rg);
	  total_reduce_count++;
	  count++;
	}
      else if (find_divisor(f->comp->mi, REDUCE_exp, q))
	{
	  if (q->syz_type == SYZ2_S_PAIR || q->syz_type == SYZ2_MAYBE_MINIMAL) 
	    {
	      if (result == NULL || auto_reduce >=2) 
		{
		  lastterm->next = R->new_term(K->negate(f->coeff), f->monom, q);
		  lastterm = lastterm->next;
		  if (result == NULL)
		    {
		      // Only do this for the first non-computed pair
		      pivot = lastterm;
		      result = q;
		    }
		  else if (auto_reduce == 2)
		    {
		      // Keep track of this element for later reduction
		      // Store it with q.  Now for a bit of a hack:
		      // We store it in the 'pivot_term' field, since
		      // it cannot have been set yet, and since we don't want
		      // to waste space.
		      auto_reduce_node *au = new auto_reduce_node;
		      au->next = (auto_reduce_node *) (q->pivot_term);
		      au->p = pivot->comp;
		      au->pivot = lastterm;
		      q->pivot_term = (res2term *) au;
		    }
		  if (auto_reduce == 0)
		    {
		      // Time to leave: 'red' has nothing in it
		      // with this option, and 'f' is set correctly.
		      return result;
		    }
		}
	      red->next = f;
	      red = red->next;
	      f = f->next;
	      continue;
	    }
	  else
	    {
	      lastterm->next = R->new_term(K->negate(f->coeff), f->monom, q);
	      lastterm = lastterm->next;
	      M->divide(f->monom, q->syz->monom, REDUCE_mon);
	      R->subtract_multiple_to(f, f->coeff, REDUCE_mon, q->syz);
	      total_reduce_count++;
	      count++;
	    }
	}
      else
	{
	  red->next = f;
	  red = red->next;
	  f = f->next;
	}
    }
  red->next = NULL;
  f = head.next;
  if (comp_printlevel >= 4)
    {
      buffer o;
      o << count;
      emit_wrapped(o.str());
    }
  return result;
}

// The 'respolyHeap' version of reduction

res2_pair *res2_comp::reduce3(res2term * &f, res2term * &fsyz, res2term * &pivot, res2_pair *)
     // Reduce f, placing the reduction history in fsyz.
     // Returns NULL if f reduces to 0, otherwise 
     // returns the res2_pair at the previous level (f will "fill in" that pair), and
     // place a pointer to the corresponding term in "pivot".
{
  // 'lastterm' is used to append the next monomial to fsyz->syz
  res2term *lastterm = (fsyz->next == NULL ? fsyz : fsyz->next);
  res2term head;
  res2term *red = &head;
  res2_pair *result = NULL;
  res2_pair *q;
  ring_elem rg;
  respolyHeap fb(R);		// No bucket is needed for fsyz, since we
				// only append elements to the end of fsyz.
  fb.add(f);
  f = NULL;
  res2term *lead;
  //  Bag *b;

  int count = 0;
  if (comp_printlevel >= 4)
    emit_wrapped(",");

  while ((lead = fb.remove_lead_term()) != NULL)
    {
      M->divide(lead->monom, lead->comp->syz->monom, REDUCE_mon);
      M->to_expvector(REDUCE_mon, REDUCE_exp);
      if (find_ring_divisor(REDUCE_exp, rg))
	{
	  // Subtract off f, leave fsyz alone
	  Nterm *r = rg;
	  M->divide(lead->monom, r->monom, REDUCE_mon);
	  ring_elem c = K->negate(lead->coeff);
	  res2term *h = R->ring_mult_by_term(r->next, c, REDUCE_mon, lead->comp);
	  R->remove(lead);
	  K->remove(c);
	  fb.add(h);
	  total_reduce_count++;
	  count++;
	}
      else if (find_divisor(lead->comp->mi, REDUCE_exp, q))
	{
	  //q = (res2_pair *) (b->basis_ptr());
	  if (q->syz_type == SYZ2_S_PAIR || q->syz_type == SYZ2_MAYBE_MINIMAL) 
	    {
	      if (result == NULL) 
		{
		  lastterm->next = R->new_term(K->negate(lead->coeff), lead->monom, q);
		  lastterm = lastterm->next;
		  pivot = lastterm;
		  result = q;
		}
	      red->next = lead;
	      red = red->next;
	      continue;
	    }
	  else
	    {
	      ring_elem c = K->negate(lead->coeff);
	      M->divide(lead->monom, q->syz->monom, REDUCE_mon);
	      res2term *h = R->mult_by_term(q->syz->next, c, REDUCE_mon);
	      lastterm->next = R->new_term(c, lead->monom, q);
	      lastterm = lastterm->next;
	      R->remove(lead);
	      fb.add(h);
	      total_reduce_count++;
	      count++;
	    }
	}
      else
	{
	  red->next = lead;
	  red = red->next;
	}
    }
  red->next = NULL;
  f = head.next;
  if (comp_printlevel >= 4)
    {
      buffer o;
      o << count;
      emit_wrapped(o.str());
    }
  return result;
}

res2_pair *res2_comp::reduce4(res2term * &f, res2term * &fsyz, res2term * &pivot,
			      res2_pair *p)
     // Reduce f, placing the reduction history in fsyz.
     // Returns NULL if f reduces to 0, otherwise 
     // returns the res2_pair at the previous level (f will "fill in" that pair), and
     // place a pointer to the corresponding term in "pivot".
     // 'p' is just here for auto-reduction...
{
  // 'lastterm' is used to append the next monomial to fsyz->syz
  res2term *lastterm = fsyz;
  while (lastterm->next != NULL) lastterm = lastterm->next;

  res2term head;
  res2term *red = &head;
  res2_pair *result = NULL;
  res2_pair *q;
  ring_elem rg;

  int count = total_reduce_count;
  if (comp_printlevel >= 4)
    emit_wrapped(",");

  while (f != NULL)
    {
      res2term *lead = f;
      f = f->next;
      lead->next = NULL;
      M->divide(lead->monom, lead->comp->syz->monom, REDUCE_mon);
      M->to_expvector(REDUCE_mon, REDUCE_exp);
      if (find_ring_divisor(REDUCE_exp, rg))
	{
	  // Subtract off f, leave fsyz alone
	  Nterm *r = rg;
	  M->divide(lead->monom, r->monom, REDUCE_mon);
	  ring_elem c = K->negate(lead->coeff);
	  res2term *h = R->ring_mult_by_term(r->next, c, REDUCE_mon, lead->comp);
	  R->remove(lead);
	  K->remove(c);
	  R->add_to(f,h);
	  total_reduce_count++;
	}
      else if (find_divisor(lead->comp->mi, REDUCE_exp, q))
	{
	  if (q->degree == p->degree + 1)
	  // if (q->syz_type == SYZ2_S_PAIR) 
	    {
	      lastterm->next = R->new_term(K->negate(lead->coeff), lead->monom, q);
	      lastterm = lastterm->next;
	      if (result == NULL && q->syz_type == SYZ2_S_PAIR)
		{
		  // Only do this for the first non-computed pair
		  // Question: do we really need to keep this information around?
		  pivot = lastterm;
		  result = q;
		}
	      red->next = lead;
	      red = red->next;
	      continue;
	    }
	  else
	    {
	      ring_elem c = K->negate(lead->coeff);
	      M->divide(lead->monom, q->syz->monom, REDUCE_mon);
	      res2term *h = R->mult_by_term(q->syz->next, c, REDUCE_mon);
	      lastterm->next = R->new_term(c, lead->monom, q); // grabs 'c'.
	      lastterm = lastterm->next;
	      R->remove(lead);
	      R->add_to(f,h);
	      total_reduce_count++;
	    }
	}
      else
	{
	  red->next = lead;
	  red = red->next;
	}
    }
  red->next = NULL;
  f = head.next;
  if (comp_printlevel >= 4)
    {
      buffer o;
      o << (total_reduce_count - count);
      emit_wrapped(o.str());
    }
  return result;
}

res2_pair *res2_comp::reduce_by_level(res2term * &f, res2term * &fsyz)
     // Reduce f, placing the reduction history in fsyz.
     // Returns NULL if f reduces to 0, otherwise 
     // returns the res2_pair at the previous level (f will "fill in" that pair), and
     // place a pointer to the corresponding term in "pivot".
{
  // 'lastterm' is used to append the next monomial to fsyz->syz
  res2term *lastterm = (fsyz->next == NULL ? fsyz : fsyz->next);

  res2_pair *q;
  ring_elem rg;

  int count = 0;
  if (comp_printlevel >= 4)
    emit_wrapped(",");
  
  while (f != NULL)
    {
      M->divide(f->monom, f->comp->syz->monom, REDUCE_mon);
      M->to_expvector(REDUCE_mon, REDUCE_exp);
      if (find_ring_divisor(REDUCE_exp, rg))
	{
	  // Subtract off f, leave fsyz alone
	  Nterm *r = rg;
	  M->divide(f->monom, r->monom, REDUCE_mon);
	  R->ring_subtract_multiple_to(f, f->coeff, REDUCE_mon, f->comp, rg);
	  total_reduce_count++;
	  count++;
	}
      else if (find_divisor(f->comp->mi, REDUCE_exp, q))
	{
	  lastterm->next = R->new_term(K->negate(f->coeff), f->monom, q);
	  lastterm = lastterm->next;
	  M->divide(f->monom, q->syz->monom, REDUCE_mon);
	  R->subtract_multiple_to(f, f->coeff, REDUCE_mon, q->pivot_term);
	  total_reduce_count++;
	  count++;
	}
      else
	{
	  res2term *tmp = f;
	  f = f->next;
	  tmp->next = NULL;
	  R->remove(tmp);
	}
    }
  if (comp_printlevel >= 4)
    {
      buffer o;
      o << count;
      emit_wrapped(o.str());
    }
  return NULL;
}

res2_pair *res2_comp::reduce_heap_by_level(res2term * &f, res2term * &fsyz)
{
  // 'lastterm' is used to append the next monomial to fsyz->syz
  res2term *lastterm = (fsyz->next == NULL ? fsyz : fsyz->next);
  res2_pair *q;
  ring_elem rg;
  respolyHeap fb(R);		// No bucket is needed for fsyz, since we
				// only append elements to the end of fsyz.
  fb.add(f);
  f = NULL;
  res2term *lead;

  int count = 0;
  if (comp_printlevel >= 4)
    emit_wrapped(",");

  while ((lead = fb.remove_lead_term()) != NULL)
    {
      M->divide(lead->monom, lead->comp->syz->monom, REDUCE_mon);
      M->to_expvector(REDUCE_mon, REDUCE_exp);
      if (find_ring_divisor(REDUCE_exp, rg))
	{
	  // Subtract off f, leave fsyz alone
	  Nterm *r = rg;
	  M->divide(lead->monom, r->monom, REDUCE_mon);
	  ring_elem c = K->negate(lead->coeff);
	  res2term *h = R->ring_mult_by_term(r->next, c, REDUCE_mon, lead->comp);
	  R->remove(lead);
	  K->remove(c);
	  fb.add(h);
	  total_reduce_count++;
	  count++;
	}
      else if (find_divisor(lead->comp->mi, REDUCE_exp, q))
	{
	  ring_elem c = K->negate(lead->coeff);
	  M->divide(lead->monom, q->syz->monom, REDUCE_mon);
	  res2term *h = R->mult_by_term(q->pivot_term->next, c, REDUCE_mon);
	  lastterm->next = R->new_term(c, lead->monom, q);
	  lastterm = lastterm->next;
	  R->remove(lead);
	  fb.add(h);
	  total_reduce_count++;
	  count++;
	}
      else
	{
	  R->remove(lead);
	}
    }
  f = NULL;
  if (comp_printlevel >= 4)
    {
      buffer o;
      o << count;
      emit_wrapped(o.str());
    }
  return NULL;
}

//////////////////////////////////////////////
//  Toplevel calculation and state machine ///
//////////////////////////////////////////////

void res2_comp::do_auto_reductions(res2_pair *p, auto_reduce_node *au)
  // For each node in 'au', remove the specified multiple of 'p->syz'.
{
  buffer o;
  while (au != NULL)
    {
      auto_reduce_node *a = au;
      au = au->next;
      o << "auto reduction: " << newline << "    ";
      R->elem_text_out(p->syz);
      o << newline << "    ";
      R->elem_text_out(a->p->syz);
      o << newline << "    by coeff = ";
      K->elem_text_out(o, a->pivot->coeff);
      ring_elem c = K->negate(a->pivot->coeff);
      res2term *h = R->mult_by_coefficient(p->syz, c);
      K->remove(c);
      R->add_to(a->p->syz, h);
      o << newline << "    result = ";
      R->elem_text_out(a->p->syz);
      o << newline;
      delete a;
    }
  emit(o.str());
}

void res2_comp::handle_pair(res2_pair *p)
{
  nleft--;
  resn[p->level]->nleft--;

  // For level 1: any non-marked GB elements in this degree are
  // minimal generators, so we need to mark them as such
  if (p->level == 1)
    {
      p->syz_type = SYZ2_MINIMAL;
      if (comp_printlevel >= 2) emit_wrapped("z");
      if (projdim == 0) projdim = 1;
      nminimal++;
      return;
    }

  res2term *f = s_pair(p->syz);
  res2_pair *q;

  // This is used only for full auto reduction type 2.
  auto_reduce_node *au = (auto_reduce_node *) p->pivot_term;

  if (use_respolyHeaps)
    q = reduce3(f, p->syz, p->pivot_term, p);
  else if (auto_reduce >= 1)
    q = reduce2(f, p->syz, p->pivot_term, p);
  else
    q = reduce(f, p->syz, p->pivot_term, p);

  // Auto reduction: here is where we 'modify' the
  // other elements by 'p'.
  if (auto_reduce == 2)
    {
      do_auto_reductions(p,au);
    }
  else if (auto_reduce == 3)
    {
    }

  if (f == NULL)
    {
      // minimal syzygy
      if (p->level == length_limit+1)
	{
	  p->syz_type = SYZ2_MAYBE_MINIMAL;
	}
      else
	{
	  p->syz_type = SYZ2_MINIMAL;
	  nminimal++;
	  if (p->level > projdim)
	    projdim = p->level;
	}
      if (comp_printlevel >= 2) emit_wrapped("z");
    }
  else 
    {
      R->make_monic(f);
      p->syz_type = SYZ2_NOT_MINIMAL;
      
      // non-minimal syzygy
      R->remove(q->syz);
      q->syz = f;
      q->syz_type = SYZ2_NOT_NEEDED;

      // Auto reduction: here is where we modify the
      // other elements by 'q'.
      if (auto_reduce == 2)
	{
	  do_auto_reductions(q, (auto_reduce_node *) q->pivot_term);
	  q->pivot_term = NULL;
	}
      else if (auto_reduce == 3)
	{
	}

      if (comp_printlevel >= 2) emit_wrapped("m");
    }
}

void res2_comp::handle_pair_by_level(res2_pair *p)
{
  nleft--;
  resn[p->level]->nleft--;

  // level 1 is easy: just mark as minimal
  if (p->level == 1)
    {
      p->syz_type = SYZ2_MINIMAL;
      if (comp_printlevel >= 2) emit_wrapped("z");
      return;
    }

  res2term *f = s_pair(p->syz);
  if (do_by_level == 2)
    {
      if (use_respolyHeaps)
	reduce_heap_by_level(f, p->syz);
      else
	reduce_by_level(f, p->syz);
    }
  else
    reduce(f, p->syz, p->pivot_term, p);

  if (f == NULL)
    {
      p->syz_type = SYZ2_MINIMAL;
      if (comp_printlevel >= 2) emit_wrapped("z");
    }
  else 
    {
      // This should not happen at all!!
      buffer o;
      o << "handle pair: should not be here!";
      o << "p->syz == ";
      R->elem_text_out(o, p->syz);
      emit(o.str());
    }
}

void res2_comp::handle_pair_by_degree(res2_pair *p)
{
  nleft--;
  resn[p->level]->nleft--;

  // For level 1: any non-marked GB elements in this degree are
  // minimal generators, so we need to mark them as such
  if (p->level == 1)
    {
      if (p->syz_type != SYZ2_NOT_NEEDED)
	{
	  p->syz_type = SYZ2_MINIMAL;
	  if (comp_printlevel >= 2) emit_wrapped("z");
	  nminimal++;
	}
      return;
    }

  res2term *f = s_pair(p->syz);
  res2_pair *q = reduce4(f, p->syz, p->pivot_term, p);
  // In this version of 'reduce', the resulting value of 'f' is irrelevant.
  // And in fact the routine should probably read:
  //  res2_pair *q = reduce4(p->syz, p->pivot_term);
  if (q == NULL)
    {
      if (p->syz_type != SYZ2_NOT_NEEDED)
	{
	  // minimal syzygy
	  p->syz_type = SYZ2_MINIMAL;
	  nminimal++;
	  resn[p->level]->nminimal++;
	  if (comp_printlevel >= 2) emit_wrapped("z");
	}
      else
	{
	  if (comp_printlevel >= 2) emit_wrapped("o");
	}
    }
  else 
    {
      p->syz_type = SYZ2_NOT_MINIMAL;
      q->syz_type = SYZ2_NOT_NEEDED;
      if (comp_printlevel >= 2) emit_wrapped("m");
    }
}




