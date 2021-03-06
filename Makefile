MODULE = qdcontour
SPEC = smartmet-qdcontour

MAINFLAGS = -MD -Wall -W -Wno-unused-parameter

ifeq (6, $(RHEL_VERSION))
  MAINFLAGS += -std=c++0x
else
  MAINFLAGS += -std=c++11 -fdiagnostics-color=always
endif

EXTRAFLAGS = \
	-Werror \
	-Winline \
	-Wpointer-arith \
	-Wcast-qual \
	-Wcast-align \
	-Wwrite-strings \
	-Wnon-virtual-dtor \
	-Wno-pmf-conversions \
	-Wsign-promo \
	-Wchar-subscripts \
	-Wredundant-decls \
	-Woverloaded-virtual

DIFFICULTFLAGS = \
	-Wunreachable-code \
	-Wconversion \
	-Wctor-dtor-privacy \
	-Weffc++ \
	-Wold-style-cast \
	-pedantic \
	-Wshadow

CC = g++

# Default compiler flags

DEFINES = -DUNIX

CFLAGS = $(DEFINES) -O2 -DNDEBUG $(MAINFLAGS)
LDFLAGS = 

# Special modes

CFLAGS_DEBUG = $(DEFINES) -O0 -g $(MAINFLAGS) $(EXTRAFLAGS) -Werror
CFLAGS_PROFILE = $(DEFINES) -O2 -g -pg -DNDEBUG $(MAINFLAGS)

LDFLAGS_DEBUG =
LDFLAGS_PROFILE =

INCLUDES = -I$(includedir) \
	-I$(includedir)/smartmet

LIBS = -L$(libdir) \
	-lsmartmet-newbase \
	-lsmartmet-imagine \
	-lsmartmet-tron \
	-lgeos \
	-lboost_iostreams \
	-lboost_system

# Common library compiling template

# Installation directories

processor := $(shell uname -p)

ifeq ($(origin PREFIX), undefined)
  PREFIX = /usr
else
  PREFIX = $(PREFIX)
endif

ifeq ($(processor), x86_64)
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

# Special modes

ifneq (,$(findstring debug,$(MAKECMDGOALS)))
  CFLAGS = $(CFLAGS_DEBUG)
  LDFLAGS = $(LDFLAGS_DEBUG)
endif

ifneq (,$(findstring profile,$(MAKECMDGOALS)))
  CFLAGS = $(CFLAGS_PROFILE)
  LDFLAGS = $(LDFLAGS_PROFILE)
endif

# Compilation directories

vpath %.cpp source main
vpath %.h include
vpath %.o $(objdir)
vpath %.d $(objdir)

# How to install

INSTALL_PROG = install -m 775
INSTALL_DATA = install -m 664

# The files to be compiled

HDRS = $(patsubst include/%,%,$(wildcard *.h include/*.h))

MAINSRCS     = $(patsubst main/%,%,$(wildcard main/*.cpp))
MAINPROGS    = $(MAINSRCS:%.cpp=%)
MAINOBJS     = $(MAINSRCS:%.cpp=%.o)
MAINOBJFILES = $(MAINOBJS:%.o=obj/%.o)

SRCS     = $(patsubst source/%,%,$(wildcard source/*.cpp))
OBJS     = $(SRCS:%.cpp=%.o)
OBJFILES = $(OBJS:%.o=obj/%.o)

INCLUDES := -Iinclude $(INCLUDES)

# For make depend:

ALLSRCS = $(wildcard main/*.cpp source/*.cpp)

.PHONY: test rpm

# The rules

all: objdir $(MAINPROGS)
debug: objdir $(MAINPROGS)
release: objdir $(MAINPROGS)
profile: objdir $(MAINPROGS)

.SECONDEXPANSION:
$(MAINPROGS): % : $(OBJFILES) $(MAINOBJFILES)
	$(CC) $(LDFLAGS) -o $@ obj/$@.o $(OBJFILES) $(LIBS)

clean:
	rm -f $(MAINPROGS) $(OBJFILES) $(MAINOBJFILES)*~ source/*~ include/*~
	rm -f obj/*.d

format:
	clang-format -i -style=file include/*.h source/*.cpp main/*.cpp

install:
	mkdir -p $(bindir)
	@list='$(MAINPROGS)'; \
	for prog in $$list; do \
	  echo $(INSTALL_PROG) $$prog $(bindir)/$$prog; \
	  $(INSTALL_PROG) $$prog $(bindir)/$$prog; \
	done

test:
	cd test && make test

objdir:
	@mkdir -p $(objdir)

rpm: clean
	@if [ -a $(SPEC).spec ]; \
	then \
	  tar -czvf $(SPEC).tar.gz --transform "s,^,$(SPEC)/," * ; \
	  rpmbuild -ta $(SPEC).tar.gz ; \
	  rm -f $(SPEC).tar.gz ; \
	else \
	  echo $(SPEC).spec missing; \
	fi;

.SUFFIXES: $(SUFFIXES) .cpp

obj/%.o : %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

-include obj/*.d
