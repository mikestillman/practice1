// Copyright 1997 by Michael E. Stillman
#ifndef _comb_hh_
#define _comb_hh_

#include "style.hpp"
#include "array.hpp"

class comb
     // Encoding, decoding of elements [i0,...,i(p-1)] to ord value.
{
// tab[d][n] is n choose d
// tab[0][n] == 1, tab[d][0] == 0, tab[0][0] = 1.
// tab[n][d] == tab[n-1][d] + tab[n-1][d-1]
  static array<array<int> > tab;

  static void expand(int n, int d);

  static int d_len(int n)
    {
      int i = tab.rawelem(n).length();
      if (i == 0)
	{
	  expand(n,0);
	  i = tab.rawelem(n).length();
	}
      return i;
    }

  static void text_out(ostream &o);
public:
  static int fast_binom(int n, int p)
    { return tab.rawelem(p).rawelem(n); }

  static int binom(int n, int p) 
    { 
      if (p < 0) return 0;
      if (p >= tab.length())
	expand(n,p);
      else if (n >= tab[p].length())
	expand(n,p);
      return tab[p][n]; 
    }
  static int encode(int *a, int p);
  static void decode(int c, int *a, int p);

  static bool increment(int p, int n, int *s);
  static bool valid_subset(int p, int n, const int *s);
  static int mult_subsets(int p, const int *s, 
			  int q, const int *t, int *&result);
};
#endif
