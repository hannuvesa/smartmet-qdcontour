#
# SConstruct for building QDContour
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
                 [windows_boost_path=<path>] [windows_prebuilt_path=<path>]
    
    Or just use 'make release|debug|profile', which point right back to us.
""") 

Decider("MD5-timestamp") 

DEBUG=      int( ARGUMENTS.get("debug", 0) ) != 0
PROFILE=    int( ARGUMENTS.get("profile", 0) ) != 0
RELEASE=    (not DEBUG) and (not PROFILE)     # default

OBJDIR=     ARGUMENTS.get("objdir","obj")
PREFIX=     ARGUMENTS.get("prefix","/usr")
LIBDIR=     ARGUMENTS.get("libdir","/usr/lib")

WINDOWS_BOOST_PATH= ARGUMENTS.get("windows_boost_path","")
WINDOWS_PREBUILT_PATH= ARGUMENTS.get("windows_prebuilt_path","")

#
# Base settings
#
env= Environment()

LINUX=  env["PLATFORM"]=="posix"
OSX=    env["PLATFORM"]=="darwin"
WINDOWS= env["PLATFORM"]=="win32"

out_postfix= WINDOWS and (DEBUG and "_debug" or "_release") or ""

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
	"-std=c++0x",
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

BOOST_POSTFIX = "-mt"
BOOST_PREFIX=""

if WINDOWS:
    # Installed from 'boost_1_36_0_setup.exe' from BoostPro Internet page.
    #
    env.Append( CPPPATH= [ WINDOWS_BOOST_PATH ],
                LIBPATH= [ WINDOWS_BOOST_PATH + "/lib" ] )
    if DEBUG:
        BOOST_POSTFIX= "-vc90-mt-gd-1_36"
    else:
        BOOST_POSTFIX= "-vc90-mt-1_36"
        BOOST_PREFIX= "lib"

    # Newbase etc. from nearby
    #
    env.Append( CPPPATH= [ "../newbase/include","../imagine2/include", "../tron/include" ],
                LIBPATH= [ "../newbase","../imagine2", "../tron" ] )

elif LINUX:

    # Newbase, Imagine & Tron from system install
    #
    env.Append( CPPPATH= [ PREFIX+"/include/smartmet/",
                           PREFIX+"/include/smartmet/newbase",
                           PREFIX+"/include/smartmet/imagine2",
                           PREFIX+"/include/smartmet/tron" ] )

elif OSX:
    # Newbase, Imagine & Tron from local CVS
    #
    env.Append( CPPPATH= [ "../newbase/include", "../imagine2/include", "../tron/include" ],
                LIBPATH= [ "../newbase", "../imagine2", "../tron" ] )

    # Boost from Fink
    #
    env.Append( CPPPATH= [ "/sw/include" ],
                LIBPATH= [ "/sw/lib" ] )

env.Append( LIBS= [ "smartmet_newbase"+out_postfix, 
                    "smartmet_imagine2"+out_postfix,
                    "smartmet_tron"+out_postfix ] )

env.Append( LIBS= [ BOOST_PREFIX+"boost_regex"+BOOST_POSTFIX, 
                    BOOST_PREFIX+"boost_filesystem"+BOOST_POSTFIX,
                    BOOST_PREFIX+"boost_system"+BOOST_POSTFIX,
                    BOOST_PREFIX+"boost_iostreams"+BOOST_POSTFIX,
                    BOOST_PREFIX+"boost_thread"+BOOST_POSTFIX ] )

#
# Freetype2, Cairomm-1.0, ...
#
if WINDOWS:
    env.Append( CPPPATH= [ WINDOWS_PREBUILT_PATH+"/cairomm-1.6.4" ],
                LIBS= [ WINDOWS_PREBUILT_PATH+"/cairomm-1.6.4/MSVC_Net2005/cairomm/Release/cairomm-1.0.lib" ] )

    env.Append( CPPPATH= [ WINDOWS_PREBUILT_PATH+"/cairo-1.6.4/src" ],
                )
else:
    env.ParseConfig("freetype-config --cflags --libs") 
    env.ParseConfig("pkg-config --cflags --libs cairomm-1.0")

print( env["CPPPATH"] )

# Other libs

if WINDOWS:
    env.Append( LIBS= [ WINDOWS_PREBUILT_PATH+"/lpng1231/lpng1231/libpng.lib", 
                        WINDOWS_PREBUILT_PATH+"/zlib123/zlib.lib" ] )
else:
    env.Append( LIBS= [ "pthread", "png", "jpeg", "z", "bz2", "rt"] )


# Debug settings

if DEBUG:
    if WINDOWS:
        if env["CC"]=="cl":
            env.AppendUnique( CPPDEFINES=["_DEBUG","DEBUG"],

                              CCFLAGS=[ "/MDd", "/Od",  # debug DLL runtime, no opt.
                                        "/Zi", "/Fd${TARGET}.pdb"   # each obj gets own .PDB so parallel building works
                                      ],
                              )
    else:
        env.Append( CPPFLAGS=[ "-O0", "-g", "-Werror",

            # EXTRAFLAGS from orig. Makefile (for 'debug' target)
            #
            "-Wpointer-arith",
            "-Wcast-qual",
            "-Wcast-align",
            "-Wwrite-strings",
            # "-Wconversion",
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

#if WINDOWS and env["CC"]=="cl":
#    if DEBUG:
#        env.AppendUnique( LINKFLAGS=["/NODEFAULTLIB:MSVCRT"] )
#    else:
#        env.AppendUnique( LINKFLAGS=["/NODEFAULTLIB:MSVCRTD"] )

#
# Profile settings
#
if PROFILE:
    if LINUX:
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
if WINDOWS:
    Depends( out, "../newbase/smartmet_newbase"+out_postfix+".lib" )
    Depends( out, "../imagine2/smartmet_imagine2"+out_postfix+".lib" )
    Depends( out, "../tron/smartmet_tron"+out_postfix+".lib" )
elif LINUX:
    Depends( out, LIBDIR+"/libsmartmet_newbase.a" )
    Depends( out, LIBDIR+"/libsmartmet_imagine2.a" )
    Depends( out, LIBDIR+"/libsmartmet_tron.a" )
