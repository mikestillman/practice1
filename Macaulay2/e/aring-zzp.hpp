// Copyright 2011 Michael E. Stillman

#ifndef _aring_zzp_hpp_
#define _aring_zzp_hpp_

#include "aring.hpp"
#include "buffer.hpp"
#include "ringelem.hpp"

class Z_mod;
class RingMap;

namespace M2 {
/**
\ingroup rings
*/
  class ARingZZp : public RingInterface
  {
    // Integers mod p, implemented as
    // exponents of a primitive element a
    // Representation:
    // 0  means 0
    // 1 <= n <= p-1  means a^n (mod p)

    // 0 represents 0
    // p-1 represents 1
    // 1..p-2 represent numbers in range 2..p-1

  public:
    static const RingID ringID = ring_ZZp;
    typedef Z_mod ring_type;
    typedef int ElementType;
    typedef int elem;

    ARingZZp(size_t prime);

    MutableMatrix* makeMutableMatrix(const Ring* R, 
                                     size_t nrows, 
                                     size_t ncols, 
                                     bool dense) const;

    // ring informational
    size_t characteristic() const { return charac; }

    static int findPrimitiveRoot(int P);

    void text_out(buffer &o) const;

    /////////////////////////////////////////////////////////
    // Routines to help in switch from coeffrings to aring //
    // these will be renamed or go away (hopefully) /////////
    /////////////////////////////////////////////////////////
    void init_set(elem &result, elem a) const { result = a; }
    void set(elem &result, elem a) const { result = a; }

    /////////////////////////////////
    // ElementType informational ////
    /////////////////////////////////

    bool is_unit(ElementType f) const { return f != 0; }
    bool is_zero(ElementType f) const { return f == 0; }
    bool is_equal(ElementType f, ElementType g) const { return f == g; }

    int compare_elems(ElementType f, ElementType g) const {
      int a = exp_table[f];
      int b = exp_table[g];
      if (a < b) return -1;
      if (a > b) return 1;
      return 0;
    }

    ////////////////////////////
    // to/from ringelem ////////
    ////////////////////////////
    // These simply repackage the element as either a ringelem or an 'ElementType'.
    // No reinitialization is done.
    // Do not take the same element and store it as two different ring_elem's!!
    void to_ring_elem(ring_elem &result, const ElementType &a) const
    {
      if (a == 0) 
	result.int_val = p1;
      else if (a == p1) 
	result.int_val = 0;
      else
	result.int_val = a;
    }

    void from_ring_elem(ElementType &result, const ring_elem &a) const
    {
      if (a.int_val == 0)
	result = p1;
      else if (a.int_val == p1)
	result = 0;
      else
	result = a.int_val;
    }

    // 'get' functions

    int get_int(elem f) const { return exp_table[f]; }

    int get_repr(elem f) const { return f; }

    // 'init', 'init_set' functions

    void init(elem &result) const { result = 0; }

    void clear(elem &result) const { /* nothing */ }

    void set_zero(elem &result) const { result = 0; }

    void copy(elem &result, elem a) const { result = a; }

    void set_from_int(elem &result, int a) const {
      a = a % p;
      if (a < 0) a += p;
      result = log_table[a];
    }

    void set_var(elem &result, int v) const { result = 1; }

    void set_from_mpz(elem &result, mpz_ptr a) const {
      int b = static_cast<int>(mpz_fdiv_ui(a, p));
      result = log_table[b];
    }

    void set_from_mpq(elem &result, mpq_ptr a) const {
      int n, d;
      set_from_mpz(n, mpq_numref(a));
      set_from_mpz(d, mpq_denref(a));
      divide(result,n,d);
    }

    // arithmetic
    void negate(elem &result, elem a) const
    {
      if (a != 0)
        result = p - a;
      else
        result = 0;
    }

    void invert(elem &result, elem a) const
      // we silently assume that a != 0.  If it is, result is set to a^0, i.e. 1
    {
      result = p1 - a;
      if (result == 0) result = p1;
    }

    void add(elem &result, elem a, elem b) const
    {
      int e1 = exp_table[a];
      int e2 = exp_table[b];
      int n = e1+e2;
      if (n >= p) n -= p;
      result = log_table[n];
    }

    void subtract(elem &result, elem a, elem b) const
    {
      int e1 = exp_table[a];
      int e2 = exp_table[b];
      int n = e1-e2;
      if (n < 0) n += p;
      result = log_table[n];
    }

    void subtract_multiple(elem &result, elem a, elem b) const
    {
      // we assume: a, b are NONZERO!!
      // result -= a*b
      int ab = a+b;
      if (ab > p1) ab -= p1;
      int n = exp_table[result] - exp_table[ab];
      if (n < 0) n += p;
      result = log_table[n];
    }

    void mult(elem &result, elem a, elem b) const
    {
      if (a != 0 && b != 0)
        {
          int c = a+b;
          if (c > p1) c -= p1;
          result = c;
        }
      else
        result = 0;
    }

    void divide(elem &result, elem a, elem b) const
    {
      ASSERT(b != 0);
      if (a != 0 && b != 0)
        {
          int c = a-b;
          if (c <= 0) c += p1;
          result = c;
        }
      else
        result = 0;
    }

    void power(elem &result, elem a, int n) const
    {
      if (a != 0)
        {
          result = (a*n) % p1;
          if (result <= 0) result += p1;
        }
      else
        result = 0;
    }

    void power_mpz(elem &result, elem a, mpz_ptr n) const
    {
      int n1 = static_cast<int>(mpz_fdiv_ui(n, p1));
      power(result,a,n1);
    }

    void swap(ElementType &a, ElementType &b) const
    {
      ElementType tmp = a;
      a = b;
      b = tmp;
    }

    void elem_text_out(buffer &o,
                       ElementType a,
                       bool p_one,
                       bool p_plus,
                       bool p_parens) const;

    void syzygy(ElementType a, ElementType b,
                ElementType &x, ElementType &y) const
    // returns x,y s.y. x*a + y*b == 0.
    // if possible, x is set to 1.
    // no need to consider the case a==0 or b==0.
    {
      x = p1;
      divide(y,a,b);
      negate(y,y);
    }

    void random(ElementType &result) const
    {
      result = rawRandomInt((int32_t)p);
    }

    void eval(const RingMap *map, const elem f, int first_var, ring_elem &result) const;
  private:
    void initialize_tables();
    
    size_t charac;
    int p; // charac == p ??
    int p1; // p-1
    int minus_one;
    int prim_root; // element we will use for our primitive root
    int *log_table; // 0..p-1
    int *exp_table; // 0..p-1
    
  };

};

#endif

// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e  "
// indent-tabs-mode: nil
// End:
