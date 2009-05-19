// Copyright 1996 Michael E. Stillman

#include <ctype.h>
#include "text-io.hpp"
#include "monoid.hpp"
#include "varpower.hpp"
#include "ntuple.hpp"
#include "../d/M2mem.h"
#include "polyring.hpp"
#include "exceptions.hpp"
#include "overflow.hpp"

Monoid *Monoid::trivial_monoid = 0;


// ONLY to be called by PolyRing::get_trivial_poly_ring()
void Monoid::set_trivial_monoid_degree_ring(const PolynomialRing *DR)
{
  Monoid *M = get_trivial_monoid();
  M->degree_ring_ = DR;
  M->degree_monoid_ = M;
}

Monoid::Monoid()
  :  nvars_(0),
     varnames_(0),
     degvals_(0),
     heftvals_(0),
     degree_ring_(0), // will be set later
     degree_monoid_(0), // will be set later
     mo_(0),
     monorder_(0),
     overflow(0),
     monomial_size_(0),
     monomial_bound_(0),
     n_invertible_vars_(0),
     n_before_component_(0),
     n_after_component_(0),
     component_up_(true),
     local_vars(0),
     EXP1_(0),
     EXP2_(0),
     EXP3_(0)
{
}

Monoid::~Monoid()
{
  deletearray(EXP1_);
  deletearray(EXP2_);
  deletearray(EXP3_);
}

Monoid *Monoid::get_trivial_monoid()
{
  if (trivial_monoid == 0)
    trivial_monoid = new Monoid;

  return trivial_monoid;
}

Monoid *Monoid::create(MonomialOrdering *mo,
		       M2_stringarray names,
		       const PolynomialRing *deg_ring,
		       M2_arrayint degs,
		       M2_arrayint hefts)
{
  unsigned int nvars = rawNumberOfVariables(mo);;
  unsigned int eachdeg = deg_ring->n_vars();
  if (degs->len != nvars * eachdeg)
    {
      ERROR("degree list should be of length %d", nvars * eachdeg);
      return 0;
    }
  if (nvars != names->len)
    {
      ERROR("expected %d variable names", nvars);
      return 0;
    }

  return new Monoid(mo,names,deg_ring,degs,hefts);
}

Monoid::Monoid(MonomialOrdering *mo,
	       M2_stringarray names,
	       const PolynomialRing *deg_ring,
	       M2_arrayint degs,
	       M2_arrayint hefts)
  :  nvars_(rawNumberOfVariables(mo)),
     varnames_(names),
     degvals_(degs),
     heftvals_(hefts),
     degree_ring_(deg_ring),
     degree_monoid_(deg_ring->getMonoid()),
     mo_(mo)
{

  monorder_ = monomialOrderMake(mo);

  monomial_size_ = monorder_->nslots;
  n_before_component_ = monorder_->nslots_before_component;
  n_after_component_ = monomial_size_ - n_before_component_;
  component_up_ = monorder_->component_up;

  // Set nslots_
  int total = 0;
  for (int i=0; i<monorder_->nblocks; i++)
    {
      total += monorder_->blocks[i].nslots;
      nslots_.push_back(total);
    }

  // Set first_weight_value_
  bool get_out = false;
  first_weights_slot_ = -1;
  for (int i=0; i<monorder_->nblocks && !get_out; i++)
    {
      switch (monorder_->blocks[i].typ) {
      case MO_LEX:
      case MO_LEX2:
      case MO_LEX4:
      case MO_NC_LEX: 
	get_out = true;
	break;
      case MO_REVLEX:
      case MO_LAURENT:
      case MO_LAURENT_REVLEX:
      case MO_GREVLEX:
      case MO_GREVLEX2:
      case MO_GREVLEX4:
      case MO_GREVLEX_WTS:
      case MO_GREVLEX2_WTS:
      case MO_GREVLEX4_WTS:
      case MO_WEIGHTS:
	first_weights_slot_ = 0;
      case MO_POSITION_UP: continue;
      case MO_POSITION_DOWN: continue;
      default:
	INTERNAL_ERROR("monomial order block type not handled");
      }
    }
  EXP1_ = newarray_atomic(int,nvars_);
  EXP2_ = newarray_atomic(int,nvars_);
  EXP3_ = newarray_atomic(int,nvars_);

  n_invertible_vars_ = rawNumberOfInvertibleVariables(mo_);

  set_degrees();
  set_overflow_flags();

  local_vars = rawNonTermOrderVariables(mo);

// Debugging only:
//  fprintf(stderr, "%d variables < 1\n", local_vars->len);
//  if (local_vars->len > 0)
//    {
//      fprintf(stderr, "they are: ");
//      for (int i=0; i<local_vars->len; i++)
//	fprintf(stderr, "%d ", local_vars->array[i]);
//      fprintf(stderr, "\n");
//    }
    }

void Monoid::set_degrees()
{
  if (degree_monoid_ == NULL)
    {
      degree_of_var_.append(static_cast<const_monomial>(NULL));
      return;
    }

  // Set 'degree_of_var
  int degvars = degree_monoid_->n_vars();
  int *t = degvals_->array;

  heft_degree_of_var_ = makearrayint(nvars_);
  if (heftvals_->len != degvars) {
    ERROR("internal error: heftvals_->len == %d != degvars == %d", heftvals_->len, degvars);
    return;
  }
  if (degvars > 0)
    for (int i=0; i<nvars_; i++)
      {
	monomial m = degree_monoid_->make_one();
	degree_monoid_->from_expvector(t, m);
	degree_of_var_.append(m);
	heft_degree_of_var_->array[i] = ntuple::weight(degvars,t,heftvals_);
	t += degvars;
      }
  else
    {
      for (int i=0; i<nvars_; i++)
	heft_degree_of_var_->array[i] = 1;
    }
  degree_of_var_.append(degree_monoid_->make_one());
}

void Monoid::set_overflow_flags()
{
  overflow = newarray_atomic(enum overflow_type, monomial_size_);
  enum overflow_type flag;
  int i=0, k=0;
  for (; i<monorder_->nblocks; i++)
    {
      mo_block *b = &monorder_->blocks[i];
      switch (monorder_->blocks[i].typ) {
      case MO_REVLEX:
      case MO_WEIGHTS:
      case MO_LAURENT:
      case MO_LAURENT_REVLEX:
      case MO_NC_LEX:
	   flag = OVER;
	   goto fillin;
      case MO_POSITION_UP:
      case MO_POSITION_DOWN:
	   ERROR("internal error - MO_POSITION_DOWN or MO_POSITION_UP encountered");
	   assert(0);
	   break;
      case MO_LEX:
      case MO_GREVLEX:
      case MO_GREVLEX_WTS:
	   flag = OVER1;
	   goto fillin;
      case MO_LEX2:
      case MO_GREVLEX2:
      case MO_GREVLEX2_WTS:
	   flag = OVER2;
	   goto fillin;
      case MO_LEX4:
      case MO_GREVLEX4:
      case MO_GREVLEX4_WTS:
	   flag = OVER4;
	   goto fillin;
      fillin:
	   assert(b->first_slot == k);
	   for (int p=b->nslots; p>0; p--) {
		assert(k < monomial_size_);
		overflow[k++] = flag;
	   }
	   break;
      default:
	   ERROR("internal error - missing case");
	   assert(0);
	   break;
      }
    }
  assert(k == monomial_size_);
}

bool Monoid::primary_degrees_of_vars_positive() const
{
  for (int i=0; i<nvars_; i++)
    if (primary_degree_of_var(i) <= 0)
      return false;
  return true;
}

#if 0
//
// dan : commenting this out because it seems to be unused
//
//Monoid *Monoid::tensor_product(const Monoid *M1, const Monoid *M2)
//{
//  int i,v;
//  MonomialOrdering_array M12 = GETMEM(MonomialOrdering_array, sizeofarray(M12,2));
//  M12->len = 2;
//  M12->array[0] = M1->mo_;
//  M12->array[1] = M2->mo_;
//  MonomialOrdering *M = rawProductMonomialOrdering(M12);
//
//  int n1 = M1->n_vars();
//  int n2 = M2->n_vars();
//  int n = n1+n2;
//  M2_stringarray names = GETMEM(M2_stringarray, sizeofarray(names, n));
//  names->len = n;
//  for (i=0; i<n1; i++)
//    names->array[i] = M1->varnames_->array[i];
//
//  for (i=0; i<n2; i++)
//    names->array[n1+i] = M2->varnames_->array[i];
//
//  const PolynomialRing *DR = M1->get_degree_ring();
//  int ndegs = DR->n_vars();
//
//  M2_arrayint degs = makearrayint(ndegs*n);
//
//  int next = 0;
//  for (v=0; v<n1; v++)
//    for (i=0; i<ndegs; i++) {
//      degs->array[next] = M1->degvals_->array[next];
//      next++;
//    }
//
//  for (v=0; v<n2; v++)
//    for (i=0; i<ndegs; i++)
//      degs->array[next++] = 0;
//
//  return Monoid::create(M,names,DR,degs, /* some hefts have to go here but what could they be??? */);
//}
//
#endif


void Monoid::text_out(buffer &o) const
{
  int i;
  o << "[";
  for (i=0; i<nvars_-1; i++)
    o << varnames_->array[i] << ",";
  if (nvars_ > 0)
    o << varnames_->array[nvars_-1];

  int ndegrees = degree_monoid()->n_vars();
  o << "," << newline << "  DegreeLength => " << ndegrees;

  o << "," << newline << "  Degrees => {";
  for (i=0; i<nvars_; i++)
    {
      if (i != 0) o << ", ";
      if (ndegrees != 1) o << '{';
      for (int j=0; j<ndegrees; j++)
	{
	  if (j != 0) o << ", ";
	  o << degvals_->array[i*ndegrees+j];
	}
      if (ndegrees != 1) o << '}';	  
    }
  o << "}";

  if (heftvals_ != NULL) {
    o << "," << newline << "  Heft => {";
    for (i=0; i<heftvals_->len; i++)
      {
	if (i != 0) o << ", ";
	o << heftvals_->array[i];
      }
    o << "}";
  }

  if (mo_ != 0)
    {
      o << "," << newline << "  ";
      o << IM2_MonomialOrdering_to_string(mo_);
    }

  o << newline << "  ]";
}

void Monoid::from_expvector(const_exponents exp, monomial result) const
{
  monomialOrderEncodeFromActualExponents(monorder_, exp, result);
}

M2_arrayint Monoid::to_arrayint(const_monomial monom) const
{
  M2_arrayint result = makearrayint(n_vars());
  to_expvector(monom, result->array);
  return result;
}

void Monoid::to_expvector(const_monomial m, exponents result_exp) const
{
  monomialOrderDecodeToActualExponents(monorder_, m, result_exp);
}

void Monoid::mult(const_monomial m, const_monomial n, monomial result) const
{
  static char err[] = "monomial overflow";
  overflow_type *t = overflow;
  for (int i = monomial_size_; i != 0; i--)
      switch (*t++) {
      case OVER:   *result++ = safe::    add  (*m++,*n++,err); break;
      case OVER1:  *result++ = safe::pos_add  (*m++,*n++,err); break;
      case OVER2:  *result++ = safe::pos_add_2(*m++,*n++,err); break;
      case OVER4:  *result++ = safe::pos_add_4(*m++,*n++,err); break;
      default: throw(exc::internal_error("missing case"));
      }
}

#if 0
// mult is called:
//   respoly2.cpp
//   res2.cpp
//   res.cpp
//   respoly.cpp
//   schorder.cpp
//   freemod.cpp
//   gbring.cpp
//   matrix.cpp
//   polyring.cpp
//   ringmap.cpp
//   skewpoly.cpp
//   weylalg.cpp
//   mat-kbasis.cpp
//   freemod2.cpp
#endif
int Monoid::num_parts() const
{
  return monorder_->nblocks;
}

int Monoid::n_slots(int nparts) const
{
  if (nparts == 0) return 0;
  nparts--;
  if (nparts < 0) return monomial_size();
  if (nparts >= num_parts()) nparts = num_parts()-1;
  return nslots_[nparts];
}

bool Monoid::in_subring(int nslots, const_monomial m) const
{
  for (int i=0; i<nslots; i++)
    if (*m++) return false;
  return true;
}

int Monoid::compare(const_monomial m, int mcomp, const_monomial n, int ncomp) const
{
  int i = n_before_component_;
  while (1)
    {
      if (i == 0) break;
      if (*m > *n) return GT;
      if (*m < *n) return LT;
      m++, n++;
      --i;
    }
  if (component_up_)
    {
      if (mcomp < ncomp) return LT;
      if (mcomp > ncomp) return GT;
    }
  else
    {
      if (mcomp < ncomp) return GT;
      if (mcomp > ncomp) return LT;
    }
  i= n_after_component_;
  while (1)
    {
      if (i==0) break;
      if (*m > *n) return GT;
      if (*m < *n) return LT;
      m++, n++;
      --i;
    }
  return EQ;
}

monomial Monoid::make_new(const_monomial d) const
{
  if (nvars_ == 0) return NULL;
  monomial result = newarray_atomic(int,monomial_size());
  copy(d, result);
  return result;
}
monomial Monoid::make_one() const
{
  if (nvars_ == 0) return NULL;
  monomial result = newarray_atomic(int,monomial_size());
  one(result);
  return result;
}
void Monoid::remove(monomial d) const
{
#if 0
//   deletearray(d);
#endif
}

void Monoid::one(monomial result) const
{
  for (int i=0; i<monomial_size(); i++) 
    *result++ = 0;
}

void Monoid::copy(const_monomial m, monomial result) const
{
  memcpy(result, m, monomial_size()*sizeof(int));
}

bool Monoid::divides(const_monomial m, const_monomial n) const
// Does m divide n?
{
  if (nvars_ == 0) return true;
  // can we speed this up by not unpacking ??
  to_expvector(m, EXP1_);
  to_expvector(n, EXP2_);
  return ntuple::divides(nvars_, EXP1_, EXP2_);
}

#if 0
// void Monoid::divide(const_monomial m, const_monomial n, monomial result) const
// {
//   if (nvars_ == 0) return;
//   to_expvector(m, EXP1_);
//   to_expvector(n, EXP2_);
//   ntuple::divide(nvars_, EXP1_, EXP2_, EXP1_);
//   from_expvector(EXP1_, result);
// }
#endif

void Monoid::power(const_monomial m, int n, monomial result) const
{
  if (nvars_ == 0) return;
  to_expvector(m, EXP1_);
  ntuple::power(nvars_, EXP1_, n, EXP1_);
  from_expvector(EXP1_, result);
}

void Monoid::monsyz(const_monomial m, const_monomial n, monomial sm, monomial sn) const
{
  if (nvars_ == 0) return;
  to_expvector(m, EXP1_);
  to_expvector(n, EXP2_);
  for (int i=0; i<nvars_; i++)
      if (EXP1_[i] > EXP2_[i])
	{
	  EXP2_[i] = EXP1_[i] - EXP2_[i];
	  EXP1_[i] = 0;
	}
      else
	{
	  EXP1_[i] = EXP2_[i] - EXP1_[i];
	  EXP2_[i] = 0;
	}
  from_expvector(EXP1_, sm);
  from_expvector(EXP2_, sn);
}

void Monoid::gcd(const_monomial m, const_monomial n, monomial p) const
{
  if (nvars_ == 0) return;
  to_expvector(m, EXP1_);
  to_expvector(n, EXP2_);
  ntuple::gcd(nvars_, EXP1_, EXP2_, EXP1_);
  from_expvector(EXP1_, p);
}

void Monoid::lcm(const_monomial m, const_monomial n, monomial p) const
{
  if (nvars_ == 0) return;
  to_expvector(m, EXP1_);
  to_expvector(n, EXP2_);
  ntuple::lcm(nvars_, EXP1_, EXP2_, EXP1_);
  from_expvector(EXP1_, p);
}

void Monoid::elem_text_out(buffer &o, const_monomial m) const
{
  to_expvector(m, EXP1_);
  ntuple::elem_text_out(o, nvars_, EXP1_, varnames_);
}

void Monoid::multi_degree(const_monomial m, monomial result) const
{
  if (degree_monoid()->n_vars() == 0) return;
  degree_monoid()->one(result);
  if (nvars_ == 0) return;
  monomial mon1 = degree_monoid()->make_one();

  to_expvector(m, EXP1_);

  for (int i=0; i<nvars_; i++)
    if (EXP1_[i] > 0)
      {
	degree_monoid()->power(degree_of_var(i), EXP1_[i], mon1);
	degree_monoid()->mult(result, mon1, result);
      }
  degree_monoid()->remove(mon1);
}

void Monoid::degree_of_varpower(const_varpower vp, monomial result) const
{
  if (nvars_ == 0) return;
  if (degree_monoid()->n_vars() == 0) return;

  degree_monoid()->one(result);
  monomial mon1 = degree_monoid()->make_one();

  for (index_varpower j = vp; j.valid(); ++j)
      {
	int v = j.var();
	int e = j.exponent();
	degree_monoid()->power(degree_of_var(v), e, mon1);
	degree_monoid()->mult(result, mon1, result);
      }
  degree_monoid()->remove(mon1);
}

int Monoid::primary_degree(const_monomial m) const
{
  return degree_weights(m, primary_degree_of_vars());
}

int Monoid::degree_weights(const_monomial m, M2_arrayint wts) const
{
  if (nvars_ == 0) return 0;
  to_expvector(m, EXP1_);
  int sz = (wts->len < nvars_ ? wts->len : nvars_);
  return ntuple::weight(sz, EXP1_, wts);
}

bool Monoid::is_one(const_monomial m) const
{
  for (int i=0; i<monomial_size(); i++)
    if (*m++ != 0) return false;
  return true;
}

bool Monoid::is_invertible(const_monomial m) const
// is every variable that occurs 
// in 'm' allowed to be negative?
{
  if (n_invertible_vars_ == 0)
    {
      // Only the trivial monomial is invertible in this case
      return is_one(m);
    }
  to_expvector(m, EXP1_);
  for (int i=0; i<nvars_; i++)
    if (!monorder_->is_laurent[i] && EXP1_[i] > 0)
      return false;
  return true;
}

void Monoid::from_varpower(const_varpower vp, monomial result) const
{
  varpower::to_ntuple(nvars_, vp, EXP1_);
  from_expvector(EXP1_, result);
}

void Monoid::to_varpower(const_monomial m, intarray &result_vp) const
{
  to_expvector(m, EXP1_);
  varpower::from_ntuple(nvars_, EXP1_, result_vp);
}


// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e "
// End:
