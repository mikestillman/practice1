/* Copyright 1997 by Daniel R. Grayson */

#include <gc.h>
#include <stdio.h>

extern void outofmem(void);
extern void trap(void);

#define FREE_DELAY       0
#define FENCE_INTS 	 2
#define FRONT_FENCE      0xaaaaaaaa
#define FRONT_FENCE_GONE 0xa0a0a0a0
#define BODY_PART        0xbbbbbbbb
#define BODY_PART_GONE   0xb0b0b0b0
#define REAR_FENCE       0xcccccccc
#define REAR_FENCE_GONE  0xc0c0c0c0
int trapset = 0;
void *trapaddr = 0;
int trapcount = 0;
void *delay_chain[FREE_DELAY];
int delay_chain_index;

typedef struct FRONT {
     int trapcount;
     size_t size;
     unsigned int fence[FENCE_INTS];
     } front;

typedef struct REAR {
     unsigned int fence[FENCE_INTS];
     size_t size;
     int trapcount;
     } rear;

void* debug_new(size_t size) {
     front *f;
     void *p;
     rear *r;
     int i;
     int INTS_BODY = (size + sizeof(int) - 1)/sizeof(int);
     f = (front *)GC_MALLOC(
	  sizeof(front) + sizeof(int)*INTS_BODY + sizeof(rear)
	  );
     if (f == NULL) outofmem();
     p = (void *)f + sizeof(front);
     r = (rear *)(p + sizeof(int)*INTS_BODY);
     f->size = r->size = size;
     ++trapcount;
     f->trapcount = r->trapcount = trapcount;
     for (i=0; i<FENCE_INTS; i++) f->fence[i] = FRONT_FENCE;
     for (i=0; i<INTS_BODY; i++) ((int *)p)[i] = BODY_PART;
     for (i=0; i<FENCE_INTS; i++) r->fence[i] = REAR_FENCE;
     if (trapcount == trapset || p == trapaddr) trap();
     return p;
     }

void* debug_new_uncollectable(size_t size) {
     front *f;
     void *p;
     rear *r;
     int i;
     int INTS_BODY = (size + sizeof(int) - 1)/sizeof(int);
     f = (front *)GC_MALLOC_UNCOLLECTABLE(
	  sizeof(front) + sizeof(int)*INTS_BODY + sizeof(rear)
	  );
     if (f == NULL) outofmem();
     p = (void *)f + sizeof(front);
     r = (rear *)(p + sizeof(int)*INTS_BODY);
     f->size = r->size = size;
     ++trapcount;
     f->trapcount = r->trapcount = trapcount;
     for (i=0; i<FENCE_INTS; i++) f->fence[i] = FRONT_FENCE;
     for (i=0; i<INTS_BODY; i++) ((int *)p)[i] = BODY_PART;
     for (i=0; i<FENCE_INTS; i++) r->fence[i] = REAR_FENCE;
     if (trapcount == trapset || p == trapaddr) trap();
     return p;
     }

void* debug_new_atomic(size_t size) {
     front *f;
     void *p;
     rear *r;
     int i;
     int INTS_BODY = (size + sizeof(int) - 1)/sizeof(int);
     f = (front *)GC_MALLOC_ATOMIC(
	  sizeof(front) + sizeof(int)*INTS_BODY + sizeof(rear)
	  );
     if (f == NULL) outofmem();
     p = (void *)f + sizeof(front);
     r = (rear *)(p + sizeof(int)*INTS_BODY);
     f->size = r->size = size;
     ++trapcount;
     f->trapcount = r->trapcount = trapcount;
     for (i=0; i<FENCE_INTS; i++) f->fence[i] = FRONT_FENCE;
     for (i=0; i<INTS_BODY; i++) ((int *)p)[i] = BODY_PART;
     for (i=0; i<FENCE_INTS; i++) r->fence[i] = REAR_FENCE;
     if (trapcount == trapset || p == trapaddr) trap();
     return p;
     }

static void volatile smashed() {
     fprintf(stderr,"smashed object found\n");
     trap();
     _exit(1);
     }

void debug_delete(void *p) {
     front *f;
     rear *r;
     int INTS_BODY, i, _trapcount;
     size_t size;
     if (p == NULL) return;
     f = (front *)(p - sizeof(front));
     size = f->size;
     INTS_BODY = (size + sizeof(int) - 1)/sizeof(int);
     r = (rear *)(p + sizeof(int)*INTS_BODY);
     _trapcount = f->trapcount;
     if (r->trapcount != _trapcount || r->size != size) smashed();
     for (i=0; i<FENCE_INTS; i++) if (f->fence[i] != FRONT_FENCE) smashed();
     for (i=0; i<FENCE_INTS; i++) if (r->fence[i] != REAR_FENCE) smashed();
     if (_trapcount == trapset || p == trapaddr) trap();
     for (i=0; i<FENCE_INTS; i++) f->fence[i] = FRONT_FENCE_GONE;
     for (i=0; i<INTS_BODY; i++) ((int *)p)[i] = BODY_PART_GONE;
     for (i=0; i<FENCE_INTS; i++) r->fence[i] = REAR_FENCE_GONE;
#if FREE_DELAY != 0
     if (delay_chain[delay_chain_index] != NULL) {
	  GC_FREE(delay_chain[delay_chain_index]);
	  }
     delay_chain[delay_chain_index] = (void *)f;
     delay_chain_index ++;
     if (delay_chain_index == FREE_DELAY) delay_chain_index = 0;
#else
     GC_FREE(f);
#endif
     }
