// (c) 1995 Michael E. Stillman

#ifndef _varpower_hh_
#define _varpower_hh_

#include "intarray.hpp"
#include "array.hpp"

class varpower
{
public:
  varpower() {}
  ~varpower() {}

  static int max_mon_size(int n);

  static int var(int n);
  static int exponent(int n);
  static int pair(int v, int e);

  static void elem_text_out(ostream &o, const int *a);
  static void elem_text_out(ostream &o, const int *a, 
			    const array<char *> &varnames);

  static bool is_one(const int *a);
  static bool is_equal(const int *a, const int *b);
  static bool is_nonneg(const int *a);
  static int topvar(const int *a);

  static void one(intarray &result);
  static void var(int v, int e, intarray &result);
  static int * copy(const int *vp, intarray &result);

  static void to_varpower(const int *a, intarray &result);
  static void from_varpower(const int *a, intarray &result);
  static void to_ntuple(int n, const int *a, intarray &result);
  static void from_ntuple(int n, const int *a, intarray &result);

  static void from_binary(char *&s, int &len, intarray &result);

  static int degree_of(int n, const int *a);
  static int simple_degree(const int *a);

  static int compare(const int *a, const int *b);
    // return EQ, LT, or GT for a == b, a < b, or a > b.
  static void mult(const int *a, const int *b, intarray &result);
  static void divide(const int *a, const int *b, intarray &result);
    // divide a by b.
  static void power(const int *a, int n, intarray &result);
  static int divides(const int *a, const int *b);
    // Is a divisible by b?
  static void monsyz(const int *a, const int *b, 
		     intarray &sa, intarray &sb);
  static void lcm(const int *a, const int *b, intarray &result);
  static void gcd(const int *a, const int *b, intarray &result);
  static void erase(const int *a, const int *b, intarray &result);
    // divide a by b^infinity
  static void radical(const int *a, intarray &result);
  static void elem_bin_out(ostream &o, const int *a);
};

class index_varpower
{
  const int *loc;
  const int *lo;
  const int *hi;
public:
  index_varpower() : loc(0), lo(0), hi(0) {}
  index_varpower(const int *m) : lo(m+1), hi(m+*m-1) { loc = lo; }
  index_varpower(const int *m, int) 
    : lo(m+1), hi(m+*m-1) { loc = hi; }
  index_varpower(const index_varpower &i) : loc(i.loc), lo(i.lo), hi(i.hi) {}

  int valid() { return loc >= lo && loc <= hi; }
  index_varpower &operator++() { loc++; return *this; }
  index_varpower &operator--() { loc--; return *this; }

  int var() { return varpower::var(*loc); }
  int exponent() { return varpower::exponent(*loc); }
};

#endif
