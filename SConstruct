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
env= DefaultEnvironment()

LINUX=  env["PLATFORM"]=="posix"
OSX=    env["PLATFORM"]=="darwin"
WINDOWS= env["PLATFORM"]=="windows"

env.Append( CPPPATH= [ "./include" ] )

if not WINDOWS: 
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
    
env.Append( LIBS= [ "png", "jpeg", "z" ] )

# From original Makefile (not sure if needed?)
#
if LINUX:
	env.Append( LIBFLAGS= "-Wl,-rpath,/usr/local/lib" )


#
# Debug settings
#
env_debug= env.Clone() 

if not WINDOWS: 
    debug_flags= [
	    "-Werror",

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
    ]
else:
    debug_flags= []

env_debug.Append( CXXFLAGS=[ "-O0", "-g" ] + debug_flags )


#
# Release settings
#
env_release= env.Clone() 

if not WINDOWS: 
    release_flags= [
        # RELEASEFLAGS from orig. Makefile (for 'release' and 'profile' targets)
        #
        "-Wuninitialized",
    ]
else:
    release_flags= []

env_release.Append( CXXFLAGS="-O2", CPPDEFINES="NDEBUG" )
env_release.Append( CXXFLAGS= release_flags )


#
# Profile settings
#
env_profile= env_release.Clone() 

if not WINDOWS: 
    env_profile.Append( CXXFLAGS="-g -pg" )


#---
# Force objects to 'objdir'
#
objs= []

if DEBUG:
    e= env_debug
elif PROFILE:
    e= env_profile
else:
    e= env_release

for fn in Glob("source/*.cpp"): 
    s= os.path.basename( str(fn) )
    objs += e.Object( OBJDIR+"/"+ s.replace(".cpp",""), fn )

objs += e.Object( OBJDIR+"/qdcontour", "qdcontour.cpp" )

e.Program( "qdcontour", objs )

