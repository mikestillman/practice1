// Copyright 1996 Michael E. Stillman

#include "polyring.hpp"
#include "text_io.hpp"
#include "monoid.hpp"
#include "ringmap.hpp"
#include "matrix.hpp"
#include "ZZ.hpp"
#include "ntuple.hpp"
#include "termideal.hpp"
#include "gbring.hpp"
#include "frac.hpp"
#include "geopoly.hpp"

#include "gb_comp.hpp"
#include "../d/M2mem.h"

#include "QQ.hpp"
#include "monomial.hpp"
#include "relem.hpp"

#define POLY(q) ((q).poly_val)

PolyRing *PolyRing::trivial_poly_ring = 0; // Will be ZZ[]

PolyRing::~PolyRing()
{
  // Nothing to do
}

const PolyRing *PolyRing::get_trivial_poly_ring()
{
  if (trivial_poly_ring == 0)
    {
      make_trivial_ZZ_poly_ring();
    }

  return trivial_poly_ring;
}

// Order of initialization of trivial monoid M, and trivial ring ZZ[].
//
// (a) make ZZ ring.  Sets degree_ring, D_ to be 0.
// (b) make trivial monoid M, with M->info->degree_ring = 0.
// (c) make trivial poly R = ring(ZZ,M).
// (d) set ZZ's degree ring to be R
// (e) set M's degree ring to be R
// (f) set R's degree ring to be R.

void PolyRing::make_trivial_ZZ_poly_ring()
{
  if (trivial_poly_ring != 0) return;

  globalZZ = new ZZ;
  Monoid *M = Monoid::get_trivial_monoid();
  trivial_poly_ring = new PolyRing();

  Monoid::set_trivial_monoid_degree_ring(trivial_poly_ring);
  globalZZ->initialize_ZZ(trivial_poly_ring);
  trivial_poly_ring->initialize_poly_ring(globalZZ,M,
					  trivial_poly_ring);
  
  const PolyRing *flatR = trivial_poly_ring;
  trivial_poly_ring->_gb_ring = GBRing::create_PolynomialRing(flatR->Ncoeffs(), 
							      flatR->Nmonoms());
}

void PolyRing::initialize_poly_ring(const Ring *K, const Monoid *M, 
				    const PolynomialRing *deg_ring)
// This version is to be called directly ONLY by initialize_poly_ring
// and make_trivial_ZZ_poly_ring.
{
  initialize_ring(K->charac(),
		  M->n_vars(),
		  deg_ring);

  K_ = K;
  M_ = M;
  _poly_size = 
    sizeof(Nterm) + sizeof(int) * (M_->monomial_size() - 1);

  _gb_ring = 0;

  _coefficients_are_ZZ = (K_->is_ZZ());
  if (deg_ring->Nmonoms() == 0)
    this->setIsGraded(true);
  else
    this->setIsGraded(deg_ring->Nmonoms()->n_vars() > 0);
  _is_skew = 0;
  _EXP1 = newarray(int,_nvars);
  _EXP2 = newarray(int,_nvars);
  _EXP3 = newarray(int,_nvars);

  _is_ZZ_quotient = false;
  _ZZ_quotient_value = ZERO_RINGELEM;

  zeroV = from_int(0);
  oneV = from_int(1);
  minus_oneV = from_int(-1);
}

void PolyRing::initialize_poly_ring(const Ring *K, const Monoid *M)
{
  initialize_poly_ring(K, M, M->get_degree_ring());
}

const PolyRing *PolyRing::create(const Ring *K, const Monoid *M)
{
  PolyRing *R = new PolyRing;
  R->initialize_poly_ring(K,M);
  R->_gb_ring = GBRing::create_PolynomialRing(K,M);
  return R;
}

#if 0
const PolyRing *PolyRing::create(const Ring *K, const Monoid *M)
  // Create the ring K[M].  
  // K must be either a basic ring, or an ambient polynomial ring,
  //  possibly non-commutative of some sort.
{
  const PolynomialRing *A = K->cast_to_PolynomialRing();
  if (A == 0 || K->is_basic_ring())
    {
      // K is a basic ring
      return create_poly_ring(K,M);
    }
  const PolyRing *B = A->getAmbientRing();
  return B->createPolyRing(M);
}


const PolyRing *PolyRing::createPolyRing(const Monoid *M) const
  // creates this[M], which is commutative in M variables, but possible skew or Weyl
  // commutative in (some of) the variables of this
{
  const Monoid *newM = Monoid::tensor_product(M, getMonoid());
  if (newM == 0) return 0;

  return create_poly_ring(getCoefficients(),
			  newM,
			  this,
			  M);
}
#endif

void PolyRing::text_out(buffer &o) const
{
  K_->text_out(o);
  M_->text_out(o);
}

Nterm *PolyRing::new_term() const
{
  Nterm *result = GETMEM(Nterm *, _poly_size);
  result->next = NULL;
  result->coeff = 0;  // This value is never used, one hopes...
  // In fact, it gets used in the line below:       K->remove(tmp->coeff);
  // which is called from the line below:           remove(idiotic);
  // and it crashes there, because this assignment only sets the integer
  // part of the union, so on a machine with 4 byte ints and 8 byte pointers, the
  // pointer part is not NULL!
  result->coeff.poly_val = NULL;  // so I added this line
  return result;
}

Nterm *PolyRing::copy_term(const Nterm *t) const
{
  Nterm *result = new_term();
  result->coeff = K_->copy(t->coeff);
  M_->copy(t->monom, result->monom);
  return result;
}

ring_elem PolyRing::from_int(int n) const
{
  ring_elem a = K_->from_int(n);
  if (K_->is_zero(a)) 
    {
      return ZERO_RINGELEM;
    }
  Nterm *result = new_term();
  result->coeff = a;
  M_->one(result->monom);
  return result;
}
ring_elem PolyRing::from_int(mpz_t n) const
{
  ring_elem a = K_->from_int(n);
  if (K_->is_zero(a)) 
    {
      return ZERO_RINGELEM;
    }
  Nterm *result = new_term();
  result->coeff = a;
  M_->one(result->monom);
  return result;
}
ring_elem PolyRing::from_double(double n) const
{
  ring_elem a = K_->from_double(n);
  if (K_->is_zero(a)) 
    {
      return ZERO_RINGELEM;
    }
  Nterm *result = new_term();
  result->coeff = a;
  M_->one(result->monom);
  return result;
}

ring_elem PolyRing::var(int v) const
{
  for (int i=0; i<_nvars; i++) _EXP1[i] = 0;
  if (v >= 0 && v < _nvars) _EXP1[v] = 1;
  else 
    {
      return ZERO_RINGELEM;
    }
  Nterm *result = new_term();
  result->coeff = K_->from_int(1);
  M_->from_expvector(_EXP1, result->monom);
  return result;
}

int PolyRing::index_of_var(const ring_elem a) const
{
  Nterm *f = a;
  if (f->next != 0) return -1;
  if (!K_->is_equal(f->coeff, K_->from_int(1))) return -1;
  M_->to_expvector(f->monom, _EXP1);
  int result = -1;
  for (int i=0; i<n_vars(); i++)
    if (_EXP1[i] > 1) return -1;
    else if (_EXP1[i] == 1)
      {
	if (result >= 0) return -1;
	result = i;
      }
  return result;
}

M2_arrayint PolyRing::support(const ring_elem a) const
{
  for (int i=0; i<n_vars(); i++) _EXP1[i] = 0;
  for (const Nterm *f = a; f != 0; f = f->next)
    {
      M_->to_expvector(f->monom, _EXP2);
      for (int j=0; j<n_vars(); j++)
	if (_EXP2[j] != 0) _EXP1[j] = 1;
    }
  int nelems = 0;
  for (int i=0; i<n_vars(); i++)
    if (_EXP1[i] > 0) nelems++;
  M2_arrayint result = makearrayint(nelems);
  int next = 0;
  for (int i=0; i<n_vars(); i++)
    if (_EXP1[i] > 0)
	result->array[next++] = i;
  return result;
}

bool PolyRing::promote(const Ring *Rf, const ring_elem f, ring_elem &result) const
{
  // case 1:  Rf = A[x]/J ---> A[x]/I  is one of the 'base_ring's of 'this'.
  // case 2:  Rf = A      ---> A[x]/I  is the ring of scalars

  const PolynomialRing *Rf1 = Rf->cast_to_PolynomialRing();
  int nvars0 = n_vars();
  if (Rf1 != 0)
    nvars0 -= Rf1->n_vars();
  if (nvars0 == 0) 
    {
      result = copy(f);
      return true;
    }
  int *exp = newarray(int,nvars0);
  result = make_logical_term(Rf,f,exp);
  return true;
}

bool PolyRing::lift(const Ring *Rg, const ring_elem f, ring_elem &result) const
{
  // case 1:  Rf = A[x]/J ---> A[x]/I  is one of the 'base_ring's of 'this'.
  // case 2:  Rf = A      ---> A[x]/I  is the ring of scalars

  // We assume that Rg is one of the coefficient rings of 'this'

  const PolynomialRing *Rg1 = Rg->cast_to_PolynomialRing();
  int nvars0 = n_vars();
  if (Rg1 != 0)
    nvars0 -= Rg1->n_vars();
  int *exp = newarray(int,nvars0);
  lead_logical_exponents(nvars0,f,exp);
  if (!ntuple::is_one(nvars0,exp))
    return false;
  result = lead_logical_coeff(Rg,f);
  return true;
}

ring_elem PolyRing::preferred_associate(ring_elem ff) const
{
  Nterm *f = ff;
  if (f == NULL) return from_int(1);
  ring_elem c = K_->preferred_associate(f->coeff);
  Nterm *t = new_term();
  t->coeff = c;
  M_->one(t->monom);
  t->next = 0;
  return t;
}

bool PolyRing::is_unit(const ring_elem ff) const
{
  Nterm *f = ff;
  if (f == NULL) return false;
  if (f->next == NULL && M_->is_one(f->monom)
      && K_->is_unit(f->coeff))
    return true;
  return false;
}

bool PolyRing::is_zero(const ring_elem f) const
{
  Nterm *a = f;
  return a == NULL;
}

bool PolyRing::is_equal(const ring_elem f, const ring_elem g) const
{
  Nterm *a = f;
  Nterm *b = g;
  for ( ;; a = a->next, b = b->next)
    {
      if (a == NULL)
	{
	  if (b == NULL) return true;
	  return false;
	}
      if (b == NULL) return false;
      if (!K_->is_equal(a->coeff, b->coeff)) return false;
      if (_nvars > 0 && (M_->compare(a->monom, b->monom) != 0))
	return false;
    }
}

bool PolyRing::is_homogeneous(const ring_elem f) const
{
  if (!is_graded()) return false;
  Nterm *t = f;
  if (t == 0) return true;
  bool result = true;
  int *e = degree_monoid()->make_one();
  int *degf = degree_monoid()->make_one();
  M_->multi_degree(t->monom, degf);
  for (t = t->next ; t != NULL; t = t->next)
    {
      M_->multi_degree(t->monom, e);
      if (0 != degree_monoid()->compare(degf, e))
	{
	  result = false;
	  break;
	}
    }
  return result;
}

void PolyRing::degree(const ring_elem f, int *degf) const
{
  multi_degree(f, degf);
}

bool PolyRing::multi_degree(const ring_elem f, int *degf) const
{
  Nterm *t = f;
  int *e = degree_monoid()->make_one();
  if (t == 0) 
    {
      degree_monoid()->one(degf);
      return true;
    }
  M_->multi_degree(t->monom, degf);
  bool result = true;
  for (t = t->next ; t != NULL; t = t->next)
    {
      M_->multi_degree(t->monom, e);
      if (0 != degree_monoid()->compare(degf, e))
	{
	  result = false;;
	  degree_monoid()->lcm(degf, e, degf);
	}
    }
  return result;
}

int PolyRing::primary_degree(const ring_elem f) const
{
  Nterm *t = f;
  if (t == NULL) return 0;
  return M_->primary_degree(t->monom);
}

void PolyRing::degree_weights(const ring_elem f, const M2_arrayint wts, 
				    int &lo, int &hi) const
{
  Nterm *t = f;
  if (t == NULL)
    {
      lo = hi = 0;
      return;
    }
  int e = M_->degree_weights(t->monom, wts);
  lo = hi = e;
  for (t = t->next; t != NULL; t = t->next)
    {
      e = M_->degree_weights(t->monom, wts);
      if (e > hi) hi = e;
      else if (e < lo) lo = e;
    }
}

ring_elem PolyRing::homogenize(const ring_elem f, 
			     int v, int d, const M2_arrayint wts) const
{
  // assert(wts[v] != 0);
  // If an error occurs, then return 0, and set gError.

  int *exp = newarray(int, _nvars);

  Nterm head;
  Nterm *result = &head;
  for (Nterm *a = f ; a != NULL; a = a->next)
    {
      M_->to_expvector(a->monom, exp);
      int e = 0;
      for (int i=0; i<_nvars; i++) e += wts->array[i] * exp[i];
      if (((d-e) % wts->array[v]) != 0)
	{
	  // We cannot homogenize, so clean up and exit.
	  result->next = NULL;
	  ERROR("homogenization impossible");
	  result = NULL;
	  return result;
	}
      exp[v] += (d - e) / wts->array[v];
      if (_is_skew && _skew.is_skew_var(v) && exp[v] > 1)
	continue;
      result->next = new_term();
      result = result->next;
      result->coeff = K_->copy(a->coeff);
      M_->from_expvector(exp, result->monom);
    }
  result->next = NULL;
  sort(head.next);		// The monomial order, etc. might all have changed.
				// Some terms might even drop out
  deletearray(exp);
  return head.next;
}

ring_elem PolyRing::homogenize(const ring_elem f, int v, M2_arrayint wts) const
{
  Nterm *result = NULL;
  if (POLY(f) == NULL) return result;
  int lo, hi;
  degree_weights(f, wts, lo, hi);
  assert(wts->array[v] != 0);
  int d = (wts->array[v] > 0 ? hi : lo);
  return homogenize(f, v, d, wts);
}

ring_elem PolyRing::copy(const ring_elem f) const
{
  Nterm *a = f;
  Nterm head;
  Nterm *result = &head;
  for ( ; a != NULL; a = a->next, result = result->next)
      result->next = copy_term(a);
  result->next = NULL;
  return head.next;
}

