// Copyright 1997 Michael E. Stillman

#include "gbbinom.hpp"
#include "ntuple.hpp"
#include "text_io.hpp"

#define monomial monomial0
extern int comp_printlevel;

/////////////////////////////
// Monomials and binomials //
/////////////////////////////

binomial_ring::binomial_ring(const Ring *RR, 
			     int *wts,
			     bool revlex)
  : R(RR),
    F(RR->make_FreeModule(1)),
    nvars(RR->n_vars()),
    have_weights(wts != NULL),
    weights(NULL),
    revlex(revlex)
{
  int i;

  nslots = nvars + 1;
  degrees = new int[nvars];
  for (i=0; i<nvars; i++) 
    degrees[i] = - R->Nmonoms()->primary_degree_of_var(i);

  if (have_weights)
    {
      nslots++;
      weights = new int[nvars];
      for (i=0; i<nvars; i++) weights[i] = -wts[i];
    }

  monstash = new stash("monomials", sizeof(int)*nslots);
}

binomial_ring::binomial_ring(const Ring * /* RR */)
{
  ERROR("MES: not implemented yet");
}

binomial_ring::~binomial_ring()
{
  delete [] degrees;
  delete [] weights;
  delete monstash;
}

void binomial_ring::remove_monomial(monomial &m) const
{
  if (m == NULL) return;
  monstash->delete_elem(m);
  m = NULL;
}

monomial binomial_ring::new_monomial() const
{
  return (monomial)((binomial_ring *) this)->monstash->new_elem();
}

monomial binomial_ring::copy_monomial(monomial m) const
{
  monomial result = new_monomial();
  for (int i=0; i<nslots; i++)
    result[i] = m[i];
  return result;
}

void binomial_ring::set_weights(monomial m) const
{
  int i;
  int deg = 0;
  for (i=0; i<nvars; i++)
      deg += degrees[i] * m[i];
  m[nvars] = deg;
  if (have_weights)
    {
      int wt = 0;
      for (i=0; i<nvars; i++)
	wt += weights[i] * m[i];
      m[nvars+1] = wt;
    }
}

monomial binomial_ring::make_monomial(int *exp) const
  // Make a monomial from an exponent vector
{
  monomial result = new_monomial();
  for (int i=0; i<nvars; i++)
    result[i] = exp[i];
  set_weights(result);
  return result;
}
void binomial_ring::remove_binomial(binomial &f) const
{
  remove_monomial(f.lead);
  remove_monomial(f.tail);
}
binomial binomial_ring::make_binomial() const
  // allocates the monomials
{
  return binomial(new_monomial(), new_monomial());
}
binomial binomial_ring::copy_binomial(const binomial &f) const
{
  return binomial(copy_monomial(f.lead), copy_monomial(f.tail));
}

int binomial_ring::weight(monomial m) const
{
  if (have_weights) return -m[nvars+1];
  return 0;
}

int binomial_ring::degree(monomial m) const
{
  return -m[nvars];
}

unsigned int binomial_ring::mask(monomial m) const
{
  return ntuple::mask(nvars, m);
}

bool binomial_ring::divides(monomial m, monomial n) const
{
  for (int i=0; i<nvars; i++)
    if (m[i] > n[i]) return false;
  return true;
}

monomial binomial_ring::quotient(monomial m, monomial n) const
  // return m:n
{
  monomial result = new_monomial();
  for (int i=0; i<nslots; i++) 
    {
      int x = m[i] - n[i];
      result[i] = (x > 0 ? x : 0);
    }
  set_weights(result);
  return result;
}

monomial binomial_ring::lcm(monomial m, monomial n) const
  // return lcm(m,n)
{
  monomial result = new_monomial();
  for (int i=0; i<nvars; i++) 
    result[i] = (m[i] > n[i] ? m[i] : n[i]);
  set_weights(result);
  return result;
}

monomial binomial_ring::divide(monomial m, monomial n) const
{
  monomial result = new_monomial();
  for (int i=0; i<nslots; i++)
    result[i] = m[i] - n[i];
  return result;
}

monomial binomial_ring::mult(monomial m, monomial n) const
{
  monomial result = new_monomial();
  for (int i=0; i<nslots; i++)
    result[i] = m[i] + n[i];
  return result;
}

monomial binomial_ring::spair(monomial lcm, monomial a, monomial b) const
  // computes lcm - a + b
{
  monomial result = new_monomial();
  for (int i=0; i<nslots; i++)
    result[i] = lcm[i] - a[i] + b[i];
  return result;
}

void binomial_ring::spair_to(monomial a, monomial b, monomial c) const
{
  for (int i=0; i<nslots; i++)
    a[i] += -b[i] + c[i];
}

bool binomial_ring::gcd_is_one(monomial m, monomial n) const
{
  // Return true if supp(m), supp(n) have empty intersection
  for (int i=0; i<nvars; i++)
    if (m[i] > 0 && n[i] > 0) return false;
  return true;
}
bool binomial_ring::gcd_is_one(binomial f) const
{
  return gcd_is_one(f.lead,f.tail);
}

bool binomial_ring::remove_content(binomial &f) const
{
  // If the monomials of f have a common monomial factor, remove it from each, and 
  // return true.  Otherwise return false.
  monomial m = f.lead;
  monomial n = f.tail;
  bool result = false;
  for (int i=0; i<nvars; i++)
    {
      if (m[i] > 0 && n[i] > 0)
	{
	  if (m[i] > n[i])
	    {
	      m[i] -= n[i];
	      n[i] = 0;
	    }
	  else
	    {
	      n[i] -= m[i];
	      m[i] = 0;
	    }
	  result = true;
	}
    }
  if (result)
    {
      set_weights(m);  // This is an inefficient way to do this...
      set_weights(n);
    }
  return result;
}

int binomial_ring::compare(monomial m, monomial n) const
{
  int i;
  if (have_weights)
    {
      i = nvars+1;
      if (m[i] < n[i]) return GT;
      if (m[i] > n[i]) return LT;
      i--;
    }
  else
    i = nvars;
  // check degree? For now...
  if (m[i] < n[i]) return GT;  // Remember: m[nvars] is NEGATIVE of degree
  if (m[i] > n[i]) return LT;
  if (revlex)
    for ( ;i>=0; i--)
      {
	if (m[i] > n[i]) return LT;
	if (m[i] < n[i]) return GT;
      }
  else
    for ( ; i>=0; i--)
      {
	if (m[i] > n[i]) return GT;
	if (m[i] < n[i]) return LT;
      }
  return EQ;
}

int binomial_ring::graded_compare(monomial m, monomial n) const
{
  int i = nvars;
  if (m[i] < n[i]) return GT;
  if (m[i] > n[i]) return LT;
  if (have_weights)
    {
      i = nvars+1;
      if (m[i] < n[i]) return GT;
      if (m[i] > n[i]) return LT;
      i--;
    }
  else
    i = nvars;
  i--;
  if (revlex)
    for ( ;i>=0; i--)
      {
	if (m[i] > n[i]) return LT;
	if (m[i] < n[i]) return GT;
      }
  else
    for ( ; i>=0; i--)
      {
	if (m[i] > n[i]) return GT;
	if (m[i] < n[i]) return LT;
      }
  return EQ;
}

void binomial_ring::translate_monomial(const binomial_ring *old_ring, monomial &m) const
{
  int i;
  if (m == NULL) return;
  monomial result = new_monomial();
  for (i=0; i<old_ring->nvars; i++)
    result[i] = m[i];
  for (i=old_ring->nvars; i<nvars; i++)
    result[i] = 0;
  old_ring->remove_monomial(m);
  set_weights(result);
  m = result;
}

void binomial_ring::translate_binomial(const binomial_ring *old_ring, binomial &f) const
{
  translate_monomial(old_ring, f.lead);
  translate_monomial(old_ring, f.tail);
}

vec binomial_ring::monomial_to_vector(monomial m) const
{
  if (m == NULL) return NULL;
  intarray vp;
  varpower::from_ntuple(nvars, m, vp);
  return F->from_varpower(vp.raw(),0);
}

vec binomial_ring::binomial_to_vector(binomial f) const
{
  vec v1 = monomial_to_vector(f.lead);
  vec v2 = monomial_to_vector(f.tail);
  F->subtract_to(v1,v2);
  return v1;
}
vec binomial_ring::binomial_to_vector(binomial f, int n) const
{
  vec v1 = monomial_to_vector(f.lead);
  bool include_tail = false;
  if (n == 0) 
    include_tail = true;
  else if (n == 1 && degree(f.tail) == degree(f.lead))
    include_tail = true;
  else if (n == 2 && degree(f.tail) == degree(f.lead)
	   && weight(f.tail) == weight(f.lead))
    include_tail = true;

  if (include_tail)
    {
      vec v2 = monomial_to_vector(f.tail);
      F->subtract_to(v1,v2);
    }
  return v1;
}

bool binomial_ring::vector_to_binomial(vec f, binomial &result) const
  // result should already have both monomials allocated
  // returns false if f is not a binomial, otherwise result is set.
{
  if (f == NULL || f->next == NULL || f->next->next != NULL) 
    return false;

  R->Nmonoms()->to_expvector(f->monom, result.lead);
  set_weights(result.lead);

  R->Nmonoms()->to_expvector(f->next->monom, result.tail);
  set_weights(result.tail);

  normalize(result);
  return true;
}

void binomial_ring::intvector_to_binomial(vec f, binomial &result) const
  // result should be a preallocated binomial
{
  for (int i=0; i<nslots; i++)
    {
      result.lead[i] = 0;
      result.tail[i] = 0;
    }

  for ( ; f != NULL; f = f->next)
    {
      int e = ZZ->coerce_to_int(f->coeff);
      if (e > 0)
	result.lead[f->comp] = e;
      else if (e < 0)
	result.tail[f->comp] = -e;
    }

  set_weights(result.lead);
  set_weights(result.tail);
  normalize(result);
}

bool binomial_ring::normalize(binomial &f) const
  // Return false if 'f' is zero.  Otherwise return true,
  // and possibly swap the terms of f so that f.lead is the lead term.
{
  int cmp = compare(f.lead, f.tail);
  if (cmp == EQ) return false;
  if (cmp == LT)
    {
      monomial a = f.lead;
      f.lead = f.tail;
      f.tail = a;
    }
  return true;
}

bool binomial_ring::one_reduction_step(binomial &f, binomial g) const
  // returns false if the reduction is zero, otherwise modifies f.
  // (f might be modified in either case).
{
  // MES: need to consider the cases: divide by content, homog_prime.
  for (int i=0; i<nslots; i++)
    f.lead[i] += - g.lead[i] + g.tail[i];
  return normalize(f);
}

bool binomial_ring::calc_s_pair(binomial_s_pair &s, binomial &result) const
{
  binomial f = s.f1->f;
  binomial g = s.f2->f;
  result = make_binomial();
  for (int i=0; i<nslots; i++)
    {
      result.lead[i] = s.lcm[i] - f.lead[i] + f.tail[i];
      result.tail[i] = s.lcm[i] - g.lead[i] + g.tail[i];
    }
  return normalize(result);
}

void binomial_ring::monomial_out(buffer &o, const monomial m) const
{
  if (m == NULL) return;
  intarray vp;
  varpower::from_ntuple(nvars, m, vp);
  int *n = R->Nmonoms()->make_one();
  R->Nmonoms()->from_varpower(vp.raw(), n);
  R->Nmonoms()->elem_text_out(o,n);
  R->Nmonoms()->remove(n);
}

void binomial_ring::elem_text_out(buffer &o, const binomial &f) const
{
  monomial_out(o, f.lead);
  if (f.tail == NULL) return;
  o << "-";
  monomial_out(o, f.tail);
}
///////////////////////
// S pair management //
///////////////////////

binomial_s_pair_set::binomial_s_pair_set(const binomial_ring *RR)
  : R(RR),
    _prev_lcm(NULL),
    _max_degree(0)
{
  _pairs = new s_pair_degree_list; // list header
  _npairs.append(0);
  _npairs.append(0);
}

void binomial_s_pair_set::enlarge(const binomial_ring *newR)
{
  const binomial_ring *old_ring = R;
  R = newR;

  old_ring->remove_monomial(_prev_lcm);
  _prev_lcm = NULL;
  for (s_pair_degree_list *thisdeg = _pairs->next; thisdeg != NULL; thisdeg = thisdeg->next)
    for (s_pair_lcm_list *thislcm = thisdeg->pairs; thislcm != NULL; thislcm = thislcm->next)
      R->translate_monomial(old_ring, thislcm->lcm);
}

void binomial_s_pair_set::remove_lcm_list(s_pair_lcm_list *p)
{
  while (p->pairs != NULL)
    {
      s_pair_elem *thispair = p->pairs;
      p->pairs = thispair->next;
      delete thispair;
    }
  R->remove_monomial(p->lcm);
  delete p;
}
void binomial_s_pair_set::remove_pair_list(s_pair_degree_list *p)
{
  while (p->pairs != NULL)
    {
      s_pair_lcm_list *thislcm = p->pairs;
      p->pairs = thislcm->next;
      remove_lcm_list(thislcm);
    }
  delete p;
}
binomial_s_pair_set::~binomial_s_pair_set()
{
  while (_pairs != NULL)
    {
      s_pair_degree_list *thisdeg = _pairs;
      _pairs = thisdeg->next;
      remove_pair_list(thisdeg);
    }
  R->remove_monomial(_prev_lcm);
}

void binomial_s_pair_set::insert_pair(s_pair_degree_list *q, binomial_s_pair &s)
{
  int cmp;
  s_pair_lcm_list head;
  head.next = q->pairs;
  s_pair_lcm_list *r = &head;
  while (true)
    {
      if (r->next == NULL || 
	  ((cmp = R->compare(s.lcm, r->next->lcm)) == GT))
	{
	  // Insert new lcm node
	  s_pair_lcm_list *r1 = new s_pair_lcm_list;
	  r1->next = r->next;
	  r1->lcm = s.lcm;
	  r1->pairs = NULL;
	  r->next = r1;
	  break;
	}
      if (cmp == EQ)
	{
	  R->remove_monomial(s.lcm);
	  break;
	}
      r = r->next;
    }
  r = r->next;
  s_pair_elem *s1 = new s_pair_elem(s.f1, s.f2);
  q->pairs = head.next;
  s1->next = r->pairs;
  r->pairs = s1;
}

void binomial_s_pair_set::insert(binomial_gb_elem *p)
{
  monomial lcm = R->make_monomial(R->lead_monomial(p->f));
  binomial_s_pair s(p, NULL, lcm);
  insert(s);
}

void binomial_s_pair_set::insert(binomial_s_pair s)
{
  int deg = R->degree(s.lcm);
  
  // Statistics control
  if (deg > _max_degree)
    {
      // Extend _npairs:
      for (int i=2*_max_degree+2; i<2*deg+2; i++)
	_npairs.append(0);
      _max_degree = deg;
    }
  _npairs[2*deg]++;
  _npairs[2*deg+1]++;

  s_pair_degree_list *q = _pairs;
  while (true)
    {
      if (q->next == NULL || q->next->deg > deg)
	{
	  // Insert new degree node
	  s_pair_degree_list *q1 = new s_pair_degree_list;
	  q1->next = q->next;
	  q1->deg = deg;
	  q1->pairs = NULL;
	  q->next = q1;
	  break;
	}
      if (q->next->deg == deg) break;
      q = q->next;
    }
  q = q->next;
  insert_pair(q, s);
  _n_elems++;
  q->n_elems++;
}

bool binomial_s_pair_set::next(const int *d, binomial_s_pair &result)
  // returns next pair in degrees <= *d, if any.
  // the caller should not free any of the three fields of the
  // s_pair!!
{
  if (_pairs->next == NULL) return false;
  if (d != NULL && _pairs->next->deg > *d) return false;
  s_pair_degree_list *thisdeg = _pairs->next;
  s_pair_lcm_list *thislcm = thisdeg->pairs;
  s_pair_elem *s = thislcm->pairs;

  thisdeg->n_elems--;
  _n_elems--;
  _npairs[2*(thisdeg->deg)+1]--;

  result = binomial_s_pair(s->f1, s->f2, thislcm->lcm);
  
  thislcm->pairs = s->next;
  if (thislcm->pairs == NULL)
    {
      // Now we must remove this set
      thisdeg->pairs = thislcm->next;
      R->remove_monomial(_prev_lcm);
      _prev_lcm = thislcm->lcm;
      thislcm->lcm = NULL;
      delete thislcm;
      
      if (thisdeg->pairs == NULL)
	{
	  // Now we must remove this larger degree list
	  _pairs->next = thisdeg->next;
	  delete thisdeg;
	}
    }

  delete s;
  return true;
}

int binomial_s_pair_set::lowest_degree() const
{
  if (_pairs->next == NULL) return -1;
  return _pairs->next->deg;
}

int binomial_s_pair_set::n_elems(int d) const
{
  s_pair_degree_list *p = _pairs;
  while (p->next != NULL && p->next->deg < d) p = p->next;
  if (p->next == NULL || p->next->deg != d) return 0;
  return p->next->n_elems;
}

int binomial_s_pair_set::n_elems() const
{
  return _n_elems;
}

void binomial_s_pair_set::stats() const
{
  buffer o;
  int np, nl;
  np = nl = 0;
  o << " degree" << "   pairs" << "   left" << "   done" << newline;
  for (int i=0; i<=_max_degree; i++)
    {
      np += _npairs[2*i];
      nl += _npairs[2*i+1];
      o.put(i, 7);              o << " ";
      o.put(_npairs[2*i], 6);   o << " ";
      o.put(_npairs[2*i+1], 6); o << " ";
      o.put(_npairs[2*i]-_npairs[2*i+1], 6);
      o << newline;
    }
  o << "  total ";
  o.put(np, 6);
  o << " ";
  o.put(nl,6);
  o << " ";
  o.put(np-nl,6);
  o << newline;
  emit(o.str());
}
///////////////////////
// Binomial GB table //
///////////////////////
binomialGB::binomialGB(const binomial_ring *R, bool bigcell,bool homogprime)
  : R(R), first(NULL), _max_degree(0),
    use_bigcell(bigcell),
    is_homogeneous_prime(homogprime)
{
}

binomialGB::~binomialGB()
{
  // Do nothing much, except maybe clear out stuff
  // so no stray pointers are around
  
  R = NULL;
  first = NULL;  // MES: BUG!! We are leaking stuff here!!
}

void binomialGB::enlarge(const binomial_ring *newR)
{
  R = newR;
}

void binomialGB::minimalize_and_insert(binomial_gb_elem *f)
  // remove elements which have lead term divisible by in(f).
  // optionally auto-reduces the other elements as well.
{
  monomial m = f->f.lead;
  gbmin_elem *fm = new gbmin_elem(f, R->mask(m));
  gbmin_elem head, *p;
  head.next = first;
  int deg = R->degree(m);
  if (deg > _max_degree) _max_degree = deg;
  for (p = &head ; p->next != NULL; p = p->next)
    {
      if (R->graded_compare(m, p->next->elem->f.lead) == LT)
	break;
    }
  fm->next = p->next;
  p->next = fm;
  first = head.next;

  p = fm;
  while (p->next != NULL)
    {
      if (R->degree(p->next->elem->f.lead) > deg 
	  && R->divides(m, p->next->elem->f.lead))
	{
	  // remove this element
	  gbmin_elem *q = p->next;
	  binomial_gb_elem *qe = q->elem;
	  p->next = q->next;
	  q->next = NULL;
	  qe->smaller = f;
	}
      else
	{
	  reduce_monomial(p->next->elem->f.tail);
	  p = p->next;
	}
    }
}

binomialGB::monomial_list *binomialGB::find_divisor(binomialGB::monomial_list *I, monomial m) const
{
  unsigned int mask = ~(R->mask(m));
  int d = R->degree(m);
  for (monomial_list *p = I; p != NULL; p = p->next)
    {
      if (R->degree(p->m) > d) return NULL;
      if (mask & p->mask) continue;
      if (R->divides(p->m, m))
	return p;
    }
  return NULL;
}

binomialGB::monomial_list *binomialGB::ideal_quotient(monomial m) const
{
  monomial_list *r;
  monomial_list **deglist = new monomial_list *[_max_degree+1];
  for (int i=0; i<=_max_degree; i++)
    deglist[i] = NULL;

  for (iterator p = begin(); p != end(); p++)
    {
      binomial_gb_elem *g = *p;
      gbmin_elem *gm = new gbmin_elem(g, R->mask(g->f.lead));
      monomial n = R->quotient(g->f.lead, m);
      monomial_list *nl = new monomial_list(n,R->mask(n),gm);
      int d = R->degree(n);
      nl->next = deglist[d];
      deglist[d] = nl;
    }
  monomial_list *result = NULL;

  for (int d=0; d<=_max_degree; d++)
    if (deglist[d] != NULL)
      {
	monomial_list *currentresult = NULL;
	while (deglist[d] != NULL)
	  {
	    monomial_list *p = deglist[d];
	    deglist[d] = p->next;
	    if (find_divisor(result, p->m))
	      {
		R->remove_monomial(p->m);
		delete p->tag;	// There is only one element at this point
		delete p;
	      }
	    else if ((r = find_divisor(currentresult, p->m)))
	      {
		gbmin_elem *p1 = new gbmin_elem(p->tag->elem, p->mask);
		R->remove_monomial(p->m);
		delete p;
		p1->next = r->tag;
		r->tag = p1;
	      }
	    else
	      {
		p->next = currentresult;
		currentresult = p;
	      }
	  }
	if (result == NULL)
	  result = currentresult;
	else if (currentresult != NULL)
	  {
	    monomial_list *q;
	    for (q = result; q->next != NULL; q = q->next);
	    q->next = currentresult;
	  }
	currentresult = NULL;
      }
  delete [] deglist;
  return result;
}

void binomialGB::make_new_pairs(binomial_s_pair_set *Pairs, binomial_gb_elem *f) const
{
  // Compute (a minimal generating set of)
  // the ideal quotient in(Gmin) : in(f).
  // Remove any that satisfy certain criteria:
  // 1. gcd(lead terms) is 1: remove.
  // 2. if homog_prime, gcd(tail terms) is not 1: remove.
  // Any that pass these tests, insert into Pairs.

  monomial m = f->f.lead;
  monomial_list *I = ideal_quotient(m);

  for (monomial_list *q = I; q != NULL; q = q->next)
    {
      gbmin_elem *ge = q->tag;  // a list of possibles
      
      binomial_gb_elem *g = ge->elem;

      // Criterion 1: gcd of lead terms must be not 1.
      if (R->gcd_is_one(R->lead_monomial(f->f), R->lead_monomial(g->f)))
	{
	  // remove each of the elements in 'g'.
	  continue;
	}

      // Criterion 2: if a homogeneous prime, 
      // gcd of tails must be 1.
      if (is_homogeneous_prime)
	{
	  if (!R->gcd_is_one(f->f.tail, g->f.tail))
	    continue;
	}

      // Finally do the insert
      monomial lcm = R->mult(m, q->m);
      Pairs->insert(binomial_s_pair(f, g, lcm));
    }
  remove_monomial_list(I);
}

void binomialGB::remove_monomial_list(monomial_list *mm) const
{
  while (mm != NULL)
    {
      R->remove_monomial(mm->m);
      monomial_list *tmp = mm;
      mm = mm->next;
      delete tmp;
    }
}

#if 0
binomial_gb_elem *binomialGB::find_divisor(monomial m) const
{
  unsigned int mask = ~(R->mask(m));
  int d = R->degree(m);
  for (iterator p = begin(); p != end(); ++p)
    {
      binomial_gb_elem *g = *p;
      if (R->degree(g->f.lead) > d) return NULL;
      if (mask & p.this_elem()->mask) continue;
      if (R->divides(g->f.lead, m))
	return g;
    }
  return NULL;
}
#endif

static int nfind = 0;
static int loop1 = 0;
static int loop2 = 0;
binomial_gb_elem *binomialGB::find_divisor(monomial m) const
{
  nfind++;
  unsigned int mask = ~(R->mask(m));
  int d = R->degree(m);
  for (gbmin_elem *p = first; p != NULL; p = p->next)
    {
      loop1++;
      if (loop1 % 1000000 == 0) emit_wrapped("m");
      if (R->degree(p->elem->f.lead) > d) return NULL;
      if (mask & p->mask) continue;
      loop2++;
      if (R->divides(p->elem->f.lead, m))
	return p->elem;
    }
  return NULL;
}

void binomialGB::reduce_monomial(monomial m) const
  // replace m with its normal form.
{
  binomial_gb_elem *p;
  while ((p = find_divisor(m)))
    R->spair_to(m, p->f.lead, p->f.tail);
}

bool binomialGB::reduce(binomial &f) const
{
  while (true)
    {
      binomial_gb_elem *p = find_divisor(f.lead);
      if (p == NULL)
	{
	  reduce_monomial(f.tail);
	  return R->normalize(f);
	}
      else 
	{
	  // Do the division:
	  if (!R->one_reduction_step(f,p->f))  // Modifies 'f'.
	    return false;
	}
    }
}
#if 0
bool binomialGB::reduce(binomial &f) const
{
  while (true)
    {
      binomial_gb_elem *p = find_divisor(f.lead);
      if (p == NULL)
	{
	  reduce_monomial(f.tail);
	  // The following should also check homog_prime, bigcell:
	  // 
	  return R->normalize(f);
	}
      else 
	{
	  // Do the division:
	  if (!R->one_reduction_step(f,p->f))  // Modifies 'f'.
	    return false;

	  if (is_homogeneous_prime)
	    {
	      if (!gcd_is_one(f)) return false;
	    }
	  else if (use_bigcell)
	    {
	      // if 'f' is divisible by a monomial, then we can strip this monomial.
	      remove_monomial_content(f);
	    }
	}
    }
}
#endif
int binomialGB::n_masks() const
{
  int *masks = new int[100000];
  buffer o;
  unsigned int nmasks = 1;
  masks[0] = first->mask;
  for (gbmin_elem *p = first; p != NULL; p = p->next)
    {
      o << " " << p->mask;
      bool found = false;
      for (unsigned int i=0; i<nmasks && !found; i++)
	if (masks[i] == p->mask)
	  {
	    found = true;
	    break;
	  }
      if (!found)
	masks[nmasks++] = p->mask;
    }
  emit(o.str());
  delete masks;
  return nmasks;
}
void binomialGB::debug_display() const
{
  buffer o;
  for (iterator p = begin(); p != end(); p++)
    {
      binomial_gb_elem *g = *p;
      R->elem_text_out(o, g->f);
      o << newline;
    }
  emit(o.str());
}

/////////////////////////////
// Binomial GB computation //
/////////////////////////////

binomialGB_comp::binomialGB_comp(const Ring *RR, int *wts, bool revlex,
				 unsigned int options)
  : gb_comp(2)
{
  // set the flags and options
  is_homogeneous = (options & GB_FLAG_IS_HOMOGENEOUS) != 0;
  is_nondegenerate = (options & GB_FLAG_IS_NONDEGENERATE) != 0;
  use_bigcell = (options & GB_FLAG_BIGCELL) != 0;
  flag_auto_reduce = true;
  flag_use_monideal = false;

  R = new binomial_ring(RR, wts, revlex);
  Pairs = new binomial_s_pair_set(R);
  Gmin = new binomialGB(R, use_bigcell, is_homogeneous && is_nondegenerate && !use_bigcell);
}

binomialGB_comp::~binomialGB_comp()
{
  int i;
  delete Gmin;
  delete Pairs;
  // remove each element of Gens
  for (i=0; i<Gens.length(); i++)
    delete Gens[i];
  // remove each element of G
  for (i=0; i<G.length(); i++)
    delete G[i];
  // The following is just to ease garbage collection
  for (i=0; i<mingens.length(); i++)
    mingens[i] = NULL;
  for (i=0; i<mingens_subring.length(); i++)
    mingens_subring[i] = NULL;
  delete R;
}

//////////////////////////
// Incremental routines //
//////////////////////////

void binomialGB_comp::enlarge(const Ring *newR, int *wts)
{
  const binomial_ring *old_ring = R;
  R = new binomial_ring(newR, wts, old_ring->revlex);

  // We need to change all of the monomials in sight.
  Gmin->enlarge(R);
  Pairs->enlarge(R);
  int i;
  for (i=0; i<Gens.length(); i++)
    R->translate_binomial(old_ring, Gens[i]->f);
  for (i=0; i<G.length(); i++)
    R->translate_binomial(old_ring, G[i]->f);

  delete old_ring;
}

void binomialGB_comp::add_generators(const Matrix *m)
{
  int i;
  binomial f;
  binomial_gb_elem *p;
  if (m->get_ring()->is_ZZ())
    {
      for (i=0; i<m->n_cols(); i++)
	{
	  f = R->make_binomial();
	  R->intvector_to_binomial((*m)[i],f);
	  p = new binomial_gb_elem(f);
	  Gens.append(p);
	  Pairs->insert(p);
	}
    }
  else
    {
      for (i=0; i<m->n_cols(); i++)
	{
	  f = R->make_binomial();
	  if (R->vector_to_binomial((*m)[i], f))
	    {
	      p = new binomial_gb_elem(f);
	      Gens.append(p);
	      Pairs->insert(p);
	    }
	  else
	    {
	      ERROR("expected binomials");
	      return;
	    }
	}
    }
}

////////////////////////
// Computation proper //
////////////////////////

void binomialGB_comp::process_pair(binomial_s_pair s)
{
  bool ismin, subringmin;
  binomial f;

  if (s.f2 == NULL)
    {
      // A generator
      ismin = true;
      subringmin = true;
      f = R->copy_binomial(s.f1->f);  // This is probably being left for garbage...
    }
  else
    {
      if (s.f1->smaller != NULL || s.f2->smaller != NULL)
	{
	  if (s.f1->smaller != s.f2 && s.f2->smaller != s.f1)
	    {
	      return;
	    }
	}

      if (comp_printlevel >= 5)
	{
	  buffer o;
	  o << "pair [";
	  R->elem_text_out(o, s.f1->f);
	  o << " ";
	  R->elem_text_out(o, s.f2->f);
	  o << "]" << newline;
	  emit(o.str());
	}
      if (!R->calc_s_pair(s, f))
	{
	  // The pair already reduces to zero.
	  return;
	}
      ismin = false;
      subringmin = (R->weight(s.lcm) > 0);
    }

  if (Gmin->reduce(f))
    {
      if (comp_printlevel >= 5)
	{
	  buffer o;
	  o << "  reduced to ";
	  R->elem_text_out(o, f);
	  emit_line(o.str());
	}
      subringmin = (subringmin && R->weight(f.lead) == 0);
      binomial_gb_elem *p = new binomial_gb_elem(f);      
      Gmin->make_new_pairs(Pairs, p);
      Gmin->minimalize_and_insert(p);
      if (ismin) mingens.append(p);
      if (subringmin) mingens_subring.append(p);
      G.append(p);
    }
  else
    R->remove_binomial(f);
}

int binomialGB_comp::gb_done(const intarray &/*stop_condtions*/) const
{
  return COMP_COMPUTING;
}

int binomialGB_comp::calc(const int *deg, const intarray &stop_conditions)
{
  binomial_s_pair s;
  while (Pairs->next(deg, s))
    {
      int ret = gb_done(stop_conditions);
      if (ret != COMP_COMPUTING) return ret;
      process_pair(s);		// consumes 's'.
      if (system_interrupted) return COMP_INTERRUPTED;
    }
  if (Pairs->n_elems() == 0)
    return COMP_DONE;
  return COMP_DONE_DEGREE_LIMIT;
}

///////////////////////
// Obtaining results //
///////////////////////

Matrix *binomialGB_comp::subring()
{
  // Subsequent calls will not receive duplicate elements
  Matrix *result = new Matrix(R->F);
  for (int i=0; i<mingens_subring.length(); i++)
    {
      result->append(R->binomial_to_vector(mingens_subring[i]->f));
      mingens_subring[i] = NULL;
    }
  mingens_subring.shrink(0);
  return result;
}

Matrix *binomialGB_comp::subringGB()
{
  Matrix *result = new Matrix(R->F);
  for (binomialGB::iterator p = Gmin->begin(); p != Gmin->end(); p++)
    if (R->weight((*p)->f.lead) == 0)
      result->append(R->binomial_to_vector((*p)->f));
  return result;
}

Matrix *binomialGB_comp::reduce(const Matrix *m, Matrix *&/*lift*/)
{
  ERROR("MES: not implemented yet");
  return 0;
}

Vector *binomialGB_comp::reduce(const Vector *v, Vector *&/*lift*/)
{
  ERROR("MES: not implemented yet");
  return 0;
}

int binomialGB_comp::contains(const Matrix */*m*/)
{
  ERROR("MES: not implemented yet");
  return 0;
}

bool binomialGB_comp::is_equal(const gb_comp * /*q*/)
{
  ERROR("MES: not implemented yet");
  return false;
}
  
Matrix *binomialGB_comp::min_gens_matrix()
{
  Matrix *result = new Matrix(R->F);
  for (int i=0; i<mingens.length(); i++)
    result->append(R->binomial_to_vector(mingens[i]->f));
  return result;
}

Matrix *binomialGB_comp::initial_matrix(int n)
{
  Matrix *result = new Matrix(R->F);
  for (binomialGB::iterator p = Gmin->begin(); p != Gmin->end(); p++)
      result->append(R->binomial_to_vector((*p)->f, n));
  return result;
}

Matrix *binomialGB_comp::gb_matrix()
{
  Matrix *result = new Matrix(R->F);
  for (binomialGB::iterator p = Gmin->begin(); p != Gmin->end(); p++)
      result->append(R->binomial_to_vector((*p)->f));
  return result;
}

Matrix *binomialGB_comp::change_matrix()
{
  return 0;
}

Matrix *binomialGB_comp::syz_matrix()
{
  return 0;
}

void binomialGB_comp::stats() const
{
  buffer o;
  o << "binomial GB ";
  if (is_homogeneous) o << "homogeneous ";
  else o << "inhomogeneous ";
  if (is_nondegenerate) o << "nondegenerate ";
  if (use_bigcell) o << "bigcell ";
  o << newline;
  o << "--- pairs ----" << newline;
  emit(o.str());
  Pairs->stats();

  o.reset();
  o << "nfind = " << nfind << newline << "loop1 = " << loop1 
    << newline << "loop2 = " << loop2 << newline;
  o << Gmin->n_masks() << newline;
  emit(o.str());

  if (comp_printlevel >= 3)
    Gmin->debug_display();
}
