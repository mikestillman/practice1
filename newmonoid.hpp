// Copyright 1996.  Michael E. Stillman

#ifndef _newmonoid_hh_
#define _newmonoid_hh_

#include "monorder.hpp"
#include "object.hpp"

class Monoid : public type
{
  // Class of commutative monoids

  int            _nvars;
  array <char *> _varnames;
  new_mon_order *_mo;		// A list of the parts of the monomial order.

  Monoid *       _D;		// Degree monoid
  int *          _primary_degrees;
  int **         _multi_degrees; // _multi_degrees[i] is an 'int *', an element of 'D'.

  int            _nskew;
  int *          _skew_vars;	// 0..nvars-1: skew_vars[v] = 1 iff v has odd (skew)degree
  int *          _skew_list;	// 0..n_skew-1: skew_list[i] = (skew var in range 0..nvars-1)

  bool           _isnormal;	// true if this is a subsemigroup of N^nvars.

  int            _nwords;	// Number of sizeof(int) words per monomial (without the component).

  // Local allocated variables, to save on allocations...
  int *_skew_mvars;
  int *_skew_nvars;		// To save ALOT of allocations...

  stash *_monom_stash;
  int *_EXP1, *_EXP2, *_EXP3;	// allocated ntuples.
				// A local routine may use these ONLY if
				// they call no other monoid routine, except
				// to/from expvector.
  int *_MONlocal;		// To be used ONLY by to/from expvector.

  void set_degrees();
  static void set_names(int nvars, const char *s, int slength, array<char *> &varnames);

  monoid_info();
  monoid_info(const mon_order *mmo, 
	      const char *s, 
	      int len, 
	      Monoid *deg_monoid,
	      const intarray &degs,
	      bool isgrp,
	      bool isskew);

  int top_bits;			// Only needed for 'divides'.
  int bit_mask;
  int mon_bound;		// Entries this size or larger may not be 
				// used.  

public:
  Monoid(monoid_info *mo, int nbits);

  bool is_group() const { return moninfo->isgroup; }

  void from_varpower(const int *vp, int *result) const;
  void to_varpower(const int *m, intarray &result_vp) const;

  void from_expvector(const int *exp, int *result) const;
  void to_expvector(const int *m, int *result_exp) const;

  void unpack(const int *m, int *result) const;
  void pack(const int *exp, int *result) const;

  int in_subring(int n, const int *m) const;
  int compare(int nslots, const int *m, const int *n) const;

  int *make_new(const int *d) const;
  int *make_one() const;
  void remove(int *d) const;

  int is_non_negative(const int *m) const;
  int is_one(const int *m) const;
  void one(int *result) const;
  void copy(const int *m, int *result) const;

  int n_vars()        const { return nvars; }
  int max_degree()    const { return mon_bound; }
  int monomial_size() const { return nwords; }

  void mult(const int *m, const int *n, int *result) const;
  void power(const int *m, int n, int *result) const;
  int compare(const int *m, const int *n) const;
  bool divides(const int *m, const int *n) const;
  void divide(const int *m, const int *n, int *result) const;
  void lcm(const int *m, const int *n, int *result) const;
  void gcd(const int *m, const int *n, int *result) const;
  void monsyz(const int *m, const int *n, int *sm, int *sn) const;

  bool is_skew() const;
  int is_skew_var(int v) const;
  int skew_mult_sign(const int *m, const int *n) const;
  int skew_mult(const int *m, const int *n, int *result) const;
  int skew_divide(const int *m, const int *n, int *result) const;
      // If the result is s (1,or -1), then m = s * n * result
  int exp_skew_vars(const int *exp, int *result) const;
      // The number s of skew variables in 'exp' is returned, and their
      // indices are placed in result[0], ..., result[s-1].
  int skew_vars(const int *m, int *result) const;
      // The number s of skew variables in 'm' is returned, and their
      // indices are placed in result[0], ..., result[s-1].
  bool skew_is_zero(const int *exp) const;
      // Return whether any skew variable in the exponent vector has exponent >= 2

  void elem_text_out(buffer &o, const int *m) const;
  void elem_bin_out(buffer &o, const int *m) const;

  int primary_value(const int *m) const;
  void multi_degree(const int *m, int *result) const;
  int primary_degree(const int *m) const;
  int degree_weights(const int *m, const int *wts) const;
  void degree_of_varpower(const int *vp, int *result) const;

  const Monoid *degree_monoid() const { return moninfo->degree_monoid; }
  const int *degree_of_var(int v) const { return moninfo->degree_of_var[v]; }
  int primary_degree_of_var(int v) const { return moninfo->primary_degree_of_var[v]; }

  // Infrastructure here
  void text_out(buffer &o) const;

  int          length_of() const      { return n_vars(); }
  const Monoid * cast_to_Monoid() const { return this; }
  Monoid *       cast_to_Monoid()       { return this; }
  object_types type_of()   const    { return TY_MONOID; }
  const char * type_name() const    { return "Monoid"; }
};

extern Monoid *trivial_monoid;		// set in x_monoid.cpp

// WARNING!! 'divide' assumes that division is possible
inline void Monoid::divide(const int *m, const int *n, int *result) const
    { for (int i=0; i<nwords; i++) *result++ = *m++ - *n++; }


inline int Monoid::compare(const int *m, const int *n) const
{
  for (int i=0; i<nwords; i++, m++, n++)
    {
      if (*m > *n) return GT;
      if (*m < *n) return LT;
    }
  return EQ;
}

#endif
