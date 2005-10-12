// Copyright 2005  Michael E. Stillman

#ifndef _monhashtable_h_
#define _monhashtable_h_

#include "moninfo.hpp"

// ValueType must implement the following:
// values should have computed hash values stored with them
//  typename ValueType::value
//  long ValueType::hash_value(value m)
//  bool ValueType::is_equal(value m, value n)
//  void ValueType::show(value m) -- prints to stderr

template <typename ValueType>
class MonomialHashTable
{
  typedef typename ValueType::value value;
  
private:
  ValueType *M;
  value *hashtab;

  unsigned long size;
  unsigned int  logsize;
  unsigned long hashmask;

  unsigned long threshold;
  unsigned long count;
  unsigned long nclashes;
  unsigned long max_run_length;

  void insert(value m);
  void grow(); // Increase logsize by 1, remake hashtab.
  void initialize(int logsize0);
public:

  MonomialHashTable(ValueType *M0, int logsize = 16);
  // The hash table size will be a power of 2, and this
  // is the initial power.
  
  ~MonomialHashTable();
  
  bool find_or_insert(value m, value &result);
  // return true if the value already exists in the table.
  // otherwise, result is set to the new value.

  void dump() const;
  // displays on stderr some info about the hash table
  // and number of values

  void show() const;
  // displays the hash table, first by dots and x'x: ...x...xxx....
  // and then each value is displayed, with hash value.
  // in form [loc, hash, value]
  // Each blank line has a dot, with multiple dots per line.
};
#endif

// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e/linalgGB "
// End:
