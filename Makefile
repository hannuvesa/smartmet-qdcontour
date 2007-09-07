HTML = qdcontour
PROG = qdcontour

MAINFLAGS = -Wall -W -Wno-unused-parameter

EXTRAFLAGS = -Wpointer-arith -Wcast-qual \
	-Wcast-align -Wwrite-strings -Wconversion -Winline \
	-Wctor-dtor-privacy -Wnon-virtual-dtor -Wno-pmf-conversions \
	-Wsign-promo -Wchar-subscripts -Wold-style-cast

DIFFICULTFLAGS = -pedantic -Weffc++ -Wredundant-decls -Wshadow -Woverloaded-virtual -Wunreachable-code

CC = g++
# CFLAGS = -DUNIX -O0 -g $(MAINFLAGS) $(EXTRAFLAGS) -Werror
# CFLAGS_RELEASE = -DUNIX -O8 -g -DNDEBUG $(MAINFLAGS)
LDFLAGS = 
CFLAGS = -DUNIX -O0 -g $(MAINFLAGS) $(EXTRAFLAGS) -Werror
CFLAGS_RELEASE = -DUNIX -O2 -g -DNDEBUG $(MAINFLAGS)
LDFLAGS = 
ARFLAGS = -r
INCLUDES = -I $(includedir) -I $(includedir)/smartmet/newbase -I $(includedir)/smartmet/tron -I $(includedir)/smartmet/imagine -I $(includedir)/freetype2
LIBS = -L$(libdir) -lsmartmet_imagine -lsmartmet_tron -lsmartmet_newbase -lsmartmet_gpc -lfreetype -lpng -ljpeg -lz

# Common library compiling template

# Installation directories
prosessor := $(shell uname -p)

ifeq ($(origin PREFIX), undefined)
  PREFIX = /usr
else
  PREFIX = $(PREFIX)
endif

ifeq ($(prosessor), x86_64)
  libdir = $(PREFIX)/lib64
else
  libdir = $(PREFIX)/lib
endif

objdir = obj
includedir = $(PREFIX)/include

ifeq ($(origin BINDIR), undefined)
  bindir = $(PREFIX)/bin
else
  bindir = $(BINDIR)
endif

# rpm variables
CWP = $(shell pwd)
BIN = $(shell basename $(CWP))
rpmsourcedir = /smartmet/src/redhat/SOURCES
rpmerr = "There's no spec file ($(specfile)). RPM wasn't created. Please make a spec file or copy and rename it into $(BIN)"


# CFLAGS

ifeq ($(MAKECMDGOALS),release)
  CFLAGS = $(CFLAGS_RELEASE)
endif

ifneq (,$(findstring gmon,$(MAKECMDGOALS)))
 objdir := pgobj
 CFLAGS := -pg $(CFLAGS)
endif

PGLIB = subspg
LIBFILE = libsubs.a
PGLIBFILE = lib$(PGLIB).a

# Compilation directories

vpath %.cpp source
vpath %.h include
vpath %.o $(objdir)

# How to install

INSTALL_PROG = install -m 775
INSTALL_DATA = install -m 664

# The files to be compiled

SRCS = $(patsubst source/%,%,$(wildcard *.cpp source/*.cpp))
HDRS = $(patsubst include/%,%,$(wildcard *.h include/*.h))
OBJS = $(SRCS:%.cpp=%.o)

OBJFILES = $(OBJS:%.o=obj/%.o)
PGOBJFILES = $(OBJS:%.o=pgobj/%.o)

MAINSRCS = $(PROG:%=%.cpp)
SUBSRCS = $(filter-out $(MAINSRCS),$(SRCS))
SUBOBJS = $(SUBSRCS:%.cpp=%.o)
SUBOBJFILES = $(SUBOBJS:%.o=obj/%.o)
PGSUBOBJFILES = $(SUBOBJS:%.o=pgobj/%.o)

INCLUDES := -I include $(INCLUDES)

.PHONY: test gmon rpm

# The rules

all: objdir $(PROG)
debug: objdir $(PROG)
release: objdir $(PROG)

$(PROG): % : $(SUBOBJS) %.o
	$(CC) $(LDFLAGS) -o $@ obj/$@.o $(SUBOBJFILES) $(LIBS)

$(LIBFILE): objdir $(OBJS)
	$(AR) $(ARFLAGS) $(LIBFILE) $(OBJFILES)

$(PGLIBFILE): objdir $(OBJS)
	$(AR) $(ARFLAGS) $(PGLIBFILE) $(PGOBJFILES)

clean:
	rm -f $(PROG) $(OBJFILES) $(PGOBJFILES) *~ source/*~ include/*~

install:
	mkdir -p $(bindir)
	@list='$(PROG)'; \
	for prog in $$list; do \
	  echo $(INSTALL_PROG) $$prog $(bindir)/$$prog; \
	  $(INSTALL_PROG) $$prog $(bindir)/$$prog; \
	done

depend:
	makedepend $(INCLUDES)

test:
	cd test && make test

html::
	mkdir -p ../../../../html/bin/$(HTML)
	doxygen $(HTML).dox

objdir:
	@mkdir -p $(objdir)

rpm:
	if [ -a $(BIN).spec ]; \
	then \
	  tar -C ../ -cf $(rpmsourcedir)/smartmet-$(BIN).tar $(BIN) ; \
	  gzip -f $(rpmsourcedir)/smartmet-$(BIN).tar ; \
	  rpmbuild -ta $(rpmsourcedir)/smartmet-$(BIN).tar.gz ; \
	else \
	  echo $(rpmerr); \
	fi;


gmon: objdir gmon.txt

gmon.txt: $(SUBOBJS)
	@echo Creating a temporary profiling program
	@echo "int main(int argc, char ** argv){}" > foobar.cpp
	@g++ -pg -o foobar foobar.cpp -Wl,-whole-archive -L. $(PGSUBOBJFILES) $(LIBS)
	@./foobar
	@rm -f foobar.cpp
	@echo Created temporary gmon.out
	@gprof -b -q -c -z foobar gmon.out > gmon.txt
	@rm -f foobar gmon.out
	@echo Created gmon.txt for analysis

.SUFFIXES: $(SUFFIXES) .cpp

.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $(objdir)/$@ $<

# -include Dependencies
# DO NOT DELETE THIS LINE -- make depend depends on it.
