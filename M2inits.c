/* this file contains all initializations needed to get going, even if Macaulay2_main() is not called at all */

#include "types.h"

static void init_gc() {
#ifdef MEM_DEBUG
     GC_all_interior_pointers = TRUE; /* set this before using gc routines!  (see gc.h) */
#endif
     GC_free_space_divisor = 2;	/* this was intended to be used only when we are about to dump data */
     GC_init();
     if (getenv("GC_free_space_divisor")) {
	  GC_free_space_divisor = atoi(getenv("GC_free_space_divisor"));
	  if (GC_free_space_divisor <= 0) {
	       fprintf(stderr, "%s: non-positive GC_free_space_divisor value, %ld\n", 
		    progname, GC_free_space_divisor);
	       exit (1);
	       }
	  }
     if (getenv("GC_enable_incremental") && atoi(getenv("GC_enable_incremental"))==1) {
	  GC_enable_incremental();
	  fprintf(stderr,"GC_enable_incremental()\n");
	  }
     if (getenv("GC_expand_hp")) {
	  GC_expand_hp(atoi(getenv("GC_expand_hp")));
	  }
#ifdef NDEBUG
     GC_set_warn_proc(dummy_GC_warn_proc);
#endif
     }

/* we test gc to whether it properly marks pointers found in registers */
static void uniq(void *p, ...) {
  va_list a;
  void *q[100];
  int n = 0, i, j;
  q[n++] = p;
  va_start(a,p);
  for (;(q[n] = va_arg(a,void *));n++) ;
  va_end(a);
  for (i=0; i<n; i++) for (j=0; j<i; j++) if (q[i] == q[j]) {
    fprintf(stderr,
	    "%s: error: gc library doesn't find all the active pointers!\n"
	    "           Perhaps GC_push_regs was configured incorrectly.\n",
	    progname
	    );
    exit(1);
  }
}
static void test_gc () {
  uniq(
       GC_malloc(12), GC_malloc(12), GC_malloc(12), (GC_gcollect(),GC_malloc(12)),
       GC_malloc(12), GC_malloc(12), GC_malloc(12), (GC_gcollect(),GC_malloc(12)),
       GC_malloc(12), GC_malloc(12), GC_malloc(12), (GC_gcollect(),GC_malloc(12)),
       GC_malloc(12), GC_malloc(12), GC_malloc(12), (GC_gcollect(),GC_malloc(12)),
       GC_malloc(12), GC_malloc(12), GC_malloc(12), (GC_gcollect(),GC_malloc(12)),
       (void *)0);
}

static void init_gmp() {
     mp_set_memory_functions(GC_malloc1,GC_realloc3,GC_free2);
     }

void M2inits() {
  init_gc();
  test_gc();
  init_gmp();
}
