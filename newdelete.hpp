#ifndef NEWDELETE_H
#define NEWDELETE_H 1

// get declarations of outofmem and getmem
#include "../d/M2mem.h"

// this replaces all uses of the construction "new T[n]":
#define newarray(T,len) reinterpret_cast<T*>(GC_MALLOC((len) * sizeof(T)))
// this replaces all uses of the construction "new T":
#define newitem(T) reinterpret_cast<T*>(GC_MALLOC(sizeof(T)))
// this replaces all uses of the construction "delete [] x":
#define deletearray(x) GC_FREE(x)
// this replaces all uses of the construction "delete x":
#define deleteitem(x) GC_FREE(x)

#include <gc.h>
#include "../d/memdebug.h"

struct our_new_delete {
  inline void* operator new( size_t size ) { void *p = GC_MALLOC( size ); if (p == NULL) outofmem(); return p; }
  inline void* operator new []( size_t size ) { void *p = GC_MALLOC( size ); if (p == NULL) outofmem(); return p; }
  inline void operator delete( void* obj ) { if (obj != NULL) GC_FREE( obj ); }
  inline void operator delete []( void* obj ) { if (obj != NULL) GC_FREE( obj ); }
};

#include <gc/new_gc_alloc.h>

// struct gc_malloc_alloc {
//   static void* allocate(size_t n) { void* p = GC_MALLOC(n); if (p == NULL) outofmem(); return p; }
//   static void deallocate(void* p, size_t /* n */) { GC_FREE(p); }
//   static void* reallocate(void* p, size_t /* old size */, size_t newsize) { void* r = GC_REALLOC(p, newsize); if (NULL == r) outofmem(); return r; }
// };

#endif

// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e "
// End:
