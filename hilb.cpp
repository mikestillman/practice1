// Copyright 1996 Michael E. Stillman

#include "hilb.hpp"

extern char system_interrupted;
extern int comp_printlevel;

stash *hilb_comp::mystash;

int partition_table::representative(int x)
{
  int i = x;
  while (dad[i] >= 0) i = dad[i];
  int j = x;
  while (j != i) { int t = dad[j]; dad[j] = i; j = t; }
  return i;
}

int partition_table::merge_in(int x, int y)
     // Returns the label representative of the merge of the two sets
{
  int t, newtop;
  int i = x;
  int j = y;
  while (dad[i] >= 0) i = dad[i];
  while (dad[j] >= 0) j = dad[j];
  if (i == j) return i;
  n_sets--;
  if (dad[i] > dad[j])
    { newtop = j; dad[newtop] += dad[i]; dad[i] = newtop; }
  else
    { newtop = i; dad[newtop] += dad[j]; dad[j] = newtop; }
  while (dad[x] >= 0)
    { t = x;  x = dad[x];  dad[t] = newtop; }
  while (dad[y] >= 0)
    { t = y;  y = dad[y];  dad[t] = newtop; }

  return newtop;
}

void partition_table::merge_in(const int *m)
     // merge in the varpower monomial 'm'.
{
  index_varpower i = m;
  int var1 = i.var();
  occurs[var1] = 1;
  ++i;
  for ( ; i.valid(); ++i)
    {
      var1 = merge_in(var1, i.var());
      occurs[i.var()] = 1;
    }
}

partition_table::partition_table(int nvars)
  : n_vars(nvars),
    n_sets(nvars),
    adad(nvars),
    aoccurs(nvars)
{
  dad = adad.alloc(nvars);
  occurs = aoccurs.alloc(nvars);
  for (int i=0; i<nvars; i++) 
    {
      dad[i] = -1;
      occurs[i] = 0;
    }
}

void partition_table::reset(int nvars)
{
  n_vars = nvars;
  n_sets = nvars;
  for (int i=0; i<nvars; i++)
    {
      dad[i] = -1;
      occurs[i] = 0;
    }
}
void partition_table::partition(MonomialIdeal &I, array<MonomialIdeal> &result)
{
  int k;
  reset(I.topvar()+1);
  // Create the sets
  for (Index<MonomialIdeal> i = I.first(); i.valid(); i++)
    if (n_sets > 1) 
      merge_in(I[i]->monom().raw());
    else
      break;

  if (n_sets == 1)
    {
      result.append(I);
      return;
    }

  int this_label = -1;
  n_sets = 0;
  for (k=0; k<n_vars; k++)
    if (occurs[k] && dad[k] < 0)
      {
	dad[k] = this_label--;
	n_sets++;
      }

  if (n_sets == 1)
    {
      result.append(I);
      return;
    }

  int first = result.length();
  for (k=0; k<n_sets; k++)
    result.append(MonomialIdeal(I.get_ring()));

  // Now partition the monomials
  Bag *b;
  while (I.remove(b))
    {
      const intarray &m = b->monom();
      int v = varpower::topvar(m.raw());
      int loc = -1-dad[representative(v)];
      result[first+loc].insert_minimal(b);
    }
}

static int popular_var(const MonomialIdeal &I, 
		int &npure, int *pure, 
		int &nhits,
                const int *& non_pure_power)
     // Return the variable which occurs the most often in non pure monomials,
     // placing the number of monomials in which it occurs into 'nhits'.
     // At the same time, set 'pure' and 'npure'.  If there is a non pure
     // power monomial, set non_pure_power to this varpower monomial.
{
  int k;
  npure = 0;
  int nvars = I.topvar()+1;
  for (k=0; k<nvars; k++)
    pure[k] = -1;

  intarray ahits(nvars);
  int *hits = ahits.alloc(nvars);
  for (k = 0; k<nvars; k++) hits[k] = 0;

  non_pure_power = NULL;

  for (Index<MonomialIdeal> i = I.first(); i.valid(); i++)
    {
      const int *m = I[i]->monom().raw();
      index_varpower j = m;
      assert(j.valid());	// The monomial cannot be '1'.
      int v = j.var();
      int e = j.exponent();
      ++j;
      if (j.valid())
	{
	  non_pure_power = m;
	  hits[v]++;
	  for ( ; j.valid(); ++j)
	    hits[j.var()]++;
	}
      else
	{
	  if (pure[v] == -1)
	    {
	      npure++;
	      pure[v] = e;
	    }
	  else
	    if (pure[v] > e) pure[v] = e;
	}
    }

  int popular = 0;
  for (k=1; k<nvars; k++)
    if (hits[k] > hits[popular])
      popular = k;
  nhits = hits[popular];
  return popular;
}

static int find_pivot(const MonomialIdeal &I, int &npure, int *pure, intarray &m)
     // If I has a single monomial which is a non pure power, return 0,
     // and set 'pure', and 'npure'.  If I has at least two non pure
     // monomials, choose a monomial 'm', not in I, but at least of degree 1,
     // which is suitable for divide and conquer in hilbert function
     // computation.
{
  int nhits;
  const int *vp;
  int v = popular_var(I, npure, pure, nhits, vp);
  
  // The following will take some tweaking...
  // Some possibilities: take just this variable, take gcd of 2 or 3
  // elements containing this variable (choose first 3, last 3, or randomly).

  // For now, let's just take this variable, if there is more than one
  // non pure power.
  if (npure >= I.length() - 1)
    {
      assert(vp != NULL);
      varpower::copy(vp, m);
      return 0;
    }
  varpower::var(v,1,m);
  return 1;
}

static void iquotient_and_sum(MonomialIdeal &I,
		       const int *m, // varpower
		       MonomialIdeal &quot, 
		       MonomialIdeal &sum)
{
  array< queue<Bag *> *> bins;
  sum = MonomialIdeal(I.get_ring());
  quot = MonomialIdeal(I.get_ring());
  Bag *bmin = new Bag(0);
  varpower::copy(m, bmin->monom());
  sum.insert_minimal(bmin);
  for (Index<MonomialIdeal> i = I.first(); i.valid(); i++)
    {
      Bag *b = new Bag(0);
      varpower::divide(I[i]->monom().raw(), m, b->monom());
      if (varpower::divides(m, I[i]->monom().raw()))
	quot.insert_minimal(b);
      else
	{
	  sum.insert_minimal(new Bag(0,I[i]->monom()));
	  int d = varpower::simple_degree(b->monom().raw());
	  if (d >= bins.length())
	    for (int j=bins.length(); j<=d; j++)
	      bins.append((queue<Bag *> *)NULL);
	  if (bins[d] == (queue<Bag *> *)NULL)
	    bins[d] = new queue<Bag *>;
	  bins[d]->insert(b);
	}
    }

  // Now insert the elements in the queue.
  // MES: is it worth looping through each degree, first checking
  // divisibility, and after that, insert_minimal of the ones that survive?
  Bag *b;
  for (int j=0; j < bins.length(); j++)
    if (bins[j] != NULL)
      {
	while (bins[j]->remove(b)) quot.insert(b);
	delete bins[j];
      }
}


void hilb_comp::next_monideal()
{
  reset();
  MonomialIdeal I = input_mat.make_skew_monideal(this_comp);
    // The above line adds the squares of the variables which are skew variables
    // into the monomial ideal.  This allows Hilbert functions of such rings
    // to be computed as usual.
  part_table.partition(I, current->monids);
  current->i = current->monids.length() - 1;
  current->first_sum = current->i + 1; // This part is not used at top level
}
void hilb_comp::reset()
{
  depth = 0;
  if (current == NULL)
    {
      current = new hilb_step;
      current->up = current->down = NULL;
      current->h0 = R->from_int(0);
      current->h1 = R->from_int(0);
    }
  else
    while (current->up != NULL) current = current->up;

  R->remove(current->h0);	// This line should not be needed...
  R->remove(current->h1);
  current->h0 = R->from_int(0);		// This top level h0 is not used
  current->h1 = R->from_int(1);
}
hilb_comp::hilb_comp(const PolynomialRing *RR, const Matrix &m)
: S(m.get_ring()->cast_to_PolynomialRing()),
  R(RR),
  M(S->Nmonoms()),
  D(S->degree_monoid()),
  input_mat(m),
  this_comp(0),
  current(NULL),
  part_table(S->n_vars())
{
  assert(D == R->Nmonoms());
  one = R->Ncoeffs()->from_int(1);
  minus_one = R->Ncoeffs()->from_int(-1);
  LOCAL_deg1 = D->make_one();

  bump_up(S);
  bump_up(R);

  result_poincare = R->from_int(0);
  nsteps = 0;
  maxdepth = 0;
  nideal = 0;
  nrecurse = 0;

  // Collect the 'this_comp' monideal of the input matrix
  // Set up the computation for that
  next_monideal();
}

#if 0
hilb_comp::hilb_comp(const PolynomialRing *RR, const MonomialIdeal &I)
: S(I.get_ring()->cast_to_PolynomialRing()),
  R(RR),
  M(S->Nmonoms()),
  D(S->degree_monoid()),
  current(NULL),
  part_table(I.topvar()+1)
{
  assert(D == R->Nmonoms());
  one = R->Ncoeffs()->from_int(1);
  minus_one = R->Ncoeffs()->from_int(-1);
  LOCAL_deg1 = D->make_one();

  bump_up(R);
  reset(I);
}
#endif

hilb_comp::~hilb_comp()
{
  // free 'current' (which is most of the stuff here...)
  while (current != NULL)
    {
      hilb_step *p = current;
      current = current->down;
      
      R->remove(p->h0);
      R->remove(p->h1);
      delete p;			// This will wipe out the monomial ideals
    }

  R->remove(result_poincare);
  R->Ncoeffs()->remove(one);
  R->Ncoeffs()->remove(minus_one);
  D->remove(LOCAL_deg1);

  bump_down(R);
  bump_down(S);
}

int hilb_comp::calc(int n_steps)
     // Possible return values: COMP_DONE, COMP_INTERRUPTED.
{
  if (input_mat.n_rows() == 0)
    return COMP_DONE;
  if (n_steps >= 0)
    {
      int calc_nsteps = nsteps + n_steps;
      while (calc_nsteps-- > 0)
	{
	  int result = step();
	  if (result == COMP_DONE)
	    return COMP_DONE;
	  system_spincursor();
	  if (system_interrupted)
	    return COMP_INTERRUPTED;
	}
      return COMP_DONE_STEPS;
    }
  else for (;;)
    {
      int result = step();
      if (result == COMP_DONE)
	return COMP_DONE;
      system_spincursor();
      if (system_interrupted)
	return COMP_INTERRUPTED;
    }
}

int hilb_comp::step()
     // Possible return values: COMP_DONE, COMP_COMPUTING
{
  nsteps++;
  if (current->i == current->first_sum)
    {
      current->h0 = current->h1;
      current->h1 = R->from_int(1);
    }
  if (current->i >= 0)
    {
      // If the i th monomial ideal is a 'special case', simply compute its value
      // Otherwise, find pivot, and quotient/sum creating next level down
      // (possibly make new hilb_step, and increment max_depth if needed)
      // and set current = current->down
      do_ideal(current->monids[current->i]);  // consumes this monomial ideal
    }
  else
    {
      // At this point, all Hilbert functions at this level have been
      // computed, so add the values, and place one step up
      R->add_to(current->h0, current->h1);
      ring_elem f = current->h0;
      current->h0 = R->from_int(0);
      current->h1 = R->from_int(0);
      current->monids.shrink(0);
      if (current->up == NULL) 
	{

	  ring_elem tmp = R->term(one, input_mat.rows()->degree(this_comp));
	  R->mult_to(f, tmp);
	  R->add_to(result_poincare, f);
	  R->remove(tmp);
	  this_comp++;

	  // Now check if we have any more components
	  if (this_comp >= input_mat.n_rows())
	    return COMP_DONE;
	  // otherwise go on to the next component:

	  next_monideal();
	  return COMP_COMPUTING;
	}
      current = current->up;
      R->mult_to(current->h1, f);
      current->i--;
      R->remove(f);
    }
  return COMP_COMPUTING;
}

void hilb_comp::recurse(MonomialIdeal &I, const int *pivot_vp)
{
  depth++;
  if (depth > maxdepth) maxdepth = depth;
  nrecurse++;
  if (current->down == NULL)
    {
      current->down = new hilb_step; // MES: is this ok?
      current->down->up = current;
      current->down->down = NULL;
    }
  current = current->down;
  current->h0 = R->from_int(0);
  M->degree_of_varpower(pivot_vp, LOCAL_deg1);
  current->h1 = R->term(one, LOCAL_deg1); // t^(deg vp)
  MonomialIdeal quot(S), sum(S);
  iquotient_and_sum(I, pivot_vp, quot, sum);
  part_table.partition(sum, current->monids);
  current->first_sum = current->monids.length() - 1;
  part_table.partition(quot, current->monids);
  current->i = current->monids.length() - 1;
}

void hilb_comp::do_ideal(MonomialIdeal &I)
{
  // This either will multiply to current->h1, or it will recurse down
  // Notice that one, and minus_one are global in this class.
  // LOCAL_deg1, LOCAL_vp are scratch variables defined in this class
  nideal++;
  ring_elem F = R->from_int(1);
  ring_elem G;
  int len = I.length();
  if (len <= 2)
    {
      // len==1: set F to be 1 - t^(deg m), where m = this one element
      // len==2: set F to be 1 - t^(deg m1) - t^(deg m2) + t^(deg lcm(m1,m2))
      M->degree_of_varpower(I.first_elem(), LOCAL_deg1);
      G = R->term(minus_one, LOCAL_deg1);
      R->add_to(F, G);

      if (len == 2)
	{
	  M->degree_of_varpower(I.second_elem(), LOCAL_deg1);
	  G = R->term(minus_one, LOCAL_deg1);
	  R->add_to(F, G);
	  LOCAL_vp.shrink(0);
	  varpower::lcm(I.first_elem(), I.second_elem(), LOCAL_vp);
	  M->degree_of_varpower(LOCAL_vp.raw(), LOCAL_deg1);
	  G = R->term(one, LOCAL_deg1);
	  R->add_to(F,G);
	}
    }
  else 
    {
      int npure;
      intarray pivot;
      intarray pure_a;
      int *pure = pure_a.alloc(I.topvar()+1);
      if (!find_pivot(I, npure, pure, pivot))
	{
	  // set F to be product(1-t^(deg x_i^e_i)) 
	  //              - t^(deg pivot) product((1-t^(deg x_i^(e_i - m_i)))
	  // where e_i >= 1 is the smallest number s.t. x_i^(e_i) is in I,
	  // and m_i = exponent of x_i in pivot element (always < e_i).
	  M->degree_of_varpower(pivot.raw(), LOCAL_deg1);
	  G = R->term(one, LOCAL_deg1);
	  for (index_varpower i = pivot.raw(); i.valid(); ++i)
	    if (pure[i.var()] != -1)
	      {
		ring_elem H = R->from_int(1);
		D->power(M->degree_of_var(i.var()), pure[i.var()], LOCAL_deg1);
		ring_elem tmp = R->term(minus_one, LOCAL_deg1);
		R->add_to(H, tmp);
		R->mult_to(F, H);
		R->remove(H);
		
		H = R->from_int(1);
		D->power(M->degree_of_var(i.var()), pure[i.var()] - i.exponent(), LOCAL_deg1);
		tmp = R->term(minus_one, LOCAL_deg1);
		R->add_to(H, tmp);
		R->mult_to(G, H);
		R->remove(H);
	      }
	  R->subtract_to(F, G);
	}
      else
	{
	  // This is the one case in which we recurse down
	  R->remove(F);
	  recurse(I, pivot.raw());
	  return;
	}
    }
  R->mult_to(current->h1, F);
  R->remove(F);
  current->i--;
}

int hilb_comp::is_done() const
{
  return (current != NULL && current->up == NULL);
}

RingElement hilb_comp::value()
{
  if (!is_done())
    {
      gError << "Hilbert function computation not complete";
      return RingElement(R,0);
    }
  RingElement result(R, R->copy(result_poincare));
  return result;
}

void hilb_comp::stats() const
{
  buffer o;
  o << "--- Hilbert Function Statistics---------------------" << newline;
  o << "#steps        = " << nsteps << newline;
  o << "Max depth     = " << maxdepth << newline;
  o << "current depth = " << depth << newline;
  o << "#ideal        = " << nideal << newline;
  o << "#recurse      = " << nrecurse << newline;
  
  hilb_step *p = current;
  int d = depth;
  while (p != NULL)
    {
      o << "----- depth " << d << " -------------" << newline;
      o << "  " << p->monids.length() << " monomial ideals total" << newline;
      o << "  " << p->i << " = current location" << newline;
      o << "  " << p->first_sum + 1 << " sum monomial ideals" << newline;
      o << "  h0 = ";  R->elem_text_out(o, p->h0); o << newline;
      o << "  h1 = ";  R->elem_text_out(o, p->h1); o << newline;
      for (int i=0; i<p->monids.length(); i++)
	{
	  o << "  ---- monomial ideal ---------------" << newline;
	  o << "  ";
	  p->monids[i].text_out(o);
	  o << newline;
	}
      p = p->up;
      d--;
    }
}
int hilb_comp::hilbertSeries(const Matrix &M, RingElement &result)
{
  const PolynomialRing *P = M.get_ring()->HilbertRing();
  hilb_comp *hf = new hilb_comp(P,M);
  int retval = hf->calc(-1);
  if (retval != COMP_DONE) return 1;
  result = hf->value();
  delete hf;
  return 0;
}
#if 0
RingElement hilb_comp::hilbertSeries(const FreeModule *F)
{
  const Ring *P = F->get_ring()->HilbertRing();
  RingElement result(P);
  for (int i=0; i<F->rank(); i++)
    result += RingElement(P,...);
  return result;
}
#endif

int hilb_comp::coeff_of(const RingElement &h, int deg)
{
  // This is a bit of a kludge of a routine.  The idea is to loop through
  // all the terms of the polynomial h, expand out the exponent, and to add
  // up the small integer values of the coefficients of those that have exp[0]=deg.
  const PolynomialRing *P = h.get_ring()->cast_to_PolynomialRing();

  int *exp = new int[P->n_vars()];
  int result = 0;
  for (Nterm *f = h.get_value(); f!=NULL; f=f->next)
    {
      P->Nmonoms()->to_expvector(f->monom, exp);
      if (exp[0] == deg)
	{
	  int n = P->Ncoeffs()->coerce_to_int(f->coeff);
	  result += n;
	}
	
    }
  delete [] exp;
  return result;
}
