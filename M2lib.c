/*		Copyright 1994 by Daniel R. Grayson		*/

#include <factoryconf.h>
extern const char factoryVersion[]; /* extracted from factory's factory.h */
extern int libfac_interruptflag; /* extracted from libfac's factor.h */
#include <NTL/version.h>
/* defining GDBM_STATIC makes the cygwin version work, and is irrelevant for the other versions */
#define GDBM_STATIC
#include <gdbm.h>

#include "M2mem.h"
#include "M2mem2.h"
#include "M2inits.h"
#include "../dumpdata/map.h"
#include "types.h"
#include "debug.h"

#if HAVE_ALLOCA_H
#include <alloca.h>
#else
#ifdef __GNUC__
#ifndef alloca
#define alloca __builtin_alloca
#endif
#endif

#endif

/* char *config_args[] = { CONFIG_ARGS 0 }; */
const char *config_args = CONFIG_ARGS ;

#if !defined(PROFILING)
#error PROFILING not defined
#endif

#if HAVE_LINUX_PERSONALITY_H
#include <linux/personality.h>
#undef personality
#endif

#if HAVE_DECL_ADDR_NO_RANDOMIZE
#else
#define ADDR_NO_RANDOMIZE 0x0040000
#endif

#if HAVE_PERSONALITY
extern long personality(unsigned long persona);
#endif

const char *get_libfac_version();	/* in version.cc */
const char *get_frobby_version();	/* in version.cc */

#ifdef HAVE_SCSCP
 #include <scscp.h>
 void scscp_dummy() { SCSCP_sc_init(NULL,NULL); /* just to force linking with the library */ }
 static const char *get_scscp_version() {
    static char buf[20];
    sprintf(buf,"%d.%d.%d", SCSCP_VERSION_MAJOR, SCSCP_VERSION_MINOR, SCSCP_VERSION_PATCH);
    return buf;
 }
#else
 static const char *get_scscp_version() {
    return "not present"; 
 }
#endif

extern const char *get_pari_version();
#if !HAVE_PARI
const char *get_pari_version() { 
  return "not present"; 
}
#else
/* it's in pari-c.c */
#endif

static const char *get_cc_version(void) {
  static char buf[100] = "cc (unknown)";
# ifdef __GNUC__
#  ifdef __GNUC_PATCHLEVEL__
   sprintf(buf,"gcc %d.%d.%d",__GNUC__,__GNUC_MINOR__,__GNUC_PATCHLEVEL__);
#  else
   sprintf(buf,"gcc %d.%d",__GNUC__,__GNUC_MINOR__);
#  endif
# endif  
  return buf;
}

static void putstderr(const char *m) {
  int r;
  r = write(STDERR,m,strlen(m));
  r = write(STDERR,NEWLINE,strlen(NEWLINE));
}

void WerrorS(const char *m) {
  putstderr(m);
  exit(1);
}

void WarnS(const char *m) {
  putstderr(m);
}

#ifdef includeX11
Display *display;
Font font;
#endif

M2_bool system_exceptionFlag = FALSE;
M2_bool system_interruptedFlag = FALSE;
M2_bool system_interruptPending = FALSE;
M2_bool system_interruptShield = FALSE;
M2_bool system_alarmedFlag = FALSE;

static void alarm_handler(int sig), interrupt_handler(int sig);
static void oursignal(int sig, void (*handler)(int)) {
  struct sigaction act;
  act.sa_flags = 0;	/* no SA_RESTART */
  act.sa_handler = handler;
  sigemptyset(&act.sa_mask);
  sigfillset(&act.sa_mask);
  sigaction(sig,&act,NULL); /* old way: signal(sig,interrupt_handler); */
}
void system_handleInterruptsSetup(int handleInterrupts) {
  oursignal(SIGALRM,handleInterrupts ? alarm_handler : SIG_DFL);
  oursignal(SIGINT,handleInterrupts ? interrupt_handler : SIG_DFL);
}

static void unblock(int sig) {
  sigset_t s;
  sigemptyset(&s);
  sigaddset(&s,sig);
  sigprocmask(SIG_UNBLOCK,&s,NULL);
}

static void alarm_handler(int sig) {
     extern void evaluate_setAlarmedFlag();
     evaluate_setAlarmedFlag();
     oursignal(SIGALRM,alarm_handler);
     }

#if __GNUC__

static sigjmp_buf stack_trace_jump;

void segv_handler2(int sig) {
     // fprintf(stderr,"--SIGSEGV during stack trace\n");
     siglongjmp(stack_trace_jump,1);
}

void stack_trace() {
     void (*old)(int) = signal(SIGSEGV,segv_handler2); /* in case traversing the stack below causes a segmentation fault */
     unblock(SIGSEGV);
     fprintf(stderr,"-- stack trace:\n");
     if (0 == sigsetjmp(stack_trace_jump,TRUE)) {
#	  define D fprintf(stderr,"level %d -- return addr: 0x%08lx -- frame: 0x%08lx\n",i,(long)__builtin_return_address(i),(long)__builtin_frame_address(i))
#	  define i 0
	  D;
#	  undef i
#	  define i 1
	  D;
#	  undef i
#	  define i 2
	  D;
#	  undef i
#	  define i 3
	  D;
#	  undef i
#	  define i 4
	  D;
#	  undef i
#	  define i 5
	  D;
#	  undef i
#	  define i 6
	  D;
#	  undef i
#	  define i 7
	  D;
#	  undef i
#	  define i 8
	  D;
#	  undef i
#	  define i 9
	  D;
#	  undef i
#	  define i 10
	  D;
#	  undef i
#	  define i 11
	  D;
#	  undef i
#	  define i 12
	  D;
#	  undef i
#	  define i 13
	  D;
#	  undef i
#	  define i 14
	  D;
#	  undef i
#	  define i 15
	  D;
#	  undef i
#	  define i 16
	  D;
#	  undef i
#	  define i 17
	  D;
#	  undef i
#	  define i 18
	  D;
#	  undef i
#	  define i 19
	  D;
#	  undef i
#	  define i 20
	  D;
#	  undef i
     }
     fprintf(stderr,"-- end stack trace\n");
     signal(SIGSEGV,old);
}

void segv_handler(int sig) {
  static int level;
  fprintf(stderr,"-- SIGSEGV\n");
  level ++;
  if (level > 1) {
    fprintf(stderr,"-- SIGSEGV handler called a second time, aborting\n");
    _exit(2);
  }
  stack_trace();
  level --;
  _exit(1);
}

#endif

#if DUMPDATA
static sigjmp_buf loaddata_jump;
#endif

static sigjmp_buf abort_jump;
static bool abort_jump_set = FALSE;

sigjmp_buf interrupt_jump;
bool interrupt_jump_set = FALSE;

#undef ABORT

#include <readline/readline.h>

static void interrupt_handler(int sig)
{
#if 0
     int r;
     if (isatty(STDIN) && isatty(STDOUT) && !reading_from_readline) {
        r = write(STDERR,"\n",1);
     }
#endif
     if (system_interruptedFlag || system_interruptPending) {
	  if (isatty(STDIN) && isatty(STDOUT)) while (TRUE) {
	       char buf[10];
#              ifdef ABORT
	       printf("\nAbort (y/n)? ");
#              else
	       printf("\nExit (y/n)? ");
#              endif
	       fflush(stdout);
	       if (NULL == fgets(buf,sizeof(buf),stdin)) {
		    fprintf(stderr,"exiting\n");
		    exit(11);
	            }
	       if (buf[0]=='y' || buf[0]=='Y') {
#                   ifdef DEBUG
     		      trap();
#                   endif
#                   ifdef ABORT
		    if (!tokens_stopIfError && abort_jump_set) {
			 extern void evaluate_clearInterruptFlag(), evaluate_determineExceptionFlag(), evaluate_clearAlarmedFlag();
     	  		 fprintf(stderr,"returning to top level\n");
     	  		 fflush(stderr);
			 evaluate_clearInterruptFlag();
			 libfac_interruptflag = FALSE;
			 system_interruptPending = FALSE;
			 system_interruptShield = FALSE;
			 evaluate_clearAlarmedFlag();
			 evaluate_determineExceptionFlag();
     	  		 siglongjmp(abort_jump,1); 
			 }
		    else {
#                   endif
			 fprintf(stderr,"exiting\n");
		    	 exit(12);
#                   ifdef ABORT
			 }
#                   endif
		    }
	       else if (buf[0]=='n' || buf[0]=='N') {
		    break;
		    }
	       }
	  else {
#              ifndef NDEBUG
     	       trap();
#              endif
	       exit(13);
	       }
	  }
     else {
	  if (system_interruptShield) system_interruptPending = TRUE;
	  else {
	       extern void evaluate_setInterruptFlag();
	       if (tokens_stopIfError) {
		    int interruptExit = 2;	/* see also interp.d */
		    fprintf(stderr,"interrupted, stopping\n");
		    exit(interruptExit);
	       }
	       evaluate_setInterruptFlag();
	       libfac_interruptflag = TRUE;
# if 0
	       /* readline doesn't cancel the partially typed line, for some reason, and this doesn't help: */
	       if (reading_from_readline) rl_free_line_state();
#endif
	       if (interrupt_jump_set) siglongjmp(interrupt_jump,1);
	       }
	  }
     oursignal(SIGINT,interrupt_handler);
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

M2_string actors5_CCVERSION;
M2_string actors5_VERSION;
M2_string actors5_OS;
M2_string actors5_ARCH;
M2_string actors5_ISSUE;
M2_string actors5_MACHINE;
M2_string actors5_NODENAME;
M2_string actors5_REL;
M2_string actors5_timestamp;
M2_string actors5_GCVERSION;
M2_string actors5_GMPVERSION;
M2_string actors5_MPIRVERSION;
M2_string actors5_MysqlVERSION;
M2_string actors5_PYTHONVERSION;
M2_string actors5_startupString;
M2_string actors5_startupFile;
M2_string actors5_NTLVERSION;
M2_string actors5_LIBFACVERSION;
M2_string actors5_FROBBYVERSION;
M2_string actors5_PARIVERSION;
M2_string actors5_SCSCPVERSION;
M2_string actors5_FACTORYVERSION;
M2_string actors5_READLINEVERSION;
M2_string actors5_MPFRVERSION;
M2_string actors5_M2SUFFIX;
M2_string actors5_EXEEXT;
M2_string actors5_endianness;
M2_string actors5_packages;
M2_string actors5_build;
M2_string actors5_host;
int actors5_pointersize;
M2_bool actors5_DUMPDATA;
M2_bool actors5_FACTORY;
M2_bool actors5_MP;

M2_stringarray system_envp;
M2_stringarray system_argv;
M2_stringarray system_args;
M2_string actors5_configargs;
int system_loadDepth;

#if !defined(CLOCKS_PER_SEC) || CLOCKS_PER_SEC > 10000
static struct itimerval it;
#define INITVAL 1000000		/* a million seconds is very long */
void system_stime(void) {
     it.it_value.tv_sec = INITVAL;
     it.it_value.tv_usec = 0;
     (void) setitimer(ITIMER_VIRTUAL,&it,(struct itimerval *)NULL);
     }
double system_etime(void) {
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
void system_stime(void) {
     start_time = clock();
     }
double system_etime(void) {
     return (double)(clock()-start_time) / CLOCKS_PER_SEC;
     }
#endif

#if HAVE___ENVIRON
    #define our_environ __environ
    #if !HAVE_DECL___ENVIRON
    extern char **__environ;
    #endif
#elif HAVE__ENVIRON
    #define our_environ _environ
    #if !HAVE_DECL__ENVIRON
    extern char **_environ;
    #endif
#elif HAVE_ENVIRON
    #define our_environ environ
    #if !HAVE_DECL_ENVIRON
    extern char **environ;
    #endif
#else
    #error "no environment variable available"
#endif


extern char timestamp[];
static void clean_up();

void dummy_GC_warn_proc(char *msg, GC_word arg) { }

#define stringify(x) #x

#if defined(__GNUC__)
#define stringize(a) #a
char CCVERSION[] = "gcc " stringize(__GNUC__) "." stringize(__GNUC_MINOR__) ;
#else
char CCVERSION[] = "unknown" ;
#endif

extern void init_readline_variables();
extern char *GC_stackbottom;
extern void arginits(int, const char **);

extern bool gotArg(const char *arg, const char ** argv);

int pid;			/* initialized below */
int system_getpid(void) {
     return pid;
}

int system_getpgrp(void) {
  return getpgrp();
}

int system_setpgid(int pid0, int pgid) {
  return setpgid(pid0,pgid);
}

static char *endianness() {
     static int32_t x[2] = {0x61626364,0};
     return (char *)x;
}

#if 0
static void warning(const char *s,...)
{
  char buf[200];
  va_list ap;
  va_start(ap,s);
  vsprintf(buf,s,ap);
  if (errno != 0)
    fprintf(stderr,"warning: %s: %s\n", buf, strerror(errno));
  else
    fprintf(stderr,"warning: %s\n", buf);
  fflush(stderr);
  va_end(ap);
}
#endif

#if 0
static void error(const char *s,...)
{
  char buf[200];
  va_list ap;
  va_start(ap,s);
  vsprintf(buf,s,ap);
  if (errno != 0)
    fprintf(stderr,"error: %s: %s\n", buf, strerror(errno));
  else
    fprintf(stderr,"error: %s\n", buf);
  fflush(stderr);
  va_end(ap);
  exit(1);
}
#endif

#include <dlfcn.h>

static void call_shared_library() {
#if 0
  const char *libname = "libM2.so";
  const char *funname = "entry";
  void *handle;
  int (*g)();
  errno = 0;
  handle = dlopen(libname, RTLD_LAZY | RTLD_GLOBAL);
  if (handle == NULL) { error("can't load library %s", libname); return; }
  g = dlsym(handle, funname);
  if (g == NULL) { error("can't link function %s from sharable library %s",funname,libname); return; }
  g();
  if (0 != dlclose(handle)) { error("can't close sharable library %s",libname); return; }
#endif
}

#ifdef HAVE_PYTHON
#include <python2.5/Python.h>
#endif

int Macaulay2_main(argc,argv)
int argc; 
char **argv;
{
     char READLINEVERSION[8];	/* big enough for "255.255" */
     char dummy;
     int returncode = 0;
     int volatile envc = 0;
#if DUMPDATA
     static int old_collections = 0;
     const char ** volatile saveenvp = NULL;
     const char ** volatile saveargv;
     int volatile savepid = 0;
#else
#define saveenvp our_environ
#define saveargv argv
#endif
     void main_inits();
     static void *reserve = NULL;
     extern void actors4_setupargv();
     extern void interp_process(), interp_process2();
     extern int interp_topLevel();

     char **x = our_environ; 
     while (*x) envc++, x++;

     call_shared_library();

#ifdef HAVE_PYTHON
     Py_SetProgramName(argv[0]);
     Py_Initialize();
#endif

#if HAVE_PERSONALITY && !PROFILING
     if (!gotArg("--no-personality", (const char **)argv)) {
	  /* this avoids mmap() calls resulting in address randomization */
	  int oldpersonality = personality(-1);
	  if ((oldpersonality & ADDR_NO_RANDOMIZE) == 0) {
	       int newpersonality;
	       personality(oldpersonality | ADDR_NO_RANDOMIZE);
	       newpersonality = personality(-1);
	       personality(oldpersonality | ADDR_NO_RANDOMIZE);	/* just in case the previous line sets the personality to -1, which can happen */
	       if ((newpersonality & ADDR_NO_RANDOMIZE) != 0) return execvp(argv[0],argv);
	  }
	  else personality(oldpersonality);
     }
#endif

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

#if HAVE__SETMODE
     {
     extern void _setmode(int, int);
     _setmode(STDIN ,_O_BINARY);
     _setmode(STDOUT,_O_BINARY);
     _setmode(STDERR,_O_BINARY);
     }
#endif

#if DUMPDATA
     {
	  int i;

	  /* save arguments on stack in case they're on the heap */
	  saveargv = (char **)alloca((argc + 1)*sizeof(char *));
	  for (i=0; i<argc; i++) {
	       saveargv[i] = alloca(strlen(argv[i]) + 1);
	       strcpy(saveargv[i],argv[i]);
	  }
	  saveargv[i] = NULL;

	  /* save environment on stack in case it's on the heap */
	  saveenvp = (char **)alloca((envc + 1)*sizeof(char *));
	  for (i=0; i<envc; i++) {
	       saveenvp[i] = alloca(strlen(our_environ[i]) + 1);
	       strcpy(saveenvp[i],our_environ[i]);
	  }
	  saveenvp[i] = NULL;
     }
#endif

     pid = getpid();

#if DUMPDATA
     savepid = pid;		/* glibc getpid() caches the result in memory and performs the system call only once, so we can't use it after dumpdata */
     if (0 != sigsetjmp(loaddata_jump,TRUE)) {
	  pid = savepid;
	  if (gotArg("--notify", saveargv)) putstderr("--loaded cached memory data");
	  struct GC_stack_base sb;
	  GC_get_stack_base(&sb);
	  GC_stackbottom = (char *)sb.mem_base;	/* the stack may have moved (since we may have reloaded all the static data) */
     	  GC_free_space_divisor = 4;
	  old_collections = GC_gc_no;
          {
	       char **environ0;
	       int i;
	       our_environ = saveenvp;	/* our_environ is a static variable that points
					to the heap and has been overwritten by
					loaddata(), thereby pointing to a previous
					incarnation of the heap. */
	       /* Make a copy of the environment on the heap for 'our_environ'. */
	       /* In some systems, putenv() calls free() on the old item,
		  so we are careful to use malloc here, and not GC_malloc. */
	       environ0 = (char **)malloc((envc + 1)*sizeof(char *));
	       /* amazing but true:
		  On linux, malloc calls getenv to get values for tunable
		  parameters, so don't trash our_environ yet.
		  */
	       if (environ0 == NULL) fatal("out of memory");
	       for (i=0; i<envc; i++) {
		    environ0[i] = malloc(strlen(saveenvp[i]) + 1);
		    if (environ0[i] == NULL) fatal("out of memory");
		    strcpy(environ0[i],saveenvp[i]);
	       }
	       environ0[i] = NULL;
	       our_environ = environ0;
               }
	  }
#endif

     system_stime();

     if (__gmp_allocate_func != (void *(*) (size_t))getmem_atomic) {
          FATAL("possible memory leak, gmp allocator not set up properly");
	  fprintf(stderr,"--internal warning: possible memory leak, gmp allocator not set up properly, resetting\n");
	  enterM2();
     }

     signal(SIGPIPE,SIG_IGN);
     system_handleInterruptsSetup(TRUE);
     arginits(argc,(const char **)saveargv);

     if (GC_stackbottom == NULL) GC_stackbottom = &dummy;
     system_newline = tostring(newline);
     actors5_CCVERSION = tostring(get_cc_version());
     actors5_VERSION = tostring(PACKAGE_VERSION);
     actors5_OS = tostring(OS);
     actors5_ARCH = tostring(ARCH);
     actors5_MACHINE = tostring(MACHINE);
     actors5_ISSUE = tostring(ISSUE);
     actors5_NODENAME = tostring(NODENAME);
     actors5_REL = tostring(REL);
     {
	  char const * p = strrchr(factoryVersion,' ');
	  p = p ? p+1 : factoryVersion;
	  actors5_FACTORYVERSION = tostring(p);
     }
     actors5_LIBFACVERSION = tostring(get_libfac_version());
     actors5_FROBBYVERSION = tostring(get_frobby_version());
     actors5_PARIVERSION = tostring(get_pari_version());
     actors5_SCSCPVERSION = tostring(get_scscp_version());
     sprintf(READLINEVERSION,"%d.%d",(rl_readline_version>>8)&0xff,rl_readline_version&0xff);
     actors5_READLINEVERSION = tostring(READLINEVERSION);
     actors5_MPFRVERSION = tostring(mpfr_version);
     actors5_M2SUFFIX = tostring(M2SUFFIX);
     actors5_EXEEXT = tostring(EXEEXT);
     actors5_timestamp = tostring(timestamp);
     actors5_startupString = tostring(startupString);
     actors5_startupFile = tostring(startupFile);
     actors5_endianness = tostring(endianness());
     actors5_packages = tostring(PACKAGES);
     actors5_pointersize = sizeof(void *);
     actors5_host = tostring(hostsystemtype);
     actors5_build = tostring(buildsystemtype);
#if DUMPDATA
     actors5_DUMPDATA = TRUE;
     if (!haveDumpdata()) actors5_DUMPDATA = FALSE; /* even if dumpdata was enabled at configuration time, we may not have implemented it in the C code */
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
#ifdef __MPIR_VERSION
     actors5_GMPVERSION = tostring("not present");
     actors5_MPIRVERSION = tostring(__mpir_version);
#else
     actors5_GMPVERSION = tostring(__gmp_version);
     actors5_MPIRVERSION = tostring("not present");
#endif
     actors5_PYTHONVERSION = tostring(
#ifdef HAVE_PYTHON
         PY_VERSION				      
#else
	 "not present"
#endif
         );
     actors5_MysqlVERSION = tostring(
#if USE_MYSQL
         mysql_get_client_info()
#else
	 "not present"
#endif
         );
     actors5_NTLVERSION = tostring(NTL_VERSION);
     system_envp = tostrings(envc,(const char **)saveenvp);
     system_argv = tostrings(argc,(const char **)saveargv);
     system_args = tostrings(argc == 0 ? 0 : argc - 1, (const char **)saveargv + 1);
     /*     actors5_configargs = tostrings(sizeof(config_args)/sizeof(char *) - 1, config_args); */
     actors5_configargs = tostring(config_args);

#ifdef includeX11
     display = XOpenDisplay(NULL);
     font = XLoadFont(display,"6x13");
#endif
     init_readline_variables();
     main_inits();		/* run all the startup code in the *.d files, see tmp_init.c */
     actors4_setupargv();
     if (reserve == NULL) {
	  reserve = GC_MALLOC_ATOMIC(102400);
	  }
     sigsetjmp(abort_jump,TRUE);
     abort_jump_set = TRUE;

#if __GNUC__
     signal(SIGSEGV, segv_handler);
#endif

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
          returncode = ! interp_topLevel();
	  }
     else {
          out_of_memory_jump_set = TRUE;
	  startThreadPool(1);
     }
     clean_up();
#if 0
     fprintf(stderr,"gc: heap size = %d, free space divisor = %ld, collections = %ld\n", 
	  GC_get_heap_size(), GC_free_space_divisor, GC_gc_no-old_collections);
#endif
     exit(returncode);
     return returncode;
     }

static void clean_up(void) {
     extern void close_all_dbms();
     close_all_dbms();
     while (pre_final_list != NULL) {
	  pre_final_list->final();
	  pre_final_list = pre_final_list->next;
	  }
     while (final_list != NULL) {
	  final_list->final();
	  final_list = final_list->next;
	  }
#ifdef HAVE_PYTHON
     if (Py_IsInitialized()) Py_Finalize();
#endif
#    ifndef NDEBUG
     trap();
#    endif
     }

void system_exit(x)
int x;
{
     clean_up();
     exit(x);
     }

void scclib__prepare(void) {}

extern int etext, end;

int system_dumpdata(M2_string datafilename)
{
     /* this routine should keep its data on the stack */
#if !DUMPDATA
     return ERROR;
#else
     bool haderror = FALSE;
     char *datafilename_s = tocharstar(datafilename);
     if (ERROR == dumpdata(datafilename_s)) haderror = TRUE;
     GC_FREE(datafilename_s);
     return haderror ? ERROR : OKAY;
#endif
     }

#define FENCE 0x47474747

int system_loaddata(int notify, M2_string datafilename){
#if !DUMPDATA
     return ERROR;
#else
     char *datafilename_s = tocharstar(datafilename);
     volatile int fence0 = FENCE;
     sigjmp_buf save_loaddata_jump;
     volatile int fence1 = FENCE;
     /* int loadDepth = system_loadDepth; */
     memcpy(save_loaddata_jump,loaddata_jump,sizeof(loaddata_jump));
     if (ERROR == loaddata(notify,datafilename_s)) return ERROR;
     memcpy(loaddata_jump,save_loaddata_jump,sizeof(loaddata_jump));
     /* system_loadDepth = loadDepth + 1; */
     if (fence0 != FENCE || fence1 != FENCE) {
       putstderr("--internal error: fence around loaddata longjmp save area on stack destroyed, aborting");
       abort();
     }
     if (notify) putstderr("--loaddata: data loaded, ready for longjmp");
     siglongjmp(loaddata_jump,1);
#endif
     }

void C__prepare(void) {}

int system_isReady(int fd) {
  int ret;
  static fd_set r, w, e;
  struct timeval timeout;
  FD_SET(fd,&r);
  timerclear(&timeout);
  ret = select(fd+1,&r,&w,&e,&timeout);
  FD_CLR(fd,&r);
  return ret;
}

int system_hasException(int fd) {
  int ret;
  static fd_set r, w, e;
  struct timeval timeout;
  FD_SET(fd,&e);
  timerclear(&timeout);
  ret = select(fd+1,&r,&w,&e,&timeout);
  FD_CLR(fd,&e);
  return ret;
}

int actors5_WindowWidth(int fd) {
     struct winsize x;
     ioctl(1,TIOCGWINSZ,&x);	/* see /usr/include/$SYSTEM/termios.h */
     return x.ws_col;
     }

int actors5_WindowHeight(int fd) {
     struct winsize x;
     ioctl(1,TIOCGWINSZ,&x);	/* see /usr/include/$SYSTEM/termios.h */
     return x.ws_row;
     }


#include "../e/rand.h"

int system_randomint(void) {
#if 0
     extern long random();
     return random();
#elif 0
     extern long random00();
     return random00();
#else
     return rawRandomInt(2<<31-1);
#endif
     }

/*
// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/d "
// tags-file-name: "TAGS"
// End:
*/
