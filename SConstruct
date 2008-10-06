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

Decider("MD5-timestamp") 

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
WINDOWS= env["PLATFORM"]=="win32"

#
# SCons does not pass env.vars automatically through to executing commands.
# On Windows, we want it to get them all (Visual C++ 2008).
#
if WINDOWS:
    env.Replace( ENV= os.environ )

env.Append( CPPPATH= [ "./include" ] )

if WINDOWS: 
    if env["CC"]=="cl":
        env.Append( CPPFLAGS= ["/EHsc"] )
else:
    env.Append( CPPDEFINES= ["UNIX"],
                CPPFLAGS= [
        # MAINFLAGS from orig. Makefile
        #
        "-Wall", 
        "-W",
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

BOOST_POSTFIX=""
BOOST_PREFIX=""

if WINDOWS:
    # Installed from 'boost_1_35_0_setup.exe' from BoostPro Internet page.
    #
    BOOST_INSTALL_PATH= "D:/Boost/1_35_0"
    env.Append( CPPPATH= [ BOOST_INSTALL_PATH ],
                LIBPATH= [ BOOST_INSTALL_PATH + "/lib" ] )
    if DEBUG:
        BOOST_POSTFIX= "-vc90-mt-gd-1_35"
    else:
        BOOST_POSTFIX= "-vc90-mt-1_35"
        BOOST_PREFIX= "lib"

    # Newbase etc. from nearby
    #
    env.Append( CPPPATH= [ "../newbase/include","../imagine/include", "../tron/include" ],
                LIBPATH= [ "../newbase","../imagine", "../tron" ] )

elif LINUX:
    BOOST_POSTFIX= "-mt"    # Boost 1.36

    # Newbase, Imagine & Tron from system install
    #
    env.Append( CPPPATH= [ PREFIX+"/include/smartmet/",
                           PREFIX+"/include/smartmet/newbase",
                           PREFIX+"/include/smartmet/imagine",
                           PREFIX+"/include/smartmet/tron" ] )

elif OSX:
    # Newbase, Imagine & Tron from local CVS
    #
    env.Append( CPPPATH= [ "../newbase/include", "../imagine/include", "../tron/include" ],
                LIBPATH= [ "../newbase", "../imagine", "../tron" ] )

    # Boost from Fink
    #
    env.Append( CPPPATH= [ "/sw/include" ],
                LIBPATH= [ "/sw/lib" ] )

env.Append( LIBS= [ "smartmet_newbase", "smartmet_imagine", "smartmet_tron" ] )

env.Append( LIBS= [ BOOST_PREFIX+"boost_regex"+BOOST_POSTFIX, 
                    BOOST_PREFIX+"boost_filesystem"+BOOST_POSTFIX,
                    BOOST_PREFIX+"boost_system"+BOOST_POSTFIX,
                    BOOST_PREFIX+"boost_iostreams"+BOOST_POSTFIX ] )

#
# Freetype2, Cairomm-1.0, ...
#
if WINDOWS:
    env.Append( CPPPATH= [ "../cairomm-1.6.4" ],
                LIBS= [ "../cairomm-1.6.4/MSVC_Net2005/cairomm/Release/cairomm-1.0.lib" ] )

    env.Append( CPPPATH= [ "../cairo-1.6.4/src" ],
                #LIBS= "../cairo-1.6.4/src/release/cairo-static.lib" 
                )
else:
    env.ParseConfig("freetype-config --cflags --libs") 
    env.ParseConfig("pkg-config --cflags --libs cairomm-1.0")

print( env["CPPPATH"] )

# Other libs

if WINDOWS:
    { }
    env.Append( LIBS= [ "../lpng1231/libpng.lib", "../zlib123/zlib.lib" ] )
else:
    env.Append( LIBS= [ "pthread", "png", "jpeg", "z", "bz2" ] )


# Debug settings

if DEBUG:
    if WINDOWS:
        if env["CC"]=="cl":
            env.AppendUnique( CPPDEFINES=["_DEBUG","DEBUG"] )
            # Debug multithreaded DLL runtime, no opt.
            env.AppendUnique( CCFLAGS=["/MDd", "/Od"] )
            # Each obj gets own .PDB so parallel building (-jN) works
            env.AppendUnique( CCFLAGS=["/Zi", "/Fd${TARGET}.pdb"] )
    else:
        env.Append( CPPFLAGS=[ "-O0", "-g", "-Werror",

            # EXTRAFLAGS from orig. Makefile (for 'debug' target)
            #
            "-Wpointer-arith",
            "-Wcast-qual",
            "-Wcast-align",
            "-Wwrite-strings",
            "-Wconversion",
            "-Winline",
            #"-Wctor-dtor-privacy",
            "-Wnon-virtual-dtor",
            "-Wno-pmf-conversions",
            "-Wsign-promo",
            "-Wchar-subscripts",
            #"-Wold-style-cast",
        ] )

#
# Release settings
#
if RELEASE or PROFILE:
    if WINDOWS:
        if env["CC"]=="cl":
            # multithreaded DLL runtime, reasonable opt.
            env.AppendUnique( CCFLAGS=["/MD", "/Ox"] )
    else:
        env.Append( CPPDEFINES="NDEBUG",
                    CPPFLAGS= ["-O2",

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
        env.Append( CPPFLAGS="-g -pg" )


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

out= env.Program( "qdcontour", objs )

# Notice if the static lib has changed (and recompile)
#
if LINUX:
    Depends( out, PREFIX+"/lib64/libsmartmet_imagine.a" )
