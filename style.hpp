// (c) 1994 Michael E. Stillman
#ifndef _style_hh_
#define _style_hh_

#include <cmath>    // to get fabs(), gcc 3.0
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <string.h>

#include <gmp.h>

// 'swap' is defined here, perhaps in namespace std
#include <algorithm>
using namespace std;

extern "C" char newline[];

#if defined(__MWERKS__)
#define Random RandomFoo
#endif

#include "error.h"
#include "buffer.hpp"
#include "mem.hpp"

typedef unsigned int unsigned_int;
typedef int *ptr_to_int;
typedef unsigned int *ptr_to_unsigned_int;

#if 0
template <class T>
inline const T &min(const T &a, const T &b)
{ return (a<b ? a : b); }

template <class T>
inline const T &max(const T &a, const T &b)
{ return (a>b ? a : b); }
#endif

#if defined(_WIN32)
template <class T> 
inline void swap(T &t1, T &t2)
{
  T tmp = t1;
  t1 = t2;
  t2 = tmp;
}
#endif

// Mike, the following typedef is wrong on some machines.
// typedef unsigned long int uint32;
#include "targettypes.h"

const int LT = -1;
const int EQ = 0;
const int GT = 1;
const int INCOMPARABLE = 2;

typedef enum {FIRST, LAST} direction;

// Used for all of the heap types: polynomial, vector, resolution vectors.
#define GEOHEAP_SIZE 15

extern int heap_size[GEOHEAP_SIZE];


#endif

// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e"
// End:
