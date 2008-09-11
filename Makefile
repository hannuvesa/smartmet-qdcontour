HTML = qdcontour
PROG = qdcontour

#
# To build serially (helps get the error messages right): make debug SCONS_FLAGS=""
#
SCONS_FLAGS=-j 4

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

# How to install

INSTALL_PROG = install -m 775
INSTALL_DATA = install -m 664

.PHONY: test rpm

#
# The rules
#
SCONS_FLAGS += objdir=$(objdir) prefix=$(PREFIX)

all release $(PROG):
	scons $(SCONS_FLAGS) $(PROG)

debug:
	scons $(SCONS_FLAGS) debug=1 $(PROG)

profile:
	scons $(SCONS_FLAGS) profile=1 $(PROG)

clean:
	@#scons -c objdir=$(objdir)
	-rm -f $(PROG) *~ source/*~ include/*~
	-rm -rf $(objdir)

install:
	mkdir -p $(bindir)
	@list='$(PROG)'; \
	for prog in $$list; do \
	  echo $(INSTALL_PROG) $$prog $(bindir)/$$prog; \
	  $(INSTALL_PROG) $$prog $(bindir)/$$prog; \
	done

test: $(PROG)
	cd test && LD_LIBRARY_PATH=/usr/local/lib make test

perltest: $(PROG)
	cd test && LD_LIBRARY_PATH=/usr/local/lib make perltest

html:
	mkdir -p ../../../../html/bin/$(HTML)
	doxygen $(HTML).dox

rpm: clean
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
