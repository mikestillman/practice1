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

char *getmem(size_t n)
{
  char *p;
  p = GC_MALLOC(n);
  if (p == NULL) outofmem();
#ifdef DEBUG
  memset(p,0xbe,n);
  trapchk(p);
#endif
  return p;
}

char *getmem_clear(size_t n)
{
  char *p;
  p = GC_MALLOC(n);
  if (p == NULL) outofmem();
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
  if (p == NULL) outofmem();
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
  if (p == NULL) outofmem();
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
  if (p == NULL) outofmem();
  bzero(p,n);
#ifdef DEBUG
  trapchk(p);
#endif
  return p;
}

/*
 Local Variables:
 compile-command: "make -C $M2BUILDDIR/Macaulay2/d "
 End:
*/
