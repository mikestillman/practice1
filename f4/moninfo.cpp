// Copyright 2005  Michael E. Stillman

#include "../newdelete.hpp"
#include "moninfo.hpp"
#include <cstdio>
#include <cstdlib>

MonomialInfo::MonomialInfo(int nvars0)
{
  nvars = nvars0;
  nslots = 2 + nvars;
  hashfcn = newarray(monomial_word,nvars);
  for (int i=0; i<nvars; i++)
    hashfcn[i] = rand();
  mask = 0x10000000;
}

MonomialInfo::~MonomialInfo()
{
  deletearray(hashfcn);
}

void MonomialInfo::show() const
{
  fprintf(stderr, "monomial info\n");
  fprintf(stderr, "  nvars  = %d",nvars);
  fprintf(stderr, "  nslots = %d",nslots);
  fprintf(stderr, "  mask   = %lld",mask);
  fprintf(stderr, "  hash values for each variable\n");
  for (int i=0; i<nvars; i++)
    fprintf(stderr, "    %lld\n",hashfcn[i]);
}

void MonomialInfo::show(const_packed_monomial m) const
{
  fprintf(stderr, "[");
  for (int v=1; v<monomial_size(m); v++)
    {
      if (v > 1) fprintf(stderr, " ");
      fprintf(stderr, "%lld", m[v]);
    }
  fprintf(stderr, "]");
}

// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e "
// End:
