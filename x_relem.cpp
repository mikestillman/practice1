// (c) 1994 Michael E. Stillman

#include "interp.hpp"

#include "monoid.hpp"
#include "monomial.hpp"
#include "relem.hpp"
#include "z_mod_p.hpp"
#include "Z.hpp"
#include "GF.hpp"
#include "polyring.hpp"
#include "matrix.hpp"
#include "schur.hpp"
#include "frac.hpp"

Z *ZZ;			// set in the init routine below

object make_object_int(int n)
{
  return RingElement(ZZ, n);
}
object make_object_int(mpz_t n)
{
  return RingElement(ZZ, n);
}

void cmd_Ring_from_int(object &oF, object &on)
{
  const Ring *F = oF->cast_to_Ring();
  const RingElement &n = on->cast_to_RingElement();

  if (n.Ring_of() != ZZ)
    {
      gError << "from_int requires an integer";
      return;
    }

  gStack.insert(RingElement(F,MPZ_VAL(n.get_value())));
}

void cmd_Ring_isequal(object &oa, object &ob)
{
  RingElement a = oa->cast_to_RingElement();
  RingElement b = ob->cast_to_RingElement();
  const Ring *Ra = a.Ring_of();
  const Ring *Rb = b.Ring_of();
  int result;
  if (Ra == Rb)
    result = a.is_equal(b);
  else if (Ra == ZZ)
    {
      RingElement c(Rb, MPZ_VAL(a.get_value()));
      result = b.is_equal(c);
    }
  else if (Rb == ZZ)
    {
      RingElement c(Ra, MPZ_VAL(b.get_value()));
      result = a.is_equal(c);
    }
  else
    {
      gError << "equality check: ring elements have different base rings";
      result = 0;
    }
  gStack.insert(make_object_int(result));
}

void cmd_Ring_iszero(object &oa)
{
  RingElement a = oa->cast_to_RingElement();
  gStack.insert(make_object_int(a.is_zero()));
}
void cmd_Ring_isunit(object &oa)
{
  RingElement a = oa->cast_to_RingElement();
  gStack.insert(make_object_int(a.is_unit()));
}

void cmd_Ring_var(object &oF, object &ov, object &oe)
{
  const Ring *F = oF->cast_to_Ring();
  int v = ov->int_of();
  int e = oe->int_of();
  gStack.insert(RingElement(F,v,e));
}
void cmd_Ring_negate(object &oa)
{
    RingElement a = oa->cast_to_RingElement();
    gStack.insert(-a);
}
void cmd_Ring_add(object &oa, object &ob)
{
    RingElement a = oa->cast_to_RingElement();
    RingElement b = ob->cast_to_RingElement();
    gStack.insert(a+b);
}
void cmd_Ring_subtract(object &oa, object &ob)
{
    RingElement a = oa->cast_to_RingElement();
    RingElement b = ob->cast_to_RingElement();
    gStack.insert(a-b);
}
void cmd_Ring_mult(object &oa, object &ob)
{
    RingElement a = oa->cast_to_RingElement();
    RingElement b = ob->cast_to_RingElement();
    gStack.insert(a*b);
}
void cmd_Ring_div(object &oa, object &ob)
{
    RingElement a = oa->cast_to_RingElement();
    RingElement b = ob->cast_to_RingElement();
    gStack.insert(a/b);
}
void cmd_Ring_mod(object &oa, object &ob)
{
    RingElement a = oa->cast_to_RingElement();
    RingElement b = ob->cast_to_RingElement();
    gStack.insert(a%b);
}
void cmd_Ring_divmod(object &oa, object &ob)
{
  RingElement a = oa->cast_to_RingElement();
  RingElement b = ob->cast_to_RingElement();
  RingElement rem;
  RingElement adivb = a.divide(b, rem);
  gStack.insert(adivb);
  gStack.insert(rem);
}
void cmd_Ring_gcd(object &oa, object &ob)
{
  RingElement a = oa->cast_to_RingElement();
  RingElement b = ob->cast_to_RingElement();
  gStack.insert(a.gcd(b));
}
void cmd_Ring_gcdextended(object &oa, object &ob)
{
  RingElement a = oa->cast_to_RingElement();
  RingElement b = ob->cast_to_RingElement();
  RingElement u;
  RingElement v;
  RingElement g = a.gcd_extended(b, u, v);
  
  gStack.insert(g);
  gStack.insert(u);
  gStack.insert(v);
}

void cmd_Ring_power(object &oa, object &on)
{
  RingElement a = oa->cast_to_RingElement();
  RingElement b = on->cast_to_RingElement();

  if (b.Ring_of() != ZZ)
    {
      gError << "must use integers for exponentiation";
      return;
    }
  gStack.insert(a.power(MPZ_VAL(b.get_value())));
}

// Polynomial routines
void cmd_PolynomialRing_term(object &oR, object &oa, object &om)
{
  // MES: this has several problems: (1) needs error checking
  // (2) the RingElement construction is incorrect
  const Ring *R = oR->cast_to_Ring();
  RingElement a = oa->cast_to_RingElement();
  Monomial m = om->cast_to_Monomial();
  if (R->Ncoeffs() != a.Ring_of())
    { gError << "term: incorrect arguments"; return; }
  if (R->n_vars() == 0)
    { gError << "term: need a polynomial ring"; return; }

  gStack.insert(RingElement(R, a, m));
}
void cmd_PolynomialRing_getterms(object &oelem, object &om, object &on)
{
  RingElement r = oelem->cast_to_RingElement();
  int m = om->int_of();
  int n = on->int_of();
  gStack.insert(r.get_terms(m,n));
}
void cmd_PolynomialRing_getcoeff(object &oelem, object &om)
{
  RingElement r = oelem->cast_to_RingElement();
  Monomial m = om->cast_to_Monomial();
  gStack.insert(r.get_coeff(m));
}
void cmd_PolynomialRing_leadcoeff(object &oelem)
{
  RingElement r = oelem->cast_to_RingElement();
  gStack.insert(r.lead_coeff());
}
void cmd_PolynomialRing_leadmonom(object &oelem)
{
  RingElement r = oelem->cast_to_RingElement();
  gStack.insert(r.lead_monom());
}
void cmd_PolynomialRing_ishomogeneous(object &oelem)
{
  RingElement r = oelem->cast_to_RingElement();
  gStack.insert(make_object_int(r.is_homogeneous()));
}
void cmd_PolynomialRing_degree(object &oelem)
{
  RingElement r = oelem->cast_to_RingElement();
  
  gStack.insert(new object_intarray(r.degree()));
}
void cmd_PolynomialRing_homogenize(object &oelem, object &ov, object &owts)
{
  RingElement r = oelem->cast_to_RingElement();
  int v = ov->int_of();
  intarray *wts = owts->intarray_of();
  const Ring *R = r.Ring_of();
  if (v < 0 || v >= R->n_vars())
    {
      gError << "homogenization: improper ring variable";
      return;
    }
  if (wts == NULL || wts->length() != R->n_vars())
    {
      gError << "homogenization: improper weight function";
      return;
    }
  if ((*wts)[v] == 0)
    {
      gError << "homogenization: variable weight is zero";
      return;
    }

  RingElement rh = r.homogenize(v, wts->raw());
  if (!error_exists())
    gStack.insert(rh);
}
void cmd_PolynomialRing_homogenize1(object &oelem, object &ov, object &odeg, object &owts)
{
  RingElement r = oelem->cast_to_RingElement();
  int v = ov->int_of();
  int deg = odeg->int_of();
  intarray *wts = owts->intarray_of();

  const Ring *R = r.Ring_of();
  if (v < 0 || v >= R->n_vars())
    {
      gError << "homogenization: improper ring variable";
      return;
    }
  if (wts == NULL || wts->length() != R->n_vars())
    {
      gError << "homogenization: improper weight function";
      return;
    }
  if ((*wts)[v] == 0)
    {
      gError << "homogenization: variable weight is zero";
      return;
    }

  RingElement rh = r.homogenize(v, deg, wts->raw());
  if (!error_exists())
    gStack.insert(rh);
}

//// Fraction field routines ////
void cmd_FractionField_numerator(object &of)
{
  RingElement f = of->cast_to_RingElement();
  const Ring *R = f.Ring_of();
  const FractionField *K = R->cast_to_FractionField();
  if (K == NULL)
    {
      gError << "fraction field required";
      return;
    }
  gStack.insert(f.numerator());
}

void cmd_FractionField_denominator(object &of)
{
  RingElement f = of->cast_to_RingElement();
  const Ring *R = f.Ring_of();
  const FractionField *K = R->cast_to_FractionField();
  if (K == NULL)
    {
      gError << "fraction field required";
      return;
    }
  gStack.insert(f.denominator());
}



void cmd_Z_mod(object &r, object &oM)
{
  int p = r->int_of();
  const Monoid *M = oM->cast_to_Monoid()->cast_to_Monoid();
  if (p > 0)
    gStack.insert(new Z_mod(p,M));
  else if (p == 0)
    gStack.insert(new Z(M));
  else
    gError << "Z/pZ: p must be non-negative";
}

void cmd_Z()
{
  gStack.insert(ZZ);
}
void cmd_GF(object &oprim)
{
  
  RingElement prim = oprim->cast_to_RingElement();

  // MES: check that K is Z/p[x], for some p.
  // MES: check that f is a monic polynomial of degree >= 2
  // MES: later, check that f is irreducible

  gStack.insert(new GF(prim));
}
#if 0
void cmd_GF2(object &oR)
{
  PolynomialRing *R = oR->cast_to_Ring()->cast_to_poly_ring();
  if (R == NULL)
    {
      gError << "GF: Polynomial ring expected";
      return;
    }
  if (R->n_vars() != 1)
    {
      gError << "GF: expected a polynomial ring with one variable";
      return;
    }

  gStack.insert(new GF2(R));
}
#endif
void cmd_PolynomialRing(object &oK, object &oMF)
{
  const Ring *K = oK->cast_to_Ring();
  const Monoid *MF = oMF->cast_to_Monoid();
  gStack.insert(new PolynomialRing(K, MF));
}
void cmd_fraction_field(object &oR)
{
  const Ring *R = oR->cast_to_Ring();
  gStack.insert(new FractionField(R));
}
void cmd_qring(object &om)
{
  Matrix m = om->cast_to_Matrix();
  array<ring_elem> I;
  if (m.n_rows() != 1)
    {
      gError << "quotient ring expects matrix with one row";
      return;
    }
  const PolynomialRing *P = m.Ring_of()->cast_to_poly_ring();
  if (P == NULL)
    {
      gError << "qring: expected polynomial ring";
      return;
    }
  for (int i=0; i<m.n_cols(); i++)
    I.append(m.elem(0,i));
  gStack.insert(new PolynomialRing(P, I));
}
void cmd_qring2(object &on, object &oR)
{
  array<ring_elem> I;
  int n = on->int_of();
  const Ring *R = oR->cast_to_Ring();
  const PolynomialRing *P = R->cast_to_poly_ring();
  if (P == NULL)
    {
      gError << "qring: expected polynomial ring";
      return;
    }
  // MES: for now, we create an array of ring elems
  // by grabbing them one by one off the stack.
  // 'n' is the number of polynomials on the stack.

  for (int i=n-1; i >= 0 ; i--) 
    {
      if (!gStack.in_bounds(i) || gStack[i]->type_of() != TY_RING_ELEM)
	{
	  gError << "qring: expected ring element on the stack";
	  break;
	}
      RingElement f = gStack[i]->cast_to_RingElement();
      if (f.Ring_of() != P)
	{
	  gError << "qring: ring element has incorrect base ring";
	  break;
	}
      I.append(P->copy(f.get_value()));
    }
  gStack.poppem(n);

  gStack.insert(new PolynomialRing(P, I));
}
void cmd_schur(object &oK, object &oMF)
{
  const Ring *K = oK->cast_to_Ring();
  const Monoid *MF = oMF->cast_to_Monoid();
  gStack.insert(new SchurRing(K, MF));
}
void cmd_schur_dim(object &of)
{
  RingElement f = of->cast_to_RingElement();
  const Ring *R = f.Ring_of();
  const SchurRing *F = R->cast_to_SchurRing();
  if (F == NULL)
    {
      gError << "schur dimension: need an element of a schur ring";
      return;
    }
  RingElement result(F->Ncoeffs(), F->dimension(f.get_value()));
  gStack.insert(result);
}

void cmd_qring_ideal(object &oR)
{
  const PolynomialRing *R = oR->cast_to_Ring()->cast_to_poly_ring();
  if (R == NULL)
    {
      gError << "polynomial ring expected";
      return;
    }
  gStack.insert(R->get_ideal());
}
void i_ring_elem_cmds(void)
{
  assert(trivial_monoid != NULL);
  ZZ = new Z(trivial_monoid);

  // Ring Creation
  install(ggZ, cmd_Z);
  install(ggcharp, cmd_Z_mod, TY_INT, TY_MONOID);
  install(ggGF, cmd_GF, TY_RING_ELEM);
  //  install(ggGF, cmd_GF2, TY_RING);
  install(ggpolyring, cmd_PolynomialRing, TY_RING, TY_MONOID);
  install(ggschur, cmd_schur, TY_RING, TY_MONOID);
  install(ggqring, cmd_qring, TY_MATRIX);
  install(ggqring, cmd_qring2, TY_INT, TY_RING);
  install(ggfractionfield, cmd_fraction_field, TY_RING);

  install(gggetideal, cmd_qring_ideal, TY_RING);

  // Informational
  install(ggisequal, cmd_Ring_isequal, TY_RING_ELEM, TY_RING_ELEM);
  install(ggiszero, cmd_Ring_iszero, TY_RING_ELEM);
  install(ggisunit, cmd_Ring_isunit, TY_RING_ELEM);

  // ring element commands
  install(ggfromint, cmd_Ring_from_int, TY_RING, TY_INT);
  install(ggvar, cmd_Ring_var, TY_RING, TY_INT, TY_INT);

  install(ggnegate, cmd_Ring_negate, TY_RING_ELEM);
  install(ggadd, cmd_Ring_add, TY_RING_ELEM, TY_RING_ELEM);
  install(ggsubtract, cmd_Ring_subtract, TY_RING_ELEM, TY_RING_ELEM);
  install(ggmult, cmd_Ring_mult, TY_RING_ELEM, TY_RING_ELEM);
  install(ggdiv, cmd_Ring_div, TY_RING_ELEM, TY_RING_ELEM);
  install(ggmod, cmd_Ring_mod, TY_RING_ELEM, TY_RING_ELEM);
  install(ggdivmod, cmd_Ring_divmod, TY_RING_ELEM, TY_RING_ELEM);
  install(gggcd, cmd_Ring_gcd, TY_RING_ELEM, TY_RING_ELEM);
  install(gggcdextended, cmd_Ring_gcdextended, TY_RING_ELEM, TY_RING_ELEM);
  install(ggpower, cmd_Ring_power, TY_RING_ELEM, TY_INT);

  // polynomial commands
  install(ggterm, cmd_PolynomialRing_term, TY_RING, TY_RING_ELEM, TY_MONOMIAL);
  install(gggetcoeff, cmd_PolynomialRing_getcoeff, TY_RING_ELEM, TY_MONOMIAL);
  install(gggetterms, cmd_PolynomialRing_getterms, TY_RING_ELEM, TY_INT, TY_INT);
  install(ggleadcoeff, cmd_PolynomialRing_leadcoeff, TY_RING_ELEM);
  install(ggleadmonom, cmd_PolynomialRing_leadmonom, TY_RING_ELEM);
  install(ggishomogeneous, cmd_PolynomialRing_ishomogeneous, TY_RING_ELEM);
  install(ggdegree, cmd_PolynomialRing_degree, TY_RING_ELEM);
  install(gghomogenize, cmd_PolynomialRing_homogenize, TY_RING_ELEM, TY_INT, TY_INTARRAY);
  install(gghomogenize, cmd_PolynomialRing_homogenize1, 
	  TY_RING_ELEM, TY_INT, TY_INT, TY_INTARRAY);

  // schur ring functions
  install(ggdim, cmd_schur_dim, TY_RING_ELEM);

  // fraction field elements
  install(ggnumerator, cmd_FractionField_numerator, TY_RING_ELEM);
  install(ggdenominator, cmd_FractionField_denominator, TY_RING_ELEM);

}