void PolyRing::remove(ring_elem &f) const
{
}

void PolyRing::internal_negate_to(ring_elem &f) const
{
  Nterm *v = f;
  while (v != NULL)
    {
      K_->negate_to(v->coeff);
      v = v->next;
    }
}

void PolyRing::internal_add_to(ring_elem &ff, ring_elem &gg) const
{
  Nterm *f = ff;
  Nterm *g = gg;
  gg = ZERO_RINGELEM;
  if (g == NULL) return;
  if (f == NULL) { ff = g; return; }
  Nterm head;
  Nterm *result = &head;
  while (1)
    switch (M_->compare(f->monom, g->monom))
      {
      case -1:
	result->next = g;
	result = result->next;
	g = g->next;
	if (g == NULL) 
	  {
	    result->next = f; 
	    ff = head.next;
	    return;
	  }
	break;
      case 1:
	result->next = f;
	result = result->next;
	f = f->next;
	if (f == NULL) 
	  {
	    result->next = g; 
	    ff = head.next;
	    return;
	  }
	break;
      case 0:
	Nterm *tmf = f;
	Nterm *tmg = g;
	f = f->next;
	g = g->next;
	K_->add_to(tmf->coeff, tmg->coeff);
	if (_is_ZZ_quotient)
	  {
	    ring_elem t = K_->remainder(tmf->coeff, _ZZ_quotient_value);
	    tmf->coeff = t;
	  }
	if (!K_->is_zero(tmf->coeff))
	  {
	    result->next = tmf;
	    result = result->next;
	  }
	if (g == NULL) 
	  {
	    result->next = f; 
	    ff = head.next;
	    return;
	  }
	if (f == NULL) 
	  {
	    result->next = g; 
	    ff = head.next;
	    return;
	  }
	break;
      }
}

void PolyRing::internal_subtract_to(ring_elem &f, ring_elem &g) const
{
  internal_negate_to(g);
  internal_add_to(f,g);
}

ring_elem PolyRing::negate(const ring_elem f) const
{
  Nterm head;
  Nterm *result = &head;
  for (Nterm *a = f; a != NULL; a = a->next)
    {
      result->next = new_term();
      result = result->next;
      result->coeff = K_->negate(a->coeff);
      M_->copy(a->monom, result->monom);
    }
  result->next = NULL;
  return head.next;
}

ring_elem PolyRing::add(const ring_elem f, const ring_elem g) const
{
  ring_elem a = copy(f);
  ring_elem b = copy(g);
  internal_add_to(a, b);
  return a;
}

ring_elem PolyRing::subtract(const ring_elem f, const ring_elem g) const
{
  ring_elem a = copy(f);
  ring_elem b = negate(g);
  internal_add_to(a, b);
  return a;
}

ring_elem PolyRing::mult_by_term(const ring_elem f, 
				 const ring_elem c, const int *m) const
   // return f*c*m
{
  Nterm head;
  Nterm *result = &head;
  for (Nterm *a = f; a != NULL; a = a->next)
    {
      result->next = new_term();
      result = result->next;
      result->coeff = K_->mult(a->coeff, c);
      M_->mult(m, a->monom, result->monom);
    }
  result->next = NULL;
  return head.next;
}

void PolyRing::mult_coeff_to(ring_elem a, ring_elem &f) const
{
  Nterm *t = f;
  if (t == NULL) return;
  for ( ; t != NULL; t = t->next)
    {
      ring_elem tmp = t->coeff;
      t->coeff = K_->mult(a, tmp);
    }
}
ring_elem PolyRing::mult(const ring_elem f, const ring_elem g) const
{
  polyheap H(this);
  for (Nterm *a = f; a != NULL; a = a->next)
    {
      ring_elem h = mult_by_term(g, a->coeff, a->monom);
      H.add(h);
    }
  return H.value();
}

ring_elem PolyRing::power(const ring_elem f0, mpz_t n) const
{
  ring_elem ff, result;
  bool isinverted = false;

  if (mpz_sgn(n) == 0) return from_int(1);
  if (is_zero(f0)) return ZERO_RINGELEM;

  if (mpz_sgn(n) > 0)
    ff = f0;
  else
    {
      isinverted = true;
      ff = invert(f0);
      mpz_neg(n,n);
    }

  Nterm *f = ff;

  int n1;
  // In this case, the computation may only be formed in two
  // cases: (1) f is a constant, or (2) n is small enough
  if (ZZ::get_si(n1,n))
    {
      result = power(f,n1);
    }
  else if (is_unit(f))  // really want a routine 'is_scalar'...
    {
      ring_elem a = K_->power(f->coeff, n);
      result = make_flat_term(a, f->monom);
    }
  else 
    {
      ERROR("exponent too large");
      result = ZERO_RINGELEM;
    }

  if (isinverted) mpz_neg(n,n);
  return result;
}

ring_elem PolyRing::power(const ring_elem f0, int n) const
{
  ring_elem ff;

  if (n > 0)
    ff = f0;
  else if (n < 0)
    {
      ff = invert(f0);
      n = -n;
    }
  else 
    return from_int(1);

  ring_elem result, g, rest, h, tmp;
  ring_elem coef1, coef2, coef3;

  Nterm *lead = ff;
  if (lead == NULL) return ZERO_RINGELEM;

  rest = lead->next;
  g = from_int(1);

  // Start the result with the n th power of the lead term
  Nterm *t = new_term();
  t->coeff = K_->power(lead->coeff, n);
  M_->power(lead->monom, n, t->monom);
  t->next = NULL;
  //  if (_base_ring != NULL) normal_form(t);  NOT NEEDED
  result = t;

  if (POLY(rest) == 0) return result;
  int *m = M_->make_one();

  mpz_t bin_c;

  mpz_init_set_ui(bin_c, 1);

  for(int i=1; i<=n ; i++)
    {
      tmp = mult(g, rest);
      g = tmp;

      mpz_mul_ui(bin_c, bin_c, n-i+1);
      mpz_div_ui(bin_c, bin_c, i);

      coef1 = K_->from_int(bin_c);

      if (!K_->is_zero(coef1))
	{
	  coef2 = K_->power(lead->coeff, n-i);
	  coef3 = K_->mult(coef1, coef2);
	  M_->power(lead->monom, n-i, m);

	  h = mult_by_term(g, coef3, m);
	  add_to(result, h);
	}
    }
  return result;
}

ring_elem PolyRing::invert(const ring_elem f) const
{
  Nterm *ft = f;
  if (is_zero(f))
    {
      ERROR("cannot divide by zero");
      return ZERO_RINGELEM;
    }
  if (ft->next == NULL)
    if (M_->is_one(ft->monom))
      {
	Nterm *t = new_term();
	t->coeff = K_->invert(ft->coeff);
	M_->one(t->monom);
	return t;
      }
    else if (M_->is_invertible(ft->monom))
      {
	Nterm *t = new_term();
	t->coeff = K_->invert(ft->coeff);
	M_->power(ft->monom, -1, t->monom);
	return t;
      }
#warning "invert doesn't handle quotient rings yet"
#if 0
  if (_nvars == 1 && _quotient_ideal.length() == 1 && K_->is_field())
    {
      ring_elem u,v;
      ring_elem F = _quotient_ideal[0];
      ring_elem g = _base_ring->gcd_extended(F, f, u, v);
      if (!_base_ring->is_unit(g))
	{
	  ERROR("element is not invertible");
	  // MES: what about setting some global error ring element
	  // which contains this 'certificate' g of non-field-ness?
	}
      return v;
    }
  else
#endif
    {
      ERROR("division is not defined in this ring");
      return ZERO_RINGELEM;
    }
}

ring_elem PolyRing::divide(const ring_elem f, const ring_elem g) const
{
  ring_elem rem, d;
  rem = remainderAndQuotient(f,g,d);
  if (is_zero(rem)) return d;
  ring_elem ginv = invert(g);
  ring_elem result = mult(f, ginv);
  return result;
}

void PolyRing::imp_subtract_multiple_to(ring_elem &f, 
				 ring_elem a, const int *m, const ring_elem g) const
{
  ring_elem b = K_->negate(a);
  ring_elem h = mult_by_term(g, b, m);
  add_to(f, h);
}

bool PolyRing::imp_attempt_to_cancel_lead_term(ring_elem &f, 
						     ring_elem g, 
						     ring_elem &coeff, 
						     int *monom) const
{
  Nterm *t = f;
  Nterm *s = g;
  if (t == NULL || s == NULL) return true;
  ring_elem r = K_->remainderAndQuotient(t->coeff, s->coeff, coeff);
  bool result = (K_->is_zero(r));  // true means lead term will be cancelled.
  if (!K_->is_zero(coeff))
    {
      if (_is_skew)
	{
	  M_->to_expvector(t->monom, _EXP1);
	  M_->to_expvector(s->monom, _EXP2);
	  int sign = _skew.divide(_EXP1, _EXP2, _EXP3);
	  M_->from_expvector(_EXP3, monom);
	  if (sign < 0) K_->negate_to(coeff);
	  imp_subtract_multiple_to(f, coeff, monom, g);
	}
      else
	{
	  M_->divide(t->monom, s->monom, monom);
	  imp_subtract_multiple_to(f, coeff, monom, g);
	}
    }
  return result;
}

ring_elem PolyRing::gcd(const ring_elem ff, const ring_elem gg) const
{
  if (_nvars != 1)
    {
      ERROR("multivariate gcd not yet implemented");
      return ZERO_RINGELEM;
    }
  ring_elem f = copy(ff);
  ring_elem g = copy(gg);
  ring_elem s, rem;
  while (!is_zero(g))
    {
      rem = remainderAndQuotient(f, g, s);
      f = g;
      g = rem;
    }
#if 0
#warning commented out make monic in gcd
  make_monic(f);
#endif
  return f;
}

ring_elem PolyRing::gcd_extended(const ring_elem f, const ring_elem g, 
			       ring_elem &u, ring_elem &v) const
  // result == gcd(f,g) = u f + v g
{
  if (!has_gcd())
    {
      ERROR("cannot use gcd_extended in this ring");
      return ZERO_RINGELEM;
    }
  u = from_int(1);
  ring_elem result = copy(f);

  if (is_zero(g))
    {
      v = from_int(0);
      return result;
    }
  ring_elem v1 = ZERO_RINGELEM;
  ring_elem v3 = copy(g);
  ring_elem t1, t3;
  ring_elem temp1, temp2, temp3;
  while (!is_zero(v3))
    {
      ring_elem q;
      t3 = remainderAndQuotient(result, v3, q);

      // The following is: t1 = u - q*v1
      temp1 = mult(q,v1);
      subtract_to(u, temp1);
      t1 = u;

      u = v1;
      result = v3;
      v1 = t1;
      v3 = t3;
    }

  // make 'result' monic. (and divde 'u' by this as well)
  if (!is_zero(result))
    {
      Nterm *t = result;
      ring_elem c = K_->invert(t->coeff);
      mult_coeff_to(c, result);
      mult_coeff_to(c, u);
    }

  // The following is v = (result - f*u)/g
  temp1 = mult(f,u);
  temp2 = subtract(result, temp1);
  temp3 = remainderAndQuotient(temp2, g, v);

  return result;
}

void PolyRing::minimal_monomial(ring_elem f, int * &monom) const
{
  // Determines the minimal monomial which divides each term of f.
  // This monomial is placed into 'monom'.
  
  Nterm *t = f;
  if (t == NULL) return;
  M_->copy(t->monom, monom);
  for (t = t->next; t!=NULL; t=t->next)
    M_->gcd(t->monom,monom,monom);
}

ring_elem PolyRing::remainder(const ring_elem f, const ring_elem g) const
{
  ring_elem quot;
  ring_elem rem;
  rem = remainderAndQuotient(f,g,quot);
  return rem;
}

ring_elem PolyRing::quotient(const ring_elem f, const ring_elem g) const
{
  ring_elem quot;
  ring_elem rem;
  rem = remainderAndQuotient(f,g,quot);
  return quot;
}

ring_elem PolyRing::remainderAndQuotient(const ring_elem f, const ring_elem g, 
					       ring_elem &quot) const
{
  Nterm *q, *r;
  ring_elem rem;
  if (is_zero(g))
    {
      quot = from_int(0);
      return copy(f);
    }
  else
    {
      if (M_->is_group())
	{
	  Nterm *f1 = f;
	  Nterm *g1 = g;
	  r = powerseries_division_algorithm(f1,g1,q);
	  quot = q;
	  return r;
	}
      else 
	{
	  rem = division_algorithm(f,g,q);
	  quot = q;
	  return rem;
	}
    }
  quot = from_int(0);
  return from_int(0);
}


void PolyRing::syzygy(const ring_elem a, const ring_elem b,
			    ring_elem &x, ring_elem &y) const
{
  // Do some special cases first.  After that: compute a GB

  // For the GB, we need to make a 1 by 2 matrix, and compute until one syzygy is found.
  // create the matrix
  // create the gb comp
  // compute until one syz is found
  // grab the answer from the syz matrix.

  // Special situations:
  if (PolyRing::is_equal(b, one()))
    {
      x = PolyRing::copy(b);
      y = PolyRing::negate(a);
    }
  else if (PolyRing::is_equal(b, minus_one()))
    {
      x = one();
      y = PolyRing::copy(a);
    }
  else
    {
#if 0
// MES Aug 2002 ifdef'ed because gb_comp is not back yet
      intarray syzygy_stop_conditions;
      syzygy_stop_conditions.append(0); // ngb
      syzygy_stop_conditions.append(1); // nsyz
      syzygy_stop_conditions.append(0); // npairs
      syzygy_stop_conditions.append(0);
      syzygy_stop_conditions.append(0); 
      syzygy_stop_conditions.append(0); // subring limit
      syzygy_stop_conditions.append(0);
      
      const FreeModule *F = make_FreeModule(1);
      Matrix *m = new Matrix(F);
      m->append(F->raw_term(a,0));
      m->append(F->raw_term(b,0));
#if 0  
  buffer o;
  o << "constructing syzygy on ";
  elem_text_out(o,a);
  o << " and ";
  elem_text_out(o,b);
  emit_line(o.str());
  o.reset();
  o << "matrix is" << newline;
  m->text_out(o);
  emit_line(o.str());
  o.reset();
#endif

      gb_comp *g = gb_comp::make(m,true,-1,0);
      g->calc(0, syzygy_stop_conditions);
      Matrix *s = g->syz_matrix();

#if 0
  if (s.n_cols() != 1)
    {
      o << "found " << s.n_cols() << " syzygies";
      emit_line(o.str());
    }
#endif
      x = s->elem(0,0);
      y = s->elem(1,0);
      ring_elem c = preferred_associate(x);
      ring_elem x1 = mult(c,x);
      ring_elem y1 = mult(c,y);
      x = x1;
      y = y1;
#if 0
  o << "result: x = ";
  elem_text_out(o,x);
  o << " and y = ";
  elem_text_out(o,y);
  emit_line(o.str());
#endif
      deleteitem(g);
#endif      
    }
}

ring_elem PolyRing::random() const
{
  ERROR("not yet implemented");
  return 0;
}

void PolyRing::elem_text_out(buffer &o, const ring_elem f) const
{
#warning "display using parentheses"
  Nterm *t = f;
  if (t == NULL)
    {
      o << '0';
      return;
    }

  int old_one = p_one;
  int old_parens = p_parens;
  int old_plus = p_plus;
  
  int two_terms = (t->next != NULL);

  int needs_parens = p_parens && two_terms;
  if (needs_parens) 
    {
      if (old_plus) o << '+';
      o << '(';
      p_plus = 0;
    }

  for (t = f; t != NULL; t = t->next)
    {
      int isone = M_->is_one(t->monom);
      p_parens = !isone;
      p_one = (isone && needs_parens) || (isone && old_one);
      K_->elem_text_out(o,t->coeff);
      if (!isone)
	{
//	  if (!K_->is_equal(t->coeff, one) && !K_->is_equal(t->coeff, minus_one))
//	    o << "*";
	  M_->elem_text_out(o, t->monom);
	}
      p_plus = 1;
    }
  if (needs_parens) o << ')';

  p_one = old_one;
  p_parens = old_parens;
  p_plus = old_plus;

}

ring_elem PolyRing::eval(const RingMap *map, const ring_elem f) const
{
  // The way we collect the result depends on whether the target ring
  // is a polynomial ring: if so, use a heap structure.  If not, just add to the result.

  const Ring *target = map->get_ring();
  const PolynomialRing *targetP = target->cast_to_PolynomialRing();
  if (targetP != 0)
    {
      intarray vp;
      polyheap H(targetP);
      
      for (Nterm *t = f; t != NULL; t = t->next)
	{
	  vp.shrink(0);
	  M_->to_varpower(t->monom, vp);
	  ring_elem g = map->eval_term(K_, t->coeff, vp.raw());
	  H.add(g);
	}
      return H.value();
    }
  else 
    {
      ring_elem result = target->from_int(0);
      intarray vp;
      
      for (Nterm *t = f; t != NULL; t = t->next)
	{
	  vp.shrink(0);
	  M_->to_varpower(t->monom, vp);
	  ring_elem g = map->eval_term(K_, t->coeff, vp.raw());
	  target->add_to(result, g);
	}
      return result;
    }
}

/////////////////////////////////////////
// Useful division algorithm routines ///
/////////////////////////////////////////
// These are private routines, called from remainder
// or remainderAndQuotient or quotient.
/////////////////////////////////////////
Nterm * PolyRing::division_algorithm(Nterm *f, Nterm *g, Nterm *&quot) const
{
  // This returns the remainder, and sets quot to be the quotient.

  // This does standard division by one polynomial.
  // However, it does work for Weyl algebra, skew commutative algebra,
  // This works if the coefficient ring is a field, or ZZ.

  ring_elem a = copy(f);
  Nterm *t = a;
  Nterm *b = g;
  Nterm divhead;
  Nterm remhead;
  Nterm *divt = &divhead;
  Nterm *remt = &remhead;
  Nterm *nextterm = new_term();

  //  buffer o;
  while (t != NULL)
    if (M_->divides(b->monom, t->monom))
      {
	//o << "t = "; elem_text_out(o,t); o << newline;
	a = t;
	bool cancelled = imp_attempt_to_cancel_lead_term(a, g, nextterm->coeff, nextterm->monom);
	t = a;
	//	o << "  new t = "; elem_text_out(o,t); o << newline;
	//      o << "  cancelled = " << (cancelled ? "true" : "false") << newline;
	//	o << "  coeff = "; K_->elem_text_out(o,nextterm->coeff); o << newline;
	//	emit(o.str());
	if (!K_->is_zero(nextterm->coeff))
	  {
	    divt->next = nextterm;
	    divt = divt->next;
	    nextterm = new_term();
	  }
	if (!cancelled)
	  {
	    remt->next = t;
	    remt = remt->next;
	    t = t->next;
	  }
      }
    else
      {
	remt->next = t;
	remt = remt->next;
	t = t->next;
      }

  nextterm = NULL;
  remt->next = NULL;
  divt->next = NULL;
  quot = divhead.next;
  return remhead.next;
}

Nterm * PolyRing::division_algorithm(Nterm *f, Nterm *g) const
{
  // This does standard division by one polynomial, returning the remainder.
  // However, it does work for Weyl algebra, skew commutative algebra,
  // as long as the coefficient ring is a field.

  ring_elem a = copy(f);
  Nterm *t = a;
  Nterm *b = g;
  Nterm remhead;
  Nterm *remt = &remhead;
  ring_elem c;
  int *m = M_->make_one();
  while (t != NULL)
    if (M_->divides(b->monom, t->monom))
      {
	a = t;
	bool cancelled = imp_attempt_to_cancel_lead_term(a, g, c, m);
	t = a;
	if (!cancelled)
	  {
	    remt->next = t;
	    remt = remt->next;
	    t = t->next;
	  }
      }
    else
      {
	remt->next = t;
	remt = remt->next;
	t = t->next;
      }

  remt->next = NULL;
  return remhead.next;
}

Nterm * PolyRing::powerseries_division_algorithm(Nterm *f, Nterm *g, Nterm *&quot) const
{
  // This is intended for use when there is one variable, inverses are present,
  // and the coefficient ring is a field, or is ZZ.
  // The algorithm used is as follows.
  // Given a non-zero polynomial f = f(t,t^-1), let v(f) = top - bottom
  //   where top is the largest exponent in f, and bottom is the smallest.
  //   So if f is a monomial, v(f) = 0.  Also, v(fg) = v(f)v(g) (at least if the
  //   coefficient ring is a domain), and v(f) >= 0 always.
  // The algorithm is as follows:
  //   Reduce f = f0 by lt(g) to obtain f1, then again to obtain f2, etc.
  //   So v(f1) >= v(f2) >= ... >= v(fi),
  //   and either fi = 0, v(fi) < v(g), or v(f(i+1)) > v(fi).
  //   In this case, the remainder returned is fi.
  //   (Note: the last case won't happen if the coefficients are a field, or the
  //   lead coefficient of g is a unit).


  // This returns the remainder, and sets quot to be the quotient.

  ring_elem a = copy(f);
  Nterm *t = a;
  Nterm *b = g;
  Nterm divhead;
  Nterm remhead;
  Nterm *divt = &divhead;
  Nterm *remt = &remhead;
  Nterm *nextterm = new_term();
  int gval = 0, flast = 0;

  if (a != 0)
    {
      Nterm *z = a;
      for ( ; z->next != 0; z = z->next);

      if (degree_monoid()->n_vars() != 0) flast = M_->primary_degree(z->monom);
      else {
	M_->to_expvector(z->monom, _EXP1);
	flast = ntuple::degree(_nvars, _EXP1);
      }

    }

  if (g != 0)
    {
      int gfirst, glast;
      Nterm *z = b;

      if (degree_monoid()->n_vars() != 0) gfirst = M_->primary_degree(z->monom);
      else {
	M_->to_expvector(z->monom, _EXP1);
	gfirst = ntuple::degree(_nvars, _EXP1);
      }

      for ( ; z->next != 0; z = z->next);

      if (degree_monoid()->n_vars() != 0) glast = M_->primary_degree(z->monom);
      else {
	M_->to_expvector(z->monom, _EXP1);
	glast = ntuple::degree(_nvars, _EXP1);
      }

      gval = abs(gfirst-glast);
    }

  
  //  buffer o;
  while (t != NULL)
    {
      int ffirst;
      
      if (degree_monoid()->n_vars() != 0) ffirst = M_->primary_degree(t->monom);
      else {
	M_->to_expvector(t->monom, _EXP1);
	ffirst = ntuple::degree(_nvars, _EXP1);
      }

      int fval = abs(ffirst-flast);
      if (fval >= gval)
	{
	  //o << "t = "; elem_text_out(o,t); o << newline;
	  a = t;
	  bool cancelled = imp_attempt_to_cancel_lead_term(a, g, nextterm->coeff, nextterm->monom);
	  t = a;
	  //	o << "  new t = "; elem_text_out(o,t); o << newline;
	  //      o << "  cancelled = " << (cancelled ? "true" : "false") << newline;
	  //	o << "  coeff = "; K_->elem_text_out(o,nextterm->coeff); o << newline;
	  //	emit(o.str());
	  if (!K_->is_zero(nextterm->coeff))
	    {
	      divt->next = nextterm;
	      divt = divt->next;
	      nextterm = new_term();
	    }
	  if (!cancelled)
	    {
	      remt->next = t;
	      remt = remt->next;
	      t = t->next;
	    }
	}
      else
	{
	  remt->next = t;
	  remt = remt->next;
	  t = t->next;
	}
    }

  nextterm = NULL;
  remt->next = NULL;
  divt->next = NULL;
  quot = divhead.next;
  return remhead.next;
}

////////////////////////////////////
// Working with logical terms //////
////////////////////////////////////

ring_elem PolyRing::get_logical_coeff(const Ring *coeffR, const Nterm *&f) const
// Given an Nterm f, return the coeff of its logical monomial, in the
// polynomial ring coeffR.  f is modified, in that it is replaced by
// the pointer to the first term of f not used (possibly 0).
// coeffR should be a coefficient ring of 'this'.
{
  if (f == 0) return 0;
  if (coeffR == K_)
    {
      ring_elem result = f->coeff;
      f = f->next;
      return result;
    }
  const PolyRing *K = coeffR->cast_to_PolyRing();
  assert(K);
  Nterm head;
  Nterm *inresult = &head;
  inresult->next = 0;
  int *exp = newarray(int, n_vars());
  int *exp2 = newarray(int, n_vars());
  int nvars = n_vars() - K->n_vars();
  M_->to_expvector(f->monom, exp);
  ntuple::copy(n_vars(), exp, exp2);
  do {
      Nterm *t = K->new_term();
      inresult->next = t;
      inresult = t;
      t->coeff = f->coeff;
      K->getMonoid()->from_expvector(exp2+nvars, t->monom);

      f = f->next;
      if (f == 0) break;
      M_->to_expvector(f->monom, exp2);
  } while (EQ == ntuple::lex_compare(nvars, exp, exp2));
  inresult->next = 0;
  return head.next;
}

void PolyRing::lead_logical_exponents(int nvars0, const ring_elem f, int * result_exp) const
{
  Nterm *g = f;
  assert(g != NULL);
  int *exp = newarray(int,n_vars());
  M_->to_expvector(g->monom, exp);
  ntuple::copy(nvars0,exp,result_exp);
}

ring_elem PolyRing::lead_logical_coeff(const Ring *coeffR, const ring_elem f) const
{
  const Nterm *t = f;
  return get_logical_coeff(coeffR, t);
}

int PolyRing::n_logical_terms(int nvars0, const ring_elem f) const
{
  // nvars0 is the number of variables in the outer monoid
  if (nvars0 == n_vars()) return n_terms(f);
  Nterm *t = f;
  if (t == 0) return 0;
  int *exp1 = newarray(int,n_vars());
  int *exp2 = newarray(int,n_vars());
  M_->to_expvector(t->monom, exp1);
  int result = 1;
  for ( ; t != 0; t = t->next)
    {
      M_->to_expvector(t->monom, exp2);
      if (EQ == ntuple::lex_compare(nvars0, exp1, exp2))
	  continue;
      int *temp = exp1;
      exp1 = exp2;
      exp2 = temp;
      result++;
    }
  deletearray(exp1);
  deletearray(exp2);
  return result;
}  

ArrayPairOrNull PolyRing::list_form(const Ring *coeffR, const ring_elem f) const
{
  const PolynomialRing *coeffR1 = coeffR->cast_to_PolynomialRing();
  int nvars0 = n_vars();
  if (coeffR1 != 0)
    nvars0 -= coeffR1->n_vars();
  int n = n_logical_terms(nvars0,f);
  Monomial_array *monoms = GETMEM(Monomial_array *, sizeofarray(monoms,n));
  RingElement_array *coeffs = GETMEM(RingElement_array *, sizeofarray(coeffs,n));
  monoms->len = n;
  coeffs->len = n;
  ArrayPairOrNull result = newitem(ArrayPair);
  result->monoms = monoms;
  result->coeffs = coeffs;

  int *exp = newarray(int, n_vars());
  intarray resultvp;
  const Nterm *t = f;
  for (int next = 0; next < n; next++)
    {
      getMonoid()->to_expvector(t->monom, exp);
      ring_elem c = get_logical_coeff(coeffR, t); // increments t to the next term of f.
      varpower::from_ntuple(nvars0, exp, resultvp);
      monoms->array[next] = Monomial::make(resultvp.raw());
      coeffs->array[next] = RingElement::make_raw(coeffR, c);
      resultvp.shrink(0);

      assert( monoms->array[next] != NULL );
      assert( coeffs->array[next] != NULL );
    }
  deletearray(exp);
  return result;
}

ring_elem PolyRing::make_logical_term(const Ring *coeffR, const ring_elem a, const int *exp0) const
{
  const PolynomialRing *logicalK = coeffR->cast_to_PolyRing();

  int nvars0 = n_vars();
  if (logicalK == 0)
    {
      int *m = M_->make_one();
      M_->from_expvector(exp0,m);
      return make_flat_term(a,m);
    }
  
  nvars0 -= logicalK->n_vars();

  Nterm head;
  Nterm *inresult = &head;
  int *exp = newarray(int,M_->n_vars());
  ntuple::copy(nvars0, exp0, exp); // Sets the first part of exp
  for (Nterm *f = a; f != 0; f = f->next)
    {
      Nterm *t = new_term();
      inresult->next = t;
      inresult = t;
      t->coeff = f->coeff;
      logicalK->getMonoid()->to_expvector(f->monom, exp+nvars0);
      M_->from_expvector(exp, t->monom);
    }
  inresult->next = 0;
  return head.next;
}

ring_elem PolyRing::get_terms(int nvars0, const ring_elem f, int lo, int hi) const
{
  int nterms = n_logical_terms(nvars0,f);
  if (lo < 0) lo = nterms + lo;
  if (hi < 0) hi = nterms + hi;

  Nterm *t = f;
  Nterm head;
  Nterm *result = &head;

  int *exp1 = newarray(int,n_vars());
  int *exp2 = newarray(int,n_vars());
  M_->to_expvector(t->monom, exp1);
  int n = 0;
  while (t != NULL)
    {
      if (n > hi) break;
      if (n >= lo)
	{
	  result->next = copy_term(t);
	  result = result->next;
	}
      t = t->next;
      if (t == 0) break;
      M_->to_expvector(t->monom, exp2);
      if (EQ == ntuple::lex_compare(nvars0, exp1, exp2))
	continue;
      int *temp = exp1;
      exp1 = exp2;
      exp2 = temp;
      n++;
    }
  result->next = NULL;
  return head.next;
}

int PolyRing::n_flat_terms(const ring_elem f) const
{
  int result = 0;
  for (Nterm *a = f; a != NULL; a = a->next)
    result++;
  return result;
}

ring_elem PolyRing::make_flat_term(const ring_elem a, const int *m) const
{
  if (K_->is_zero(a)) return ZERO_RINGELEM;
  Nterm *t = new_term();
  t->coeff = K_->copy(a);
  M_->copy(m, t->monom);
  t->next = NULL;
  return t;
}

ring_elem PolyRing::lead_flat_coeff(const ring_elem f) const
{
  Nterm *t = f;
  if (t == NULL) return K_->from_int(0);
  return K_->copy(t->coeff);
}

const int * PolyRing::lead_flat_monomial(const ring_elem f) const
{
  Nterm *t = f;
  assert(t != NULL);
  return t->monom;
}

ring_elem PolyRing::get_coeff(const Ring *coeffR, const ring_elem f, const int *vp) const
  // note: vp is a varpower monomial.
{
#warning "uses flat monomials"
  const PolynomialRing *coeffR1 = coeffR->cast_to_PolynomialRing();
  int nvars0 = n_vars();
  if (coeffR1 != 0)
    nvars0 -= coeffR1->n_vars();

  int *exp = newarray(int, nvars0);
  int *exp2 = newarray(int, n_vars()); // FLAT number of variables
  varpower::to_ntuple(nvars0, vp, exp);

  // Now loop thru f until exponents match up.
  const Nterm *t = f;
  for ( ; t != 0; t = t->next)
    {
      M_->to_expvector(t->monom, exp2);
      if (EQ == ntuple::lex_compare(nvars0, exp, exp2))
	break;
    }

  ring_elem result = get_logical_coeff(coeffR1, t);
  deletearray(exp2);
  deletearray(exp);
  return result;
}

ring_elem PolyRing::diff(ring_elem a, ring_elem b, int use_coeff) const
{
  polyheap H(this);
  Nterm *d = new_term();
  for (Nterm *s = a; s != 0; s = s->next)
    {
      for (Nterm *t = b; t != 0; t = t->next)
	{
	  d->coeff = diff_term(s->monom, t->monom, d->monom, use_coeff);
	  if (!K_->is_zero(d->coeff))
	    {
	      K_->mult_to(d->coeff, s->coeff);
	      K_->mult_to(d->coeff, t->coeff);
	      d->next = 0;
	      H.add(d);
	      d = new_term();
	    }
	}
    }
  return H.value();
}

ring_elem PolyRing::diff_term(const int *m, const int *n, 
				    int *resultmon,
				    int use_coeff) const
{
  int sign = 0;
  if (!M_->divides(m, n)) return K_->from_int(0);
  if (_is_skew && use_coeff)
    {
      M_->to_expvector(m, _EXP1);
      M_->to_expvector(n, _EXP2);
      sign = _skew.diff(_EXP1, _EXP2, _EXP3);
      M_->from_expvector(_EXP3, resultmon);
    }
  else
    M_->divide(n, m, resultmon);
  ring_elem result = K_->from_int(1);
  if (!use_coeff) return result;
  intarray e1, e2;
  int *exp1 = e1.alloc(n_vars());
  int *exp2 = e2.alloc(n_vars());
  M_->to_expvector(m, exp1);
  M_->to_expvector(n, exp2);

  if (_is_skew && sign < 0)
    K_->negate_to(result);

  for (int i=0; i<n_vars(); i++)
    {
      for (int j=exp1[i]-1; j>=0; j--)
	{
	  ring_elem g = K_->from_int(exp2[i]-j);
	  K_->mult_to(result, g);
	  if (K_->is_zero(result)) return result;
	}
    }
  return result;
}

void PolyRing::sort(Nterm *&f) const
{
  // Divide f into two lists of equal length, sort each,
  // then add them together.  This allows the same monomial
  // to appear more than once in 'f'.
  
  if (f == NULL || f->next == NULL) return;
  Nterm *f1 = NULL;
  Nterm *f2 = NULL;
  while (f != NULL)
    {
      Nterm *t = f;
      f = f->next;
      t->next = f1;
      f1 = t;

      if (f == NULL) break;
      t = f;
      f = f->next;
      t->next = f2;
      f2 = t;
    }
  
  sort(f1);
  sort(f2);
  ring_elem g = f1;
  ring_elem h = f2;
  add_to(g, h);
  f = g;
}

bool PolyRing::in_subring(int nslots, const ring_elem a) const
{
  for (Nterm *t = a; t != 0; t = t->next)
    if (!M_->in_subring(nslots,t->monom)) return false;
  return true;
}

void PolyRing::degree_of_var(int n, const ring_elem a, int &lo, int &hi) const
{
  Nterm *t = a;
  if (t == NULL)
    {
      ERROR("attempting to find degree of a zero element");
      return;
    }
  int *exp = newarray_atomic(int,n_vars());
  M_->to_expvector(t->monom, exp);
  lo = hi = exp[n];
  for (t = t->next; t!=0; t=t->next)
    {
      M_->to_expvector(t->monom, exp);
      if (exp[n] < lo) 
	lo = exp[n];
      else if (exp[n] > hi)
	hi = exp[n];
    }
  deletearray(exp);
}

void PolyRing::monomial_divisor(const ring_elem a, int *exp) const
// Replaces the flat exponent vector 'exp' with its gcd with the gcd of all
// monomials of 'a'.
{
  int *exp1 = newarray_atomic(int,n_vars());
  for (const Nterm *t = a; t != 0; t = t->next)
    {
      M_->to_expvector(t->monom, exp1);
      ntuple::gcd(n_vars(),exp1,exp,exp);
    }
}

ring_elem PolyRing::divide_by_var(int n, int d, const ring_elem a) const
  // Divide each monomial of 'a' by x^d, where x is the n th variable.
  // If a monomial is not divisible by x^d, then that monomial is not put
  // into the result.
{
  if (d == 0) return a;
  Nterm head;
  Nterm *result = &head;
  int *exp = newarray(int,n_vars());
  for (Nterm *t = a; t != 0; t = t->next)
    {
      M_->to_expvector(t->monom, exp);
      if (exp[n] >= d)
	exp[n] -= d;
      else
	continue;
      result->next = new_term();
      result = result->next;
      result->coeff = t->coeff;
      M_->from_expvector(exp, result->monom);
    }
  deletearray(exp);
  result->next = 0;
  return head.next;
}

ring_elem PolyRing::divide_by_expvector(const int *exp, const ring_elem a) const
{
  Nterm * result = 0;
  int *exp0 = newarray(int,n_vars());
  for (Nterm *t = a; t != 0; t = t->next)
    {
      M_->to_expvector(t->monom, exp0);
      ntuple::quotient(n_vars(), exp0, exp, exp0);
      Nterm *u = new_term();
      u->coeff = t->coeff;
      M_->from_expvector(exp0, u->monom);
      u->next = result;
      result = u;
    }
  deletearray(exp0);
  sort(result);
  return result;
}
///////////////////////////////////
// vec routines for polynomials ///
///////////////////////////////////
ring_elem PolyRing::lead_term(int nparts, const ring_elem f) const
{
  Nterm *lead = f;
  Nterm head;
  Nterm *result = &head;
  int nslots = M_->n_slots(nparts);
  for (Nterm *a = f; a != NULL; a = a->next)
    {
      if (M_->compare(nslots, lead->monom, a->monom) != EQ)
	break;
      result->next = new_term();
      result = result->next;
      result->coeff = a->coeff;
      M_->copy(a->monom, result->monom);
    }
  result->next = NULL;
  return head.next;
}

const vecterm * PolyRing::vec_locate_lead_term(const FreeModule *F, vec v) const
// Returns a pointer to the lead vector of v.
// This works if F has a Schreyer order, or an up/down order.
{
  if (v == 0) return v;
  const vecterm * lead = v;
  const SchreyerOrder *S = F->get_schreyer_order();
  if (S)
    {
      for (vec w = v->next; w != 0; w = w->next)
	{
	  if (S->schreyer_compare(POLY(lead->coeff)->monom, 
				  lead->comp,
				  POLY(w->coeff)->monom, 
				  w->comp) == LT)
	    {
	      lead = w;
	    }
	}
    }
  else
    {
      for (vec w = v->next; w != 0; w = w->next)
	{
	  if (M_->compare(POLY(lead->coeff)->monom, 
			  lead->comp,
			  POLY(w->coeff)->monom, 
			  w->comp) == LT)
	    {
	      lead = w;
	    }
	}
    }
  return lead;
}

vec PolyRing::vec_lead_term(int nparts, const FreeModule *F, vec v) const
{
  // The first step is to find the lead monomial.

  const vecterm * lead = vec_locate_lead_term(F,v);

  // Now that we have the lead term, use the first n parts of the monomial
  // ordering

  ring_elem r = PolyRing::lead_term(nparts, lead->coeff);
  return make_vec(lead->comp, r);
}

vec PolyRing::vec_coefficient_of_var(vec v, int x, int e) const
// Find the coefficient of x^e in v.
{
  int *exp = newarray_atomic(int,n_vars());
  vecterm vec_head;
  vecterm *vec_result = &vec_head;
  for (vecterm *t = v; t != NULL; t = t->next)
    {
      Nterm head;
      Nterm *result = &head;
      for (Nterm *f = t->coeff; f != NULL; f=f->next)
	{
	  M_->to_expvector(f->monom, exp);
	  if (exp[x] != e) continue;
	  exp[x] = 0;
	  result->next = new_term();
	  result = result->next;
	  result->coeff = f->coeff;
	  M_->from_expvector(exp, result->monom);
	}
      result->next = NULL;
      vec_result->next = make_vec(t->comp, head.next);
      vec_result = vec_result->next;
    }
  deletearray(exp);
  vec_result->next = NULL;
  return vec_head.next;
}

vec PolyRing::vec_top_coefficient(const vec v, int &x, int &e) const
// find the smallest variable x which occurs in v, and also find e s.t. x^e is
// the largest power of x occuring in v.  Set x and e accordingly.
// Return the coefficient of x^e,
{
  x = n_vars();
  e = 0;
  if (v == NULL)
    {
      return NULL;
    }

  int *exp = newarray(int, n_vars());
  for (vec t = v; t != 0; t = t->next)
    for (Nterm *f = t->coeff; f != 0; f = f->next)
      {
	M_->to_expvector(f->monom, exp);
	for (int i=0; i<x; i++)
	  {
	    if (exp[i] > 0)
	      {
		x = i;
		e = exp[i];
		break;
	      }
	  }
	if (exp[x] > e)
	  e = exp[x];
      }
      
  // Now we have the variable, and its exponent.
  deletearray(exp);
  if (x == n_vars()) return v;
  return vec_coefficient_of_var(v, x, e);
}


///////////////////////////////////
// translation gbvector <--> vec //
///////////////////////////////////
#include "geovec.hpp"

gbvector *PolyRing::translate_gbvector_from_ringelem(ring_elem coeff) const
{
  GBRing *GR = get_gb_ring();
  gbvector head;
  gbvector *inresult = &head;
  for (Nterm *t = coeff; t != 0; t = t->next)
    {
      // make a gbvector node.
      gbvector *g = GR->gbvector_term(0, t->coeff, t->monom, 0);
      inresult->next = g;
      inresult = inresult->next;
    }
  return head.next;
}

gbvector * PolyRing::translate_gbvector_from_vec(const FreeModule *F, 
					     const vec v,
					     ring_elem &result_denominator) const
{
  GBRing *GR = get_gb_ring();
  gbvectorHeap H(GR,F);
  gbvector head;
  gbvector *inresult;
  for (vec w = v; w != 0; w=w->next)
    {
      inresult = &head;
      int comp = w->comp + 1;
      for (Nterm *t = w->coeff; t != 0; t = t->next)
	{
	  // make a gbvector node.
	  gbvector *g = GR->gbvector_term(F, t->coeff, t->monom, comp);
	  inresult->next = g;
	  inresult = inresult->next;
	}
      H.add(head.next);
    }

  result_denominator = getCoefficients()->one();
  return H.value();
}

vec PolyRing::translate_gbvector_to_vec(const FreeModule *F, const gbvector *v) const
{
#warning "is this too inefficient?"
  GBRing *GR = get_gb_ring();
  vecHeap H(F);

  for (const gbvector *t = v; t != 0; t=t->next)
    {
      Nterm *s = new_term();
      GR->gbvector_get_lead_monomial(F, t, s->monom);
      s->coeff = t->coeff; 
      s->next = 0;
      vec w = make_vec(t->comp-1, s);
      H.add(w);
    }

  return H.value();
}

vec PolyRing::translate_gbvector_to_vec_denom(const FreeModule *F, 
					  const gbvector *v,
					  const ring_elem denom) const
{
  GBRing *GR = get_gb_ring();
  const ring_elem c = K_->invert(denom);
  gbvector *w = GR->gbvector_mult_by_coeff(v, c);
  return translate_gbvector_to_vec(F,w);
}

// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e "
// End:

