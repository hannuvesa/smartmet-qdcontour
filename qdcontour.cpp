// ======================================================================
/*!
 * \file
 * \brief Main program for qdcontour
 */
// ======================================================================

#include "Globals.h"
#include "ColorTools.h"
#include "ContourSpec.h"
#include "GramTools.h"
#include "LazyQueryData.h"
#include "MetaFunctions.h"
#include "ProjectionFactory.h"
#include "TimeTools.h"

#include "imagine/NFmiColorTools.h"
#include "imagine/NFmiImage.h"			// for rendering
#include "imagine/NFmiGeoShape.h"		// for esri data
#include "imagine/NFmiText.h"			// for labels
#include "imagine/NFmiFontHershey.h"	// for Hershey fonts

#include "newbase/NFmiAreaFactory.h"
#include "newbase/NFmiCmdLine.h"			// command line options
#include "newbase/NFmiDataMatrix.h"
#include "newbase/NFmiDataModifierClasses.h"
#include "newbase/NFmiEnumConverter.h"		// FmiParameterName<-->string
#include "newbase/NFmiFileSystem.h"			// FileExists()
#include "newbase/NFmiLatLonArea.h"			// Geographic projection
#include "newbase/NFmiSettings.h"			// Configuration
#include "newbase/NFmiSmoother.h"		// for smoothing data
#include "newbase/NFmiStereographicArea.h"	// Stereographic projection
#include "newbase/NFmiStringTools.h"
#include "newbase/NFmiPreProcessor.h"

#include "boost/shared_ptr.hpp"

#include <fstream>
#include <list>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace std;
using namespace boost;
using namespace Imagine;

// ----------------------------------------------------------------------
// Global instance of global variables
// ----------------------------------------------------------------------

static Globals globals;

// ----------------------------------------------------------------------
// Usage
// ----------------------------------------------------------------------

void Usage(void)
{
  cout << "Usage: qdcontour [options] [conffiles]" << endl
	   << endl
	   << "Available options:" << endl
	   << "   -h\tDisplay this help information" << endl
	   << "   -v\tVerbose mode" << endl
	   << "   -f\tForce overwriting old images" << endl
	   << "   -q [querydata]\tSpecify querydata to be rendered" << endl
	   << endl;
}

// ----------------------------------------------------------------------
/*!
 * Test whether the given pixel coordinate is masked. This by definition
 * means the respective pixel in the given mask is not fully transparent.
 * Also, we define all pixels outside the mask image to be masked similarly
 * as pixel(0,0).
 *
 * \param thePoint The pixel coordinate
 * \param theMask The mask filename
 * \param theMaskImage The mask image
 * \return True, if the pixel is masked out
 */
// ----------------------------------------------------------------------

bool IsMasked(const NFmiPoint & thePoint,
			  const std::string & theMask,
			  const NFmiImage & theMaskImage)
{
  if(theMask.empty())
	return false;

  long theX = static_cast<int>(FmiRound(thePoint.X()));
  long theY = static_cast<int>(FmiRound(thePoint.Y()));

  // Handle outside pixels the same way as pixel 0,0
  if( theX<0 ||
	  theY<0 ||
	  theX>=theMaskImage.Width() ||
	  theY>=theMaskImage.Height())
	{
	  theX = 0;
	  theY = 0;
	}

  const NFmiColorTools::Color c = theMaskImage(theX,theY);
  const int alpha = NFmiColorTools::GetAlpha(c);

  return (alpha != NFmiColorTools::Transparent);
}

// ----------------------------------------------------------------------
/*!
 * \brief Parse the command line options
 */
// ----------------------------------------------------------------------

void parse_command_line(int argc, const char * argv[])
{

  NFmiCmdLine cmdline(argc,argv,"hvfq!");

  // Check for parsing errors

  if(cmdline.Status().IsError())
	throw runtime_error(cmdline.Status().ErrorLog().CharPtr());

  // Handle -h option

  if(cmdline.isOption('h'))
	{
	  Usage();
	  exit(0);
	}

  // Read -v option

  if(cmdline.isOption('v'))
    globals.verbose = true;

  // Read -f option

  if(cmdline.isOption('f'))
    globals.force = true;

  if(cmdline.isOption('q'))
	globals.cmdline_querydata = cmdline.OptionValue('q');

  // Read command filenames

  if(cmdline.NumberofParameters() == 0)
	throw runtime_error("Atleast one command line parameter is required");

  for(int i=1; i<=cmdline.NumberofParameters(); i++)
	globals.cmdline_files.push_back(cmdline.Parameter(i));

}

// ----------------------------------------------------------------------
/*!
 * \brief Read the given configuration script
 *
 * \param theName The file to read
 * \return The contents of the file, preprocessed
 */
// ----------------------------------------------------------------------

const string read_script(const string & theName)
{
  const bool strip_pound = false;
  NFmiPreProcessor processor(strip_pound);

  processor.SetDefine("#define");
  processor.SetIncluding("include", "", "");

  if(!processor.ReadAndStripFile(theName))
	{
	  if(!NFmiFileSystem::FileExists(theName))
		throw runtime_error("Script file '"+theName+"' does not exist");
	  throw runtime_error("Preprocessor failed to parse '"+theName+"'");

	}

  return processor.GetString();

}

// ----------------------------------------------------------------------
/*!
 * \brief Preprocess a configuration script for execution
 *
 * Currently the preprocessing consists only of handling the
 * possible -q command line option. When the option is present,
 * the equivalent 'querydata' command is inserted into the first
 * line of the script.
 *
 * \param theScript The script to preprocess
 * \return The preprocessed script
 */
// ----------------------------------------------------------------------

const string preprocess_script(const string & theScript)
{
  string ret;

  if(!globals.cmdline_querydata.empty())
	{
	  ret += "querydata ";
	  ret += globals.cmdline_querydata;
	  ret += '\n';
	}
  ret += theScript;

  return ret;
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle a comment token
 */
// ----------------------------------------------------------------------

void do_comment(istream & theInput)
{
  theInput.ignore(numeric_limits<std::streamsize>::max(),'\n');
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle the "cache" command
 */
// ----------------------------------------------------------------------

void do_cache(istream & theInput)
{
  int flag;
  theInput >> flag;
  
  if(theInput.fail())
	throw runtime_error("Processing the 'cache' command failed");

  globals.calculator.cache(flag);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle the "querydata" command
 */
// ----------------------------------------------------------------------

void do_querydata(istream & theInput)
{
  string newnames;
  theInput >> newnames;

  if(theInput.fail())
	throw runtime_error("Processing the 'querydata' command failed");

  if(globals.queryfilelist != newnames)
	{
	  globals.queryfilelist = newnames;
	  
	  // Delete possible old infos
	  
	  globals.clear_querystreams();

	  // Split the comma separated list into a real list

	  vector<string> qnames = NFmiStringTools::Split(globals.queryfilelist);
	  
	  // Read the queryfiles
	  
	  {
		vector<string>::const_iterator iter;
		for(iter=qnames.begin(); iter!=qnames.end(); ++iter)
		  {
			LazyQueryData * tmp = new LazyQueryData();
			string filename = NFmiFileSystem::FileComplete(*iter,globals.datapath);
			globals.queryfilenames.push_back(filename);
			tmp->Read(filename);
			globals.querystreams.push_back(tmp);
		  }
	  }
	}
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "querydatalevel" command
 */
// ----------------------------------------------------------------------

void do_querydatalevel(istream & theInput)
{
  theInput >> globals.querydatalevel;

  if(theInput.fail())
	throw runtime_error("Processing the 'querydatalevel' command failed");

}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "filter" command
 */
// ----------------------------------------------------------------------

void do_filter(istream & theInput)
{
  theInput >> globals.filter;

  if(theInput.fail())
	throw runtime_error("Processing the 'filter' command failed");

  if(globals.filter != "none" &&
	 globals.filter != "linear" &&
	 globals.filter != "min" &&
	 globals.filter != "max" &&
	 globals.filter != "mean" &&
	 globals.filter != "msum")
	{
	  throw runtime_error("Filtering mode '"+globals.filter+"' is not recognized");
	}

}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "timestepskip" command
 */
// ----------------------------------------------------------------------

void do_timestepskip(istream & theInput)
{
  theInput >> globals.timestepskip;

  if(theInput.fail())
	throw runtime_error("Processing the 'timestepskip' command failed");
  
  if(globals.timestepskip < 0)
	throw runtime_error("timestepskip cannot be negative");

  const int ludicruous = 30*24*60;	// 1 month
  if(globals.timestepskip > ludicruous)
	throw runtime_error("timestepskip "+NFmiStringTools::Convert(globals.timestepskip)+" is ridiculously large");

}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "timestep" command
 */
// ----------------------------------------------------------------------

void do_timestep(istream & theInput)
{
  theInput >> globals.timestep;
  globals.timeinterval = globals.timestep;

  if(theInput.fail())
	throw runtime_error("Processing the 'timestep' command failed");

  if(globals.timestep < 0)
	throw runtime_error("timestep cannot be negative");

  const int ludicruous = 30*24*60;	// 1 month
  if(globals.timestep > ludicruous)
	throw runtime_error("timestep "+NFmiStringTools::Convert(globals.timestep)+" is ridiculously large");

}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "timeinterval" command
 */
// ----------------------------------------------------------------------

void do_timeinterval(istream & theInput)
{
  theInput >> globals.timeinterval;

  if(theInput.fail())
	throw runtime_error("Processing the 'timeinterval' command failed");

  if(globals.timeinterval < 0)
	throw runtime_error("timeinterval cannot be negative");

  const int ludicruous = 30*24*60;	// 1 month
  if(globals.timeinterval > ludicruous)
	throw runtime_error("timestep "+NFmiStringTools::Convert(globals.timeinterval)+" is ridiculously large");

}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "timesteps" command
 */
// ----------------------------------------------------------------------

void do_timesteps(istream & theInput)
{
  theInput >> globals.timesteps;

  if(theInput.fail())
	throw runtime_error("Processing the 'timeinterval' command failed");

  if(globals.timesteps < 0)
	throw runtime_error("timesteps cannot be negative");

  const int ludicruous = 30*24*60;	// 1 month
  if(globals.timesteps > ludicruous)
	throw runtime_error("timesteps "+NFmiStringTools::Convert(globals.timesteps)+" is ridiculously large");

}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "timestamp" command
 */
// ----------------------------------------------------------------------

void do_timestamp(istream & theInput)
{
  theInput >> globals.timestampflag;

  if(theInput.fail())
	throw runtime_error("Processing the 'timestamp' command failed");

}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "timestampzone" command
 */
// ----------------------------------------------------------------------

void do_timestampzone(istream & theInput)
{
  theInput >> globals.timestampzone;

  if(theInput.fail())
	throw runtime_error("Processing the 'timestampzone' command failed");

}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "timesteprounding" command
 */
// ----------------------------------------------------------------------

void do_timesteprounding(istream & theInput)
{
  theInput >> globals.timesteprounding;

  if(theInput.fail())
	throw runtime_error("Processing the 'timesteprounding' command failed");

}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "timestampimage" command
 */
// ----------------------------------------------------------------------

void do_timestampimage(istream & theInput)
{
  theInput >> globals.timestampimage;

  if(theInput.fail())
	throw runtime_error("Processing the 'timestampimage' command failed");

  if(globals.timestampimage != "none" &&
	 globals.timestampimage != "obs" &&
	 globals.timestampimage != "for" &&
	 globals.timestampimage != "forobs")
	{
	  throw runtime_error("Unrecognized timestampimage mode '"+globals.timestampimage+"'");
	}
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "timestampimagexy" command
 */
// ----------------------------------------------------------------------

void do_timestampimagexy(istream & theInput)
{
  theInput >> globals.timestampimagex >> globals.timestampimagey;

  if(theInput.fail())
	throw runtime_error("Processing the 'timestampimagexy' command failed");

}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "projection" command
 */
// ----------------------------------------------------------------------

void do_projection(istream & theInput)
{
  theInput >> globals.projection;

  if(theInput.fail())
	throw runtime_error("Processing the 'projection' command failed");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "erase" command
 */
// ----------------------------------------------------------------------

void do_erase(istream & theInput)
{
  theInput >> globals.erase;

  if(theInput.fail())
	throw runtime_error("Processing the 'erase' command failed");

  ColorTools::checkcolor(globals.erase);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "fillrule" command
 */
// ----------------------------------------------------------------------

void do_fillrule(istream & theInput)
{
  theInput >> globals.fillrule;

  if(theInput.fail())
	throw runtime_error("Processing the 'fillrule' command failed");

  ColorTools::checkrule(globals.fillrule);

  if(!globals.shapespecs.empty())
	globals.shapespecs.back().fillrule(globals.fillrule);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "strokerule" command
 */
// ----------------------------------------------------------------------

void do_strokerule(istream & theInput)
{
  theInput >> globals.strokerule;

  if(theInput.fail())
	throw runtime_error("Processing the 'strokerule' command failed");

  ColorTools::checkrule(globals.strokerule);

  if(!globals.shapespecs.empty())
	globals.shapespecs.back().strokerule(globals.strokerule);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "directionparam" command
 */
// ----------------------------------------------------------------------

void do_directionparam(istream & theInput)
{
  theInput >> globals.directionparam;

  if(theInput.fail())
	throw runtime_error("Processing the 'directionparam' command failed");

  if(NFmiEnumConverter().ToEnum(globals.directionparam) == kFmiBadParameter)
	throw runtime_error("Unrecognized directionparam '"+globals.directionparam+"'");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "speedparam" command
 */
// ----------------------------------------------------------------------

void do_speedparam(istream & theInput)
{
  theInput >> globals.speedparam;

  if(theInput.fail())
	throw runtime_error("Processing the 'speedparam' command failed");

  if(NFmiEnumConverter().ToEnum(globals.speedparam) == kFmiBadParameter)
	throw runtime_error("Unrecognized speedparam '"+globals.speedparam+"'");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "arrowscale" command
 */
// ----------------------------------------------------------------------

void do_arrowscale(istream & theInput)
{
  theInput >> globals.arrowscale;

  if(theInput.fail())
	throw runtime_error("Processing the 'arrowscale' command failed");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "windarrowscale" command
 */
// ----------------------------------------------------------------------

void do_windarrowscale(istream & theInput)
{
  theInput >> globals.windarrowscaleA
		   >> globals.windarrowscaleB
		   >> globals.windarrowscaleC;

  if(theInput.fail())
	throw runtime_error("Processing the 'windarrowscale' command failed");

  if(globals.windarrowscaleB < 0)
	throw runtime_error("Second parameter of windarrowscale must be nonnegative");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "arrowfill" command
 */
// ----------------------------------------------------------------------

void do_arrowfill(istream & theInput)
{
  theInput >> globals.arrowfillcolor  >> globals.arrowfillrule;

  if(theInput.fail())
	throw runtime_error("Processing the 'arrowfill' command failed");

  ColorTools::checkcolor(globals.arrowfillcolor);
  ColorTools::checkrule(globals.arrowfillrule);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "arrowstroke" command
 */
// ----------------------------------------------------------------------

void do_arrowstroke(istream & theInput)
{
  theInput >> globals.arrowstrokecolor  >> globals.arrowstrokerule;

  if(theInput.fail())
	throw runtime_error("Processing the 'arrowstroke' command failed");

  ColorTools::checkcolor(globals.arrowstrokecolor);
  ColorTools::checkrule(globals.arrowstrokerule);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "arrowpath" command
 */
// ----------------------------------------------------------------------

void do_arrowpath(istream & theInput)
{
  theInput >> globals.arrowfile;

  if(theInput.fail())
	throw runtime_error("Processing the 'arrowpath' command failed");

  if(!NFmiFileSystem::FileExists(globals.arrowfile) &&
	 globals.arrowfile != "meteorological")
	{
	  throw runtime_error("The arrowpath file '"+globals.arrowfile+"' does not exist");
	}
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "windarrow" command
 */
// ----------------------------------------------------------------------

void do_windarrow(istream & theInput)
{
  double lon,lat;
  theInput >> lon >> lat;

  if(theInput.fail())
	throw runtime_error("Processing the 'windarrow' command failed");

  globals.arrowpoints.push_back(NFmiPoint(lon,lat));

}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "windarrows" command
 */
// ----------------------------------------------------------------------

void do_windarrows(istream & theInput)
{
  theInput >> globals.windarrowdx >> globals.windarrowdy;

  if(theInput.fail())
	throw runtime_error("Processing the 'windarrow' command failed");

  if(globals.windarrowdx < 0 || globals.windarrowdy < 0)
	throw runtime_error("windarrows parameters must be nonnegative");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "background" command
 */
// ----------------------------------------------------------------------

void do_background(istream & theInput)
{
  using NFmiFileSystem::FileComplete;

  theInput >> globals.background;

  if(globals.background == "none")
	globals.background = "";
  else
	globals.backgroundimage.Read(FileComplete(globals.background,
											  globals.mapspath));
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "foreground" command
 */
// ----------------------------------------------------------------------

void do_foreground(istream & theInput)
{
  using NFmiFileSystem::FileComplete;

  theInput >> globals.foreground;

  if(globals.foreground == "none")
	globals.foreground = "";
  else
	globals.foregroundimage.Read(FileComplete(globals.foreground,
											  globals.mapspath));
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "mask" command
 */
// ----------------------------------------------------------------------

void do_mask(istream & theInput)
{
  using NFmiFileSystem::FileComplete;

  theInput >> globals.mask;

  if(globals.mask == "none")
	globals.mask = "";
  else
	globals.maskimage.Read(FileComplete(globals.mask,
										globals.mapspath));
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "combine" command
 */
// ----------------------------------------------------------------------

void do_combine(istream & theInput)
{
  using NFmiFileSystem::FileComplete;

  theInput >> globals.combine;

  if(globals.combine == "none")
	globals.combine = "";
  else
	{
	  theInput >> globals.combinex
			   >> globals.combiney
			   >> globals.combinerule
			   >> globals.combinefactor;

	  ColorTools::checkrule(globals.combinerule);
	  
	  if(globals.combinefactor < 0 || globals.combinefactor > 1)
		throw runtime_error("combine blending factor must be in range 0-1");

	  globals.combineimage.Read(FileComplete(globals.combine,
											 globals.mapspath));
	}
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "foregroundrule" command
 */
// ----------------------------------------------------------------------

void do_foregroundrule(istream & theInput)
{
  theInput >> globals.foregroundrule;

  if(theInput.fail())
	throw runtime_error("Processing the 'foregroundrule' command failed");

  ColorTools::checkrule(globals.foregroundrule);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "savepath" command
 */
// ----------------------------------------------------------------------

void do_savepath(istream & theInput)
{
  theInput >> globals.savepath;

  if(theInput.fail())
	throw runtime_error("Processing the 'savepath' command failed");

  if(!NFmiFileSystem::DirectoryExists(globals.savepath))
	throw runtime_error("savepath "+globals.savepath+" does not exist");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "prefix" command
 */
// ----------------------------------------------------------------------

void do_prefix(istream & theInput)
{
  theInput >> globals.prefix;

  if(theInput.fail())
	throw runtime_error("Processing the 'prefix' command failed");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "suffix" command
 */
// ----------------------------------------------------------------------

void do_suffix(istream & theInput)
{
  theInput >> globals.suffix;

  if(theInput.fail())
	throw runtime_error("Processing the 'suffix' command failed");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "format" command
 */
// ----------------------------------------------------------------------

void do_format(istream & theInput)
{
  theInput >> globals.format;

  if(theInput.fail())
	throw runtime_error("Processing the 'format' command failed");

  if(globals.format != "png" &&
	 globals.format != "jpg" &&
	 globals.format != "jpeg" &&
	 globals.format != "gif")
	{
	  throw runtime_error("Image format +'"+globals.format+"' is not supported");
	}
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "gamma" command
 */
// ----------------------------------------------------------------------

void do_gamma(istream & theInput)
{
  theInput >> globals.gamma;

  if(theInput.fail())
	throw runtime_error("Processing the 'gamma' command failed");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "intent" command
 */
// ----------------------------------------------------------------------

void do_intent(istream & theInput)
{
  theInput >> globals.intent;

  if(theInput.fail())
	throw runtime_error("Processing the 'intent' command failed");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "pngquality" command
 */
// ----------------------------------------------------------------------

void do_pngquality(istream & theInput)
{
  theInput >> globals.pngquality;

  if(theInput.fail())
	throw runtime_error("Processing the 'pngquality' command failed");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "jpegquality" command
 */
// ----------------------------------------------------------------------

void do_jpegquality(istream & theInput)
{
  theInput >> globals.jpegquality;

  if(theInput.fail())
	throw runtime_error("Processing the 'jpegquality' command failed");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "savealpha" command
 */
// ----------------------------------------------------------------------

void do_savealpha(istream & theInput)
{
  theInput >> globals.savealpha;

  if(theInput.fail())
	throw runtime_error("Processing the 'savealpha' command failed");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "wantpalette" command
 */
// ----------------------------------------------------------------------

void do_wantpalette(istream & theInput)
{
  theInput >> globals.wantpalette;

  if(theInput.fail())
	throw runtime_error("Processing the 'wantpalette' command failed");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "forcepalette" command
 */
// ----------------------------------------------------------------------

void do_forcepalette(istream & theInput)
{
  theInput >> globals.forcepalette;

  if(theInput.fail())
	throw runtime_error("Processing the 'forcepalette' command failed");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "alphalimit" command
 */
// ----------------------------------------------------------------------

void do_alphalimit(istream & theInput)
{
  theInput >> globals.alphalimit;

  if(theInput.fail())
	throw runtime_error("Processing the 'alphalimit' command failed");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "hilimit" command
 */
// ----------------------------------------------------------------------

void do_hilimit(istream & theInput)
{
  float limit;
  theInput >> limit;

  if(theInput.fail())
	throw runtime_error("Processing the 'hilimit' command failed");

  if(!globals.specs.empty())
	globals.specs.back().exactHiLimit(limit);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "datalolimit" command
 */
// ----------------------------------------------------------------------

void do_datalolimit(istream & theInput)
{
  float limit;
  theInput >> limit;

  if(theInput.fail())
	throw runtime_error("Processing the 'datalolimit' command failed");

  if(!globals.specs.empty())
	globals.specs.back().dataLoLimit(limit);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "datahilimit" command
 */
// ----------------------------------------------------------------------

void do_datahilimit(istream & theInput)
{
  float limit;
  theInput >> limit;

  if(theInput.fail())
	throw runtime_error("Processing the 'datahilimit' command failed");

  if(!globals.specs.empty())
	globals.specs.back().dataHiLimit(limit);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "datareplace" command
 */
// ----------------------------------------------------------------------

void do_datareplace(istream & theInput)
{
  float src, dst;
  theInput >> src >> dst;

  if(theInput.fail())
	throw runtime_error("Processing the 'datareplace' command failed");

  if(!globals.specs.empty())
	globals.specs.back().replace(src,dst);
}

// ----------------------------------------------------------------------
// Main program.
// ----------------------------------------------------------------------

int domain(int argc, const char *argv[])
{
  // Initialize configuration variables

  NFmiSettings::Init();

  // Parse command line

  parse_command_line(argc,argv);

  // Komentotiedostosta luettavat optiot

  string theParam = "";
  string theShapeFileName = "";
  string theContourInterpolation = "Linear";
  string theSmoother = "None";
  float theSmootherRadius = 1.0;
  int theSmootherFactor = 1;


  int theContourDepth	= 0;
  int theContourTrianglesOn = 1;

  // Related variables

  NFmiImage theMaskImage;
  NFmiImage theCombineImage;


  // Process all command files
  // ~~~~~~~~~~~~~~~~~~~~~~~~~

  list<string>::const_iterator fileiter = globals.cmdline_files.begin();
  for( ; fileiter!=globals.cmdline_files.end(); ++fileiter)
    {
      // Get the script to be executed

      if(globals.verbose)
		cout << "Processing file: " << *fileiter << endl;

	  string text = read_script(*fileiter);
	  text = preprocess_script(text);

      // Process the commands

	  istringstream input(text);
      string command;
      while( input >> command)
		{
		  // Handle comments

		  if(command == "#")						do_comment(input);
		  else if(command[0] == '#')				do_comment(input);
		  else if(command == "//")					do_comment(input);
		  else if(command == "cache")				do_cache(input);
		  else if(command == "querydata")			do_querydata(input);
		  else if(command == "querydatalevel")		do_querydatalevel(input);
		  else if(command == "filter")				do_filter(input);
		  else if(command == "timestepskip")		do_timestepskip(input);
		  else if(command == "timestep")			do_timestep(input);
		  else if(command == "timeinterval")		do_timeinterval(input);
		  else if(command == "timesteps")			do_timesteps(input);
		  else if(command == "timestamp")			do_timestamp(input);
		  else if(command == "timestampzone")		do_timestampzone(input);
		  else if(command == "timesteprounding")	do_timesteprounding(input);
		  else if(command == "timestampimage")		do_timestampimage(input);
		  else if(command == "timestampimagexy")	do_timestampimagexy(input);
		  else if(command == "projection")			do_projection(input);
		  else if(command == "erase")				do_erase(input);
		  else if(command == "fillrule")			do_fillrule(input);
		  else if(command == "strokerule")			do_strokerule(input);
		  else if(command == "directionparam")		do_directionparam(input);
		  else if(command == "speedparam")			do_speedparam(input);
		  else if(command == "arrowscale")			do_arrowscale(input);
		  else if(command == "windarrowscale")		do_windarrowscale(input);
		  else if(command == "arrowfill")			do_arrowfill(input);
		  else if(command == "arrowstroke")			do_arrowstroke(input);
		  else if(command == "arrowpath")			do_arrowpath(input);
		  else if(command == "windarrow")			do_windarrow(input);
		  else if(command == "windarrows")			do_windarrows(input);
		  else if(command == "background")			do_background(input);
		  else if(command == "foreground")			do_foreground(input);
		  else if(command == "mask")				do_mask(input);
		  else if(command == "combine")				do_combine(input);
		  else if(command == "foregroundrule")		do_foregroundrule(input);
		  else if(command == "savepath")			do_savepath(input);
		  else if(command == "prefix")				do_prefix(input);
		  else if(command == "suffix")				do_suffix(input);
		  else if(command == "format")				do_format(input);
		  else if(command == "gamma")				do_gamma(input);
		  else if(command == "intent")				do_intent(input);
		  else if(command == "pngquality")			do_pngquality(input);
		  else if(command == "jpegquality")			do_jpegquality(input);
		  else if(command == "savealpha")			do_savealpha(input);
		  else if(command == "wantpalette")			do_wantpalette(input);
		  else if(command == "forcepalette")		do_forcepalette(input);
		  else if(command == "alphalimit")			do_alphalimit(input);
		  else if(command == "hilimit")				do_hilimit(input);
		  else if(command == "datalolimit")			do_datalolimit(input);
		  else if(command == "datahilimit")			do_datahilimit(input);
		  else if(command == "datareplace")			do_datareplace(input);

		  else if(command == "contourdepth")
			{
			  input >> theContourDepth;
			  if(!globals.specs.empty())
				globals.specs.back().contourDepth(theContourDepth);
			}

		  else if(command == "contourinterpolation")
			{
			  input >> theContourInterpolation;
			  if(!globals.specs.empty())
				globals.specs.back().contourInterpolation(theContourInterpolation);
			}
		  else if(command == "contourtriangles")
			{
			  input >> theContourTrianglesOn;
			}

		  else if(command == "smoother")
			{
			  input >> theSmoother;
			  if(!globals.specs.empty())
				globals.specs.back().smoother(theSmoother);
			}
		  else if(command == "smootherradius")
			{
			  input >> theSmootherRadius;
			  if(!globals.specs.empty())
				globals.specs.back().smootherRadius(theSmootherRadius);
			}
		  else if(command == "smootherfactor")
			{
			  input >> theSmootherFactor;
			  if(!globals.specs.empty())
				globals.specs.back().smootherFactor(theSmootherFactor);
			}
		  else if(command == "param")
			{
			  input >> theParam;
			  globals.specs.push_back(ContourSpec(theParam,
												  theContourInterpolation,
												  theSmoother,
												  theContourDepth,
												  theSmootherRadius,
												  theSmootherFactor));
			}

		  else if(command == "shape")
			{
			  input >> theShapeFileName;
			  string arg1;

			  input >> arg1;

			  if(arg1=="mark")
				{
				  string marker, markerrule;
				  float markeralpha;
				  input >> marker >> markerrule >> markeralpha;

				  ColorTools::checkrule(markerrule);
				  ShapeSpec spec(theShapeFileName);
				  spec.marker(marker,markerrule,markeralpha);
				  globals.shapespecs.push_back(spec);
				}
			  else
				{
				  string fillcolor = arg1;
				  string strokecolor;
				  input >> strokecolor;
				  NFmiColorTools::Color fill = ColorTools::checkcolor(fillcolor);
				  NFmiColorTools::Color stroke = ColorTools::checkcolor(strokecolor);

				  globals.shapespecs.push_back(ShapeSpec(theShapeFileName,
													fill,stroke,
													globals.fillrule,globals.strokerule));
				}
			}

		  else if(command == "contourfill")
			{
			  string slo,shi,scolor;
			  input >> slo >> shi >> scolor;

			  float lo,hi;
			  if(slo == "-")
				lo = kFloatMissing;
			  else
				lo = atof(slo.c_str());
			  if(shi == "-")
				hi = kFloatMissing;
			  else
				hi = atof(shi.c_str());

			  NFmiColorTools::Color color = ColorTools::checkcolor(scolor);

			  if(!globals.specs.empty())
				globals.specs.back().add(ContourRange(lo,hi,color,globals.fillrule));
			}

		  else if(command == "contourpattern")
			{
			  string slo,shi,spattern,srule;
			  float alpha;
			  input >> slo >> shi >> spattern >> srule >> alpha;

			  float lo,hi;
			  if(slo == "-")
				lo = kFloatMissing;
			  else
				lo = atof(slo.c_str());
			  if(shi == "-")
				hi = kFloatMissing;
			  else
				hi = atof(shi.c_str());

			  if(!globals.specs.empty())
				globals.specs.back().add(ContourPattern(lo,hi,spattern,srule,alpha));
			}

		  else if(command == "contourline")
			{
			  string svalue,scolor;
			  input >> svalue >> scolor;

			  float value;
			  if(svalue == "-")
				value = kFloatMissing;
			  else
				value = atof(svalue.c_str());

			  NFmiColorTools::Color color = ColorTools::checkcolor(scolor);
			  if(!globals.specs.empty())
				globals.specs.back().add(ContourValue(value,color,globals.strokerule));
			}

		  else if(command == "contourfills")
			{
			  float lo,hi,step;
			  string scolor1,scolor2;
			  input >> lo >> hi >> step >> scolor1 >> scolor2;

			  int color1 = ColorTools::checkcolor(scolor1);
			  int color2 = ColorTools::checkcolor(scolor2);

			  int steps = static_cast<int>((hi-lo)/step);

			  for(int i=0; i<steps; i++)
				{
				  float tmplo=lo+i*step;
				  float tmphi=lo+(i+1)*step;
				  int color = color1;	// in case steps=1
				  if(steps!=1)
					color = NFmiColorTools::Interpolate(color1,color2,i/(steps-1.0));
				  if(!globals.specs.empty())
					globals.specs.back().add(ContourRange(tmplo,tmphi,color,globals.fillrule));
				}
			}

		  else if(command == "contourlines")
			{
			  float lo,hi,step;
			  string scolor1,scolor2;
			  input >> lo >> hi >> step >> scolor1 >> scolor2;

			  int color1 = ColorTools::checkcolor(scolor1);
			  int color2 = ColorTools::checkcolor(scolor2);

			  int steps = static_cast<int>((hi-lo)/step);

			  for(int i=0; i<=steps; i++)
				{
				  float tmplo=lo+i*step;
				  int color = color1;	// in case steps=1
				  if(steps!=0)
					color = NFmiColorTools::Interpolate(color1,color2,i/static_cast<float>(steps));
				  if(!globals.specs.empty())
					globals.specs.back().add(ContourValue(tmplo,color,globals.strokerule));
				}
			}

		  else if(command == "clear")
			{
			  input >> command;
			  if(command=="contours")
				globals.specs.clear();
			  else if(command=="shapes")
				globals.shapespecs.clear();
			  else if(command=="cache")
				globals.calculator.clearCache();
			  else if(command=="arrows")
				{
				  globals.arrowpoints.clear();
				  globals.windarrowdx = 0;
				  globals.windarrowdy = 0;
				}
			  else if(command=="labels")
				{
				  list<ContourSpec>::iterator piter;
				  for(piter=globals.specs.begin(); piter!=globals.specs.end(); ++piter)
					piter->clearLabels();
				}
			  else
				throw runtime_error("Unknown clear target: " + command);
			}

		  else if(command == "labelmarker")
			{
			  string filename, rule;
			  float alpha;

			  input >> filename >> rule >> alpha;

			  if(!globals.specs.empty())
				{
				  globals.specs.back().labelMarker(filename);
				  globals.specs.back().labelMarkerRule(rule);
				  globals.specs.back().labelMarkerAlphaFactor(alpha);
				}
			}

		  else if(command == "labelfont")
			{
			  string font;
			  input >> font;
			  if(!globals.specs.empty())
				globals.specs.back().labelFont(font);
			}

		  else if(command == "labelsize")
			{
			  float size;
			  input >> size;
			  if(!globals.specs.empty())
				globals.specs.back().labelSize(size);
			}

		  else if(command == "labelstroke")
			{
			  string color,rule;
			  input >> color >> rule;
			  if(!globals.specs.empty())
				{
				  globals.specs.back().labelStrokeColor(ColorTools::checkcolor(color));
				  globals.specs.back().labelStrokeRule(rule);
				}
			}

		  else if(command == "labelfill")
			{
			  string color,rule;
			  input >> color >> rule;
			  if(!globals.specs.empty())
				{
				  globals.specs.back().labelFillColor(ColorTools::checkcolor(color));
				  globals.specs.back().labelFillRule(rule);
				}
			}

		  else if(command == "labelalign")
			{
			  string align;
			  input >> align;
			  if(!globals.specs.empty())
				globals.specs.back().labelAlignment(align);
			}

		  else if(command == "labelformat")
			{
			  string format;
			  input >> format;
			  if(format == "-") format = "";
			  if(!globals.specs.empty())
				globals.specs.back().labelFormat(format);
			}

		  else if(command == "labelmissing")
			{
			  string label;
			  input >> label;
			  if(label == "none") label = "";
			  if(!globals.specs.empty())
				globals.specs.back().labelMissing(label);
			}

		  else if(command == "labelangle")
			{
			  float angle;
			  input >> angle;
			  if(!globals.specs.empty())
				globals.specs.back().labelAngle(angle);
			}

		  else if(command == "labeloffset")
			{
			  float dx,dy;
			  input >> dx >> dy;
			  if(!globals.specs.empty())
				{
				  globals.specs.back().labelOffsetX(dx);
				  globals.specs.back().labelOffsetY(dy);
				}
			}

		  else if(command == "labelcaption")
			{
			  string name,align;
			  float dx,dy;
			  input >> name >> dx >> dy >> align;
			  if(!globals.specs.empty())
				{
				  globals.specs.back().labelCaption(name);
				  globals.specs.back().labelCaptionDX(dx);
				  globals.specs.back().labelCaptionDY(dy);
				  globals.specs.back().labelCaptionAlignment(align);
				}
			}

		  else if(command == "label")
			{
			  float lon,lat;
			  input >> lon >> lat;
			  if(!globals.specs.empty())
				globals.specs.back().add(NFmiPoint(lon,lat));
			}

		  else if(command == "labelxy")
			{
			  float lon,lat;
			  input >> lon >> lat;
			  int dx, dy;
			  input >> dx >> dy;
			  if(!globals.specs.empty())
				globals.specs.back().add(NFmiPoint(lon,lat),NFmiPoint(dx,dy));
			}

		  else if(command == "labels")
			{
			  int dx,dy;
			  input >> dx >> dy;
			  if(!globals.specs.empty())
				{
				  globals.specs.back().labelDX(dx);
				  globals.specs.back().labelDY(dy);
				}

			}

		  else if(command == "labelfile")
			{
			  string datafilename;
			  input >> datafilename;
			  ifstream datafile(datafilename.c_str());
			  if(!datafile)
				throw runtime_error("No data file named " + datafilename);
			  string datacommand;
			  while( datafile >> datacommand)
				{
				  if(datacommand == "#" || datacommand == "//")
					datafile.ignore(1000000,'\n');
				  else if(datacommand == "label")
					{
					  float lon,lat;
					  datafile >> lon >> lat;
					  if(!globals.specs.empty())
						globals.specs.back().add(NFmiPoint(lon,lat));
					}
				  else
					throw runtime_error("Unknown datacommand " + datacommand);
				}
			  datafile.close();
			}

		  else if(command == "draw")
			{
			  // Draw what?

			  input >> command;

			  // --------------------------------------------------
			  // Render shapes
			  // --------------------------------------------------

			  if(command == "shapes")
				{
				  // The output filename

				  string filename;
				  input >> filename;

				  auto_ptr<NFmiArea> theArea;

				  if(globals.projection.empty())
					throw runtime_error("No projection has been specified for rendering shapes");
				  else
					theArea.reset(NFmiAreaFactory::Create(globals.projection).release());


				  if(globals.verbose)
					cout << "Area corners are"
						 << endl
						 << "bottomleft\t= "
						 << theArea->BottomLeftLatLon().X()
						 << ','
						 << theArea->BottomLeftLatLon().Y()
						 << endl
						 << "topright\t= "
						 << theArea->TopRightLatLon().X()
						 << ','
						 << theArea->TopRightLatLon().Y()
						 << endl;

				  int imgwidth = static_cast<int>(theArea->Width()+0.5);
				  int imgheight = static_cast<int>(theArea->Height()+0.5);

				  // Initialize the background

				  NFmiImage theImage(imgwidth, imgheight);
				  theImage.SaveAlpha(globals.savealpha);
				  theImage.WantPalette(globals.wantpalette);
				  theImage.ForcePalette(globals.forcepalette);
				  if(globals.gamma>0) theImage.Gamma(globals.gamma);
				  if(!globals.intent.empty()) theImage.Intent(globals.intent);
				  if(globals.pngquality>=0) theImage.PngQuality(globals.pngquality);
				  if(globals.jpegquality>=0) theImage.JpegQuality(globals.jpegquality);
				  if(globals.alphalimit>=0) theImage.AlphaLimit(globals.alphalimit);

				  NFmiColorTools::Color erasecolor = ColorTools::checkcolor(globals.erase);
				  theImage.Erase(erasecolor);

				  // Draw all the shapes

				  list<ShapeSpec>::const_iterator iter;
				  list<ShapeSpec>::const_iterator begin = globals.shapespecs.begin();
				  list<ShapeSpec>::const_iterator end   = globals.shapespecs.end();

				  for(iter=begin; iter!=end; ++iter)
					{
					  NFmiGeoShape geo(iter->filename(),kFmiGeoShapeEsri);
					  geo.ProjectXY(*theArea);

					  if(iter->marker()=="")
						{
						  NFmiColorTools::NFmiBlendRule fillrule = ColorTools::checkrule(iter->fillrule());
						  NFmiColorTools::NFmiBlendRule strokerule = ColorTools::checkrule(iter->strokerule());
						  geo.Fill(theImage,iter->fillcolor(),fillrule);
						  geo.Stroke(theImage,iter->strokecolor(),strokerule);
						}
					  else
						{
						  NFmiColorTools::NFmiBlendRule markerrule = ColorTools::checkrule(iter->markerrule());

						  NFmiImage marker;
						  marker.Read(iter->marker());
						  geo.Mark(theImage,marker,markerrule,
								   kFmiAlignCenter,
								   iter->markeralpha());
						}
					}

				  string outfile = filename + "." + globals.format;
				  if(globals.verbose)
					cout << "Writing " << outfile << endl;
				  if(globals.format=="png")
					theImage.WritePng(outfile);
				  else if(globals.format=="jpg" || globals.format=="jpeg")
					theImage.WriteJpeg(outfile);
				  else if(globals.format=="gif")
					theImage.WriteGif(outfile);
				}

			  // --------------------------------------------------
			  // Generate imagemap data
			  // --------------------------------------------------

			  else if(command == "imagemap")
				{
				  // The relevant field name and filenames

				  string fieldname, filename;
				  input >> fieldname >> filename;


				  auto_ptr<NFmiArea> theArea;

				  if(globals.projection.empty())
					throw runtime_error("No projection has been specified for rendering shapes");
				  else
					theArea.reset(NFmiAreaFactory::Create(globals.projection).release());

				  // Generate map from all shapes in the list

				  list<ShapeSpec>::const_iterator iter;
				  list<ShapeSpec>::const_iterator begin = globals.shapespecs.begin();
				  list<ShapeSpec>::const_iterator end   = globals.shapespecs.end();

				  string outfile = filename + ".map";
				  ofstream out(outfile.c_str());
				  if(!out)
					throw runtime_error("Failed to open "+outfile+" for writing");
				  if(globals.verbose)
					cout << "Writing " << outfile << endl;

				  for(iter=begin; iter!=end; ++iter)
					{
					  NFmiGeoShape geo(iter->filename(),kFmiGeoShapeEsri);
					  geo.ProjectXY(*theArea);
					  geo.WriteImageMap(out,fieldname);
					}
				  out.close();
				}

			  // --------------------------------------------------
			  // Draw contours
			  // --------------------------------------------------

			  else if(command == "contours")
				{
				  // 1. Make sure query data has been read
				  // 2. Make sure image has been initialized
				  // 3. Loop over all times
				  //   4. If the time is acceptable,
				  //   5. Loop over all parameters
				  //     6. Fill all specified intervals
				  //     7. Patternfill all specified intervals
				  //     8. Stroke all specified contours
				  //   9. Overwrite with foreground if so desired
				  //   10. Loop over all parameters
				  //     11. Label all specified points
				  //   12. Draw arrows if requested
				  //   13. Save the image

				  if(globals.querystreams.empty())
					throw runtime_error("No query data has been read!");

				  auto_ptr<NFmiArea> theArea;

				  if(globals.projection.empty())
					throw runtime_error("No projection has been specified for rendering shapes");
				  else
					theArea.reset(NFmiAreaFactory::Create(globals.projection).release());

				  // This message intentionally ignores globals.verbose

				  if(!globals.background.empty())
					cout << "Contouring for background " << globals.background << endl;


				  if(globals.verbose)
					cout << "Area corners are"
						 << endl
						 << "bottomleft\t= "
						 << theArea->BottomLeftLatLon().X()
						 << ','
						 << theArea->BottomLeftLatLon().Y()
						 << endl
						 << "topright\t= "
						 << theArea->TopRightLatLon().X()
						 << ','
						 << theArea->TopRightLatLon().Y()
						 << endl;

				  // Establish querydata timelimits and initialize
				  // the XY-coordinates simultaneously.

				  // Note that we use world-coordinates when smoothing
				  // so that we can use meters as the smoothing radius.
				  // Also, this means the contours are independent of
				  // the image size.

				  NFmiTime utctime, time1, time2;

				  NFmiDataMatrix<float> vals;

				  unsigned int qi;
				  for(qi=0; qi<globals.querystreams.size(); qi++)
					{
					  // Initialize the queryinfo

					  globals.queryinfo = globals.querystreams[qi];
					  globals.queryinfo->FirstLevel();
					  if(globals.querydatalevel>0)
						{
						  int level = globals.querydatalevel;
						  while(--level > 0)
							globals.queryinfo->NextLevel();
						}

					  // Establish time limits
					  globals.queryinfo->LastTime();
					  utctime = globals.queryinfo->ValidTime();
					  NFmiTime t2 = TimeTools::ConvertZone(utctime,globals.timestampzone);
					  globals.queryinfo->FirstTime();
					  utctime = globals.queryinfo->ValidTime();
					  NFmiTime t1 = TimeTools::ConvertZone(utctime,globals.timestampzone);

					  if(qi==0)
						{
						  time1 = t1;
						  time2 = t2;
						}
					  else
						{
						  if(time1.IsLessThan(t1))
							time1 = t1;
						  if(!time2.IsLessThan(t2))
							time2 = t2;
						}

					}

				  if(globals.verbose)
					{
					  cout << "Data start time " << time1 << endl
						   << "Data end time " << time2 << endl;
					}

				  // Skip to first time

				  NFmiMetTime tmptime(time1,
									  globals.timesteprounding ?
									  (globals.timestep>0 ? globals.timestep : 1) :
									  1);

				  tmptime.ChangeByMinutes(globals.timestepskip);
				  if(globals.timesteprounding)
					tmptime.PreviousMetTime();
				  NFmiTime t = tmptime;

				  // Loop over all times

				  int imagesdone = 0;
				  bool labeldxdydone = false;
				  for(;;)
					{
					  if(imagesdone>=globals.timesteps)
						break;

					  // Skip to next time to be drawn

					  t.ChangeByMinutes(globals.timestep > 0 ? globals.timestep : 1);

					  // If the time is after time2, we're done

					  if(time2.IsLessThan(t))
						break;

					  // Search first time >= the desired time
					  // This is quaranteed to succeed since we've
					  // already tested against time2, the last available
					  // time.

					  bool ok = true;
					  for(qi=0; ok && qi<globals.querystreams.size(); qi++)
						{
						  globals.queryinfo = globals.querystreams[qi];
						  globals.queryinfo->ResetTime();
						  while(globals.queryinfo->NextTime())
							{
							  NFmiTime utc = globals.queryinfo->ValidTime();
							  NFmiTime loc = TimeTools::ConvertZone(utc,globals.timestampzone);
							  if(!loc.IsLessThan(t))
								break;
							}
						  NFmiTime utc = globals.queryinfo->ValidTime();
						  NFmiTime tnow = TimeTools::ConvertZone(utc,globals.timestampzone);

						  // we wanted

						  if(globals.timestep==0)
							t = tnow;

						  // If time is before time1, ignore it

						  if(t.IsLessThan(time1))
							{
							  ok = false;
							  break;
							}

						  // Is the time exact?

						  bool isexact = t.IsEqual(tnow);

						  // The previous acceptable time step in calculations
						  // Use NFmiTime, not NFmiMetTime to avoid rounding up!

						  NFmiTime tprev = t;
						  tprev.ChangeByMinutes(-globals.timeinterval);

						  bool hasprevious = !tprev.IsLessThan(time1);

						  // Skip this image if we are unable to render it

						  if(globals.filter=="none")
							{
							  // Cannot draw time with filter none
							  // if time is not exact.

							  ok = isexact;

							}
						  else if(globals.filter=="linear")
							{
							  // OK if is exact, otherwise previous step required

							  ok = !(!isexact && !hasprevious);
							}
						  else
							{
							  // Time must be exact, and previous steps
							  // are required

							  ok = !(!isexact || !hasprevious);
							}
						}

					  if(!ok)
						continue;

					  // The image is accepted for rendering, but
					  // we might not overwrite an existing one.
					  // Hence we update the counter here already.

					  imagesdone++;

					  // Create the filename

					  // The timestamp as a string

					  NFmiString datatimestr = t.ToStr(kYYYYMMDDHHMM);

					  if(globals.verbose)
						cout << "Time is " << datatimestr.CharPtr() << endl;

					  string filename =
						globals.savepath
						+ "/"
						+ globals.prefix
						+ datatimestr.CharPtr();

					  if(globals.timestampflag)
						{
						  for(qi=0; qi<globals.queryfilenames.size(); qi++)
							{
							  time_t secs = NFmiFileSystem::FileModificationTime(globals.queryfilenames[qi]);
							  NFmiTime tlocal(secs);
							  filename += "_" + tlocal.ToStr(kDDHHMM);
							}
						}

					  filename +=
						globals.suffix
						+ "."
						+ globals.format;

					  // In force-mode we always write, but otherwise
					  // we first check if the output image already
					  // exists. If so, we assume it is up to date
					  // and skip to the next time stamp.

					  if(!globals.force && !NFmiFileSystem::FileEmpty(filename))
						{
						  if(globals.verbose)
							cout << "Not overwriting " << filename << endl;
						  continue;
						}

					  // Initialize the background

					  int imgwidth = static_cast<int>(theArea->Width()+0.5);
					  int imgheight = static_cast<int>(theArea->Height()+0.5);

					  NFmiImage theImage(imgwidth,imgheight);
					  theImage.SaveAlpha(globals.savealpha);
					  theImage.WantPalette(globals.wantpalette);
					  theImage.ForcePalette(globals.forcepalette);
					  if(globals.gamma>0) theImage.Gamma(globals.gamma);
					  if(!globals.intent.empty()) theImage.Intent(globals.intent);
					  if(globals.pngquality>=0) theImage.PngQuality(globals.pngquality);
					  if(globals.jpegquality>=0) theImage.JpegQuality(globals.jpegquality);
					  if(globals.alphalimit>=0) theImage.AlphaLimit(globals.alphalimit);

					  NFmiColorTools::Color erasecolor = ColorTools::checkcolor(globals.erase);
					  theImage.Erase(erasecolor);

					  if(!globals.background.empty())
						theImage = globals.backgroundimage;

					  // Loop over all parameters

					  list<ContourSpec>::iterator piter;
					  list<ContourSpec>::iterator pbegin = globals.specs.begin();
					  list<ContourSpec>::iterator pend   = globals.specs.end();

					  for(piter=pbegin; piter!=pend; ++piter)
						{
						  // Establish the parameter

						  string name = piter->param();

						  bool ismeta = false;
						  ok = false;
						  FmiParameterName param = FmiParameterName(NFmiEnumConverter().ToEnum(name));

						  if(param==kFmiBadParameter)
							{
							  if(!MetaFunctions::isMeta(name))
								throw runtime_error("Unknown parameter "+name);
							  ismeta = true;
							  ok = true;
							  // We always assume the first querydata is ok
							  qi = 0;
							  globals.queryinfo = globals.querystreams[0];
							}
						  else
							{
							  // Find the proper queryinfo to be used
							  // Note that qi will be used later on for
							  // getting the coordinate matrices

							  for(qi=0; qi<globals.querystreams.size(); qi++)
								{
								  globals.queryinfo = globals.querystreams[qi];
								  globals.queryinfo->Param(param);
								  ok = globals.queryinfo->IsParamUsable();
								  if(ok) break;
								}
							}

						  if(!ok)
							throw runtime_error("The parameter is not usable: " + name);

						  if(globals.verbose)
							{
							  cout << "Param " << name << " from queryfile number "
								   << (qi+1) << endl;
							}

						  // Establish the contour method

						  string interpname = piter->contourInterpolation();
						  NFmiContourTree::NFmiContourInterpolation interp
							= NFmiContourTree::ContourInterpolationValue(interpname);
						  if(interp==NFmiContourTree::kFmiContourMissingInterpolation)
							throw runtime_error("Unknown contour interpolation method " + interpname);

						  // Get the values.

						  if(!ismeta)
							globals.queryinfo->Values(vals);
						  else
							vals = MetaFunctions::values(piter->param(),
														 globals.queryinfo);

						  // Replace values if so requested

						  if(piter->replace())
							vals.Replace(piter->replaceSourceValue(),piter->replaceTargetValue());

						  if(globals.filter=="none")
							{
							  // The time is known to be exact
							}
						  else if(globals.filter=="linear")
							{
							  NFmiTime utc = globals.queryinfo->ValidTime();
							  NFmiTime tnow = TimeTools::ConvertZone(utc,globals.timestampzone);
							  bool isexact = t.IsEqual(tnow);

							  if(!isexact)
								{
								  NFmiDataMatrix<float> tmpvals;
								  NFmiTime t2utc = globals.queryinfo->ValidTime();
								  NFmiTime t2 = TimeTools::ConvertZone(t2utc,globals.timestampzone);
								  globals.queryinfo->PreviousTime();
								  NFmiTime t1utc = globals.queryinfo->ValidTime();
								  NFmiTime t1 = TimeTools::ConvertZone(t1utc,globals.timestampzone);
								  if(!ismeta)
									globals.queryinfo->Values(tmpvals);
								  else
									tmpvals = MetaFunctions::values(piter->param(), globals.queryinfo);
								  if(piter->replace())
									tmpvals.Replace(piter->replaceSourceValue(),
													piter->replaceTargetValue());

								  // Data from t1,t2, we want t

								  long offset = t.DifferenceInMinutes(t1);
								  long range = t2.DifferenceInMinutes(t1);

								  float weight = (static_cast<float>(offset))/range;

								  vals.LinearCombination(tmpvals,weight,1-weight);

								}
							}
						  else
							{
							  NFmiTime tprev = t;
							  tprev.ChangeByMinutes(-globals.timeinterval);

							  NFmiDataMatrix<float> tmpvals;
							  int steps = 1;
							  for(;;)
								{
								  globals.queryinfo->PreviousTime();
								  NFmiTime utc = globals.queryinfo->ValidTime();
								  NFmiTime tnow = TimeTools::ConvertZone(utc,globals.timestampzone);
								  if(tnow.IsLessThan(tprev))
									break;

								  steps++;
								  if(!ismeta)
									globals.queryinfo->Values(tmpvals);
								  else
									tmpvals = MetaFunctions::values(piter->param(), globals.queryinfo);
								  if(piter->replace())
									tmpvals.Replace(piter->replaceSourceValue(),
													piter->replaceTargetValue());

								  if(globals.filter=="min")
									vals.Min(tmpvals);
								  else if(globals.filter=="max")
									vals.Max(tmpvals);
								  else if(globals.filter=="mean")
									vals += tmpvals;
								  else if(globals.filter=="sum")
									vals += tmpvals;
								}

							  if(globals.filter=="mean")
								vals /= steps;
							}


						  // Smoothen the values

						  NFmiSmoother smoother(piter->smoother(),
												piter->smootherFactor(),
												piter->smootherRadius());

						  shared_ptr<NFmiDataMatrix<NFmiPoint> > worldpts = globals.queryinfo->LocationsWorldXY(*theArea);
						  vals = smoother.Smoothen(*worldpts,vals);

						  // Find the minimum and maximum

						  float valmin = kFloatMissing;
						  float valmax = kFloatMissing;
						  for(unsigned int j=0; j<vals.NY(); j++)
							for(unsigned int i=0; i<vals.NX(); i++)
							  if(vals[i][j]!=kFloatMissing)
								{
								  if(valmin==kFloatMissing || vals[i][j]<valmin)
									valmin = vals[i][j];
								  if(valmax==kFloatMissing || vals[i][j]>valmax)
									valmax = vals[i][j];
								}

						  if(globals.verbose)
							cout << "Data range for " << name << " is " << valmin << "," << valmax << endl;

						  // Setup the contourer with the values

						  globals.calculator.data(vals);

						  // Save the data values at desired points for later
						  // use, this lets us avoid using InterpolatedValue()
						  // which does not use smoothened values.

						  // First, however, if this is the first image, we add
						  // the grid points to the set of points, if so requested

						  if(!labeldxdydone && piter->labelDX() > 0 && piter->labelDY() > 0)
							{
							  for(unsigned int j=0; j<worldpts->NY(); j+=piter->labelDY())
								for(unsigned int i=0; i<worldpts->NX(); i+=piter->labelDX())
								  piter->add(theArea->WorldXYToLatLon((*worldpts)[i][j]));
							}

						  piter->clearLabelValues();
						  if((piter->labelFormat() != "") &&
							 !piter->labelPoints().empty() )
							{
							  list<pair<NFmiPoint,NFmiPoint> >::const_iterator iter;

							  for(iter=piter->labelPoints().begin();
								  iter!=piter->labelPoints().end();
								  ++iter)
								{
								  NFmiPoint latlon = iter->first;
								  NFmiPoint ij = globals.queryinfo->LatLonToGrid(latlon);

								  float value;

								  if(fabs(ij.X()-FmiRound(ij.X()))<0.00001 &&
									 fabs(ij.Y()-FmiRound(ij.Y()))<0.00001)
									{
									  value = vals[FmiRound(ij.X())][FmiRound(ij.Y())];
									}
								  else
									{
									  int i = static_cast<int>(ij.X()); // rounds down
									  int j = static_cast<int>(ij.Y());
									  float v00 = vals.At(i,j,kFloatMissing);
									  float v10 = vals.At(i+1,j,kFloatMissing);
									  float v01 = vals.At(i,j+1,kFloatMissing);
									  float v11 = vals.At(i+1,j+1,kFloatMissing);
									  if(!globals.queryinfo->BiLinearInterpolation(ij.X(),
																			  ij.Y(),
																			  value,
																			  v00,v10,
																			  v01,v11))
										value = kFloatMissing;

									}
								  piter->addLabelValue(value);
								}
							}

						  // Fill the contours

						  list<ContourRange>::const_iterator citer;
						  list<ContourRange>::const_iterator cbegin;
						  list<ContourRange>::const_iterator cend;

						  cbegin = piter->contourFills().begin();
						  cend   = piter->contourFills().end();

						  for(citer=cbegin ; citer!=cend; ++citer)
							{
							  // Skip to next contour if this one is outside
							  // the value range. As a special case
							  // min=max=missing is ok, if both the limits
							  // are missing too. That is, when we are
							  // contouring missing values.

							  if(valmin==kFloatMissing || valmax==kFloatMissing)
								{
								  if(citer->lolimit()!=kFloatMissing &&
									 citer->hilimit()!=kFloatMissing)
									continue;
								}
							  else
								{
								  if(citer->lolimit()!=kFloatMissing &&
									 valmax<citer->lolimit())
									continue;
								  if(citer->hilimit()!=kFloatMissing &&
									 valmin>citer->hilimit())
									continue;
								}

							  bool exactlo = true;
							  bool exacthi = (citer->hilimit()!=kFloatMissing &&
											  piter->exactHiLimit()!=kFloatMissing &&
											  citer->hilimit()==piter->exactHiLimit());

							  NFmiPath path =
								globals.calculator.contour(*globals.queryinfo,
														   citer->lolimit(),
														   citer->hilimit(),
														   exactlo,
														   exacthi,
														   piter->dataLoLimit(),
														   piter->dataHiLimit(),
														   piter->contourDepth(),
														   interp,
														   theContourTrianglesOn);

							  if(globals.verbose && globals.calculator.wasCached())
								cout << "Using cached "
									 << citer->lolimit() << " - "
									 << citer->hilimit() << endl;

							  NFmiColorTools::NFmiBlendRule rule = ColorTools::checkrule(citer->rule());
							  path.Project(theArea.get());
							  path.Fill(theImage,citer->color(),rule);

							}

						  // Fill the contours with patterns

						  list<ContourPattern>::const_iterator patiter;
						  list<ContourPattern>::const_iterator patbegin;
						  list<ContourPattern>::const_iterator patend;

						  patbegin = piter->contourPatterns().begin();
						  patend   = piter->contourPatterns().end();

						  for(patiter=patbegin ; patiter!=patend; ++patiter)
							{
							  // Skip to next contour if this one is outside
							  // the value range. As a special case
							  // min=max=missing is ok, if both the limits
							  // are missing too. That is, when we are
							  // contouring missing values.

							  if(valmin==kFloatMissing || valmax==kFloatMissing)
								{
								  if(patiter->lolimit()!=kFloatMissing &&
									 patiter->hilimit()!=kFloatMissing)
									continue;
								}
							  else
								{
								  if(patiter->lolimit()!=kFloatMissing &&
									 valmax<patiter->lolimit())
									continue;
								  if(patiter->hilimit()!=kFloatMissing &&
									 valmin>patiter->hilimit())
									continue;
								}

							  bool exactlo = true;
							  bool exacthi = (patiter->hilimit()!=kFloatMissing &&
											  piter->exactHiLimit()!=kFloatMissing &&
											  patiter->hilimit()==piter->exactHiLimit());

							  NFmiPath path =
								globals.calculator.contour(*globals.queryinfo,
														   patiter->lolimit(),
														   patiter->hilimit(),
														   exactlo, exacthi,
														   piter->dataLoLimit(),
														   piter->dataHiLimit(),
														   piter->contourDepth(),
														   interp,
														   theContourTrianglesOn);

							  if(globals.verbose && globals.calculator.wasCached())
								cout << "Using cached "
									 << patiter->lolimit() << " - "
									 << patiter->hilimit() << endl;

							  NFmiColorTools::NFmiBlendRule rule = ColorTools::checkrule(patiter->rule());
							  NFmiImage pattern(patiter->pattern());

							  path.Project(theArea.get());
							  path.Fill(theImage,pattern,rule,patiter->factor());

							}

						  // Stroke the contours

						  list<ContourValue>::const_iterator liter;
						  list<ContourValue>::const_iterator lbegin;
						  list<ContourValue>::const_iterator lend;

						  lbegin = piter->contourValues().begin();
						  lend   = piter->contourValues().end();

						  for(liter=lbegin ; liter!=lend; ++liter)
							{
							  // Skip to next contour if this one is outside
							  // the value range.

							  if(valmin!=kFloatMissing && valmax!=kFloatMissing)
								{
								  if(liter->value()!=kFloatMissing &&
									 valmax<liter->value())
									continue;
								  if(liter->value()!=kFloatMissing &&
									 valmin>liter->value())
									continue;
								}

							  NFmiPath path =
								globals.calculator.contour(*globals.queryinfo,
														   liter->value(),
														   kFloatMissing,
														   true, false,
														   piter->dataLoLimit(),
														   piter->dataHiLimit(),
														   piter->contourDepth(),
														   interp,
														   theContourTrianglesOn);

							  NFmiColorTools::NFmiBlendRule rule = ColorTools::checkrule(liter->rule());
							  path.Project(theArea.get());
							  path.SimplifyLines(10);
							  path.Stroke(theImage,liter->color(),rule);

							}
						}

					  // Bang the foreground

					  if(!globals.foreground.empty())
						{
						  NFmiColorTools::NFmiBlendRule rule = ColorTools::checkrule(globals.foregroundrule);

						  theImage.Composite(globals.foregroundimage,rule,kFmiAlignNorthWest,0,0,1);

						}

					  // Draw wind arrows if so requested

					  NFmiEnumConverter converter;
					  if((!globals.arrowpoints.empty() ||
						  (globals.windarrowdx!=0 && globals.windarrowdy!=0)) &&
						 (globals.arrowfile!=""))
						{

						  FmiParameterName param = FmiParameterName(NFmiEnumConverter().ToEnum(globals.directionparam));
						  if(param==kFmiBadParameter)
							throw runtime_error("Unknown parameter "+globals.directionparam);

						  // Find the proper queryinfo to be used
						  // Note that qi will be used later on for
						  // getting the coordinate matrices

						  ok = false;
						  for(qi=0; qi<globals.querystreams.size(); qi++)
							{
							  globals.queryinfo = globals.querystreams[qi];
							  globals.queryinfo->Param(param);
							  ok = globals.queryinfo->IsParamUsable();
							  if(ok) break;
							}

						  if(!ok)
							throw runtime_error("Parameter is not usable: " + globals.directionparam);

						  // Read the arrow definition

						  NFmiPath arrowpath;
						  if(globals.arrowfile != "meteorological")
							{
							  ifstream arrow(globals.arrowfile.c_str());
							  if(!arrow)
								throw runtime_error("Could not open " + globals.arrowfile);
							  // Read in the entire file
							  string pathstring = NFmiStringTools::ReadFile(arrow);
							  arrow.close();

							  // Convert to a path

							  arrowpath.Add(pathstring);
							}

						  // Handle all given coordinates

						  list<NFmiPoint>::const_iterator iter;

						  for(iter=globals.arrowpoints.begin();
							  iter!=globals.arrowpoints.end();
							  ++iter)
							{

							  // The start point
							  NFmiPoint xy0 = theArea->ToXY(*iter);

							  // Skip rendering if the start point is masked

							  if(IsMasked(xy0,globals.mask,globals.maskimage))
								continue;

							  float dir = globals.queryinfo->InterpolatedValue(*iter);
							  if(dir==kFloatMissing)	// ignore missing
								continue;

							  float speed = -1;

							  if(globals.queryinfo->Param(FmiParameterName(converter.ToEnum(globals.speedparam))))
								speed = globals.queryinfo->InterpolatedValue(*iter);
							  globals.queryinfo->Param(FmiParameterName(converter.ToEnum(globals.directionparam)));


							  // Direction calculations

							  const float pi = 3.141592658979323;
							  const float length = 0.1;	// degrees

							  float x1 = iter->X()+sin(dir*pi/180)*length;
							  float y1 = iter->Y()+cos(dir*pi/180)*length;

							  NFmiPoint xy1 = theArea->ToXY(NFmiPoint(x1,y1));

							  // Calculate the actual angle

							  float alpha = atan2(xy1.X()-xy0.X(),
												  xy1.Y()-xy0.Y());

							  // Create a new path

							  NFmiPath thispath;

							  if(globals.arrowfile == "meteorological")
								thispath.Add(GramTools::metarrow(speed*globals.windarrowscaleC));
							  else
								thispath.Add(arrowpath);

							  if(speed>0 && speed!=kFloatMissing)
								thispath.Scale(globals.windarrowscaleA*log10(globals.windarrowscaleB*speed+1)+globals.windarrowscaleC);
							  thispath.Scale(globals.arrowscale);
							  thispath.Rotate(alpha*180/pi);
							  thispath.Translate(xy0.X(),xy0.Y());

							  // And render it

							  thispath.Fill(theImage,
											ColorTools::checkcolor(globals.arrowfillcolor),
											ColorTools::checkrule(globals.arrowfillrule));
							  thispath.Stroke(theImage,
											  ColorTools::checkcolor(globals.arrowstrokecolor),
											  ColorTools::checkrule(globals.arrowstrokerule));
							}

						  // Draw the full grid if so desired

						  if(globals.windarrowdx!=0 && globals.windarrowdy!=0)
							{

							  NFmiDataMatrix<float> speedvalues(vals.NX(),vals.NY(),-1);
							  if(globals.queryinfo->Param(FmiParameterName(converter.ToEnum(globals.speedparam))))
								globals.queryinfo->Values(speedvalues);
							  globals.queryinfo->Param(FmiParameterName(converter.ToEnum(globals.directionparam)));

							  shared_ptr<NFmiDataMatrix<NFmiPoint> > worldpts = globals.queryinfo->LocationsWorldXY(*theArea);
							  for(unsigned int j=0; j<worldpts->NY(); j+=globals.windarrowdy)
								for(unsigned int i=0; i<worldpts->NX(); i+=globals.windarrowdx)
								  {
									// The start point

									NFmiPoint latlon = theArea->WorldXYToLatLon((*worldpts)[i][j]);
									NFmiPoint xy0 = theArea->ToXY(latlon);

									// Skip rendering if the start point is masked
									if(IsMasked(xy0,
												globals.mask,
												globals.maskimage))
									  continue;

									float dir = vals[i][j];
									if(dir==kFloatMissing)	// ignore missing
									  continue;

									float speed = speedvalues[i][j];

									// Direction calculations

									const float pi = 3.141592658979323;
									const float length = 0.1;	// degrees

									float x0 = latlon.X();
									float y0 = latlon.Y();

									float x1 = x0+sin(dir*pi/180)*length;
									float y1 = y0+cos(dir*pi/180)*length;

									NFmiPoint xy1 = theArea->ToXY(NFmiPoint(x1,y1));

									// Calculate the actual angle

									float alpha = atan2(xy1.X()-xy0.X(),
														xy1.Y()-xy0.Y());

									// Create a new path

									NFmiPath thispath;
									if(globals.arrowfile == "meteorological")
									  thispath.Add(GramTools::metarrow(speed*globals.windarrowscaleC));
									else
									  thispath.Add(arrowpath);
									if(speed>0 && speed != kFloatMissing)
									  thispath.Scale(globals.windarrowscaleA*log10(globals.windarrowscaleB*speed+1)+globals.windarrowscaleC);
									thispath.Scale(globals.arrowscale);
									thispath.Rotate(alpha*180/pi);
									thispath.Translate(xy0.X(),xy0.Y());

									// And render it

									thispath.Fill(theImage,
												  ColorTools::checkcolor(globals.arrowfillcolor),
												  ColorTools::checkrule(globals.arrowfillrule));
									thispath.Stroke(theImage,
													ColorTools::checkcolor(globals.arrowstrokecolor),
													ColorTools::checkrule(globals.arrowstrokerule));
								  }
							}
						}

					  // Draw labels

					  for(piter=pbegin; piter!=pend; ++piter)
						{

						  // Draw label markers first

						  if(!piter->labelMarker().empty())
							{
							  // Establish that something is to be done

							  if(piter->labelPoints().empty())
								continue;

							  // Establish the marker specs

							  NFmiImage marker;
							  marker.Read(piter->labelMarker());

							  NFmiColorTools::NFmiBlendRule markerrule = ColorTools::checkrule(piter->labelMarkerRule());

							  float markeralpha = piter->labelMarkerAlphaFactor();

							  // Draw individual points

							  unsigned int pointnumber = 0;
							  list<pair<NFmiPoint,NFmiPoint> >::const_iterator iter;
							  for(iter=piter->labelPoints().begin();
								  iter!=piter->labelPoints().end();
								  ++iter)
								{
								  // The point in question

								  NFmiPoint xy = theArea->ToXY(iter->first);

								  // Skip rendering if the start point is masked

								  if(IsMasked(xy,
											  globals.mask,
											  globals.maskimage))
									continue;

								  // Skip rendering if LabelMissing is "" and value is missing
								  if(piter->labelMissing().empty())
									{
									  float value = piter->labelValues()[pointnumber++];
									  if(value == kFloatMissing)
										continue;
									}

								  theImage.Composite(marker,
													 markerrule,
													 kFmiAlignCenter,
													 FmiRound(xy.X()),
													 FmiRound(xy.Y()),
													 markeralpha);
								}

							}

						  // Label markers now drawn, only label texts remain

						  // Quick exit from loop if no labels are
						  // desired for this parameter

						  if(piter->labelFormat() == "")
							continue;

						  // Create the font object to be used

						  NFmiFontHershey font(piter->labelFont());

						  // Create the text object to be used

						  NFmiText text("",
										font,
										piter->labelSize(),
										0.0,	// x
										0.0,	// y
										AlignmentValue(piter->labelAlignment()),
										piter->labelAngle());


						  NFmiText caption(piter->labelCaption(),
										   font,
										   piter->labelSize(),
										   0.0,
										   0.0,
										   AlignmentValue(piter->labelCaptionAlignment()),
										   piter->labelAngle());

						  // The rules

						  NFmiColorTools::NFmiBlendRule fillrule
							= ColorTools::checkrule(piter->labelFillRule());

						  NFmiColorTools::NFmiBlendRule strokerule
							= ColorTools::checkrule(piter->labelStrokeRule());

						  // Draw labels at specifing latlon points if requested

						  list<pair<NFmiPoint,NFmiPoint> >::const_iterator iter;

						  int pointnumber = 0;
						  for(iter=piter->labelPoints().begin();
							  iter!=piter->labelPoints().end();
							  ++iter)
							{

							  // The point in question

							  float x,y;
							  if(iter->second.X() == kFloatMissing)
								{
								  NFmiPoint xy = theArea->ToXY(iter->first);
								  x = xy.X();
								  y = xy.Y();
								}
							  else
								{
								  x = iter->second.X();
								  y = iter->second.Y();
								}

							  // Skip rendering if the start point is masked

							  if(IsMasked(NFmiPoint(x,y),
										  globals.mask,
										  globals.maskimage))
								continue;

							  float value = piter->labelValues()[pointnumber++];

							  // Convert value to string
							  string strvalue = piter->labelMissing();

							  if(value!=kFloatMissing)
								{
								  char tmp[20];
								  sprintf(tmp,piter->labelFormat().c_str(),value);
								  strvalue = tmp;
								}

							  // Don't bother drawing empty strings
							  if(strvalue.empty())
								continue;

							  // Set new text properties

							  text.Text(strvalue);
							  text.X(x + piter->labelOffsetX());
							  text.Y(y + piter->labelOffsetY());

							  // And render the text

							  text.Fill(theImage,piter->labelFillColor(),fillrule);
							  text.Stroke(theImage,piter->labelStrokeColor(),strokerule);

							  // Then the label caption

							  if(!piter->labelCaption().empty())
								{
								  caption.X(text.X() + piter->labelCaptionDX());
								  caption.Y(text.Y() + piter->labelCaptionDY());
								  caption.Fill(theImage,piter->labelFillColor(),fillrule);
								  caption.Stroke(theImage,piter->labelStrokeColor(),strokerule);
								}

							}

						}



					  // Bang the combine image (legend, logo, whatever)

					  if(!globals.combine.empty())
						{
						  NFmiColorTools::NFmiBlendRule rule = ColorTools::checkrule(globals.combinerule);

						  theImage.Composite(globals.combineimage,
											 rule,
											 kFmiAlignNorthWest,
											 globals.combinex,
											 globals.combiney,
											 globals.combinefactor);

						}

					  // Finally, draw a time stamp on the image if so
					  // requested

					  string thestamp = "";

					  {
						int obsyy = t.GetYear();
						int obsmm = t.GetMonth();
						int obsdd = t.GetDay();
						int obshh = t.GetHour();
						int obsmi = t.GetMin();

						// Interpretation: The age of the forecast is the age
						// of the oldest forecast

						NFmiTime tfor;
						for(qi=0; qi<globals.querystreams.size(); qi++)
						  {
							globals.queryinfo = globals.querystreams[qi];
							NFmiTime futctime = globals.queryinfo->OriginTime();
							NFmiTime tlocal = TimeTools::ConvertZone(futctime,globals.timestampzone);
							if(qi==0 || tlocal.IsLessThan(tfor))
							  tfor = tlocal;
						  }

						int foryy = tfor.GetYear();
						int formm = tfor.GetMonth();
						int fordd = tfor.GetDay();
						int forhh = tfor.GetHour();
						int formi = tfor.GetMin();

						char buffer[100];

						if(globals.timestampimage == "obs")
						  {
							// hh:mi dd.mm.yyyy
							sprintf(buffer,"%02d:%02d %02d.%02d.%04d",
									obshh,obsmi,obsdd,obsmm,obsyy);
							thestamp = buffer;
						  }
						else if(globals.timestampimage == "for")
						  {
							// hh:mi dd.mm.yyyy
							sprintf(buffer,"%02d:%02d %02d.%02d.%04d",
									forhh,formi,fordd,formm,foryy);
							thestamp = buffer;
						  }
						else if(globals.timestampimage == "forobs")
						  {
							// hh:mi dd.mm.yyyy +hh
							long diff = t.DifferenceInMinutes(tfor);
							if(diff%60==0 && globals.timestep%60==0)
							  sprintf(buffer,"%02d.%02d.%04d %02d:%02d %s%ldh",
									  fordd,formm,foryy,forhh,formi,
									  (diff<0 ? "" : "+"), diff/60);
							else
							  sprintf(buffer,"%02d.%02d.%04d %02d:%02d %s%ldm",
									  fordd,formm,foryy,forhh,formi,
									  (diff<0 ? "" : "+"), diff);
							thestamp = buffer;
						  }
					  }

					  if(!thestamp.empty())
						{
						  NFmiFontHershey font("TimesRoman-Bold");

						  int x = globals.timestampimagex;
						  int y = globals.timestampimagey;

						  if(x<0) x+= theImage.Width();
						  if(y<0) y+= theImage.Height();

						  NFmiText text(thestamp,font,14,x,y,kFmiAlignNorthWest,0.0);

						  // And render the text

						  NFmiPath path = text.Path();

						  NFmiEsriBox box = path.BoundingBox();

						  NFmiPath rect;
						  int w = 4;
						  rect.MoveTo(box.Xmin()-w,box.Ymin()-w);
						  rect.LineTo(box.Xmax()+w,box.Ymin()-w);
						  rect.LineTo(box.Xmax()+w,box.Ymax()+w);
						  rect.LineTo(box.Xmin()-w,box.Ymax()+w);
						  rect.CloseLineTo();

						  rect.Fill(theImage,
									NFmiColorTools::MakeColor(180,180,180,32),
									NFmiColorTools::kFmiColorOver);

						  path.Stroke(theImage,
									  NFmiColorTools::Black,
									  NFmiColorTools::kFmiColorCopy);

						}

					  // dx and dy labels have now been extracted into a list,
					  // disable adding them again and again and again..

					  labeldxdydone = true;

					  // Save

					  if(globals.verbose)
						cout << "Writing " << filename << endl;
					  if(globals.format=="png")
						theImage.WritePng(filename);
					  else if(globals.format=="jpg" || globals.format=="jpeg")
						theImage.WriteJpeg(filename);
					  else if(globals.format=="gif")
						theImage.WriteGif(filename);
					}
				}

			  else
				throw runtime_error("draw " + command + " not implemented");
			}
		  else
			throw runtime_error("Unknown command " + command);
		}
    }
  return 0;
}

// ----------------------------------------------------------------------
// Main program.
// ----------------------------------------------------------------------

int main(int argc, const char* argv[])
{
  try
	{
	  return domain(argc, argv);
	}
  catch(const runtime_error & e)
	{
	  cerr << "Error: qdcontour failed due to" << endl
		   << "--> " << e.what() << endl;
	  return 1;
	}
}

// ======================================================================
