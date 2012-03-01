// Copyright 2011 Michael E. Stillman

#ifndef _aring_hpp_
#define _aring_hpp_

#ifdef DEBUG
#include <cassert>
#define ASSERT(X) assert(X);
#define IF_DEBUG(X) X
#else
#define ASSERT(X)
#define IF_DEBUG(X)
#endif

#include "engine.h"
#include "ringelem.hpp"
#include "buffer.hpp"

#if 0
#define RING(T,A) static_cast<const ConcreteRing<T> *>(A)->R_
#define RELEM(T,a) static_cast<RElementWrap<T> &>(a).val_
#define constRELEM(T,a) static_cast<const RElementWrap<T> &>(a).val_
#endif


class RingMap;


namespace M2 {

  ////////////////////////////////////////////////////////
  // Programming level interfaces ////////////////////////
  ////////////////////////////////////////////////////////

/**
\ingroup rings
*/
  enum RingID {
    ring_example = 0,
    ring_ZZZ,
    ring_ZZp,
    ring_logZZp,
    ring_GF,
    ring_GFM2,
    ring_FFPACK,
    ring_RRR,
    ring_CCC,
    ring_old,  ///< refers to all rings which are not ConcreteRing's.
    ring_top = 8 ///< used to determine the number of ring types
  };

/**
\ingroup rings
*/
  class RingInterface {}; ///< inherit from this if the class is to be used as a template parameter for ConcreteRing

class DummyRing : RingInterface
  {
 public:
      

        typedef int    FieldType;
        typedef int      ElementType;
     
    
        typedef ElementType     elem;

        virtual int characteristic()  const  {return 0; }


        virtual  int get_int(elem f) const {return 0;}
        virtual  int get_repr(elem f) const {return 0;}
        virtual  M2_arrayint getModPolynomialCoeffs() const {return 0;}
        virtual  M2_arrayint getGeneratorCoeffs() const {return 0;}
        
        virtual  ring_elem   getGenerator() const {return 0;}
    
    
        virtual  M2_arrayint fieldElementToM2Array(ElementType el) const {return 0; }
    
        virtual  void to_ring_elem(ring_elem &result, const ElementType &a) const    {       }
        
        virtual  void from_ring_elem(ElementType &result, const ring_elem &a) const {       }

        virtual  bool promote(const Ring *Rf, const ring_elem f, ElementType &result) const {return false;}
    
        virtual  bool lift(const Ring *Rg, const ElementType f, ring_elem &result) const {return false;}

    
        virtual  void eval(const RingMap *map, const elem f, int first_var, ring_elem &result) const { }

        virtual  void text_out(buffer &o) const { o << "GF(dummy)"; }

        virtual      void elem_text_out(buffer &o, 
                                const  ElementType a,
                                bool p_one, 
                                bool p_plus, 
                                bool p_parens) const {};

       virtual   void init_set(elem &result, elem a) const { result = a; }

       virtual  void set(elem &result, elem a) const { result = a; }

        virtual void set_from_int(elem &result, int a) const { result=a;}

         virtual  void init(elem &result) const    { result = 0; }

        virtual  void set_from_mpz(elem &result,const mpz_ptr a) const {result=0;}

       virtual void set_from_mpq(elem &result,const mpq_ptr a) const {result=0;}

        virtual void set_var(elem &result, int v) const         { result = 1; }

        virtual bool is_unit(const ElementType f) const  {return false;}
        virtual bool is_zero(const ElementType f) const  {return true;}
        virtual bool is_equal(const ElementType f,const ElementType g) const     {return false;}
        virtual int compare_elems(const ElementType f,const ElementType g) const  {return 1;}

        virtual void clear(elem &result) const  {result=0;}

        virtual void set_zero(elem &result) const   {result=0;}

        virtual void copy(elem &result,const elem a) const  {result=a;}


      virtual void negate(elem &result,const elem a) const  {};;

       virtual void invert(elem &result,const elem a) const  {};;

        virtual void add(elem &result, const elem a,const elem b) const  {};;

        virtual void subtract(ElementType &result,const  ElementType a,const  ElementType b) const  {};;

        virtual void subtract_multiple(elem &result,const  elem a,const  elem b) const  {};;

        virtual void mult(elem &result,const  elem a,const  elem b) const  {};;

         ///@brief test doc
        virtual  void divide(elem &result,const  elem a,const  elem b) const  {};;

        virtual void power(elem &result,const  elem a,const  int n) const  {};;

        virtual void power_mpz(elem &result,const  elem a,const  mpz_ptr n) const  {};;

        virtual void syzygy(const ElementType a, const ElementType b,
                    ElementType &x, ElementType &y) const  {};;

        virtual void random(ElementType &result) const {result=0;}


  };


/**
\ingroup rings
*/
  class PolynomialRingInterface {}; ///< inherit from this if the class is to be used as a template param for PolynomialConcreteRing
  class UserObject {};


  template <class RingType> class AConcreteRing;


  class RElement
  {
  public:
    RElement() {}
    virtual ~RElement() {}
    //    virtual RElement *clone() = 0;
    //    virtual RElement &operator=(const RElement &a) = 0;
  };

/**
\ingroup rings
*/
  class ARing : public UserObject
  {
  public:
    template<class RingType>
    const AConcreteRing<RingType> * cast_to_AConcreteRing() const { return dynamic_cast< const AConcreteRing<RingType> * >(this) ; }
    // result will be either 0, or this.

    virtual RingID getRingID() const = 0;

    virtual void init_set(RElement &a, long b) const = 0;

    virtual void add_to(RElement &a, const RElement &b) const = 0;

    static bool converter(const ARing *sourceR, const ARing *targetR, const RElement &a, RElement &b);
  };

};


#endif

// Some basic functions:
// (a) Create a ring
//  create a ZZp ring:
//    top level function (in x-relem.cpp)
//    function: create an ARing from a basic ring.
//      issue: the basic ring needs to know its ARing?  Preferably not?
//      if so: the createRing needs to set the ARing in the basic ring object afterwards.
//      if not: to make a toplevel object, need to know ARing from somewhere else.
//       problem: make the lead coefficient (i.e. need to know the coefficient ring.  So
//       APolynomialRing will need to know this, and it will mirror what the RingInterface object knows already)
//  CLEANER: keep the two layers separate...  SO:
//    An APolynomialRing must have functions: getCoefficientRing.
//    And what about creating a matrix over a ring?
// (b) m1 = m + n (matrices).
// (c) Create a new matrix

// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e  "
// indent-tabs-mode: nil
// End:
