/*		Copyright 1994 by Daniel R. Grayson		*/

#include <factoryconf.h>

const char *get_libfac_version();	/* in version.cc */

unsigned GC_version;		/* in libgc.a */
#define GC_NOT_ALPHA 0xff
#define GC_VERSION_MAJOR ((GC_version >> 16) & 0xffff)
#define GC_VERSION_MINOR ((GC_version >> 8) & 0xff)
#define GC_ALPHA_VERSION (GC_version & 0xff)

#ifndef NO_GNU_GET_LIBC_VERSION
char *gnu_get_libc_version();
#endif

#include "readline.h"
#include "types.h"

#ifdef NEWDUMPDATA
#include "../dumpdata/dumpdata.h"
#endif

#ifndef __GNUC__
#define __attribute__(x)
#endif

/* defining GDBM_STATIC makes the cygwin version work, and is irrelevant for the other versions */
#define GDBM_STATIC

#include <gdbm.h>
#define DBM_REPLACE GDBM_REPLACE
#define DBM_WRCREAT GDBM_WRCREAT
#define DBM_RD GDBM_READER
#define DBM_FILE GDBM_FILE
#define dbm_close gdbm_close
#define dbm_open gdbm_open
#define dbm_store gdbm_store
#define dbm_fetch gdbm_fetch
#define dbm_delete gdbm_delete
#define dbm_firstkey gdbm_firstkey
#define dbm_nextkey gdbm_nextkey
#define DBM_CREAT GDBM_CREAT
#define DBM_FAST GDBM_FAST

static void putstderr(char *m) {
     write(STDERR,m,strlen(m));
     write(STDERR,NEWLINE,strlen(NEWLINE));
     }

void WerrorS(char *m) {
  putstderr(m);
  exit(1);
}

void WarnS(char *m) {
  putstderr(m);
}

static char *progname;
#ifdef includeX11
Display *display;
Font font;
#endif

bool system_interrupted = FALSE;
bool system_interruptPending = FALSE;
bool system_interruptShield = FALSE;
bool system_alarmed = FALSE;

#ifdef FACTORY
extern int libfac_interruptflag;
#endif

#if 0
void unblock(int sig)
{
  /* following a suggestion of Tom Hageman  <tom@basil.icce.dev.rug.null.nl>  [NeXTmail/Mime OK] */
  sigset_t s;
  sigemptyset(&s);
  sigaddset(&s, sig);
  sigprocmask(SIG_UNBLOCK, &s, NULL);
}
#endif

static void alarm_handler(int sig)
{
     system_alarmed = TRUE;
     if (system_interruptShield) system_interruptPending = TRUE;
     else {
	  system_interrupted = TRUE;
#     	  ifdef FACTORY
     	  libfac_interruptflag = TRUE;
#     	  endif
	  }
#ifdef SIGALRM
     signal(SIGALRM,alarm_handler);
#endif
     }

extern bool interp_StopIfError;

#if defined(__MWERKS__) || (defined(_WIN32) && !defined(__CYGWIN__))
#define sigjmp_buf jmp_buf
#define siglongjmp(j,c) longjmp(j,c)
#define sigsetjmp(j,m) setjmp(j)
#endif

static sigjmp_buf loaddata_jump, out_of_memory_jump, abort_jump;
static bool out_of_memory_jump_set = FALSE, abort_jump_set = FALSE;

static void interrupt_handler(int sig)
{
     if (system_interrupted || system_interruptPending) {
	  if (isatty(STDIN) && isatty(STDOUT)) while (TRUE) {
	       char buf[10];
	       printf("\nAbort (y/n)? ");
	       fflush(stdout);
	       if (NULL == fgets(buf,sizeof(buf),stdin)) {
		    fprintf(stderr,"exiting\n");
		    exit(1);
	            }
	       if (buf[0]=='y' || buf[0]=='Y') {
     		    trap();
		    if (!interp_StopIfError && abort_jump_set) {
     	  		 fprintf(stderr,"returning to top level\n");
     	  		 fflush(stderr);
			 system_interrupted = FALSE;
#     	   	     	 ifdef FACTORY
			 libfac_interruptflag = FALSE;
#     	   	     	 endif
			 system_interruptPending = FALSE;
			 system_interruptShield = FALSE;
			 system_alarmed = FALSE;
     	  		 siglongjmp(abort_jump,1);
			 }
		    else {
			 fprintf(stderr,"exiting\n");
		    	 exit(1);
			 }
		    }
	       else if (buf[0]=='n' || buf[0]=='N') {
		    break;
		    }
	       }
	  else {
     	       trap();
	       exit(1);
	       }
	  }
     else {
	  if (system_interruptShield) system_interruptPending = TRUE;
	  else {
	       if (!isatty(STDIN)) {
		    fprintf(stderr,"interrupted%s",NEWLINE);
		    exit(1);
	       }
	       system_interrupted = TRUE;
#     	       ifdef FACTORY
	       libfac_interruptflag = TRUE;
#     	       endif
	       }
	  }
     signal(SIGINT,interrupt_handler);
     }

void outofmem(){
     static int count = 0;
     if (!interp_StopIfError && out_of_memory_jump_set && count++ < 5) {
     	  fprintf(stderr,"out of memory, returning to top level");
     	  fflush(stderr);
     	  siglongjmp(out_of_memory_jump,1);
	  }
     else {
     	  fprintf(stderr,"out of memory, exiting\n");
	  exit(1);
	  }
     }

static struct COUNTER { 
     int *count; char *filename; int lineno; char *funname;
     struct COUNTER *next;
     } *counters = NULL;

int register_fun(count, filename, lineno, funname)
int *count;
char *filename;
int lineno;
char *funname;
{
     struct COUNTER *p = (struct COUNTER *) getmem(sizeof(struct COUNTER));
     p->count = count;
     p->filename = filename;
     p->lineno = lineno;
     p->funname = funname;
     p->next = counters;
     counters = p;
     return 0;
     }

int system_returncode;

M2_string actors5_CCVERSION;
M2_string actors5_VERSION;
M2_string actors5_OS;
M2_string actors5_ARCH;
M2_string actors5_NODENAME;
M2_string actors5_REL;
M2_string actors5_DATE;
M2_string actors5_TIME;
M2_string actors5_GCVERSION;
M2_string actors5_GMPVERSION;
M2_string actors5_LIBFACVERSION;
M2_string actors5_FACTORYVERSION;
M2_bool actors5_DUMPDATA;
M2_bool actors5_FACTORY;
M2_bool actors5_MP;

M2_stringarray system_envp;
M2_stringarray system_argv;
M2_stringarray system_args;
int system_LoadDepth;

int system_randomint() {
#if 0
     extern long random();
     return random();
#else
     extern long random00();
     return random00();
#endif
     }

void initrandom(){
#if 0
     extern char *initstate();
#endif
#if 0
     static char state[32];
     unsigned int seed = time(NULL);
     initstate(seed,(void *)state,sizeof(state));
#endif
     }

#ifdef __DJGPP__
void system_stime(){
     extern double start_timer();
     start_timer();
     }
double system_etime(){
     double return_elapsed_time(double);
     return return_elapsed_time(0.);
     }
#elif !defined(CLOCKS_PER_SEC) || CLOCKS_PER_SEC > 10000
static struct itimerval it;
#define INITVAL 1000000		/* a million seconds is very long */
void system_stime(){
     it.it_value.tv_sec = INITVAL;
     it.it_value.tv_usec = 0;
     (void) setitimer(ITIMER_VIRTUAL,&it,(struct itimerval *)NULL);
     }
double system_etime(){
     long sec,usec;
     (void) getitimer(ITIMER_VIRTUAL,&it);
     sec = INITVAL - it.it_value.tv_sec;
     usec =   0    - it.it_value.tv_usec;
     if (usec<0) usec+=1000000, sec-=1;
     return sec + usec / 1000000.;
     }
#else
				/* ANSI C */
static clock_t start_time;
void system_stime(){
     start_time = clock();
     }
double system_etime(){
     return (double)(clock()-start_time) / CLOCKS_PER_SEC;
     }
#endif

#if defined(DUMPDATA)
#if defined(__sun__) || defined(_WIN32) || defined(__CYGWIN__)
#define __environ _environ
#elif defined(__FreeBSD__)
#define __environ environ
#endif

extern char **__environ;
#endif

extern char timestamp[];
static void clean_up();

static void nop (p)		/* used below to keep variables out of registers */
void *p;
{}

#define NOTHING(p) nop((void *)p)
#define ONSTACK(p) nop((void *)&p)

#ifdef NDEBUG
static void dummy_GC_warn_proc(char *msg, GC_word arg) {
}
#endif

#if defined(__MWERKS__)

void SetMinimumStack(long minSize)
{
	long newApplLimit;

	if (minSize > LMGetDefltStack())
	{
		newApplLimit = (long) GetApplLimit()
				- (minSize - LMGetDefltStack());
		SetApplLimit((Ptr) newApplLimit);
		MaxApplZone();
	}
}

#define cMinStackSpace (512L * 1024L)
#endif

#define stringify(x) #x

#if defined(__GNUC__)
char CCVERSION[30] = "gcc" ;
#else
char CCVERSION[] = "unknown" ;
#endif

void M2_init_gmp() {
     mp_set_memory_functions(GC_malloc1,GC_realloc3,GC_free2);
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

int main(argc,argv)
int argc; 
char **argv;
{
     char dummy;
     extern char *GC_stackbottom;
     char *p, **x;
     char **saveenvp = NULL;
     int envc = 0;
     static int old_collections = 0;
     char **saveargv;
     int i, n;
     void main_inits();
     static void *reserve = NULL;
     extern void actors4_setupargv();
     extern void interp_process(), interp_process2(), interp_topLevel();

#if defined(__MWERKS__) && !defined(__BUILDING_MPW__)
	int n_mac_args = 4;
	char *mac_args[4] = {"-e phase=1","setup.m2","-e phase=0","-e runStartFunctions()"};
	char *mac_argv[5] = {"M2", "-e phase=1","setup.m2","-e phase=0","-e runStartFunctions()"};
	argv = mac_argv;
	argc = 5;
#endif

	//MES     GC_stackbottom = &dummy;

     ONSTACK(saveenvp);

#if defined(_WIN32)
     if (argv[0][0]=='/' && argv[0][1]=='/' && argv[0][3]=='/') {
       /* we must be in Windows 95 or NT running under CYGWIN32, and
	  the path to our executable has been mangled from D:/a/b/c
	  into //D/a/b/c */
       argv[0][0] = argv[0][2];
       argv[0][1] = ':';
       strcpy(argv[0]+2,argv[0]+3);
     }
     {
       /* change all \ in path to executable to / */
       char *p;
       for (p=argv[0]; *p; p++) if (*p == '\\') *p = '/';
     }
#endif

     out_of_memory_jump_set = FALSE;
     abort_jump_set = FALSE;

#if defined(_WIN32) && !defined(__CYGWIN32__)
     _setmode(STDIN ,_O_BINARY);
     _setmode(STDOUT,_O_BINARY);
     _setmode(STDERR,_O_BINARY);
#endif

#ifdef __DJGPP__
     __file_handle_modes[STDIN ] = O_BINARY;
     __file_handle_modes[STDOUT] = O_BINARY;
     __file_handle_modes[STDERR] = O_BINARY;
#endif

#ifdef __MWERKS__
	/* Make sure we have lots and lots of stack space. 	*/
	SetMinimumStack(cMinStackSpace);
	/* Cheat and let stdio initialize toolbox for us.	*/
	/* printf("Macaulay2 for the MacOS\n"); */
     saveargv = argv;
#else
     /* save arguments on stack in case they're on the heap */
     saveargv = (char **)alloca((argc + 1)*sizeof(char *));
     for (i=0; i<argc; i++) {
	  saveargv[i] = alloca(strlen(argv[i]) + 1);
	  strcpy(saveargv[i],argv[i]);
     }
     saveargv[i] = NULL;
#endif

#if defined(DUMPDATA) && !defined(__MWERKS__) && !defined(__CYGWIN__)
     /* save environment on stack in case it's on the heap */
     for (envc=0, x=__environ; *x; x++) envc++;
     saveenvp = (char **)alloca((envc + 1)*sizeof(char *));
     for (i=0; i<envc; i++) {
	  saveenvp[i] = alloca(strlen(__environ[i]) + 1);
	  strcpy(saveenvp[i],__environ[i]);
     }
     saveenvp[i] = NULL;
#endif

#if defined(__GNUC__)
     sprintf(CCVERSION, "gcc %d.%d", __GNUC__, __GNUC_MINOR__);
#else
#endif

     for (n=1; ; n++) {
	  if (n >= argc) {
	       char buf[100];
	       if (-1 == sprintf(buf,"Macaulay 2, version %s",PACKAGE_VERSION)) {
		 putstderr("  Warning: perhaps stdio is not initialized properly by _IO_init.");
	       }
	       putstderr(buf);
	       putstderr("--Copyright 1993-2002, D. R. Grayson and M. E. Stillman");
	       putstderr("--Singular-Factory " 
		    FACTORYVERSION
		    ", copyright 1993-2001, G.-M. Greuel, et al.");
	       sprintf(buf,"--Singular-Libfac %s, copyright 1996-2001, M. Messollen",
		    get_libfac_version());
	       putstderr(buf);
#              ifdef PORTA
	       sprintf(buf,"--PORTA %s, copyright 1997, T. Christof and A. Loebel",PORTA_VERSION);
	       putstderr(buf);
#              endif
# if 0
	       if (GC_ALPHA_VERSION == GC_NOT_ALPHA) {
		 sprintf(buf,
			 "--GC %d.%d, copyright, H-J. Boehm, A. J. Demers",
			 GC_VERSION_MAJOR, GC_VERSION_MINOR);
	       }
	       else {
		 sprintf(buf,
			 "--GC %d.%d alpha %d, copyright, H-J. Boehm, A. J. Demers",
			 GC_VERSION_MAJOR, GC_VERSION_MINOR, GC_ALPHA_VERSION);
	       }
	       putstderr(buf);
#ifndef NO_GNU_GET_LIBC_VERSION
	       sprintf(buf,"--GNU C Library (glibc-%s), copyright, Free Software Foundation", gnu_get_libc_version());
	       putstderr(buf);
#endif
	       sprintf(buf,"--GNU MP Library (gmp-%s), copyright, Free Software Foundation",__gmp_version);
	       putstderr(buf);
# endif
	       break;
       	       }
	  if (0 == strcmp(argv[n],"-silent")) break;
	  }
#if !defined(__MWERKS__)
     ONSTACK(envc);
#endif

#ifdef MEM_DEBUG
     GC_all_interior_pointers = TRUE; /* set this before using gc routines!  (see gc.h) */
#endif
     GC_free_space_divisor = 3;	/* this was intended to be used only when we are about to dump data */

     if (0 != sigsetjmp(loaddata_jump,TRUE)) {
	  char **environ0;
     	  GC_free_space_divisor = 4;
	  if (GC_stackbottom == NULL) GC_stackbottom = &dummy;
	  old_collections = GC_gc_no;
#if defined(DUMPDATA) && !defined(__MWERKS__) && !defined(__CYGWIN__)
     	  __environ = saveenvp;	/* __environ is a static variable that points
				   to the heap and has been overwritten by
				   loaddata(), thereby pointing to a previous
				   incarnation of the heap. */
	  /* Make a copy of the environment on the heap for '__environ'. */
	  /* In some systems, putenv() calls free() on the old item,
	     so we are careful to use malloc here, and not GC_malloc. */
	  environ0 = (char **)malloc((envc + 1)*sizeof(char *));
	  /* amazing but true:
	     On linux, malloc calls getenv to get values for tunable
	     parameters, so don't trash __environ yet.
	     */
	  if (environ0 == NULL) fatal("out of memory");
	  for (i=0; i<envc; i++) {
	       environ0[i] = malloc(strlen(saveenvp[i]) + 1);
	       if (environ0[i] == NULL) fatal("out of memory");
	       strcpy(environ0[i],saveenvp[i]);
	  }
	  environ0[i] = NULL;
	  __environ = environ0;
#endif
	  }

     system_stime();
     signal(SIGINT,interrupt_handler);

#ifdef SIGALRM
     signal(SIGALRM,alarm_handler);
#endif

#ifdef SIGPIPE
     signal(SIGPIPE, SIG_IGN);
#endif

     trap();
     progname = saveargv[0];
     for (p=progname; *p; p++) if (*p=='/') progname = p+1;

     if (GC_stackbottom == NULL) GC_stackbottom = &dummy;
     M2_init_gmp();
     initrandom();
     system_newline = tostring(newline);
     actors5_CCVERSION = tostring(CCVERSION);
     actors5_VERSION = tostring(PACKAGE_VERSION);
     actors5_OS = tostring(OS);
     actors5_ARCH = tostring(ARCH);
     actors5_NODENAME = tostring(NODENAME);
     actors5_REL = tostring(REL);
     actors5_LIBFACVERSION = tostring(get_libfac_version());
     actors5_FACTORYVERSION = tostring(FACTORYVERSION);
     actors5_DATE = tostring(current_date);
     actors5_TIME = tostring(current_time);
#ifdef DUMPDATA
     actors5_DUMPDATA = TRUE;
#else
     actors5_DUMPDATA = FALSE;
#endif
     {
	  char buf[100];
	  unsigned major, minor, alpha;
	  major = GC_version >> 16;
	  minor = (GC_version >> 8) & 0xff;
	  alpha = GC_version & 0xff;
	  if (alpha == 0xff) {
	       sprintf(buf,"%d.%d", major, minor);
	       }
	  else {
	       sprintf(buf,"%d.%d alpha %d", major, minor, alpha);
	       }
	  actors5_GCVERSION = tostring(buf);
	  }
     actors5_GMPVERSION = tostring(__gmp_version);
     system_envp = tostrings(envc,saveenvp);
     system_argv = tostrings(argc,saveargv);
#if defined(__MWERKS__) && !defined(__BUILDING_MPW__)
	 system_args = tostrings(n_mac_args,mac_args);
#else
     system_args = tostrings(argc == 0 ? 0 : argc - 1, saveargv + 1);
#endif

#ifdef includeX11
     display = XOpenDisplay(NULL);
     font = XLoadFont(display,"6x13");
#endif
     main_inits();
     actors4_setupargv();
     if (reserve == NULL) {
	  reserve = GC_MALLOC_ATOMIC(102400);
	  }
     sigsetjmp(abort_jump,TRUE);
     abort_jump_set = TRUE;

     /* setup_readline(); */

     if (sigsetjmp(out_of_memory_jump,TRUE)) {
	  if (reserve != NULL) {
	       GC_FREE(reserve);
	       reserve = NULL;
	       }
#if 0
	  fprintf(stderr,", collecting garbage");
	  fflush(stderr);
	  GC_gcollect();
#endif
	  fprintf(stderr,"\n");
	  fflush(stderr);
          interp_topLevel();
	  }
     else {
          out_of_memory_jump_set = TRUE;
          interp_process();
     }
     clean_up();
#if 0
     fprintf(stderr,"heap size = %d, divisor = %ld, collections = %ld\n", 
	  GC_get_heap_size(), GC_free_space_divisor, GC_gc_no-old_collections);
#endif
     exit(system_returncode);
     return(system_returncode);
     }

static void close_all_dbms();

static void clean_up() {
     close_all_dbms();
     while (pre_final_list != NULL) {
	  pre_final_list->final();
	  pre_final_list = pre_final_list->next;
	  }
     while (final_list != NULL) {
	  final_list->final();
	  final_list = final_list->next;
	  }
     trap();
     }

void system_exit(x)
int x;
{
     clean_up();
     exit(x);
     }
     
int SPINCOUNT = 10000;
int spincount = 10000;		/* this one is decremented during loops */

void spincursor(){
     spincount = SPINCOUNT;
#ifdef __MWERKS__
     SpinCursor();
#endif
     }

int system_setspinspan(int n){
  int result = SPINCOUNT;
  SPINCOUNT = spincount = n;
  return result;
}

void system_spincursor(){
#ifdef __MWERKS__
  if (--spincount == 0) {
    SpinCursor();
    spincount = SPINCOUNT;
  }
#endif
}
void scclib__prepare(){}

extern int etext, end;

#ifndef PAGESIZE
#define PAGESIZE 4096
#endif

int system_dumpdata(datafilename)
M2_string datafilename;
{
     /* this routine should keep its data on the stack */
#ifndef DUMPDATA
     return ERROR;
#else
     bool haderror = FALSE;
     char *datafilename_s = tocharstar(datafilename);
     if (ERROR == dumpdata(datafilename_s)) haderror = TRUE;
     GC_FREE(datafilename_s);
     return haderror ? ERROR : OKAY;
#endif
     }

#undef min

int min(int i, int j) {
     return i<j ? i : j;
     }

#if defined(DUMPDATA) && !defined(NEWDUMPDATA)
static void extend_memory(void *newbreak) {
     if (ERROR == brk(newbreak)) {
	  char buf[200];
	  sprintf(buf,"loaddata: out of memory (extending break from 0x%p to 0x%p)",
	       sbrk(0), newbreak);
	  perror(buf);
	  _exit(1);
	  }
     }
#endif

int system_loaddata(M2_string datafilename){
#ifndef DUMPDATA
     return ERROR;
#else
     char *datafilename_s = tocharstar(datafilename);
     sigjmp_buf save_loaddata_jump;
     int LoadDepth = system_LoadDepth;
     memcpy(save_loaddata_jump,loaddata_jump,sizeof(loaddata_jump));
     if (ERROR == loaddata(datafilename_s)) return ERROR;
     memcpy(loaddata_jump,save_loaddata_jump,sizeof(loaddata_jump));
     system_LoadDepth = LoadDepth + 1;
     siglongjmp(loaddata_jump,1);
#endif
     }

/**********************************************
 *                  dbm stuff                 *
 **********************************************/

static int numfiles = 0;
static DBM_FILE *dbm_files = NULL;
static void close_all_dbms() {
     int i;
     for (i=0; i<numfiles; i++) {
	  if (dbm_files[i] != NULL) dbm_close(dbm_files[i]);
	  }
     }

int system_dbmopen(M2_string filename, bool mutable) {
     int dbm_handle;
     int flags = mutable ? DBM_WRCREAT : DBM_RD;
     int mode = 0666;
     char *FileName = tocharstar(filename);
     DBM_FILE f = dbm_open(FileName, 0, flags, mode, NULL);
     GC_FREE(FileName);
     if (f == NULL) return ERROR;
     if (numfiles == 0) {
	  int i;
	  numfiles = 10;
	  dbm_files = (DBM_FILE *) getmem(numfiles * sizeof(DBM_FILE));
	  for (i=0; i<numfiles; i++) dbm_files[i] = NULL;
	  dbm_handle = 0;
	  }
     else {
	  for (dbm_handle=0; TRUE ; dbm_handle++) {
	       if (dbm_handle==numfiles) {
		    DBM_FILE *p;
		    int j;
		    numfiles *= 2;
		    p = (DBM_FILE *) getmem(numfiles * sizeof(DBM_FILE));
		    for (j=0; j<dbm_handle; j++) p[j] = dbm_files[j];
		    dbm_files = p;
	  	    for (j=dbm_handle; j<numfiles; j++) dbm_files[j] = NULL;
		    break;
		    }
	       else if (dbm_files[dbm_handle] == NULL) break;
	       }
	  }
     dbm_files[dbm_handle] = f;
     return dbm_handle;
     }

int system_dbmclose(int handle) {
     dbm_close(dbm_files[handle]);
     dbm_files[handle] = NULL;
     return 0;
     }

static datum todatum(M2_string x) {
     datum y;
     y.dptr = x->array;
     y.dsize = x->len;
     return y;
     }

static M2_string fromdatum(datum y) {
     M2_string x;
     if (y.dptr == NULL) return NULL;
     x = (M2_string)getmem(sizeofarray(x,y.dsize));
     x->len = y.dsize;
     memcpy(x->array, y.dptr, y.dsize);
     return x;
     }

int system_dbmstore(int handle, M2_string key, M2_string content) {
     return dbm_store(dbm_files[handle],todatum(key),todatum(content),DBM_REPLACE);
     }

M2_string /* or NULL */ system_dbmfetch(int handle, M2_string key) {
     return fromdatum(dbm_fetch(dbm_files[handle],todatum(key)));
     }

int system_dbmdelete(int handle, M2_string key) {
     return dbm_delete(dbm_files[handle],todatum(key));
     }

static datum lastkey;
static bool hadlastkey = FALSE;

M2_string /* or NULL */ system_dbmfirst(int handle) {
     lastkey = dbm_firstkey(dbm_files[handle]);
     hadlastkey = TRUE;
     return fromdatum(lastkey);
     }

M2_string /* or NULL */ system_dbmnext(int handle) {
     if (hadlastkey) {
	  lastkey = dbm_nextkey(dbm_files[handle]
	       ,lastkey
	       );
	  hadlastkey = TRUE;
	  return fromdatum(lastkey);
	  }
     else {
	  return system_dbmfirst(handle);
	  }
     }

int system_dbmreorganize(int handle) {
     return gdbm_reorganize(dbm_files[handle]);
     }

M2_string system_dbmstrerror() {
     return tostring(gdbm_strerror(gdbm_errno));
     }

void C__prepare() {}

int actors4_isReady(int fd) {
#if defined(__MWERKS__) || defined(_WIN32)
     return 1;
#else
  int ret;
  static fd_set r, w, e;
  struct timeval timeout;
  FD_SET(fd,&r);
  timerclear(&timeout);
  ret = select(fd+1,&r,&w,&e,&timeout);
  FD_CLR(fd,&r);
  return ret;
#endif
}

int actors5_WindowWidth(int fd) {
#if defined(__DJGPP__) || defined(__alpha) || defined(__MWERKS__) || defined(_WIN32)
     return 0;
#else
     struct winsize x;
     ioctl(1,TIOCGWINSZ,&x);	/* see /usr/include/$SYSTEM/termios.h */
     return x.ws_col;
#endif
     }

#if 0
#include <regex.h>

regex_t regex;
M2_string last_pattern;

int actors5_rxmatch(M2_string text, M2_string pattern) {
     regmatch_t match;
     char *s_text;
     int ret;
     if (pattern != last_pattern) {
	  char *s_pattern = tocharstar(pattern);
	  ret = regcomp(&regex, s_pattern, REG_NEWLINE|REG_NOSUB);
	  GC_FREE(s_pattern);
	  if (ret != 0) return ERROR;
	  }
     s_text = tocharstar(text); /* end strings with 0's! */
     ret = regexec(&regex, s_text, 1, &match, REG_NOTEOL);
     GC_FREE(s_text);
     if (ret == 0) return match.rm_so;
     else if (ret == REG_NOMATCH) return -2;
     else return ERROR;
     }
#endif

#if defined(__DJGPP__) || defined(_WIN32)
double lgamma(double x) { return -1. ; }	/* sigh, fix later */
#endif

#if defined(__CYGWIN32__)
void abort() {
  putstderr("abort() called");
  *(int*)-1=0;
  exit(1);
}
#endif

#if defined(_WIN32) && !defined(__CYGWIN32__)
#ifndef ENOSYS
#define ENOSYS 0
#endif
int kill() { return ERROR; }
int waitpid() { return ERROR; }
int fork() { return ERROR; }
int pipe(int v[2]) { return ERROR; }
int wait() { return ERROR; }
int alarm(int i) { return ERROR ; }
int sleep(int i) { return ERROR; }
/* int getpagesize() { return 4096; } */
int brk() { return 0; }
void *sbrk(int i) { return 0; }
void *getprotobyname() { errno = ENOSYS; return 0; }
int accept() { errno = ENOSYS; return -1; }
int bind() { errno = ENOSYS; return -1; }
int listen() { errno = ENOSYS; return -1; }
int socket() { errno = ENOSYS; return -1; }
void *gethostbyname() { errno = ENOSYS; return (void *)0; }
int inet_addr() { errno = ENOSYS; return -1; }
void *getservbyname() { errno = ENOSYS; return (void *)0; }
void *authdes_create() { errno = ENOSYS; return (void *)0; }
void *xdrmem_create() { errno = ENOSYS; return (void *)0; }
int connect() { errno = ENOSYS; return -1; }
int setsockopt() { errno = ENOSYS; return -1; }
short htons(short x) { return x; }
#endif

#ifdef __MWERKS__
#ifndef ENOSYS
#define ENOSYS 0
#endif
#undef getpid
/* Added for MPW support: I can't find the routines yet! */
/* MES: 8/27/98 */
#if defined(__BUILDING_MPW__)
int system(const char * s) { return ERROR; }
char * getcwd(char * s, int n) { return ""; }
int exec(const char *s,...) { return ERROR; }
unsigned int sleep(unsigned int i) { return ERROR; }
/* End of MPW missing stuff */
#else
char * getcwd(char * s, int n) { return ""; }
#endif

int getpid(){ return ERROR; }
int dup2() { return ERROR; }
int fork() { return ERROR; }
int pipe(int v[2]) { return ERROR; }
int wait() { return ERROR; }
int alarm(int i) { return ERROR ; }
//unsigned int sleep(unsigned int i) { return ERROR; }
unsigned long getpagesize() { return 4096; }
int brk() { return 0; }
void *sbrk(int i) { return 0; }
void *getprotobyname() { errno = ENOSYS; return 0; }
int accept() { errno = ENOSYS; return -1; }
int bind() { errno = ENOSYS; return -1; }
int listen() { errno = ENOSYS; return -1; }
int socket() { errno = ENOSYS; return -1; }
void *gethostbyname() { errno = ENOSYS; return (void *)0; }
int inet_addr() { errno = ENOSYS; return -1; }
void *getservbyname() { errno = ENOSYS; return (void *)0; }
void *authdes_create() { errno = ENOSYS; return (void *)0; }
void *xdrmem_create() { errno = ENOSYS; return (void *)0; }
int connect() { errno = ENOSYS; return -1; }
int setsockopt() { errno = ENOSYS; return -1; }
short htons(short x) { return x; }
int kill(int pid, int signal) { errno = ENOSYS; return -1; }
#endif
