// Copyright 2001 Michael E. Stillman

#include "RR.hpp"
#include <stdio.h>
#include <math.h>
#include "text_io.hpp"
#include "monoid.hpp"
#include "relem.hpp"
#include "ringmap.hpp"
#include "random.hpp"
#include "gbring.hpp"
#include "../d/M2mem.h"
#include "coeffrings.hpp"

#if 0
#define RRELEM_VAL(f) (RRelem ((f).poly_val))
#define RR_VAL(f) ((RRELEM_VAL(f))->val)
#define RR_RINGELEM(a) ((ring_elem) ((Nterm *) (a)))
#endif

#define RRELEM_VAL(f) (reinterpret_cast<RRelem>((f).poly_val))
#define RR_RINGELEM(a) (ring_elem(reinterpret_cast<Nterm *>(a)))
#define RR_VAL(f) ((RRELEM_VAL(f))->val)

RingRR::~RingRR()
{
}

bool RingRR::initialize_RR(double epsilon) 
{
  initialize_ring(0);

  declare_field();

  _epsilon = epsilon;

  zeroV = from_int(0);
  oneV = from_int(1);
  minus_oneV = from_int(-1);

  coeffR = new CoefficientRingRR;
  return true;
}

RingRR *RingRR::create(double epsilon)
{
  RingRR *result = new RingRR;
  result->initialize_RR(epsilon);
  return result;
}


void RingRR::text_out(buffer &o) const
{
  o << "RR";
}

ring_elem RingRR::from_int(int n) const
{
  double a = n;
  return RingRR::from_double(a);
}

RingRR::RRelem RingRR::new_elem() const
{
  RRelem result = reinterpret_cast<RRelem>(getmem(sizeof(RRelem_rec)));
  result->val = 0.0;
  return result;
}
ring_elem RingRR::from_double(double a) const
{
  RRelem result = reinterpret_cast<RRelem>(getmem(sizeof(RRelem_rec)));
  result->val = a;
  return RR_RINGELEM(result);
}
double RingRR::to_double(ring_elem a)
{
  return RR_VAL(a);
}

ring_elem RingRR::from_rational(mpq_ptr r) const
{
  RRelem result = reinterpret_cast<RRelem>(getmem(sizeof(RRelem_rec)));
  result->val = mpq_get_d(r);
  return RR_RINGELEM(result);
}
ring_elem RingRR::from_BigReal(mpf_ptr a) const
{
  RRelem result = reinterpret_cast<RRelem>(getmem(sizeof(RRelem_rec)));
  result->val = mpf_get_d(a);
  return RR_RINGELEM(result);
}

void RingRR::remove_elem(RRelem f) const
{
}

ring_elem RingRR::random() const
{
  int d = 1000000;
  long r = Random::random0(2*d);
  double a = (r*1.0)/d - 1.0;
  return RingRR::from_double(a);
}

int RingRR::compare_RR(double a, double b) const
  // This is our notion of equality
{
  double c = a-b;
  if (c > _epsilon) return GT;
  if (c < -_epsilon) return LT;
  return EQ;
}

void RingRR::elem_text_out(buffer &o, const ring_elem ap) const
{
  double a = RR_VAL(ap);

  char s[100];

  bool is_neg = compare_RR(a,0.0) == LT;
  a = fabs(a);
  bool is_one = compare_RR(a,1.0) == EQ;

  if (!is_neg && p_plus) o << '+';
  if (is_neg) o << '-';
  if (is_one) 
    {  
      if (p_one) o << '1'; 
    }
  else
    {
      sprintf(s, "%f", a);
      o << s;
    }
}


ring_elem RingRR::from_int(mpz_ptr n) const
{
  double a = mpz_get_d(n);
  return RingRR::from_double(a);
}

bool RingRR::promote(const Ring *, const ring_elem, ring_elem &) const
{
  return false;
}

bool RingRR::lift(const Ring *, const ring_elem, ring_elem &) const
{
  return false;
}

bool RingRR::is_zero_RR(double a) const
{
  return compare_RR(a,0.0) == EQ;
}

bool RingRR::is_zero(const ring_elem f) const
{
  return compare_RR(RR_VAL(f),0.0) == EQ;;
}

bool RingRR::is_unit(const ring_elem f) const
{
  return !RingRR::is_zero(f);
}

bool RingRR::is_equal(const ring_elem f, const ring_elem g) const
{
  return compare_RR(RR_VAL(f),RR_VAL(g)) == EQ;
}
int RingRR::compare(const ring_elem f, const ring_elem g) const
{
  return compare_RR(RR_VAL(f),RR_VAL(g));
}
int RingRR::is_positive(const ring_elem f) const
{
  return compare_RR(RR_VAL(f),0.0) == GT;
}

ring_elem RingRR::copy(const ring_elem f) const
{
  return RingRR::from_double(RR_VAL(f));
}

void RingRR::remove(ring_elem &f) const
{
}

ring_elem RingRR::preferred_associate(ring_elem f) const
{
  if (RingRR::is_positive(f) >= 0) return RingRR::from_double(1.0);
  return RingRR::from_double(-1.0);
}

void RingRR::internal_negate_to(ring_elem &f) const
{
  RRelem a = RRELEM_VAL(f);
  a->val = - a->val;
}

void RingRR::internal_add_to(ring_elem &f, ring_elem &g) const
{
  RRelem a = RRELEM_VAL(f);
  a->val += RR_VAL(g);
  remove(g);
}

void RingRR::internal_subtract_to(ring_elem &f, ring_elem &g) const
{
  RRelem a = RRELEM_VAL(f);
  a->val -= RR_VAL(g);
  remove(g);
}

ring_elem RingRR::negate(const ring_elem f) const
{
  return RingRR::from_double(- RR_VAL(f));
}

ring_elem RingRR::add(const ring_elem f, const ring_elem g) const
{
  return RingRR::from_double(RR_VAL(f) + RR_VAL(g));
}

ring_elem RingRR::subtract(const ring_elem f, const ring_elem g) const
{
  return RingRR::from_double(RR_VAL(f) - RR_VAL(g));
}

ring_elem RingRR::mult(const ring_elem f, const ring_elem g) const
{
  return RingRR::from_double(RR_VAL(f) * RR_VAL(g));
}

ring_elem RingRR::power(const ring_elem f, int n) const
{
  double a = RR_VAL(f);
  double b = 1.0;
  for (int i=0; i<n; i++)
    b *= a;
  return RingRR::from_double(b);
}
ring_elem RingRR::power(const ring_elem f, mpz_t n) const
{
  int n1;
  if (!RingZZ::get_si(n1, n)) 
    { ERROR("exponent too large"); }
  return RingRR::power(f,n1);
}

ring_elem RingRR::invert(const ring_elem f) const
{
  double result = 1/RR_VAL(f);
  return from_double(result);
}

ring_elem RingRR::divide(const ring_elem f, const ring_elem g) const
{
  return RingRR::from_double(RR_VAL(f) / RR_VAL(g));
}

ring_elem RingRR::remainder(const ring_elem f, const ring_elem g) const
{
  // If g == 0.0 then rem = f
  // If g != 0.0 then rem = 0
  double b = RR_VAL(f);
  if (RingRR::is_zero(g))
    return RingRR::from_double(b);
  else
    return RingRR::from_double(0.0);
}

ring_elem RingRR::quotient(const ring_elem f, const ring_elem g) const
{
  // If g == 0.0 then rem = f, return 0.
  // If g != 0.0 then rem = 0, return f/g
  double a = RR_VAL(g);
  double b = RR_VAL(f);
  if (RingRR::is_zero_RR(a))
    return RingRR::from_double(0.0);
  else
    return RingRR::from_double(b/a);
}

ring_elem RingRR::remainderAndQuotient(const ring_elem f, const ring_elem g, 
				  ring_elem &quot) const
{
  // If g == 0.0 then rem = f, quot 0.
  // If g != 0.0 then rem = 0, quot f/g
  double a = RR_VAL(g);
  double b = RR_VAL(f);
  if (RingRR::is_zero_RR(a))
    {
      quot = RingRR::from_double(0.0);
      return RingRR::from_double(b);
    }
  else
    {
      quot = RingRR::from_double(b/a);
      return RingRR::from_double(0.0);
    }
}


ring_elem RingRR::gcd(const ring_elem f, const ring_elem g) const
{
  double a1 = RR_VAL(f);
  double b1 = RR_VAL(g);
  if (RingRR::is_zero_RR(b1) && RingRR::is_zero_RR(a1))
    return RingRR::from_double(0.0);
  else
    return RingRR::from_double(1.0);
}

ring_elem RingRR::gcd_extended(const ring_elem f, const ring_elem g, 
			    ring_elem &u, ring_elem &v) const
{
  double a1 = RR_VAL(f);
  double b1 = RR_VAL(g);
  if (!RingRR::is_zero_RR(b1))
    {
      u = RingRR::from_double(0.0);
      v = RingRR::from_double(1/b1);
      return RingRR::from_double(1.0);
    }
  else if (!RingRR::is_zero_RR(a1))
    {
      u = RingRR::from_double(1/a1);
      v = RingRR::from_double(0.0);
      return RingRR::from_double(1.0);
    }
  else
    {
      u = RingRR::from_double(0.0);
      v = RingRR::from_double(0.0);
      return RingRR::from_double(0.0);
    }
}

void RingRR::syzygy(const ring_elem a, const ring_elem b,
	       ring_elem &x, ring_elem &y) const
{
  double a1 = RR_VAL(a);
  double b1 = RR_VAL(b);
  if (RingRR::is_zero_RR(b1))
    {
      x = RingRR::from_double(0);
      y = RingRR::from_double(1);
    }
  else 
    {
      x = RingRR::from_double(1);
      y = RingRR::from_double(-a1/b1);
    }
}

ring_elem RingRR::eval(const RingMap *map, const ring_elem f, int) const
{
  return map->get_ring()->from_double(RR_VAL(f));
}

// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e "
// End:
