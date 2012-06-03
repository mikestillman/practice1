// Copyright 2010-2012 Michael E. Stillman.
#ifndef _aring_tower_hpp_
#define _aring_tower_hpp_

#include <vector>
#include <string>
#include "aring-ffpack.hpp"

#include "style.hpp"
#include "aring.hpp"
#include "ringelem.hpp"

class RingMap;

namespace M2 {
  /**
   * \ingroup polynomialrings
   */
  typedef struct poly_struct * poly;

  struct poly_struct : public our_new_delete {
    unsigned long deg;
    unsigned long len;
    union {
      long* ints;  // array of integers.  at level == 0
      poly* polys; // array of more ptrs to poly structs, at level > 0
    };
  };

  typedef int * exponents;
  
  class DRing;

  //TODO: make this a template type, with the base e.g. ZZ/p, ZZ, QQ, etc.
  /**
  \ingroup rings
  */
  class ARingTower : public RingInterface
  {
    friend class ARingTowerEvaluator;
    
  public:
    typedef ARingZZpFFPACK BaseRingType;
    static const RingID ringID = ring_tower_ZZp;
    typedef poly ElementType;
    typedef ElementType elem;

    //////////////////////////////
    // Creation and destruction //
    //////////////////////////////

    ARingTower(const BaseRingType &baseRing, 
               const std::vector<std::string> &names,
               const std::vector<ElementType> &extensions);

    virtual ~ARingTower();

    //TODO: the interface for these three need to change    
    static const ARingTower * create(const BaseRingType &baseRing, const std::vector<std::string> &names);
    static const ARingTower * create(const ARingTower &R, const std::vector<std::string> &new_names);
    static const ARingTower * create(const ARingTower &R, const std::vector<ElementType> &extensions);


    int n_vars() const { return mNumVars; }
    const ARingZZpFFPACK& baseRing() const { return mBaseRing; }
    const std::vector<std::string>& varNames() const { return mVarNames; }
    unsigned long characteristic() const { return mBaseRing.characteristic(); }

    ///////////////////////////////////
    // The following are he routines required to be of type "RingInterface"
    ///////////////////////////////////

    void text_out(buffer &o) const;

    /////////////////////////////////////////////////////////
    // Routines to help in switch from coeffrings to aring //
    // these will be renamed or go away (hopefully) /////////
    /////////////////////////////////////////////////////////
    void init_set(elem &result, elem a) const {}
    void set(elem &result, elem a) const {}

    /////////////////////////////////
    // ElementType informational ////
    /////////////////////////////////

    bool is_unit(ElementType f) const { return false; }
    bool is_zero(ElementType f) const { return false; }
    bool is_equal(ElementType f, ElementType g) const { return false; }

    int compare_elems(ElementType f, ElementType g) const {
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
    }

    void from_ring_elem(ElementType &result, const ring_elem &a) const
    {
    }

    // 'get' functions

    //needed?    int get_int(elem f) const { return 0; }

    //needed?    int get_repr(elem f) const { return f; }

    // 'init', 'init_set' functions

    void init(elem &result) const { }

    void clear(elem &result) const { }

    void set_zero(elem &result) const { }

    void copy(elem &result, elem a) const { }

    void set_from_int(elem &result, int a) const {
    }

    void set_var(elem &result, int v) const { }
    
    void set_from_mpz(elem &result, mpz_ptr a) const { }
    
    void set_from_mpq(elem &result, mpq_ptr a) const { }
    
    // arithmetic
    void negate(elem &result, elem a) const { }

    // we silently assume that a != 0.  If it is, result is set to a^0, i.e. 1    
    void invert(elem &result, elem a) const  { }

    void add(elem &result, elem a, elem b) const { }

    void subtract(elem &result, elem a, elem b) const { }

    void subtract_multiple(elem &result, elem a, elem b) const { }

    void mult(elem &result, elem a, elem b) const { }

    void divide(elem &result, elem a, elem b) const { }

    void power(elem &result, elem a, int n) const { }

    void power_mpz(elem &result, elem a, mpz_ptr n) const { }

    void swap(ElementType &a, ElementType &b) const { }

    void elem_text_out(buffer &o,
                       ElementType a,
                       bool p_one,
                       bool p_plus,
                       bool p_parens) const { }

    // returns x,y s.y. x*a + y*b == 0.
    // if possible, x is set to 1.
    // no need to consider the case a==0 or b==0.
    void syzygy(ElementType a, ElementType b,
                ElementType &x, ElementType &y) const { }

    void random(ElementType &result) const { }

    void eval(const RingMap *map, const elem f, int first_var, ring_elem &result) const { }

  private:
    void extensions_text_out(buffer &o) const;

    void elem_text_out(buffer &o,
                       int level,
                       const ElementType f,
                       bool p_one,
                       bool p_plus,
                       bool p_parens) const;

  private:
    bool is_one(int level, poly f) const;

#if 0
  public:
    bool is_unit(ElementType f) const;
    bool is_zero(ElementType f) const { return f == 0; }
    bool is_equal(ElementType f, ElementType g) const { return mRing.is_equal(mStartLevel, f, g); }
    int compare_elems(ElementType f, ElementType g) const { return mRing.compare(mStartLevel, f, g); }

    //TODO: NO! this should be a copy!!
    void init_set(ElementType &result, ElementType a) const { result = a; } 

    //TODO: NO! this should be a copy!!
    void set(ElementType &result, ElementType a) const { mRing.remove(mStartLevel, result); result = a; }

    //TODO: should this remove previous value??
    void set_zero(ElementType &result) const { result = 0; }
    
    void set_var(ElementType &result, int n) {
      // n from 0..nvars-1, sets result to 0 f n is out of range
      result = mRing.var(mStartLevel,n);
    }
    
    void set_from_int(ElementType &result, long r) {
      r = r % mCharacteristic;
      if (r < 0) r += P;
      result = mRing.from_int(mStartLevel, r);
    }
    
    void set_from_int(ElementType &result, mpz_ptr r);
    
    bool set_from_rational(ElementType &result, mpq_ptr r);
    
    void set_random(ElementType &result) { result = mRing.random(mStartLevel); }
    
    bool invert(ElementType &result, ElementType a) const
    // returns true if invertible.  Returns false if not, and then result is set to 0.
    {
      result = mRing.invert(mStartLevel, a);
      return result != 0;
    }
    
    void add(ElementType &result, ElementType a, ElementType b) const
    {
      if (a == 0) result = b;
      else if (b == 0) result = a;
      else
        {
          ElementType a1 = mRing.copy(mStartLevel, a);
          mRing.add_in_place(mStartLevel, a1, b);
          result = a1;
        }
    }
    
    void subtract(ElementType &result, ElementType a, ElementType b) const
    {
      ElementType a1 = mRing.copy(mStartLevel, a);
      mRing.subtract_in_place(mStartLevel, a1, b);
      result = a1;
    }
    
    void subtract_multiple(ElementType &result, ElementType a, ElementType b) const
    {
      if (a == 0 || b == 0) return;
      ElementType ab = mRing.mult(mStartLevel,a,b,true);
      mRing.subtract_in_place(mStartLevel, result, ab);
    }
    
    void mult(ElementType &result, ElementType a, ElementType b) const
    {
      if (a == 0 || b == 0)
        result = 0;
      else
        result = mRing.mult(mStartLevel, a, b, true);
    }
    
    void divide(ElementType &result, ElementType a, ElementType b) const
    {
      if (a == 0 || b == 0)
        result = 0;
      else
        {
          ElementType a1 = mRing.copy(mStartLevel, a);
          if (!mRing.division_in_place(mStartLevel, a1, b, result))
            result = 0;
          mRing.dealloc_poly(a1);
        }
    }
    
    void remainder(ElementType &result, ElementType a, ElementType b) const
    {
      if (a == 0 || b == 0)
        result = 0;
      else
        {
          result = mRing.copy(mStartLevel, a);
          mRing.remainder(mStartLevel, result, b);
        }
    }
    
    void to_ring_elem(ring_elem &result, const ElementType a) const
    {
      ElementType h = mRing.copy(mStartLevel, a);
      result = TOWER_RINGELEM(h);
    }
    
    void from_ring_elem(ElementType &result, const ring_elem &a) const
    {
      ElementType a1 = TOWER_VAL(a);
      result = mRing.copy(mStartLevel, a1);
    }
    
    void swap(ElementType &a, ElementType &b) const
    {
      ElementType tmp = a;
      a = b;
      b = tmp;
    }
    
    void elem_text_out(buffer &o,
                       const ElementType f,
                       bool p_one,
                       bool p_plus,
                       bool p_parens) const;
    
    void gcd(ElementType &result, const ElementType f, const ElementType g) { result = mRing.gcd(mStartLevel,f,g); }
    
    void gcd_coefficients(ElementType &result_gcd,
                          ElementType &result_u, ElementType &result_v,
                          const ElementType f, const ElementType g)
    {
      result_gcd = mRing.gcd_coefficients(mStartLevel, f, g, result_u, result_v);
    }
    
    
    int degree(int var, const ElementType f) const { return mRing.degree(mStartLevel,var,f); }
    void diff(int var, ElementType &result, const ElementType f) const { result = mRing.diff(mStartLevel, var, f); }
    int extension_degree(int firstvar); // returns -1 if infinite
    void power_mod(ElementType &result, const ElementType f, mpz_t n, const ElementType g) const { result = mRing.power_mod(mStartLevel, f, n, g); } // f^n mod g
    void lowerP(ElementType &result, const ElementType f) { result = mRing.lowerP(mStartLevel, f); }
    
  private:
    void extensions_text_out(buffer &o) const;

    /////////////////////////////////////
    // Predicates ///////////////////////
    /////////////////////////////////////
    bool is_equal(int level, const poly f, const poly g);
    bool is_zero(poly f) { return f == 0; }

    /////////////////////////////////////
    // Construction of new elements /////
    /////////////////////////////////////

    poly copy(int level, const_poly f);
    poly var(int level, int v); // make the variable v (but at level 'level')
    poly from_int(int level, long c);  // c should be reduced mod p
    
    /////////////////////////////////////
    // Private routines for arithmetic //
    /////////////////////////////////////
    void reset_degree_0(poly &f); // possibly sets f to 0
    void reset_degree_n(int level, poly &f); // ditto

    void mult_by_coeff_0(poly &f, long b);
    void mult_by_coeff_n(int level, poly &f, poly b);
    // f *= b.  b should have level 'level-1'.

    void make_monic_0(poly & f, long &result_multiplier);
    void make_monic_n(int level, poly & f, poly &result_multiplier);

    bool make_monic3(int level, poly & u1, poly &u2, poly &u3);


    void add_in_place_0(poly &f, const poly g);
    void add_in_place_n(int level, poly &f, const poly g);
    void add_in_place(int level, poly &f, const poly g);

    void subtract_in_place_0(poly &f, const poly g);
    void subtract_in_place_n(int level, poly &f, const poly g);
    void subtract_in_place(int level, poly &f, const poly g);

    poly mult_0(const poly f, const poly g, bool reduce_by_extension);
    poly mult_n(int level, const poly f, const poly g, bool reduce_by_extension);
    poly mult(int level, const poly f, const poly g, bool reduce_by_extension);

    poly random_0(int deg);
    poly random_n(int level, int deg);
    poly random(int level, int deg);
    poly random(int level); // obtains a random element, using only variables which are algebraic over the base

    poly diff_0(const poly f);
    poly diff_n(int level, int whichvar, const poly f);

    poly mult_by_int_0(long c, const poly f);
    poly mult_by_int_n(int level, long c, const poly f);
    poly mult_by_int(int level, long c, const poly f);

    /////////////////////////////////////
    // Translation to/from other rings //
    /////////////////////////////////////
    void add_term(int level, poly &result, long coeff, exponents exp) const; // modifies result.
#endif
  private:
    const ARingZZpFFPACK &mBaseRing;
    unsigned int mStartLevel;
    unsigned int mNumVars;

    const std::vector<std::string> mVarNames;
    std::vector<ElementType> mExtensions;
  };

}; // M2 namespace

#endif

// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e "
// indent-tabs-mode: nil
// End:
