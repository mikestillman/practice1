// (c) 1995 Michael E. Stillman

#include "monomial.hpp"

Monomial::Monomial() : 
  immutable_object(0)
{
  // This routine is private because it leaves the object in
  // an incorrect state... to be filled in by varpower routines.
}

Monomial::Monomial(int v, int e) : 
  immutable_object(0)
{
  varpower::var(v, e, val);
}

Monomial::Monomial(const int *vp) : 
  immutable_object(0)
{
  varpower::copy(vp, val);
}

Monomial::Monomial(M2_arrayint m) :
  immutable_object(0)
{
  varpower::from_arrayint(m, val);
}

Monomial *Monomial::make(int v, int e)
{
  Monomial *result = new Monomial(v,e);
  if (error()) return 0;
  result->set_hash_code();
  return result;
}

Monomial *Monomial::make(M2_arrayint m)
{
  if ((m->len % 2) != 0)
    {
      ERROR("Monomial expected an even number of elements");
      return 0;
    }
  for (unsigned int i=2; i<m->len; i+=2)
    if (m->array[i-2] <= m->array[i])
      {
	ERROR("Monomial expects variables in descending order");
	return 0;
      }
  Monomial *result = new Monomial(m);
  if (error()) return 0;
  result->set_hash_code();
  return result;
}

Monomial *Monomial::make(const int * vp)
{
  assert( error() == 0 );	// did we forget to check for a previous error?
  Monomial *result = new Monomial(vp);
  if (error()) return 0;
  result->set_hash_code();
  return result;
}

void Monomial::set_hash_code()
{
  unsigned long hashval = 0;
  const int *vp = val.raw();
  for (int i=1; i<=*vp; i++)
    {
      hashval += i*(*++vp);
    }
  _hashval = hashval;
}

bool Monomial::is_one() const
{
  return varpower::is_one(ints());
}

bool Monomial::is_equal(const Monomial &b) const
{
  if (this == &b) return true;
  return varpower::is_equal(ints(), b.ints());
}

int Monomial::compare(const Monomial &b) const
{
  return varpower::compare(ints(), b.ints());
}

int Monomial::simple_degree() const
{
  return varpower::simple_degree(ints());
}

bool Monomial::divides(const Monomial &b) const
{
  return varpower::divides(ints(), b.ints());
}

Monomial *Monomial::lcm(const Monomial &b) const
{
  Monomial *result = new Monomial;
  varpower::lcm(ints(), b.ints(), result->val);
  result->set_hash_code();
  return result;
}

Monomial *Monomial::gcd(const Monomial &b) const
{
  Monomial *result = new Monomial;
  varpower::gcd(ints(), b.ints(), result->val);
  result->set_hash_code();
  return result;
}

void Monomial::monsyz(const Monomial &b, Monomial *&sa, Monomial *&sb) const
{
  sa = new Monomial;
  sb = new Monomial;
  varpower::monsyz(ints(), b.ints(), 
		    sa->val, sb->val);
  sa->set_hash_code();
  sb->set_hash_code();
}

Monomial *Monomial::operator*(const Monomial &b) const
{
  Monomial *result = new Monomial;
  varpower::mult(ints(), b.ints(), result->val);
  if (error()) return 0;
  result->set_hash_code();
  return result;
}

Monomial *Monomial::operator/(const Monomial &b) const
{
  Monomial *result = new Monomial;
  varpower::quotient(ints(), b.ints(), result->val);
  result->set_hash_code();
  return result;
}

Monomial *Monomial::erase(const Monomial &b) const
{
  Monomial *result = new Monomial;
  varpower::erase(ints(), b.ints(), result->val);
  result->set_hash_code();
  return result;
}

Monomial *Monomial::power(int n) const
{
  Monomial *result = new Monomial;
  varpower::power(ints(), n, result->val);
  if (error()) return 0;
  result->set_hash_code();
  return result;
}

Monomial *Monomial::radical() const
{
  Monomial *result = new Monomial;
  varpower::radical(ints(), result->val);
  result->set_hash_code();
  return result;
}

// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e"
// End:
