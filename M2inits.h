#ifdef __cplusplus
extern "C" {
#endif

extern void M2inits(), M2inits1(), M2inits2();
extern void enterFactory(), enterM2();
extern int M2inits_run;

  /*extern void *	(*__gmp_allocate_func) (unsigned);*/
  /*extern void *	(*__gmp_reallocate_func) (void *, unsigned, unsigned);*/
  /*extern void	(*__gmp_free_func) (void *, unsigned);*/

extern void *	(*__gmp_allocate_func) (size_t);
extern void *	(*__gmp_reallocate_func) (void *, size_t, size_t);
extern void	(*__gmp_free_func) (void *, size_t);

#ifdef __cplusplus
}
#endif
