// Copyright 2002  Michael E. Stillman
#ifndef _hash_hh_
#define _hash_hh_

#include <assert.h>

class immutable_object {
protected:
  unsigned long _hashval;
public:
  immutable_object(unsigned long hashval) : _hashval(hashval) {}
  unsigned long get_hash_value() const { return _hashval; }
};

class mutable_object {
  static unsigned long next_hash_sequence_number;
  unsigned long _hashval;
public:
  mutable_object() : _hashval(next_hash_sequence_number++) {}
  unsigned long get_hash_value() const { return _hashval; }
};

class object {
  // Hashed objects, which are either mutable or immutable.
  static int next_hash_sequence_number;
  int hashval; 
  // == 0 means hash value is not set.  
  // > 0 means object is immutable.  This hash value is set when the object is
  //   made immutable.
public:
  object() : hashval(0) {}
  virtual ~object() {}
  void make_mutable() { if (hashval == 0) hashval = next_hash_sequence_number--; }
  void make_immutable(int hash) {
    assert(hash > 0);
    if (hashval == 0) hashval = hash;
  }
  
  bool is_mutable() const { return hashval < 0; }
  bool is_immutable() const { return hashval > 0; }
  int get_hash_value() { return hashval; }
};

#endif



// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e "
// End:
