PROG = qdcontour

CC = g++
CFLAGS = -DUNIX -O0 -g -Wall
LDFLAGS = -s
ARFLAGS = -r
INCLUDES = -I $(includedir)/newbase -I $(includedir)/imagine
LIBS = -L$(libdir) -limagine -lnewbase -lpng -ljpeg

# Common library compiling template

include ../../makefiles/makefile.prog
