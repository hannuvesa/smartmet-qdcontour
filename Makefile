HTML = qdcontour
PROG = qdcontour

MAINFLAGS = -Wall -W -Wno-unused-parameter

EXTRAFLAGS = -Wpointer-arith -Wcast-qual \
	-Wcast-align -Wwrite-strings -Wconversion -Winline \
	-Wnon-virtual-dtor -Wno-pmf-conversions \
	-Wsign-promo -Wchar-subscripts

# Comment this out to compile as non-Cairo (NFmiImage)
#
# TBD: This should really come from the Imagine library, via one way or the other
#      (it can be compiled with IMAGINE_WITH_CAIRO or not, but that affects
#       us, too)
#
CAIRO_CFLAGS=-DIMAGINE_WITH_CAIRO


# Exposes ArrowCache, ContourCache, ... (most likely does not like them being
# without explicity constructor?)
#
# -Wctor-dtor-privacy

# Boost has old type casts
#
# -Wold-style-cast

DIFFICULTFLAGS = -pedantic -Weffc++ -Wredundant-decls -Wshadow -Woverloaded-virtual -Wunreachable-code

CC = g++

# Use the CVS version of Newbase (not the system installed one, which is old)

ifeq "$(shell uname -s)" "Darwin"
  NEWBASE_PATH = $(HOME)/Work/IL/cvs/newbase
  #IMAGINE_PATH = $(HOME)/Work/IL/cvs/imagine
  TRON_PATH = $(HOME)/Work/IL/cvs/tron

  # OS X (Boost from fink)
  INCLUDES += -I/sw/include -I../Imagine
  LIBS += -L/sw/lib -lboost_regex -lboost_filesystem -lboost_system
else
  NEWBASE_PATH = /home/kauppi/IL/cvs/newbase
  #IMAGINE_PATH = /home/kauppi/IL/cvs/imagine
  TRON_PATH = /home/kauppi/IL/cvs/tron
  
  # Linux
  INCLUDES += -I/usr/local/include/boost-1_35
  LIBS += -L/usr/local/lib \
    -lboost_regex-gcc41-mt \
	-lboost_filesystem-gcc41-mt \
	-lboost_system-gcc41-mt
endif

# Rendering either with Cairo or Imagine

ifeq "$(IMAGINE_PATH)" ""
  INCLUDES += $(shell pkg-config --cflags cairomm-1.0)
  LIBS += $(shell pkg-config --libs cairomm-1.0)
else
  INCLUDES += -DUSE_OLD_IMAGINE -I$(IMAGINE_PATH)/include
  LIBS += -L$(IMAGINE_PATH) -lsmartmet_imagine
endif

MAINFLAGS += $(CAIRO_CFLAGS)

# Default compile options

CFLAGS = -DUNIX -O2 -DNDEBUG $(MAINFLAGS)
LDFLAGS =

# "-s is obsolete and being ignored" (gcc 4.01, AKa 17-Jul-2008)
#LDFLAGS = -s

# Special modes

CFLAGS_DEBUG = -DUNIX -O0 -g $(MAINFLAGS) $(EXTRAFLAGS) -Werror
CFLAGS_PROFILE = -DUNIX -O2 -g -pg -DNDEBUG $(MAINFLAGS)

LDFLAGS_DEBUG = 
LDFLAGS_PROFILE = 

# Platform independent Freetype2 support
#
FT2_LIBS= $(shell freetype-config --libs)
FT2_CFLAGS= $(shell freetype-config --cflags)

#INCLUDES += \
#	-I$(NEWBASE_PATH)/include \
#	-I$(TRON_PATH)/include \
#	$(FT2_CFLAGS)
#	-I.

INCLUDES += -I$(includedir) \
	-I$(includedir)/smartmet/newbase \
	-I$(includedir)/smartmet/tron \
	-I$(includedir)/smartmet/imagine \
	$(FT2_CFLAGS)

LIBS += \
	-L$(TRON_PATH) -lsmartmet_tron \
	-L$(NEWBASE_PATH) -lsmartmet_newbase \
	-lsmartmet_imagine \
	-Wl,-rpath,/usr/local/lib \
	-L /usr/local/lib \
	-lboost_regex-gcc41-mt \
	-lboost_filesystem-gcc41-mt \
	-lboost_system-gcc41-mt \
	-lboost_iostreams-gcc41-mt \
	$(FT2_LIBS) -lpng -ljpeg -lz

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

# rpm variables

CWP = $(shell pwd)
BIN = $(shell basename $(CWP))
rpmsourcedir = /smartmet/src/redhat/SOURCES
rpmerr = "There's no spec file ($(specfile)). RPM wasn't created. Please make a spec file or copy and rename it into $(BIN)"

rpmversion := $(shell grep "^Version:" $(HTML).spec  | cut -d\  -f 2 | tr . _)
rpmrelease := $(shell grep "^Release:" $(HTML).spec  | cut -d\  -f 2 | tr . _)

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

MAINSRCS = $(PROG:%=%.cpp)
SUBSRCS = $(filter-out $(MAINSRCS),$(SRCS))
SUBOBJS = $(SUBSRCS:%.cpp=%.o)
SUBOBJFILES = $(SUBOBJS:%.o=obj/%.o)

INCLUDES := -Iinclude $(INCLUDES)

# For make depend:

ALLSRCS = $(wildcard *.cpp source/*.cpp)

.PHONY: test rpm

# The rules

all: objdir $(PROG)
debug: objdir $(PROG)
release: objdir $(PROG)
profile: objdir $(PROG)

$(PROG): % : $(SUBOBJS) %.o
	$(CC) $(LDFLAGS) -o $@ obj/$@.o $(SUBOBJFILES) $(LIBS)

clean:
	rm -f $(PROG) $(OBJFILES) *~ source/*~ include/*~

install:
	mkdir -p $(bindir)
	@list='$(PROG)'; \
	for prog in $$list; do \
	  echo $(INSTALL_PROG) $$prog $(bindir)/$$prog; \
	  $(INSTALL_PROG) $$prog $(bindir)/$$prog; \
	done

depend:
	gccmakedep -fDependencies -- $(CFLAGS) $(INCLUDES) -- $(ALLSRCS)

test: $(PROG)
	cd test && LD_LIBRARY_PATH=/usr/local/lib make test

perltest: $(PROG)
	cd test && LD_LIBRARY_PATH=/usr/local/lib make perltest

html::
	mkdir -p ../../../../html/bin/$(HTML)
	doxygen $(HTML).dox

objdir:
	@mkdir -p $(objdir)

rpm: clean depend
	if [ -e $(BIN).spec ]; \
	then \
	  tar -C ../ -cf $(rpmsourcedir)/smartmet-$(BIN).tar $(BIN) ; \
	  gzip -f $(rpmsourcedir)/smartmet-$(BIN).tar ; \
	  rpmbuild -ta $(rpmsourcedir)/smartmet-$(BIN).tar.gz ; \
	else \
	  echo $(rpmerr); \
	fi;

tag:
	cvs -f tag 'smartmet_$(HTML)_$(rpmversion)-$(rpmrelease)' .

.SUFFIXES: $(SUFFIXES) .cpp

.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $(objdir)/$@ $<

-include Dependencies
