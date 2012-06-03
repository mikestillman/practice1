// Copyright 2012 Michael E. Stillman

#include "aring-tower.hpp"

namespace M2 {

ARingTower::~ARingTower()
{
  //TODO: write me.
  // needs to free the extension polynomials
}

// The main construction routine for tower rings
//  (1) names[i] is the name of the i-th variable, used solely for display.
//  (2) extensions is a std vector of length <= #variables (size of names array).
//     The i-th element is a polynomial of level i (- <= i < mNumVars
//     (which allows NULL as a value too)
ARingTower::ARingTower(const BaseRingType &baseRing, 
		       const std::vector<std::string>& names, 
		       const std::vector<ElementType> &extensions)
  : mBaseRing(baseRing),
    mVarNames(names),
    mExtensions()
{
  ASSERT(names.size() >= 1);
  mNumVars = names.size();
  mStartLevel = mNumVars - 1;

  // Now copy all of the extension polynomials
  ASSERT(extensions.size() <= names.size());
  for (size_t i=0; i<names.size(); i++)
    {
      if (extensions.size() < i)
	{
	  //mExtensions.push_back(mRing.copy(i, extensions[i]));
	}
      else
	mExtensions.push_back(static_cast<ElementType>(NULL));
    }
}

const ARingTower* ARingTower::create(const ARingZZpFFPACK & baseRing,
                                     const std::vector<std::string>& names)
{
  std::vector<ElementType> extensions;
  return new ARingTower(baseRing, names, extensions);
}

const ARingTower* ARingTower::create(const ARingTower &R, const std::vector<std::string> &new_names)
{
  //TODO: write
  return 0;
}

const ARingTower* ARingTower::create(const ARingTower &R, const std::vector<ElementType>& extensions)
{
  //TODO: check that 'extensions' has the correct form for R.
  //  if not: throw an exception
  //  else:
  return new ARingTower(R.baseRing(), R.varNames(), extensions);
}

///////////////////
// Display ////////
///////////////////

void ARingTower::text_out(buffer &o) const
{
  o << "Tower[ZZ/" << characteristic() << "[";
  for (size_t i=0; i<n_vars()-1; i++)
    o << varNames()[i] << ",";
  if (n_vars() > 0)
    o << varNames()[n_vars()-1];
  o << "]]";
  extensions_text_out(o);
}

void ARingTower::extensions_text_out(buffer &o) const
{
  for (int i=0; i<mExtensions.size(); i++)
    {
      if (mExtensions[i] != 0)
        {
          o << newline << "    ";
          elem_text_out(o, i, mExtensions[i], true, false, false);
        }
    }
}

namespace {
  int n_nonzero_terms(int level, poly f)
  {
    if (f == 0) return 0;
    int nterms = 0;
    if (level == 0)
      {
        for (int i=0; i<=f->deg; i++)
          if (f->ints[i] != 0) nterms++;
      }
    else
      {
        for (int i=0; i<=f->deg; i++)
          if (f->polys[i] != 0) nterms++;
      }
    return nterms;
  }
};

bool ARingTower::is_one(int level, const poly f) const
{
  if (f == 0) return false;
  if (f->deg != 0) return false;
  if (level == 0)
    return 1 == f->ints[0];
  else
    return is_one(level-1, f->polys[0]);
}

void ARingTower::elem_text_out(buffer &o,
                                int level,
                                const poly f,
                                bool p_one,
                                bool p_plus,
                                bool p_parens) const
{
  //o << to_string(level, f);
  if (f == 0)
    {
      o << "0";
      return;
    }

  int nterms = n_nonzero_terms(level,f);
  bool needs_parens = p_parens && (nterms >= 2);

  if (needs_parens)
    {
      if (p_plus) o << '+';
      o << '(';
      p_plus = false;
    }

  bool one = is_one(level, f);

  if (one)
    {
      if (p_plus) o << "+";
      if (p_one) o << "1";
      return;
    }

  const std::string& this_varname = varNames()[level];

  if (level == 0)
    {

      bool firstterm = true;
      for (int i=f->deg; i>=0; i--)
        if (f->ints[i] != 0)
          {
            if (!firstterm || p_plus) o << "+";
            firstterm = false;
            if (i == 0 || f->ints[i] != 1)
              o << f->ints[i];
            if (i  > 0)
              o << this_varname;
            if (i > 1)
              o << i;
          }
      if (needs_parens) o << ")";
    }
  else
    {
      bool firstterm = true;
      for (int i=f->deg; i>=0; i--)
        if (f->polys[i] != 0)
          {
            bool this_p_parens = p_parens || (i > 0);

            if (i == 0 || !is_one(level-1,f->polys[i]))
              elem_text_out(o, level-1,f->polys[i], p_one, p_plus || !firstterm, this_p_parens);
            else if (p_plus || !firstterm)
              o << "+";
            if (i  > 0)
              o << this_varname;
            if (i > 1)
              o << i;

            firstterm = false;
          }
      if (needs_parens) o << ")";
    }
}

}; // namespace M2

// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e  "
// indent-tabs-mode: nil
// End:

