// Copyright 1994-2002 by Michael E. Stillman

#include "style.hpp"
#include "mem.hpp"
#include "intarray.hpp"
#include "hash.hpp"
#include "engine.h"

#include "random.hpp"

#include "array.hpp"
#include "queue.hpp"
#include "Z.hpp"
#include "QQ.hpp"

unsigned long mutable_object::next_hash_sequence_number = 100000;

template class array< char * >;
template class queue< int >;

Matrix_int_pair global_Matrix_int_pair;

int heap_size[GEOHEAP_SIZE] = {4, 16, 64, 256, 1024, 4096, 
			       16384, 65536, 262144, 1048576, 4194304,
			       16777216, 67108864, 268435456,
			       1073741824};

static bool initialized = false;

/** Initialize the engine.
 *  This routine must be called before any other engine routine is called.
 *  May be called multiple times.  The subsequent calls do nothing.
 */
void IM2_initialize()
{
  if (initialized) return;
  initialized = true;
  doubles                  = new doubling_stash;

  ZZ = Z::create(Monoid::get_trivial_monoid());
  globalQQ = QQ::create(Monoid::get_trivial_monoid());
  Random::i_random();
}

/** Engine error handling mechanism.
 *  Any engine routine which encounters an error (e.g. Rings not
 *  the same) often returns a NULL value, and sets an error
 *  message, which can be obtained from this routine.  Any routine that can set
 *  this will return a type such as "MatrixOrNull *".  This routine
 *  clears the error message and returns it.
 */

M2_string IM2_last_error_message()
{
  M2_string result = tostring(error_message());
  clear_error();
  return result;
}
