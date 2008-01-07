// Copyright 1995 Michael E. Stillman

#include "ZZ.hpp"
#include "RRR.hpp"
#include "CCC.hpp"
#include "text-io.hpp"
#include "monoid.hpp"
#include "relem.hpp"
#include "ringmap.hpp"
#include "random.hpp"
#include "gbring.hpp"
#include "../d/M2mem.h"

mpfr_ptr CCC::_epsilon = NULL;

bool CCC::initialize_CCC() 
{
  initialize_ring(0);
  declare_field();
  _elem_size = sizeof(M2_CCC_struct);
  _zero_elem = new_elem();
  if (_epsilon == NULL) {
    _epsilon = reinterpret_cast<mpfr_ptr>(getmem(sizeof(mpfr_t)));
    mpfr_init(_epsilon);
  }

  zeroV = from_int(0);
  oneV = from_int(1);
  minus_oneV = from_int(-1);

  return true;
}

CCC *CCC::create()
{
  CCC *result = new CCC;
  result->initialize_CCC();
  return result;
}

void CCC::text_out(buffer &o) const
{
  o << "CCC";
}

mpfr_ptr CCC::new_mpfr() const
{
  mpfr_ptr result = reinterpret_cast<mpfr_ptr>(getmem(_elem_size));
  mpfr_init(result);
  return result;
}

M2_CCC CCC::new_elem() const
{
  M2_CCC result = reinterpret_cast<M2_CCC>(getmem(_elem_size));
  mpfr_init(&result->re);
  mpfr_init(&result->im);
  return result;
}
void CCC::remove_elem(M2_CCC f) const
{
  // mpfr_clear(f);
}

ring_elem CCC::random() const
{
  return from_int(0);
}

void CCC::elem_text_out(buffer &o, const ring_elem ap) const
{
  mpfr_ptr a = BIGCC_RE(ap);
  mpfr_ptr b = BIGCC_IM(ap);
  mp_exp_t expptr;

  // easier to call RRR::elem_text_out here ???

  // size_t size = 1000;
  char *s = newarray_atomic(char,1000);
  char *str;

  bool is_neg = (mpfr_cmp_si(a, 0) == -1);
  bool is_one = (mpfr_cmp_si(a, 1) == 0 || mpfr_cmp_si(a, -1) == 0);

  // int size = mpfr_sizeinbase(a, 10) + 2;

  // char *allocstr = (size > 1000 ? newarray(char,size) : s);

  if (!is_neg && p_plus) o << '+';
  if (is_one) 
    {  
      if (is_neg) o << '-';
      if (p_one) o << '1'; 
    }
  else
    {
      str = mpfr_get_str(s, &expptr, 10, 0, a, GMP_RNDN);
      o << "." << str << "*10^" << expptr;
    }

  is_neg = (mpfr_cmp_si(b, 0) == -1);
  is_one = (mpfr_cmp_si(b, 1) == 0 || mpfr_cmp_si(b, -1) == 0);

  str = mpfr_get_str(s, &expptr, 10, 0, b, GMP_RNDN);
  o << "+ (." << str << "*10^" << expptr << ") ii";

  // if (size > 1000) deletearray(allocstr);
}

void CCC::set_epsilon(mpfr_ptr epsilon)
{
  if (_epsilon == NULL) {
    _epsilon = reinterpret_cast<mpfr_ptr>(getmem(sizeof(mpfr_t)));
    mpfr_init(_epsilon);
  }
  mpfr_set(_epsilon, epsilon, GMP_RNDN);
}

mpfr_ptr CCC::get_epsilon()
{
  mpfr_ptr epsilon = reinterpret_cast<mpfr_ptr>(getmem(sizeof(mpfr_t)));
  mpfr_init(epsilon);
  mpfr_set(epsilon, _epsilon, GMP_RNDN);
  return epsilon;
}

ring_elem CCC::from_int(int n) const
{
  M2_CCC result = new_elem();
  mpfr_set_si(&result->re, n, GMP_RNDN);

  return BIGCC_RINGELEM(result);
}

ring_elem CCC::from_int(mpz_ptr n) const
{
  M2_CCC result = new_elem();
  mpfr_set_z(&result->re, n, GMP_RNDN);

  return BIGCC_RINGELEM(result);
}

ring_elem CCC::from_double(double r) const
{
  M2_CCC result = new_elem();
  mpfr_set_d(&result->re, r, GMP_RNDN);

  return BIGCC_RINGELEM(result);
}

ring_elem CCC::from_doubles(double r, double s) const
{
  M2_CCC result = new_elem();
  mpfr_set_d(&result->re, r, GMP_RNDN);
  mpfr_set_d(&result->im, s, GMP_RNDN);

  return BIGCC_RINGELEM(result);
}

ring_elem CCC::from_rational(mpq_ptr r) const
{
  M2_CCC result = new_elem();
  mpfr_set_q(&result->re, r, GMP_RNDN);

  return BIGCC_RINGELEM(result);
}

ring_elem CCC::from_complex(M2_CC z) const
{
  M2_CCC result = new_elem();
  mpfr_set_d(&result->re, z->re, GMP_RNDN);
  mpfr_set_d(&result->im, z->im, GMP_RNDN);

  return BIGCC_RINGELEM(result);
}

ring_elem CCC::from_BigReal(mpfr_ptr r) const
{
  M2_CCC result = new_elem();
  mpfr_set(&result->re, r, GMP_RNDN);

  return BIGCC_RINGELEM(result);
}

ring_elem CCC::from_BigReals(mpfr_ptr a, mpfr_ptr b) const
{
  M2_CCC result = new_elem();
  mpfr_set(&result->re, a, GMP_RNDN);
  mpfr_set(&result->im, b, GMP_RNDN);

  return BIGCC_RINGELEM(result);
}

ring_elem CCC::from_BigComplex(M2_CCC z) const
{
  M2_CCC result = new_elem();
  mpfr_set(&result->re, &z->re, GMP_RNDN);
  mpfr_set(&result->im, &z->im, GMP_RNDN);

  return BIGCC_RINGELEM(result);
}

mpfr_ptr CCC::to_BigReal(ring_elem f) const
{
  return BIGCC_RE(f);
}

bool CCC::promote(const Ring *Rf, const ring_elem f, ring_elem &result) const
{
  if (Rf == globalRRR)
    {
      M2_CCC g = new_elem();
      mpfr_set(&g->re, MPF_VAL(f), GMP_RNDN);
      mpfr_init(&g->im);
      
      result = BIGCC_RINGELEM(g);
    }
  return false;
}

bool CCC::lift(const Ring *Rg, const ring_elem f, ring_elem &result) const
{
  return false;
}

bool CCC::is_unit(const ring_elem f) const
{
  return !is_zero(f);
}

bool CCC::is_zero(const ring_elem f) const
{
  mpfr_ptr a = BIGCC_RE(f);
  mpfr_ptr b = BIGCC_IM(f);
  return (mpfr_sgn(a) == 0 && mpfr_sgn(b) == 0);
}

bool CCC::is_equal(const ring_elem f, const ring_elem g) const
{
  mpfr_ptr ar = BIGCC_RE(f);
  mpfr_ptr ai = BIGCC_IM(f);
  mpfr_ptr br = BIGCC_RE(g);
  mpfr_ptr bi = BIGCC_IM(g);

  if (mpfr_sgn(_epsilon) == 0) 
    {
      return (mpfr_cmp(ar, br) == 0 && mpfr_cmp(ai, bi) == 0);
    }    
  else
    {
      mpfr_ptr cr = new_mpfr();
      mpfr_ptr ci = new_mpfr();
      mpfr_ptr dr = new_mpfr();
      mpfr_ptr di = new_mpfr();
      mpfr_sub(cr, ar, br, GMP_RNDN);
      mpfr_sub(ci, ai, bi, GMP_RNDN);
      mpfr_sub(dr, br, ar, GMP_RNDN);
      mpfr_sub(di, bi, ai, GMP_RNDN);
      return (mpfr_cmp(cr, _epsilon) < 0 && mpfr_cmp(ci, _epsilon) < 0 &&
	      mpfr_cmp(dr, _epsilon) < 0 && mpfr_cmp(di, _epsilon) < 0);
    }
}
bool CCC::is_greater(const ring_elem f, const ring_elem g) const
{
  M2_CCC a = new_elem();
  M2_CCC b = new_elem();

  mpfr_pow_ui(&a->re, BIGCC_RE(f), 2, GMP_RNDN);
  mpfr_pow_ui(&a->im, BIGCC_IM(f), 2, GMP_RNDN);
  mpfr_pow_ui(&b->re, BIGCC_RE(g), 2, GMP_RNDN);
  mpfr_pow_ui(&b->im, BIGCC_IM(g), 2, GMP_RNDN);
  mpfr_add(&a->re, &a->re, &a->im, GMP_RNDN);
  mpfr_add(&b->re, &b->re, &b->im, GMP_RNDN);
  return mpfr_cmp(&a->re,&b->re) > 0;
}

int CCC::compare_elems(const ring_elem f, const ring_elem g) const
{
  int cmp = mpfr_cmp(BIGCC_RE(f), BIGCC_RE(g));
  if (cmp > 0) return 1;
  if (cmp < 0) return -1;
  cmp = mpfr_cmp(BIGCC_IM(f),BIGCC_IM(g));
  if (cmp > 0) return 1;
  if (cmp < 0) return -1;
  return 0;
}

bool CCC::is_real(const ring_elem f) const
{
  mpfr_ptr im = BIGCC_IM(f);
  return mpfr_sgn(im) == 0;
}

void CCC::zeroize_tiny_lead_components(vec &v, mpfr_ptr epsilon) const
{
  while (v != NULL) {
    mpfr_ptr re = new_mpfr();
    mpfr_ptr im = new_mpfr();
    mpfr_abs(re, BIGCC_RE(v->coeff), GMP_RNDN);
    mpfr_abs(im, BIGCC_IM(v->coeff), GMP_RNDN);
    if (epsilon != NULL && mpfr_cmp(re, epsilon) < 0 && mpfr_cmp(im, epsilon)) {
      v = v->next;
    } else return;    
  }
}

ring_elem CCC::absolute(const ring_elem f) const
{
  M2_CCC result = new_elem();

  mpfr_pow_ui(&result->re, BIGCC_RE(f), 2, GMP_RNDN);
  mpfr_pow_ui(&result->im, BIGCC_IM(f), 2, GMP_RNDN);
  mpfr_add(&result->re, &result->re, &result->im, GMP_RNDN);
  mpfr_sqrt(&result->re, &result->re, GMP_RNDN);
  mpfr_set_ui(&result->im, 0, GMP_RNDN);

  return BIGCC_RINGELEM(result);
}

ring_elem CCC::copy(const ring_elem f) const
{
  mpfr_ptr a = BIGCC_RE(f);
  mpfr_ptr b = BIGCC_IM(f);

  M2_CCC result = new_elem();
  mpfr_set(&result->re, a, GMP_RNDN);
  mpfr_set(&result->im, b, GMP_RNDN);
  return BIGCC_RINGELEM(result);
}

void CCC::remove(ring_elem &f) const
{
#if 0
//   M2_CCC z = BIGCC_VAL(f);
//   remove_elem(z); // does nothing... get rid of this code?
//   f = BIGCC_RINGELEM(NULL);
#endif
}

// TO DO: MAKE IT SAME AS CC
ring_elem CCC::preferred_associate(ring_elem f) const
{
  return from_int(1);
}

void CCC::internal_negate_to(ring_elem &f) const
{
  mpfr_neg(BIGCC_RE(f), BIGCC_RE(f), GMP_RNDN);
  mpfr_neg(BIGCC_IM(f), BIGCC_IM(f), GMP_RNDN);
}

void CCC::internal_add_to(ring_elem &f, ring_elem &g) const
{
  mpfr_add(BIGCC_RE(f), BIGCC_RE(f), BIGCC_RE(g), GMP_RNDN);
  mpfr_add(BIGCC_IM(f), BIGCC_IM(f), BIGCC_IM(g), GMP_RNDN);
  // remove(g); should this be removed?
}

void CCC::internal_subtract_to(ring_elem &f, ring_elem &g) const
{
  mpfr_sub(BIGCC_RE(f), BIGCC_RE(f), BIGCC_RE(g), GMP_RNDN);
  mpfr_sub(BIGCC_IM(f), BIGCC_IM(f), BIGCC_IM(g), GMP_RNDN);
  // remove(g); should g be removed?
}

ring_elem CCC::negate(const ring_elem f) const
{
  M2_CCC result = new_elem();
  mpfr_neg(&result->re, BIGCC_RE(f), GMP_RNDN);
  mpfr_neg(&result->im, BIGCC_IM(f), GMP_RNDN);
  return BIGCC_RINGELEM(result);
}

ring_elem CCC::add(const ring_elem f, const ring_elem g) const
{
  M2_CCC result = new_elem();
  mpfr_add(&result->re, BIGCC_RE(f), BIGCC_RE(g), GMP_RNDN);
  mpfr_add(&result->im, BIGCC_IM(f), BIGCC_IM(g), GMP_RNDN);
  return BIGCC_RINGELEM(result);
}

ring_elem CCC::subtract(const ring_elem f, const ring_elem g) const
{
  M2_CCC result = new_elem();
  mpfr_sub(&result->re, BIGCC_RE(f), BIGCC_RE(g), GMP_RNDN);
  mpfr_sub(&result->im, BIGCC_IM(f), BIGCC_IM(g), GMP_RNDN);
  return BIGCC_RINGELEM(result);
}

ring_elem CCC::mult(const ring_elem f, const ring_elem g) const
{
  M2_CCC result = new_elem();
  M2_CCC tmp = new_elem();
  mpfr_mul(&result->re, BIGCC_RE(f), BIGCC_RE(g), GMP_RNDN);
  mpfr_mul(&tmp->re, BIGCC_IM(f), BIGCC_IM(g), GMP_RNDN);
  mpfr_sub(&result->re, &result->re, &tmp->re, GMP_RNDN);
  mpfr_mul(&result->im, BIGCC_RE(f), BIGCC_IM(g), GMP_RNDN);
  mpfr_mul(&tmp->im, BIGCC_IM(f), BIGCC_RE(g), GMP_RNDN);
  mpfr_add(&result->im, &result->im, &tmp->im, GMP_RNDN);
  return BIGCC_RINGELEM(result);
}

ring_elem CCC::power(const ring_elem f, int n) const
{
  ring_elem curr_pow;
  ring_elem result = from_int(1);
  if (n == 0)
    {
      return result;
    }
  else if (n < 0)
    {
      n = -n;
      curr_pow = invert(f);
    }
  else
    {
      curr_pow = copy(f);
    }

  while (n > 0)
    {
      if (n%2) {
	result = CCC::mult(result, curr_pow);
      }
      n = n/2;
      curr_pow = CCC::mult(curr_pow, curr_pow);
    }
  return result;
}

ring_elem CCC::power(const ring_elem f, mpz_t n) const
{
  int n1;
  if (!RingZZ::get_si(n1, n)) 
    { 
      ERROR("exponent too large"); 
      return from_int(1);
    }
  return power(f, n1);
}

ring_elem CCC::invert(const ring_elem f) const
{
  if (is_zero(f))
    return from_int(0);
  else {
    M2_CCC result = new_elem();
    M2_CCC tmp = new_elem();
    mpfr_mul(&tmp->re, BIGCC_RE(f), BIGCC_RE(f), GMP_RNDN);
    mpfr_mul(&tmp->im, BIGCC_IM(f), BIGCC_IM(f), GMP_RNDN);
    mpfr_add(&tmp->re, &tmp->re, &tmp->im, GMP_RNDN);
    mpfr_div(&result->re, BIGCC_RE(f), &tmp->re, GMP_RNDN);
    mpfr_div(&result->im, BIGCC_IM(f), &tmp->re, GMP_RNDN);
    mpfr_neg(&result->im, &result->im, GMP_RNDN);
    return BIGCC_RINGELEM(result);
  }
}

ring_elem CCC::divide(const ring_elem f, const ring_elem g) const
{
  ring_elem h = CCC::invert(g);
  return CCC::mult(f, h);
}

void CCC::syzygy(const ring_elem a, const ring_elem b,
	       ring_elem &x, ring_elem &y) const
{
  if (is_zero(b))
    {
      x = from_int(0);
      y = from_int(1);
    }
  else 
    {
      x = from_int(1);
      y = divide(negate(a),b);
    }
}

ring_elem CCC::eval(const RingMap *map, const ring_elem f, int) const
{
  return map->get_ring()->from_BigComplex(BIGCC_VAL(f));
}




// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e "
// End:
