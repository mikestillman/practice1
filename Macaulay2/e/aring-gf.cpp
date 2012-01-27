// Copyright 2011 Michael E. Stillman

#include "aring-gf.hpp"
#include "error.h"

#if defined(HAVE_FFLAS_FFPACK) && defined(HAVE_GIVARO)

namespace M2 {

//std::vector<GFqDom<long>::Residu_t> irreducible_11_2;
//GFqDom<long> gfqField(11,2,irreducible_11_2);

  ARingGF::ARingGF(	 UTT charact_, 
             UTT extensionDegree_)  :   charac(charact_),
                                        dimension(extensionDegree_),
                                        givaroField(FieldType(charact_, extensionDegree_)),
                                        givaroRandomIterator( FieldType::randIter(givaroField ))     
  {

        /// @todo: remove debug code
        /// debug code:
        getModPolynomialCoeffs();
        getGeneratorPolynomialCoeffs();
        ARingGF *testGF = new ARingGF(charact_, getModPolynomialCoeffs() );

        std::cerr <<"random"<< std::endl;
        ElementType rnd ;
        this->random(rnd);
        std::cerr << " rnd = "<< rnd << std::endl;
        fieldElementToM2Array(rnd);    
        
        
        /// end debug
  }
  ARingGF::ARingGF(  UTT charact_, 
                    const M2_arrayint & modPolynomial)  :   charac(charact_),
                                            dimension( modPolynomial->len-1 ),
                                            givaroField( FieldType( charact_,dimension, ARingGF::M2arrayToStdVec(charact_, modPolynomial) )),
                                            givaroRandomIterator( FieldType::randIter(givaroField ))      

  {
        /// @jakob: find out if the irreducible polynomial is checked in givaro.     
        UTT localdegree = modPolynomial->len-1;
        if ( !( modPolynomial->len > 1 && modPolynomial->array[ localdegree-1 ]>0 ))
          {
            std::cout << "assertion would have failed" << std::endl;
          }

        getModPolynomialCoeffs();
        getGeneratorPolynomialCoeffs();
  }


    ARingGF::UTT ARingGF::M2arrayToGFRepresentation( ARingGF::UTT pCharac ,const  M2_arrayint & m2array ) 
    {
       std::cerr << "M2arrayToGFRepresentation"  << std::endl;
        ARingGF::UTT rep=0;
        assert( m2array->len > 1  );
        assert( sizeof( m2array->array[0] ) < sizeof( ARingGF::UTT) );

        for ( ARingGF::STT pos =  m2array->len-1 ; pos>=0;pos--)
        {
            std::cerr << " m2array->array["<< pos << "]" <<  m2array->array[pos] << std::endl;

            if (m2array->array[pos]>=0 ) 
            {
                 assert( (ARingGF::UTT)(m2array->array[pos]) < pCharac); 
                rep= rep*pCharac+   (m2array->array[pos])    ;
            }
            if (m2array->array[pos]<0 ) 
            {           
                 assert( (ARingGF::UTT)( - (m2array->array[pos]) ) < pCharac);
                rep= rep*pCharac+  (m2array->array[pos]+pCharac)  ;
            }
        }
        return rep;
        std::cerr << "rep" << rep << std::endl;
    }


    M2_arrayint     ARingGF::fieldElementToM2Array(ElementType el) const
    {
        UTT  rep;
        rep = this->givaroField.convert(rep,el);
        std::cerr << "rep" << rep<< std::endl;
        return representationToM2Array(rep);
    }

    

    std::vector< ARingGF::UTT> ARingGF::M2arrayToStdVec( ARingGF::UTT pCharac, const  M2_arrayint &  m2array ) 
    {
        // std::vector< UTT > stdvec;
       std::cerr << "M2arrayToStdVec"  << std::endl;
        assert( m2array->len > 0  );

        std::vector< ARingGF::UTT> vec;

        vec.resize(m2array->len );

        for ( UTT pos=0 ;pos < m2array->len; pos++)
        {
           vec[pos]=  m2array->array[pos] ;          
            if (m2array->array[pos]>=0 ) 
            {
                 assert( (ARingGF::UTT)(m2array->array[pos]) < pCharac); 
                vec[pos]=  m2array->array[pos] ;          
            }
            if (m2array->array[pos]<0 ) 
            {           
               assert( (ARingGF::UTT)( - (m2array->array[pos]) ) < pCharac);
               vec[pos]=  (m2array->array[pos]+pCharac)  ;
            }             
        }
        return vec;
    }

/// @mike correct output : print generator variable of the ring instead of 'X', whatever generator variable will be 
        void ARingGF::elem_text_out(buffer &o, 
        const ElementType a, 
        bool p_one, 
        bool p_plus, 
        bool p_parens) const
  {
    UTT  rep;
    rep = givaroField.convert(rep,a);
    long exp=0;
    if (  rep==0)  o << "0";
    while (rep !=0)
    {
        UTT  remainder = rep%charac;
        rep=rep/charac;
        if (exp >0 )
             o << " + ";
        o << remainder <<"*" << "X^" << exp;
        exp++;
    }
}

M2_arrayint ARingGF::representationToM2Array(UTT representation) const
{
  std::cerr << "representationToM2Array:\n";
  long coeffNum=this->dimension + 1;
    M2_arrayint     polynomialCoeffs = M2_makearrayint(coeffNum);

  long exp = 0;
    assert( representation !=0 );
    while ( representation !=0 )
    {
        assert(exp < coeffNum);
        UTT  remainder = representation%charac;
        representation = representation/charac;
        polynomialCoeffs->array[ exp ]= remainder;
        
        //debug:
        if (exp >0 )
             std::cerr << " + ";
         std::cerr << remainder <<"*" << "X^" << exp;
        // end debug
        exp++;
    }

    for ( ;exp < coeffNum ;exp ++)
    {
         polynomialCoeffs->array[ exp ] = 0;
    }
    std::cerr << "\n";
    return polynomialCoeffs;
}


/// returns mod polynomial coefficients as array of integers.
/// @todo problems, if characteristic does not fit in a int.
M2_arrayint ARingGF::getModPolynomialCoeffs() const
{
    std::cerr << "getModPolynomialCoeffs\n";
    long coeffNum=this->dimension + 1;
    M2_arrayint     modPolynomialCoeffs = M2_makearrayint(coeffNum);
    UTT             modPolynomialRepresentation = this->givaroField.irreducible();
    return representationToM2Array(modPolynomialRepresentation);

}

M2_arrayint ARingGF::getGeneratorPolynomialCoeffs() const
{
    std::cerr << "getGeneratorPolynomialCoeffs\n";
    long coeffNum = this->dimension + 1;
    M2_arrayint     generatorPolynomialCoeffs = M2_makearrayint(coeffNum);
    ElementType gen;
    givaroField.generator(gen);
    UTT  generatorRepresentation;
    generatorRepresentation = this->givaroField.convert(generatorRepresentation,gen);
    return representationToM2Array(generatorRepresentation);
}


bool ARingGF::is_unit(const ElementType f) const 	
    {   return givaroField.isunit(f); }

bool ARingGF::is_zero(const ElementType f) const 	
    {   return givaroField.isZero(f); }

bool ARingGF::is_equal(const ElementType f, const ElementType g) const 	
{   return	givaroField.areEqual(f,g); }


/// compare exponents of the used generator
int ARingGF::compare_elems(const ElementType f, const ElementType g) const 
{
    std::cerr << "ARingGF::compare_elems" << std::endl;
    if (f < g) return -1; 
    if (f > g) return 1;

    return 0;
}


///@todo how should this function behave? 
///@todo also consider conversion problems depending on 'elem' type
int ARingGF::get_int(const ElementType f) const 
{
    
    std::cerr << "ARingGF::get_int" << std::endl;
    assert(false);
    return f; 
}


/// @todo where this function will be used? ; 
/// @todo problems if type 'elem' is bigger than  int
int ARingGF::get_repr(const ElementType f) const 
{
    std::cerr << "get_repr" << std::endl;
    int result;
    givaroField.convert(result,f);
    return result; 
}

    // 'init', 'init_set' functions

    void ARingGF::init(ElementType &result) const              { result = givaroField.zero;  }

    void ARingGF::clear(ElementType &result) const             { /* nothing */ }

    void ARingGF::set_zero(ElementType &result) const          { result = givaroField.zero; }

    void ARingGF::copy(ElementType &result, const ElementType a) const { result = a; }


/// @todo possible problem if type UTT is smaller than an int?
void ARingGF::set_from_int(ElementType &result, int a) const 
{
    //std::cerr << "ARingGF::set_from_int" << std::endl;
    a = a % charac; 
    if (a < 0) a += charac;
    givaroField.init(result, a);
}

    void ARingGF::set_from_mpz(ElementType &result, const mpz_ptr a) const 
    {
        //std::cerr << "set_from_mpz" << std::endl;
        UTT b = static_cast< UTT>(mpz_fdiv_ui(a, charac));
       // std::cerr << "b " << b << std::endl;
        givaroField.init(result,  b);
       // std::cerr << "result " << result << std::endl;
    }

    void ARingGF::set_from_mpq(ElementType &result, const mpq_ptr a) const {
      //  std::cerr << "set_from_mpq" << std::endl;
        ElementType n, d;
        set_from_mpz(n, mpq_numref(a));
        set_from_mpz(d, mpq_denref(a));
        divide(result,n,d);
    }

    // arithmetic
    void ARingGF::negate(ElementType &result, const ElementType a) const
    {
        givaroField.neg(result,a);
    }

    /// if a is zero, the result is 1 , but is that what we expect?
    /// I vote for two invert functions, one with this check and one without.(Jakob)
    void ARingGF::invert(ElementType &result, const ElementType a) const
    {
       // std::cerr << "ARingGF::invert" << std::endl;
        if ( givaroField.isZero(a))
            ERROR(" division by zero");
        givaroField.inv(result,a);
    }


    void ARingGF::add(ElementType &result, const ElementType a, const ElementType b) const
    {
        givaroField.add(result,a,b);
    }

    void ARingGF::subtract(ElementType &result, const ElementType a, const ElementType b) const
    {
        givaroField.sub(result,a,b);
    }

    /// @param c[in][out] c = c- a*b
    void ARingGF::subtract_multiple(ElementType &c, const ElementType a, const ElementType b) const
    {
        givaroField. maxpyin(c,a,b);
    }

    void ARingGF::mult(ElementType &result, const ElementType a, const ElementType b) const
    {
        givaroField.mul(result,a,b);
    }

    void ARingGF::divide(ElementType &result, const ElementType a, const ElementType b) const
    {
        if ( givaroField.isZero(b))
           ERROR(" division by zero");
        givaroField.div(result,a,b);
    }

   /// @jakob: overflow can be occured due to multiplication. use exact mpz for multiply and modulo operation instead!
    void ARingGF::power(ElementType &result, const ElementType a, const  STT n) const
    {
        if (givaroField.isnzero(a)) 
        {
            mpz_t  mpz_a;
            mpz_t  mpz_n;
            mpz_t  mpz_tmp;
            mpz_init( mpz_a);
            mpz_init( mpz_n);
            mpz_init( mpz_tmp);
            mpz_set_si   (mpz_n,n);
            mpz_set_ui (mpz_a,a);
            //std::cerr << "a = " << a << std::endl;
            //std::cerr << "mpz_a = " << mpz_a << std::endl;
            //std::cerr << "n = " << n << std::endl;
            //std::cerr << "mpz_n = " << mpz_n << std::endl;
            mpz_fdiv_r_ui(mpz_tmp, mpz_n, givaroField.cardinality() -1)  ;
            mpz_mul(mpz_n, mpz_a, mpz_tmp);
            STT tmp = static_cast< STT>(mpz_fdiv_ui( mpz_n, givaroField.cardinality() -1))  ;
            if ( tmp==0 )
            {
                tmp+=givaroField.cardinality()-1;
                // result=givaroField.one;
            }
             
            //std::cerr << "tmp = " << tmp << std::endl;
            assert(tmp>=0); // tmp<0 should never occur
             if (tmp < 0) tmp += givaroField.cardinality()-1 ; 
            result = tmp;
        }
        else
        {
            if (n<0)
                 ERROR(" division by zero");
            result = 0;
        }
    }

    ///@todo ensure that  givaroField.cardinality() fits in a unsigned long, otherwise instead of mpz_fdiv_ui a different function has to be called)
    void ARingGF::power_mpz(ElementType &result, const  ElementType a, const  mpz_ptr n) const
    {
        STT n1 = static_cast< STT>(mpz_fdiv_ui(n, givaroField.cardinality()-1));

        //std::cerr << "exponent = " << n << std::endl;
        //std::cerr << "n1 = " << n1 << std::endl;
        power(result,a,n1);
    }

    ///@note dublicate code
    void ARingGF::swap(ElementType &a, ElementType &b) const
    {
      ElementType tmp = a;
      a = b;
      b = tmp;
    }

    void ARingGF::elem_text_out(buffer &o, 
                const ElementType a, 
                bool p_one, 
                bool p_plus, 
                bool p_parens) const;

    /** @brief returns x,y  s.y.  x*a + y*b == 0.
       if possible, x is set to 1.
       no need to consider the case a==0 or b==0.
    */

    void ARingGF::syzygy(const ElementType a, const ElementType b,
                        ElementType &x, ElementType &y) const
   
    {
      x = givaroField.one;
      divide(y,a,b);
      negate(y,y);
    }

    /// @jakob: document possible overflow and other nasty things
    void ARingGF::random(FieldType::randIter &it, ElementType &result) const
    {
         givaroField.random( it,result);
      //   std::cerr << " givaroField.cardinality()" << givaroField.cardinality();
      //   std::cerr << " givaroRandomIterator()" << it();
    }

    void ARingGF::random(ElementType &result) const
    {
        return  random(givaroRandomIterator,result);
        //result = rawRandomInt((int32_t) givaroField.cardinality());
        //result = givaroRandomIterator() %   givaroField.cardinality();
    }

};

#endif
// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e  "
// indent-tabs-mode: nil
// End:
