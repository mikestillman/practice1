// Copyright 2011 Michael E. Stillman

#ifndef _aring_gf_hpp_
#define _aring_gf_hpp_

#include "aring.hpp"
#include "buffer.hpp"
#include "ringelem.hpp"
#include <iostream>


//enable this lines to trick Kdevelop
// #define HAVE_FFLAS_FFPACK 1 
// #define HAVE_GIVARO 1

#if defined(HAVE_FFLAS_FFPACK) && defined(HAVE_GIVARO)
#include <givaro/givgfq.h>
#include <givaro/givpower.h>
#include <givaro/givtimer.h>
//#include <givaro/givextension.h>     //multiple definition problem...   solvable by encapsulating (see linbox)? Also solvable with the namespace trick, but do not overuse that...


namespace M2 {

/**
    @ingroup rings

    @brief wrapper for the  Givaro::GFqDom<>  galois field implementation
*/
  /// @todo: think about deriving from RingInterface AND from Ring
  class ARingGF : RingInterface
  {


  public:
    static const RingID ringID = ring_GF;

    typedef Givaro::GFqDom<long> FieldType;
    typedef FieldType::Element ElementType;
    typedef M2::ARingGF    ring_type ;
    //  CoefficientRingZZp * get_CoeffRing() const { return coeffR; }
    M2::ARingGF * get_ARing() const { return new M2::ARingGF(charac,dimension); }
  
    typedef ElementType elem;

    typedef  FieldType::Residu_t UTT; ///< types depends on FieldType definition!
    typedef Signed_Trait<FieldType::Residu_t>::signed_type STT;///< types depends on FieldType definition!



    ARingGF( UTT charac_,   UTT dimension_);

  private:
    mutable  FieldType::randIter     givaroRandomIterator;
    UTT charac;
    UTT dimension; ///< same as extensionDegree

    const FieldType givaroField;


    //  int p1; // p-1
    // int minus_one;
    // int prim_root; // element we will use for our primitive root

  public:
    // ring informational
   UTT   characteristic() const { return charac; }

    /** @name IO
    @{ */
            void text_out(buffer &o) const { o << "GF(" << charac << "," << dimension << ")"; }

            void elem_text_out(buffer &o, 
                                const  ElementType a,
                                bool p_one, 
                                bool p_plus, 
                                bool p_parens) const;

    /** @} */


    /** @name properties
    @{ */
        bool is_unit(const ElementType f) const ;
        bool is_zero(const ElementType f) const ;
    /** @} */


        /** @name translation functions
        @{ */
    void to_ring_elem(ring_elem &result, const ElementType &a) const
    {
      result.int_val = a;
    }
 
    void from_ring_elem(ElementType &result, const ring_elem &a) const
    {
      result = a.int_val;
    }
    /** @} */

  /** @name operators
    @{ */
        bool is_equal(const ElementType f,const ElementType g) const    ;
        int compare_elems(const ElementType f,const ElementType g) const ;
    /** @} */

    /** @name get functions
        @{ */
        int get_int(elem f) const ;
        int get_repr(elem f) const ;
    /** @} */


    /** @name init_set
    @{ */

        void init_set(elem &result, elem a) const { result = a; }

        void set(elem &result, elem a) const { result = a; }

        void init(elem &result) const ;

        void clear(elem &result) const ;

        void set_zero(elem &result) const ;

        void copy(elem &result,const elem a) const ;

        void set_from_int(elem &result, int a) const ;

        void set_from_mpz(elem &result,const mpz_ptr a) const ;

        void set_from_mpq(elem &result,const mpq_ptr a) const ;

        void set_var(elem &result, int v) const         { result = 1; }

    /** @} */


    /** @name arithmetic
    @{ */
        void negate(elem &result,const elem a) const;

        void invert(elem &result,const elem a) const;

        void add(elem &result, const elem a,const elem b) const;

        void subtract(ElementType &result,const  ElementType a,const  ElementType b) const;

        void subtract_multiple(elem &result,const  elem a,const  elem b) const;

        void mult(elem &result,const  elem a,const  elem b) const;

         ///@brief test doc
        void divide(elem &result,const  elem a,const  elem b) const;

        void power(elem &result,const  elem a,const  STT n) const;

        void power_mpz(elem &result,const  elem a,const  mpz_ptr n) const;

        void syzygy(const ElementType a, const ElementType b,
                    ElementType &x, ElementType &y) const;
    /** @} */


    /** @name misc
    @{ */
            void swap(ElementType &a, ElementType &b) const;

            void random(ElementType &result) const;
    /** @} */
  };

};


#endif
#endif

// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e  "
// indent-tabs-mode: nil
// End:
