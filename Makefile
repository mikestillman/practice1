############################## main target
all :: misc compat.h ../bin/Macaulay2
############################## includes
include ../../Makeconf
.PHONY : misc
misc :
	@echo 'making Macaulay2 for ARCH=$(ARCH), OS=$(OS), REL=$(REL)'
############################## files
# the names on these lines should be in sequence from most dependent to
# least dependent so that "make" can remake the *.dep makefiles
MISC  := t.d u.d optim.d actorsX.d
GRAPHICS := gr.d grx.d
DTESTS := testall.d pp.d redblk.d
INTERPRETER := interpret.d

OPTIONS := 
ifdef MP
OPTIONS += mp.d
endif
ifdef includeX11
OPTIONS += actorsX.d
endif

ACTORS := actors5.d actors4.d actors3.d actors2.d actors.d \
		objects.d structure.d GC.d
COMPILER := converter.d basic.d binding.d
PARSER := parser.d lex.d tokens.d 
MOSTPACKAGES := arithmetic.d err.d stdiop.d ctype.d stdio.d \
		nets.d varstrings.d strings.d 
SYSTEM := X.d GB.d system.d C.d
############################## more files
WRAPPERS := scclib.c getpagesize.h gc_cpp.cc memdebug.c memdebug.h
############################## file collections
PACKAGES := $(MOSTPACKAGES) $(SYSTEM)
PROJECT := $(INTERPRETER) $(OPTIONS) $(ACTORS) \
	$(COMPILER) $(PARSER) $(MOSTPACKAGES) $(SYSTEM)
MOSTPROJECT := $(INTERPRETER) $(OPTIONS) $(ACTORS) \
	$(COMPILER) $(PARSER) $(MOSTPACKAGES)
DNAMES := $(MISC) $(GRAPHICS) $(DTESTS) $(PROJECT)
#############################################################################
SRCFILES := $(WRAPPERS) $(DNAMES)
SCRIPTS := reverse
WC1FILES := $(SCRIPTS) $(SRCFILES) \
	Makefile abc grxwrap.c g scc.gdb README COPYRIGHT \
	bench sets.m bignum.h bignum.c alloca.h \
	configure remake probe.c sizes.c mp.d
ALLFILES := $(WC1FILES) $(PROJECT:.d=.dep) $(PROJECT:.d=.sig)
##############################
ifndef NODEPENDS
include $(PROJECT:.d=.dep)
endif
############################## rules
SCC1 := ../c/scc1
.SUFFIXES: .d .oo .sig .dep .res .test .m2
.PHONY : clean all tests
%.dep : %.d
	$(SCC1) -nogcc -dep -J. $*.d
	mv $*.dp $*.dep
	../bin/update $*.sg $*.sig
interpret.dep : interpret.d
structure.dep : structure.d
varstrings.dep : varstrings.d
arithmetic.dep : arithmetic.d
converter.dep : converter.d
%.c   : %.d
	$(SCC1) -O +gc -J. -noline -nogcc $<
%.oo  : %.d
	$(SCC1) -O +gc -J. -nogcc $<
	$(CC) -o $*.oo $(CPPFLAGS) $(CCFLAGS) -c $*.c
	rm $*.c
%     : %.oo;	$(CC) -o $@ $< $(LDFLAGS) $(LOADLIBES)
%.o   : %.c;	$(CC) -o $@ $< -c $(CPPFLAGS) $(CFLAGS)
############################## flags
CC       := gcc

DEBUGFLAGS := 
# DEBUGFLAGS := -DMEM_DEBUG

STRIPFLAG :=
STRIPFLAG := -s

STRIPCMD := :
STRIPCMD := strip

PURIFYCMD :=
# PURIFYCMD := purify -always-use-cache-dir

CPPFLAGS := -I$(INCDIR) -I. -DGaCo=1 $(DEBUGFLAGS)
# the purpose of the -I. is so scclib.c can find ./alloca.h if it's missing
# from the gcc installation, as it often is

CPPFLAGS += '-DVERSION="$(VERSION)"'

WARNINGS := -Wall -Wshadow -Wcast-qual
CCFLAGS  := -O3 -g
CFLAGS   := $(CCFLAGS) $(WARNINGS)
CXXFLAGS := $(CFLAGS)
LOADLIBES:= 
LDFLAGS  := -L${LIBDIR} $(STRIPFLAG) #-pg
#################################
# libgc.a is the Boehm garbage collector
LOADLIBES += -lgc

ifdef includeX11
CPPFLAGS += -DincludeX11
LOADLIBES += -lX11
endif

ifdef MP
CPPFLAGS += -DMP
LOADLIBES += -lMP
endif

## we use one of these in scclib.c
# libdbm2.a is our own database manager
# libgdbm.a is the gnu database manager
# libndbm.a is the new database manager
LOADLIBES += ../dbm/libdbm2.a

# these next two must hang together
# LOADLIBES += -lX11
# CFLAGS += -DincludeX11

# hopefully, this will prevent us from getting the buggy version of 
# __underflow
# LOADLIBES += -lc

# -s leaves out the symbol table when linking
# LDFLAGS += -s

# ifneq ($(OS),Linux)
LDFLAGS += -static
# endif

#ifeq ($(OS),Linux)
#LOADLIBES += -lieee
#endif

# Messollen's multivariate factoring stuff
ifdef FACTOR
CPPFLAGS += -DFACTOR
# LDFLAGS += -L$(LIBDIR)/factory/1.2c
# LOADLIBES += -lfac-g
# LDFLAGS += -Xlinker -y_sqrFreeZ__FRC13CanonicalForm
LOADLIBES += -lfac
# LOADLIBES += -lcf-debug
# LOADLIBES += -lcf-optimize -lmemman
# LOADLIBES += -lsingcf
LOADLIBES += -lcf -lcfmem
# the gmp-macros will be needed if we use their binaries and they still
# use the old version of gmp, older than version 2.00
# LOADLIBES += -lgmp-macros
endif

LOADLIBES += -lgmp -lmpz -lmpn

# -lsunmath makes suns obey ieee for floating point operations, at
# -liberty is /usr/local/lib/libiberty.a, and it has random() in it
# least under solaris
ifeq ($(OS) $(REL),SunOS 5.4)
LOADLIBES += -lsunmath -liberty
endif

ifeq ($(OS),MS-DOS)
LOADLIBES += -lgpp
else
LOADLIBES += -lg++ -lstdc++
endif

# but on some machines, with non-gnu ld being used, libiostream is
# not used automatically, so put it in anyway.
# LOADLIBES += -liostream

LOADLIBES += -lm

############################## compiling

ifeq "$(OS)" "MS-DOS"
compat.c : ../msdos/compat.c; cp $< $@
compat.h : ../msdos/compat.h; cp $< $@
else
compat.c compat.h : configure; ./configure
endif

scclib.o : ../c/compat.h ../c/compat.c ../../Makeconf
memdebug.o scclib.o actors5.oo gc_cpp.o : memdebug.h
allc : $(PROJECT:.d=.c) tmp_init.c
ALLOBJ := $(PROJECT:.d=.oo) scclib.o compat.o gc_cpp.o tmp_init.o memdebug.o
ALLC := $(PROJECT:.d=.c)
RMC :; rm -rf $(ALLC)

tmp_init.c : Makefile ../bin/timestmp
	timestmp >>tmp
	@echo "echoout '>>tmp' ..."
	@echoout '>>tmp' $(foreach f, $(PROJECT:.d=), 'void $(f)__prepare();') 
	echoout '>>tmp' 'int main_inits() {'
	@echo "echoout '>>tmp' ..."
	@echoout '>>tmp' $(foreach f, $(PROJECT:.d=), '   $(f)__prepare();')
	echoout '>>tmp' '   return 0;}'
	mv tmp tmp_init.c

.._c_compat.c: ../c/compat.c; cp $^ $@
.._c_compat.h: ../c/compat.h; cp $^ $@

c-port: $(ALLC) tmp_init.c gc_cpp.cc scclib.c memdebug.c memdebug.h compat.c compat.h \
		.._c_compat.c .._c_compat.h
	tar cfz /tmp/c-port.tgz $^

interpret.a : $(ALLOBJ)
	ar rcs $@ $^ tmp_init.o
############################## probe memory for dumpdata
probe : probe.c
	$(CC) -static -I$(INCDIR) -g -o probe probe.c
test-probe : probe
	nm probe |grep -v "gcc2_compiled\|gnu_compiled\0| \." >syms
	probe a b c d >> syms
	sort syms > addresses
	rm syms
############################## miscellaneous
#		../../lib/factory/1.2b/libcf-optimize.a
#		../../lib/factory/1.2b/libmem-optimize.a \

../bin/Macaulay2 : $(ALLOBJ) ../e/*.o tmp_init.o \
		../../lib/libgc.a \
		../../lib/libgmp.a \
		../../lib/libMP.a \
		../../lib/libcf.a ../../lib/libcfmem.a \
		../../lib/libmpf.a \
		../../lib/libmpn.a \
		../../lib/libmpq.a \
		../../lib/libmpz.a \
		../../lib/libfac.a
	rm -f $@
	@ echo 'linking $@ with $(LDFLAGS) $(LOADLIBES)'
	@ time $(PURIFYCMD) $(CC) -o $@ $(LDFLAGS) $^ $(LOADLIBES)
	$(STRIPCMD) $@
TAGS: Makefile
	@echo making TAGS
	@echoout -r2 '>TAGS' $(foreach i, $(SRCFILES),  $(i),0)
allfiles: Makefile
	@echo making allfiles
	@echoout '>allfiles.tmp' $(ALLFILES)
	@<allfiles.tmp sort|uniq >allfiles
	@rm allfiles.tmp
wc:
	wc -l $(WC1FILES)
clean :
	rm -f *.log *.sym *.out *.o *.a *.oo \
		$(DTESTS:.d=) *_inits.c *.sg *.sgn \
		$(DNAMES:.d=.c) allfiles TAGS \
		core core.*
