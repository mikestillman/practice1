// Copyright 1995 Michael E. Stillman

#include "ring.hpp"
#include "monoid.hpp"
#include "monideal.hpp"
#include "respoly.hpp"
#include "polyring.hpp"

Ring::Ring(int P, 
	     int n, 
	     int totaln,
	     const Ring *KK,
	     const Monoid *MM,
	     const Monoid *DD)
: type(), P(P), nvars(n), totalvars(totaln), K(KK), M(MM), D(DD),
HRing(NULL)
{
  if (K != NULL) bump_up((Ring *) K);

  if (D->n_vars() > 0)
    HRing = new PolynomialRing(ZZ, D);

  int msize = M->monomial_size();
  vecstash = new stash("vectors",
		       sizeof(vecterm) +
		       (msize-1) * sizeof(int));
  // Set up the resterm stash
  resstash = new stash("respoly", sizeof(resterm *) + sizeof(res_pair *)
		     + sizeof(ring_elem)
		     + sizeof(int) * M->monomial_size());
}

Ring::Ring(const Ring &R)
: type(),
  P(R.P),
  nvars(R.nvars),
  totalvars(R.totalvars),
  K(R.K),
  M(R.M),
  D(R.D),
  HRing(R.HRing),
  vecstash(R.vecstash),
  resstash(R.resstash)
{
  if (K != NULL) bump_up((Ring *) K);
}

Ring::~Ring()
{
  if (K != NULL) bump_down((Ring *) K);
}

void Ring::mult_to(ring_elem &f, const ring_elem g) const
{
  ring_elem h = mult(f,g);
  remove(f);
  f = h;
}

int Ring::coerce_to_int(ring_elem) const
{
  *gError << "cannot coerce given ring element to an integer";
  return 0;
}

MonomialIdeal Ring::get_quotient_monomials() const
{
  return MonomialIdeal(this);
}

ring_elem Ring::random() const
{
  *gError << "random scalar elements for this ring are not implemented";
  return 0;
}
ring_elem Ring::random(int /*homog*/, const int * /*deg*/) const
{
  *gError << "random non-scalar elements for this ring are not implemented";
  return 0;
}
