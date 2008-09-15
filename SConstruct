#
# SConstruct for building QDContour
#
# Usage:
#       scons [-j 4] [-Q] [debug=1|profile=1] [objdir=<path>] [prefix=<path>]
#
# Notes:
#       The three variants share the same output and object file names;
#       changing from one version to another will cause a full recompile.
#       This is intentional (instead of keeping, say, out/release/...)
#       for compatibility with existing test suite etc. and because normally
#       different machines are used only for debug/release/profile.
#           -- AKa 11-Sep-2008
#
# Change log:
#       AKa 15-Sep-2008: Simplified (using just one build environment)
#       AKa 11-Sep-2008: Initial version (same as existing Makefile)
#

import os.path

Help(""" 
    Usage: scons [-j 4] [-Q] [debug=1|profile=1] [objdir=<path>] [prefix=<path>]
    
    Or just use 'make release|debug|profile', which point right back to us.
""") 

DEBUG=      int( ARGUMENTS.get("debug", 0) ) != 0
PROFILE=    int( ARGUMENTS.get("profile", 0) ) != 0
RELEASE=    (not DEBUG) and (not PROFILE)     # default

OBJDIR=     ARGUMENTS.get("objdir","obj")
PREFIX=     ARGUMENTS.get("prefix","/usr")

#
# Base settings
#
env= Environment()

LINUX=  env["PLATFORM"]=="posix"
OSX=    env["PLATFORM"]=="darwin"
WINDOWS= env["PLATFORM"]=="windows"

env.Append( CPPPATH= [ "./include" ] )

if WINDOWS: 
    { }     # TBD
else:
    env.Append( CPPDEFINES= ["UNIX"] )
    env.Append( CXXFLAGS= [
        # MAINFLAGS from orig. Makefile
        #
        "-Wall", 
        "-Wno-unused-parameter",
	    
	    # DIFFICULTFLAGS from orig. Makefile
	    #
	    # These are flags that would be good, but give hard to bypass errors
	    #
	    #"-pedantic",
	    #"-Weffc++",
	    #"-Wredundant-decls",
	    #"-Wshadow",
	    #"-Woverloaded-virtual",
	    #"-Wunreachable-code",
	    #"-Wctor-dtor-privacy",    problems with ArrowCache, ContourCache
	    #"-Wold-style-cast",       Boost 1.35 has old style casts
    ] )

if LINUX:
    # Newbase, Imagine & Tron from system install
    #
    env.Append( CPPPATH= [ PREFIX+"/include/smartmet/newbase",
                           PREFIX+"/include/smartmet/imagine",
                           PREFIX+"/include/smartmet/tron" ] )

    # Boost 1.35 built to /usr/local
    #
    env.Append( CPPPATH= [ "/usr/local/include/boost-1_35" ] )
    env.Append( LIBPATH= [ "/usr/local/lib" ] )

elif OSX:
    # Newbase, Imagine & Tron from local CVS
    #
    env.Append( CPPPATH= [ "../newbase/include", "../imagine/include", "../tron/include" ] )
    env.Append( LIBPATH= [ "../newbase", "../imagine", "../tron" ] )

    # Boost from Fink
    #
    env.Append( CPPPATH= [ "/sw/include" ] )
    env.Append( LIBPATH= [ "/sw/lib" ] )

env.Append( LIBS= [ "smartmet_newbase", "smartmet_imagine", "smartmet_tron" ] )

BOOST_POSTFIX= ""
if LINUX:
    BOOST_POSTFIX= "-gcc41-mt"

env.Append( LIBS= [ "boost_regex"+BOOST_POSTFIX, 
                    "boost_filesystem"+BOOST_POSTFIX,
                    "boost_system"+BOOST_POSTFIX,
                    "boost_iostreams"+BOOST_POSTFIX ] )

#
# Freetype2, Cairomm-1.0, ...
#
if not WINDOWS:
    env.ParseConfig("freetype-config --cflags --libs") 
    env.ParseConfig("pkg-config --cflags --libs cairomm-1.0")
    
# From original Makefile (not sure if needed?)
#
if LINUX:
	env.Append( LIBFLAGS= "-Wl,-rpath,/usr/local/lib" )

env.Append( LIBS= [ "png", "jpeg", "z" ] )


#
# Debug settings
#
if DEBUG:
    if WINDOWS:
        { }     # TBD
    else:
        env.Append( CXXFLAGS=[ "-O0", "-g", "-Werror",

            # EXTRAFLAGS from orig. Makefile (for 'debug' target)
            #
            "-Wpointer-arith",
            "-Wcast-qual",
            "-Wcast-align",
            "-Wwrite-strings",
            "-Wconversion",
            "-Winline",
            "-Wnon-virtual-dtor",
            "-Wno-pmf-conversions",
            "-Wsign-promo",
            "-Wchar-subscripts",
        ] )

#
# Release settings
#
if RELEASE or PROFILE:
    if WINDOWS:
        { }     # TBD
    else:
        env.Append( CPPDEFINES="NDEBUG",
                    CXXFLAGS= ["-O2",

            # RELEASEFLAGS from orig. Makefile (for 'release' and 'profile' targets)
            #
            "-Wuninitialized",
        ] )

#
# Profile settings
#
if PROFILE:
    if WINDOWS:
        { }     # TBD
    else: 
        env.Append( CXXFLAGS="-g -pg" )


#---
# Force objects to 'objdir'
#
# Note: Samples show 'env.Replace( OBJDIR=... )' to be able to do this, but
#       it did not work.    -- AKa 15-Sep-2008
#
#env.Replace( OBJDIR=OBJDIR )
#env.Program( "qdcontour", Glob("source/*.cpp") )

objs= []

for fn in Glob("source/*.cpp"): 
    s= os.path.basename( str(fn) )
    objs += env.Object( OBJDIR+"/"+ s.replace(".cpp",""), fn )

objs += env.Object( OBJDIR+"/qdcontour", "qdcontour.cpp" )

env.Program( "qdcontour", objs )

