#include <stdio.h>

#include "types.h"
#include "M2mem.h"
#include "M2mem2.h"
#include "debug.h"

sigjmp_buf out_of_memory_jump;
char out_of_memory_jump_set = FALSE;

void outofmem(void) {
#if 0
     static int count = 0;
     if (!tokens_stopIfError && out_of_memory_jump_set && count++ < 5) {
     	  fprintf(stderr,"out of memory, returning to top level\n");
     	  fflush(stderr);
     	  siglongjmp(out_of_memory_jump,1);
	  }
     else 
#endif
     {
	  const char *msg = "\n\n *** out of memory, exiting ***\n";
	  int r = write(STDERR,msg,strlen(msg));
	  if (r == ERROR) exit(1);
	  exit(1);
     }
}

void outofmem2(size_t new) {
#if 0
     static int count = 0;
     if (!tokens_stopIfError && out_of_memory_jump_set && count++ < 5) {
     	  fprintf(stderr,"out of memory, returning to top level\n");
     	  fflush(stderr);
     	  siglongjmp(out_of_memory_jump,1);
	  }
     else 
#endif
     {
	  const char *msg = "\n\n *** out of memory trying to allocate %ld bytes, exiting ***\n";
	  static char buf[sizeof(msg) + 20 - 2];
	  sprintf(buf,msg,(long)new);
	  int r = write(STDERR,buf,strlen(buf));
	  if (r == ERROR) exit(1);
	  exit(1);
     }
}

char *getmem(size_t n)
{
  char *p;
  p = GC_MALLOC(n);
  if (p == NULL) outofmem2(n);
#ifdef DEBUG
  memset(p,0xbe,n);
  trapchk(p);
#endif
  return p;
}

void freememlen(void *s, size_t old) {
#    ifdef DEBUG
     trapchk(s);
#    endif
     GC_FREE(s);
}

void freemem(void *s) {
#    ifdef DEBUG
     trapchk(s);
#    endif
     GC_FREE(s);
}

char *getmem_clear(size_t n)
{
  char *p;
  p = GC_MALLOC(n);
  if (p == NULL) outofmem2(n);
  /* 
     note: GC_MALLOC clears memory before returning.
     If you switch to another memory allocator, you must clear it explicitly:
  bzero(p,n);
  */
#ifdef DEBUG
  trapchk(p);
#endif
  return p;
}

char *getmem_atomic(size_t n)
{
  char *p;
  p = GC_MALLOC_ATOMIC(n);
  if (p == NULL) outofmem2(n);
#ifdef DEBUG
  memset(p,0xac,n);
  trapchk(p);
#endif
  return p;
}

char *getmem_malloc(size_t n)
{
  char *p;
  p = malloc(n);
  if (p == NULL) outofmem2(n);
#ifdef DEBUG
  memset(p,0xca,n);
  trapchk(p);
#endif
  return p;
}

char *getmem_atomic_clear(size_t n)
{
  char *p;
  p = GC_MALLOC_ATOMIC(n);
  if (p == NULL) outofmem2(n);
  bzero(p,n);
#ifdef DEBUG
  trapchk(p);
#endif
  return p;
}

char *getmoremem (char *s, size_t old, size_t new) {
     void *p = GC_REALLOC(s,new);
     if (p == NULL) outofmem2(new);
#    ifdef DEBUG
     trapchk(p);
#    endif
     return p;
     }

char *getmoremem1 (char *s, size_t new) {
     void *p = GC_REALLOC(s,new);
     if (p == NULL) outofmem2(new);
#    ifdef DEBUG
     trapchk(p);
#    endif
     return p;
     }

char *getmoremem_atomic (char *s, size_t old, size_t new) {
     void *p = GC_MALLOC_ATOMIC(new);
     size_t min = old<new ? old : new;
     if (p == NULL) outofmem2(new);
     memcpy(p, s, min);
     GC_FREE(s);
#    ifdef DEBUG
     {
       int excess = new - min;
       if (excess > 0) memset(p+min,0xbe,excess);
     }
     trapchk(p);
#    endif
     return p;
     }

/*
 Local Variables:
 compile-command: "make -C $M2BUILDDIR/Macaulay2/d "
 End:
*/
