/* Copyright 1997 by Daniel R. Grayson */

#ifdef MEMDEBUG

#if 0

  I have phased use of this file out, with these changes (and this one):

	Index: configure.ac
	===================================================================
	--- configure.ac	(revision 3701)
	+++ configure.ac	(working copy)
	@@ -52,9 +52,9 @@

	 AC_ARG_VAR(CFLAGS,[C compiler flags, default: -g -O2])

	-AC_ARG_VAR(LOADLIBES,[first path for linker to search for libraries])
	+AC_ARG_VAR(LOADLIBES,[first list of libraries to link against])
	 test "$LOADLIBES"  && echo "   LOADLIBES=$LOADLIBES"
	-AC_ARG_VAR(LDLIBS,[second path for linker to search for libraries])
	+AC_ARG_VAR(LDLIBS,[second list of libraries to link against])
	 test "$LDLIBS"     && echo "   LDLIBS=$LDLIBS"
	 AC_ARG_VAR(LIBS,[path for linker to search for libraries during configuration])
	 test "$LIBS"     && echo "   LIBS=$LIBS"
	@@ -150,9 +150,6 @@
	 AC_SUBST(STATIC)
	 AC_ARG_ENABLE(static, AC_HELP_STRING(--enable-static,enable static linking), STATIC="$enableval", STATIC=no)

	-AC_SUBST(MEMDEBUG)
	-AC_ARG_ENABLE(memdebug, AC_HELP_STRING(--enable-memdebug,enable memory allocation debugging), MEMDEBUG="$enableval", MEMDEBUG=no)
	-
	 AC_SUBST(OPTIMIZE)
	 AC_ARG_ENABLE(optimize, AC_HELP_STRING(--disable-optimize,disable optimization), OPTIMIZE="$enableval", OPTIMIZE=yes)

	Index: Macaulay2/d/Makefile.in
	===================================================================
	--- d/Makefile.in	(revision 3699)
	+++ d/Makefile.in	(working copy)
	@@ -50,7 +50,7 @@
	 endif

	 OPTCFILES += M2inits1.cc M2inits2.cc factory_init1.cc factory_init2.cc
	-CFILES += M2inits.c M2types.c M2mem.c scclib.c M2lib.c gmp_aux.c memdebug.c gdbm_interface.c gmp_memory.c
	+CFILES += M2inits.c M2types.c M2mem.c scclib.c M2lib.c gmp_aux.c gdbm_interface.c gmp_memory.c
	 CFILES += factory_allocator.c main.c
	 SRCFILES += $(OPTCFILES) $(CFILES)
	 M2_OBJECTS += $(CFILES:.c=.o)
	@@ -72,7 +72,7 @@

	 ## *.h files

	-HFILES := M2types.h getpagesize.h memdebug.h types.h M2inits.h M2mem.h debug.h M2mem2.h
	+HFILES := M2types.h getpagesize.h types.h M2inits.h M2mem.h debug.h M2mem2.h
	 SRCFILES += $(HFILES)

	 ## *.d files
	@@ -198,9 +198,8 @@
		FLAGS="$(M2_LDFLAGS) $(M2_LOADLIBES) $(M2_LDLIBS)" @srcdir@/configure

	 M2types.o : M2types.h
	-M2lib.o scclib.o : compat.h ../c/compat.h ../c/compat.c types.h memdebug.h
	+M2lib.o scclib.o : compat.h ../c/compat.h ../c/compat.c types.h
	 gmp_int.o M2lib.o scclib.o gdbm_interface.o gc_cpp.o version.o M2inits.o debug.o : ../../include/config.h
	-memdebug.o scclib.o actors5.o : memdebug.h

	 clean::; rm -f startup.c
	 SSTRING := -e 's/\\/\\\\/g' -e 's/"/\\"/g' -e 's/\(.*\)/"\1\\n"/'
	Index: Macaulay2/d/factory_allocator.c
	===================================================================
	--- d/factory_allocator.c	(revision 3699)
	+++ d/factory_allocator.c	(working copy)
	@@ -1,7 +1,6 @@
	 #include <stdlib.h>
	 #include <unistd.h>
	 #include <gc.h>
	-#include "memdebug.h"
	 #include "debug.h"
	 #include "M2mem.h"
	 #include <string.h>
	Index: Macaulay2/d/M2types.h
	===================================================================
	--- d/M2types.h	(revision 3699)
	+++ d/M2types.h	(working copy)
	@@ -73,7 +73,6 @@
	 #define sizeofarray(s,len) (sizeof(*s) - sizeof(s->array) + (len)*sizeof(s->array[0]))

	 #include <gc.h>
	-#include "memdebug.h"
	 extern void dummy_GC_warn_proc(char *, GC_word);

	 #endif
	Index: Macaulay2/d/debug.c
	===================================================================
	--- d/debug.c	(revision 3699)
	+++ d/debug.c	(working copy)
	@@ -2,7 +2,6 @@
	 #include <unistd.h>
	 #include <stdio.h>
	 #include <gc.h>
	-#include "memdebug.h"
	 #include "config.h"
	 #include "debug.h"
	 #include <gmp.h>
	Index: Macaulay2/d/gmp_init.cc
	===================================================================
	--- d/gmp_init.cc	(revision 3699)
	+++ d/gmp_init.cc	(working copy)
	@@ -1,5 +1,4 @@
	 #include <gc.h>
	-#include "memdebug.h"
	 #include "gmp_init.h"
	 #include "config.h"

	Index: Macaulay2/d/M2inits.c
	===================================================================
	--- d/M2inits.c	(revision 3699)
	+++ d/M2inits.c	(working copy)
	@@ -9,7 +9,6 @@
	 #include "M2types.h"
	 #include "M2inits.h"
	 #include "M2mem.h"
	-#include "memdebug.h"
	 #include "debug.h"
	 #include "gmp_init.h"
	 #define TRUE 1
	@@ -25,7 +24,7 @@
	      extern char *get_etext(), *get_end();
	      GC_add_roots(get_etext(),get_end());
	 #endif
	-     GC_all_interior_pointers = TRUE; /* especially important if MEMDEBUG is on, call first; gc is compiled by default with this on */
	+     GC_all_interior_pointers = TRUE; /* gc is now compiled by default with this on */
	 #if 0 /* commented out, because we haven't tested this value lately */
	      GC_free_space_divisor = 2;
	 #endif
	Index: Macaulay2/e/table.h
	===================================================================
	--- e/table.h	(revision 3699)
	+++ e/table.h	(working copy)
	@@ -9,7 +9,6 @@
	 /*these next lines added by MES, July 2002, to use our gc routines..*/
	 #include <gc.h>
	 #include "../d/M2mem.h"
	-#include "../d/memdebug.h"
	 #define ALLOC getmem
	 #define  NEW(p) ((p) = (void *) ALLOC((long)sizeof *(p)))
	 #define FREE(ptr) ((void)(GC_FREE((ptr)), (ptr) = 0))
	Index: Macaulay2/e/newdelete.hpp
	===================================================================
	--- e/newdelete.hpp	(revision 3699)
	+++ e/newdelete.hpp	(working copy)
	@@ -4,7 +4,6 @@
	 // get declarations of outofmem and getmem
	 #include "../d/M2mem.h"
	 #include <gc.h>
	-#include "../d/memdebug.h"
	 #include "../d/debug.h"

	 // this replaces all uses of the construction "new T[n]":

#endif


/* note: the debugging facilities in this file partially conflict with
   the debugging facilities gc.h provides, since when DEBUG is
   defined, it records the location of the calls to GC_malloc.  In
   fact, we bypass their debugging entirely, by calling the functions
   instead of the macros.
*/

/*

    Here is how you might use this.  (Turn it on by configuring with --enable-memdebug.)

	(gdb) run
	Starting program: /home/dan/local/src/M2/tmp/Macaulay2/bin/M2 -q --no-loaddata
	[New Thread 16384 (LWP 5498)]
	[Switching to Thread 16384 (LWP 5498)]

	Breakpoint 1, trap () at ../../../Macaulay2/d/debug.c:7

    We always set a breakpoint in trap().

    I want to look at the millionth memory allocation:

	(gdb) set trapset=1000000

	(gdb) c
	Continuing.
	Macaulay 2, version 0.9.5
	--package Main installed

	Breakpoint 1, trap () at ../../../Macaulay2/d/debug.c:7
	(gdb) up
	#1  0x0804d7d9 in trapchk (p=0x998cfd0) at ../../../Macaulay2/d/debug.c:11
	(gdb) up
	#2  0x08050b88 in M2_debug_malloc_atomic (size=19)
	    at ../../../Macaulay2/d/memdebug.c:110
	(gdb) up
	#3  0x0804df20 in getmem_atomic (n=19) at ../../../Macaulay2/d/M2mem.c:34
	(gdb) up
	#4  0x0804dd86 in strings_join (x=0x83dde80, y=0x9cf54e0)
	    at ../../../Macaulay2/d/M2types.c:119
	(gdb) up
	#5  0x08118521 in strings_plus_ (s_1=0x83dde80, t=0x9cf54e0) at strings.d:21
	(gdb) up
	#6  0x0809ff29 in presentfun (e_33={type_ = 34, ptr_ = 0x9cf54e0})
	    at actors4.d:820
	(gdb) up
	#7  0x080d0441 in evaluate_apply_4 (f_12={type_ = 5, ptr_ = 0x83e08b0}, e_1=
	      {type_ = 34, ptr_ = 0x9cf54e0}) at evaluate.d:585

    Let's say I'm suddenly interested in this pointer, f_12;

    I have a routine that will tell me the size of the memory area:

	(gdb) p M2_debug_size(f_12.ptr_)
	$2 = 8

    and a variable that tells how bytes are appended in front

	(gdb) p front
	$3 = 16

    and in the rear:

	(gdb) p rear
	$4 = 16

    So now look at memory, including the two fences:

	(gdb) x/10x f_12.ptr_-front
	0x83e08a0:	0x00001193	0x00000008	0xaaaaaaaa	0xaaaaaaaa
	0x83e08b0:	0x0809fed0	0x000f43e5	0xcccccccc	0xcccccccc
	0x83e08c0:	0x00000008	0x00001193

    The words 0xaaaaaaaa are the (intact) fence words in front, and the
    words 0xcccccccc are the fence words behind, while the memory is
    active.  They get changed to 0xa0a0a0a0 and to 0xc0c0c0c0 when the
    memory is freed.

    The data bytes themselves are initialized to 0xbbbbbbbb upon
    allocation, and changed to 0xb0b0b0b0 when the memory is freed.

    The two copies of 0x00001193 are the sequence number, and the two
    copies of 0x00000008 are the size.  They should agree.

*/

#include "config.h"
#if defined(HAVE_GC_GC_H)
#include <gc/gc.h>
#elif defined(HAVE_GC_H)
#include <gc.h>
#else
#error missing include file gc.h
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#define MEMDEBUG_INTERNAL
#include "memdebug.h"
#include "debug.h"
#include "M2mem.h"

#define FREE_DELAY       10
#define FENCE_INTS 	 2
#define FRONT_FENCE      0xaaaaaaaa
#define FRONT_FENCE_GONE 0xa0a0a0a0
#define BODY_PART        0xbbbbbbbb
#define BODY_PART_GONE   0xb0b0b0b0
#define REAR_FENCE       0xcccccccc
#define REAR_FENCE_GONE  0xc0c0c0c0

struct FRONT {
     int trapcount;
     size_t size;
     unsigned int fence[FENCE_INTS];
     };

struct REAR {
     unsigned int fence[FENCE_INTS];
     size_t size;
     int trapcount;
     };

int front = sizeof(struct FRONT); /* available to the user in gdb */
int rear  = sizeof(struct FRONT);

void *delay_chain[FREE_DELAY];
int delay_chain_index;

size_t M2_debug_size(void *p) {
     struct FRONT *f;
     if (p == NULL) return 0;
     f = (struct FRONT *)(p - sizeof(struct FRONT));
     return f->size;
}

void* M2_debug_malloc(size_t size) {
     struct FRONT *f;
     char *p;
     struct REAR *r;
     int i;
     int INTS_BODY = (size + sizeof(int) - 1)/sizeof(int);
     f = (struct FRONT *)GC_malloc( sizeof(struct FRONT) + sizeof(int)*INTS_BODY + sizeof(struct REAR) );
     if (f == NULL) outofmem();
     p = (char *)f + sizeof(struct FRONT);
     r = (struct REAR *)(p + sizeof(int)*INTS_BODY);
     f->size = r->size = size;
     for (i=0; i<FENCE_INTS; i++) f->fence[i] = FRONT_FENCE;
     for (i=0; i<INTS_BODY; i++) ((int *)p)[i] = BODY_PART;
     for (i=0; i<FENCE_INTS; i++) r->fence[i] = REAR_FENCE;
     f->trapcount = r->trapcount = trapcount+1;
     trapchk(p);			/* trapchk increments trapcount before possibly calling trap() -- set your breakpoint in trap() */
     return p;
     }

void* M2_debug_malloc_uncollectable(size_t size) {
     struct FRONT *f;
     char *p;
     struct REAR *r;
     int i;
     int INTS_BODY = (size + sizeof(int) - 1)/sizeof(int);
     f = (struct FRONT *)GC_malloc_uncollectable(
	  sizeof(struct FRONT) + sizeof(int)*INTS_BODY + sizeof(struct REAR)
	  );
     if (f == NULL) outofmem();
     p = (char *)f + sizeof(struct FRONT);
     r = (struct REAR *)(p + sizeof(int)*INTS_BODY);
     f->size = r->size = size;
     for (i=0; i<FENCE_INTS; i++) f->fence[i] = FRONT_FENCE;
     for (i=0; i<INTS_BODY; i++) ((int *)p)[i] = BODY_PART;
     for (i=0; i<FENCE_INTS; i++) r->fence[i] = REAR_FENCE;
     f->trapcount = r->trapcount = trapcount+1;
     trapchk(p);
     return p;
     }

void* M2_debug_malloc_atomic(size_t size) {
     struct FRONT *f;
     char *p;
     struct REAR *r;
     int i;
     int INTS_BODY = (size + sizeof(int) - 1)/sizeof(int);
     f = (struct FRONT *)GC_malloc_atomic( sizeof(struct FRONT) + sizeof(int)*INTS_BODY + sizeof(struct REAR) );
     if (f == NULL) outofmem();
     p = (void *)f + sizeof(struct FRONT);
     r = (struct REAR *)(p + sizeof(int)*INTS_BODY);
     f->size = r->size = size;
     for (i=0; i<FENCE_INTS; i++) f->fence[i] = FRONT_FENCE;
     for (i=0; i<INTS_BODY; i++) ((int *)p)[i] = BODY_PART;
     for (i=0; i<FENCE_INTS; i++) r->fence[i] = REAR_FENCE;
     f->trapcount = r->trapcount = trapcount+1;
     trapchk(p);
     return p;
     }

void* M2_debug_malloc_atomic_uncollectable(size_t size) {
     struct FRONT *f;
     char *p;
     struct REAR *r;
     int i;
     int INTS_BODY = (size + sizeof(int) - 1)/sizeof(int);
     f = (struct FRONT *)GC_malloc_atomic_uncollectable( sizeof(struct FRONT) + sizeof(int)*INTS_BODY + sizeof(struct REAR) );
     if (f == NULL) outofmem();
     p = (void *)f + sizeof(struct FRONT);
     r = (struct REAR *)(p + sizeof(int)*INTS_BODY);
     f->size = r->size = size;
     for (i=0; i<FENCE_INTS; i++) f->fence[i] = FRONT_FENCE;
     for (i=0; i<INTS_BODY; i++) ((int *)p)[i] = BODY_PART;
     for (i=0; i<FENCE_INTS; i++) r->fence[i] = REAR_FENCE;
     f->trapcount = r->trapcount = trapcount+1;
     trapchk(p);
     return p;
     }

static void volatile smashed(void) {
     fprintf(stderr,"smashed object found\n");
     trap();
     _exit(1);
     }

void M2_debug_free(void *p) {
     struct FRONT *f;
     struct REAR *r;
     int INTS_BODY, i, _trapcount;
     size_t size;
     if (p == NULL) return;
     f = (struct FRONT *)(p - sizeof(struct FRONT));
     size = f->size;
     INTS_BODY = (size + sizeof(int) - 1)/sizeof(int);
     r = (struct REAR *)(p + sizeof(int)*INTS_BODY);
     _trapcount = f->trapcount;
     if (r->trapcount != _trapcount || r->size != size) smashed();
     for (i=0; i<FENCE_INTS; i++) if (f->fence[i] != FRONT_FENCE) smashed();
     for (i=0; i<FENCE_INTS; i++) if (r->fence[i] != REAR_FENCE) smashed();
     if (_trapcount == trapset) trap();
     trapchk(p);
     for (i=0; i<FENCE_INTS; i++) f->fence[i] = FRONT_FENCE_GONE;
     for (i=0; i<INTS_BODY; i++) ((int *)p)[i] = BODY_PART_GONE;
     for (i=0; i<FENCE_INTS; i++) r->fence[i] = REAR_FENCE_GONE;
#if FREE_DELAY != 0
     if (delay_chain[delay_chain_index] != NULL) {
	  GC_free(delay_chain[delay_chain_index]);
	  }
     delay_chain[delay_chain_index] = (void *)f;
     delay_chain_index ++;
     if (delay_chain_index == FREE_DELAY) delay_chain_index = 0;
#else
     GC_free(f);
#endif
     }

void* M2_debug_realloc(void *old, size_t size) {
     void *new = M2_debug_malloc(size);
     size_t oldsize = M2_debug_size(old);
     if (new == NULL) outofmem();
     memcpy(new,old,size < oldsize ? size : oldsize);
     return new;
     }

#endif
