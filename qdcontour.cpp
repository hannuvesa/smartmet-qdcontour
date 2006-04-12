// ======================================================================
/*!
 * \file
 * \brief Main program for qdcontour
 */
// ======================================================================

#ifdef __GNUC__
 #if __GNUC__ < 3
  #define PERKELEEN_296
 #endif
#endif

#include "Globals.h"
#include "ColorTools.h"
#include "ContourSpec.h"
#include "GramTools.h"
#include "LazyCoordinates.h"
#include "LazyQueryData.h"
#include "MetaFunctions.h"
#include "ProjectionFactory.h"
#include "TimeTools.h"
#include "ExtremaLocator.h"

#include "NFmiColorTools.h"
#include "NFmiFace.h"
#include "NFmiFreeType.h"
#include "NFmiGpcTools.h"
#include "NFmiImage.h"			// for rendering
#include "NFmiGeoShape.h"		// for esri data

#include "NFmiCmdLine.h"			// command line options
#include "NFmiDataMatrix.h"
#include "NFmiDataModifierClasses.h"
#include "NFmiEnumConverter.h"		// FmiParameterName<-->string
#include "NFmiFileSystem.h"			// FileExists()
#include "NFmiInterpolation.h"		// Interpolation functions
#include "NFmiLatLonArea.h"			// Geographic projection
#include "NFmiSettings.h"			// Configuration
#include "NFmiSmoother.h"			// for smoothing data
#include "NFmiStereographicArea.h"	// Stereographic projection
#include "NFmiStringTools.h"
#include "NFmiPreProcessor.h"

#include <boost/shared_ptr.hpp>

#include <fstream>
#include <iomanip>
#include <list>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace std;
using namespace boost;
using namespace Imagine;

// ----------------------------------------------------------------------
// Global instance of enum converter for speed
// ----------------------------------------------------------------------

static NFmiEnumConverter converter;

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
 *
 * Any pixel outside the mask image is considered to be masker similarly
 * to the mask pixel nearest to it. This "extends" sea and land as is
 * usually expected when masking wind arrows etc.
 *
 * \param thePoint The pixel coordinate
 * \param theMask The mask filename
 * \param theMaskImage The mask image
 * \return True, if the pixel is masked out
 */
// ----------------------------------------------------------------------

bool IsMasked(const NFmiPoint & thePoint,
			  const std::string & theMask)
{
  if(theMask.empty())
	return false;
  
  int x = static_cast<int>(FmiRound(thePoint.X()));
  int y = static_cast<int>(FmiRound(thePoint.Y()));

  // Get the mask

  const NFmiImage & mask = globals.getImage(theMask);

  // Clip outside pixels

  x = min(max(x,0),mask.Width()-1);
  y = min(max(y,0),mask.Height()-1);

  const NFmiColorTools::Color c = mask(x,y);
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
 * \brief Check input stream validity
 */
// ----------------------------------------------------------------------

void check_errors(istream & theInput, const string & theFunction)
{
  if(theInput.fail())
	throw runtime_error("Processing the '"+theFunction+"' command failed");
}

// ----------------------------------------------------------------------
/*!
 * \brief Set queryinfo level
 *
 * A negative level value implies the first level in the data
 *
 * \param theInfo The queryinfo
 * \param theLevel The level value
 */
// ----------------------------------------------------------------------

bool set_level(LazyQueryData & theInfo, int theLevel)
{
  if(theLevel < 0)
	{
	  theInfo.FirstLevel();
	  return true;
	}
  else
	{
	  for(theInfo.ResetLevel(); theInfo.NextLevel(); )
		if(theInfo.Level()->LevelValue() == static_cast<unsigned int>(theLevel))
		  return true;
	  return false;
	}
}

// ----------------------------------------------------------------------
/*!
 * \brief Print debugging information on area object
 */
// ----------------------------------------------------------------------

void report_area(const NFmiArea & theArea)
{
  cout << "Area corners are"
	   << endl
	   << "bottomleft\t= "
	   << theArea.BottomLeftLatLon().X()
	   << ','
	   << theArea.BottomLeftLatLon().Y()
	   << endl
	   << "topright\t= "
	   << theArea.TopRightLatLon().X()
	   << ','
	   << theArea.TopRightLatLon().Y()
	   << endl;
}

// ----------------------------------------------------------------------
/*!
 * \brief Print debugging information on chosen querydata
 */
// ----------------------------------------------------------------------

void report_queryinfo(const string & theParam,
					  unsigned int theIndex)
{
  cout << "Param "
	   << theParam
	   << " from queryfile number "
	   << theIndex+1
	   << endl;
}

// ----------------------------------------------------------------------
/*!
 * \brief Print debugging information on data extrema
 */
// ----------------------------------------------------------------------

void report_extrema(const string & theParam,
					float theMin,
					float theMax)
{
  cout << "Data range for "
	   << theParam
	   << " is "
	   << theMin
	   << "..."
	   << theMax
	   << endl;
}

// ----------------------------------------------------------------------
/*!
 * \brief Write image to file with desired format
 */
// ----------------------------------------------------------------------

void write_image(NFmiImage & theImage,
				 const string & theName,
				 const string & theFormat)
{
  if(globals.verbose)
	cout << "Writing '" << theName << "'" << endl;

  if(globals.reducecolors)
	theImage.ReduceColors();

  theImage.Write(theName,theFormat);
}

// ----------------------------------------------------------------------
/*!
 * \brief Create a face from a font specification string
 *
 * The string is of the form <fontname>:<width>x<height>
 * If width or height is zero, Freetype will calculate it
 * so that proper aspect ratio is preserved.
 */
// ----------------------------------------------------------------------

Imagine::NFmiFace make_face(const string & theSpec)
{
  return NFmiFace(theSpec);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle a comment token
 */
// ----------------------------------------------------------------------

void do_comment(istream & theInput)
{
#ifdef PERKELEEN_296
  theInput.ignore(1000000,'\n');
#else
  theInput.ignore(numeric_limits<std::streamsize>::max(),'\n');
#endif
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

  check_errors(theInput,"cache");

  globals.calculator.cache(flag != 0);
  globals.maskcalculator.cache(flag != 0);
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

  check_errors(theInput,"querydata");

  if(globals.queryfilelist != newnames)
	{
	  globals.queryfilelist = newnames;

	  // Delete possible old infos

	  globals.querystreams.clear();

	  // Split the comma separated list into a real list

	  vector<string> qnames = NFmiStringTools::Split(globals.queryfilelist);

	  // Read the queryfiles

	  {
		vector<string>::const_iterator iter;
		for(iter=qnames.begin(); iter!=qnames.end(); ++iter)
		  {
			boost::shared_ptr<LazyQueryData> tmp(new LazyQueryData());
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
 * \brief Handle "level" command
 */
// ----------------------------------------------------------------------

void do_level(istream & theInput)
{
  theInput >> globals.querydatalevel;

  check_errors(theInput,"level");

  if(!globals.specs.empty())
	globals.specs.back().level(globals.querydatalevel);

}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "filter" command
 */
// ----------------------------------------------------------------------

void do_filter(istream & theInput)
{
  theInput >> globals.filter;

  check_errors(theInput,"filter");

  if(globals.filter != "none" &&
	 globals.filter != "linear" &&
	 globals.filter != "min" &&
	 globals.filter != "max" &&
	 globals.filter != "mean" &&
	 globals.filter != "sum")
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

  check_errors(theInput,"timestepskip");

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

  check_errors(theInput,"timestep");

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

  check_errors(theInput,"timeinterval");

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

  check_errors(theInput,"timesteps");

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

  check_errors(theInput,"timestamp");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "timestampzone" command
 */
// ----------------------------------------------------------------------

void do_timestampzone(istream & theInput)
{
  theInput >> globals.timestampzone;

  check_errors(theInput,"timestampzone");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "timesteprounding" command
 */
// ----------------------------------------------------------------------

void do_timesteprounding(istream & theInput)
{
  theInput >> globals.timesteprounding;

  check_errors(theInput,"timesteprounding");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "timestampimage" command
 */
// ----------------------------------------------------------------------

void do_timestampimage(istream & theInput)
{
  theInput >> globals.timestampimage;

  check_errors(theInput,"timestampimage");

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

  check_errors(theInput,"timestampimagexy");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "timestampimageformat" command
 */
// ----------------------------------------------------------------------

void do_timestampimageformat(istream & theInput)
{
  theInput >> globals.timestampimageformat;

  check_errors(theInput,"timestampimageformat");

  if(globals.timestampimageformat != "hour" &&
	 globals.timestampimageformat != "hourdate" &&
	 globals.timestampimageformat != "hourdateyear")
	{
	  throw runtime_error("Unrecognized timestampimageformat '"+globals.timestampimageformat+"'");
	}
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "timestampimagefont" command
 */
// ----------------------------------------------------------------------

void do_timestampimagefont(istream & theInput)
{
  theInput >> globals.timestampimagefont;

  check_errors(theInput,"timestampimagefont");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "timestampimagecolor" command
 */
// ----------------------------------------------------------------------

void do_timestampimagecolor(istream & theInput)
{
  string scolor;
  theInput >> scolor;

  check_errors(theInput,"timestampimagecolor");

  globals.timestampimagecolor = ColorTools::checkcolor(scolor);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "timestampimagebackground" command
 */
// ----------------------------------------------------------------------

void do_timestampimagebackground(istream & theInput)
{
  string scolor;
  theInput >> scolor;

  check_errors(theInput,"timestampimagebackground");

  globals.timestampimagebackground = ColorTools::checkcolor(scolor);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "timestampimagemargin" command
 */
// ----------------------------------------------------------------------

void do_timestampimagemargin(istream & theInput)
{
  theInput >> globals.timestampimagexmargin
		   >> globals.timestampimageymargin;

  check_errors(theInput,"timestampimagemargin");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "projection" command
 */
// ----------------------------------------------------------------------

void do_projection(istream & theInput)
{
  theInput >> globals.projection;

  check_errors(theInput,"projection");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "erase" command
 */
// ----------------------------------------------------------------------

void do_erase(istream & theInput)
{
  theInput >> globals.erase;

  check_errors(theInput,"projection");

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

  check_errors(theInput,"fillrule");

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

  check_errors(theInput,"strokerule");

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

  check_errors(theInput,"directionparam");

  if(converter.ToEnum(globals.directionparam) == kFmiBadParameter)
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

  check_errors(theInput,"speedparam");

  if(converter.ToEnum(globals.speedparam) == kFmiBadParameter)
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

  check_errors(theInput,"arrowscale");
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

  check_errors(theInput,"windarrowscale");

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

  check_errors(theInput,"arrowfill");

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

  check_errors(theInput,"arrowstroke");

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

  check_errors(theInput,"arrowpath");

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

  check_errors(theInput,"windarrow");

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

  check_errors(theInput,"windarrows");

  if(globals.windarrowdx < 0 || globals.windarrowdy < 0)
	throw runtime_error("windarrows parameters must be nonnegative");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "windarrowsxy" command
 */
// ----------------------------------------------------------------------

void do_windarrowsxy(istream & theInput)
{
  theInput >> globals.windarrowsxyx0
		   >> globals.windarrowsxyy0
		   >> globals.windarrowsxydx
		   >> globals.windarrowsxydy;

  check_errors(theInput,"windarrowsxy");

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

  check_errors(theInput,"background");

  if(globals.background == "none")
	globals.background = "";
  else
	globals.background = FileComplete(globals.background,
									  globals.mapspath);
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

  check_errors(theInput,"foreground");

  if(globals.foreground == "none")
	globals.foreground = "";
  else
	globals.foreground = FileComplete(globals.foreground,
									  globals.mapspath);
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

  check_errors(theInput,"mask");

  if(globals.mask == "none")
	globals.mask = "";
  else
	globals.mask = FileComplete(globals.mask,
								globals.mapspath);
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

  check_errors(theInput,"combine");

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

	  globals.combine = FileComplete(globals.combine,
									 globals.mapspath);
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

  check_errors(theInput,"foregroundrule");

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

  check_errors(theInput,"savepath");

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

  check_errors(theInput,"prefix");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "suffix" command
 */
// ----------------------------------------------------------------------

void do_suffix(istream & theInput)
{
  theInput >> globals.suffix;

  check_errors(theInput,"suffix");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "format" command
 */
// ----------------------------------------------------------------------

void do_format(istream & theInput)
{
  theInput >> globals.format;

  check_errors(theInput,"format");

  if(globals.format != "png" &&
	 globals.format != "jpg" &&
	 globals.format != "jpeg" &&
	 globals.format != "pnm" &&
	 globals.format != "pgm" &&
	 globals.format != "wbmp" &&
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

  check_errors(theInput,"gamma");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "intent" command
 */
// ----------------------------------------------------------------------

void do_intent(istream & theInput)
{
  theInput >> globals.intent;

  check_errors(theInput,"intent");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "pngquality" command
 */
// ----------------------------------------------------------------------

void do_pngquality(istream & theInput)
{
  theInput >> globals.pngquality;

  check_errors(theInput,"pngquality");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "jpegquality" command
 */
// ----------------------------------------------------------------------

void do_jpegquality(istream & theInput)
{
  theInput >> globals.jpegquality;

  check_errors(theInput,"jpegquality");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "savealpha" command
 */
// ----------------------------------------------------------------------

void do_savealpha(istream & theInput)
{
  theInput >> globals.savealpha;

  check_errors(theInput,"savealpha");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "reducecolors" command
 */
// ----------------------------------------------------------------------

void do_reducecolors(istream & theInput)
{
  theInput >> globals.reducecolors;

  check_errors(theInput,"reducecolors");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "wantpalette" command
 */
// ----------------------------------------------------------------------

void do_wantpalette(istream & theInput)
{
  theInput >> globals.wantpalette;

  check_errors(theInput,"wantpalette");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "forcepalette" command
 */
// ----------------------------------------------------------------------

void do_forcepalette(istream & theInput)
{
  theInput >> globals.forcepalette;

  check_errors(theInput,"forcepalette");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "alphalimit" command
 */
// ----------------------------------------------------------------------

void do_alphalimit(istream & theInput)
{
  theInput >> globals.alphalimit;

  check_errors(theInput,"alphalimit");
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

  check_errors(theInput,"hilimit");

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

  check_errors(theInput,"datalolimit");

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

  check_errors(theInput,"datahilimit");

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

  check_errors(theInput,"datareplace");

  if(!globals.specs.empty())
	globals.specs.back().replace(src,dst);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "expanddata" command
 */
// ----------------------------------------------------------------------

void do_expanddata(istream & theInput)
{
  theInput >> globals.expanddata;
  check_errors(theInput,"expanddata");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "contourdepth" command
 */
// ----------------------------------------------------------------------

void do_contourdepth(istream & theInput)
{
  cerr << "Warning: contourdepth command is deprecated" << endl;
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "contourinterpolation" command
 */
// ----------------------------------------------------------------------

void do_contourinterpolation(istream & theInput)
{
  theInput >> globals.contourinterpolation;

  check_errors(theInput,"contourinterpolation");

  if(!globals.specs.empty())
	globals.specs.back().contourInterpolation(globals.contourinterpolation);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "contourtriangles" command
 */
// ----------------------------------------------------------------------

void do_contourtriangles(istream & theInput)
{
  theInput >> globals.contourtriangles;

  check_errors(theInput,"contourtriangles");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "smoother" command
 */
// ----------------------------------------------------------------------

void do_smoother(istream & theInput)
{
  theInput >> globals.smoother;

  check_errors(theInput,"smoother");

  if(!globals.specs.empty())
	globals.specs.back().smoother(globals.smoother);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "smootherradius" command
 */
// ----------------------------------------------------------------------

void do_smootherradius(istream & theInput)
{
  theInput >> globals.smootherradius;

  check_errors(theInput,"smootherradius");

  if(!globals.specs.empty())
	globals.specs.back().smootherRadius(globals.smootherradius);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "smootherfactor" command
 */
// ----------------------------------------------------------------------

void do_smootherfactor(istream & theInput)
{
  theInput >> globals.smootherfactor;

  check_errors(theInput,"smootherfactor");

  if(!globals.specs.empty())
	globals.specs.back().smootherFactor(globals.smootherfactor);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "param" command
 */
// ----------------------------------------------------------------------

void do_param(istream & theInput)
{
  string param;

  theInput >> param;

  check_errors(theInput,"param");

  ContourSpec spec(param,
				   globals.contourinterpolation,
				   globals.smoother,
				   globals.querydatalevel,
				   globals.smootherradius,
				   globals.smootherfactor);

  spec.contourMask(globals.contourmaskparam,
				   globals.contourmasklolimit,
				   globals.contourmaskhilimit);

  globals.specs.push_back(spec);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "shape" command
 */
// ----------------------------------------------------------------------

void do_shape(istream & theInput)
{
  string shapename, arg1;

  theInput >> shapename >> arg1;

  check_errors(theInput,"shape");

  if(arg1=="mark")
	{
	  string marker, markerrule;
	  float markeralpha;
	  theInput >> marker >> markerrule >> markeralpha;

	  ColorTools::checkrule(markerrule);
	  ShapeSpec spec(shapename);
	  spec.marker(marker,markerrule,markeralpha);
	  globals.shapespecs.push_back(spec);
	}
  else
	{
	  string fillcolor = arg1;
	  string strokecolor;
	  theInput >> strokecolor;
	  NFmiColorTools::Color fill = ColorTools::checkcolor(fillcolor);
	  NFmiColorTools::Color stroke = ColorTools::checkcolor(strokecolor);

	  globals.shapespecs.push_back(ShapeSpec(shapename,
											 fill,stroke,
											 globals.fillrule,
											 globals.strokerule));
	}

  check_errors(theInput,"shape");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "contourmask" command
 */
// ----------------------------------------------------------------------

void do_contourmask(istream & theInput)
{
  string sparam,slo,shi;
  theInput >> sparam >> slo >> shi;

  check_errors(theInput,"contourmask");

  float lo,hi;
  if(slo == "-")
	lo = kFloatMissing;
  else
	lo = NFmiStringTools::Convert<float>(slo);
  if(shi == "-")
	hi = kFloatMissing;
  else
	hi = NFmiStringTools::Convert<float>(shi);

  globals.contourmaskparam = sparam;
  globals.contourmasklolimit = lo;
  globals.contourmaskhilimit = hi;
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "contourfill" command
 */
// ----------------------------------------------------------------------

void do_contourfill(istream & theInput)
{
  string slo,shi,scolor;
  theInput >> slo >> shi >> scolor;

  check_errors(theInput,"contourfill");

  float lo,hi;
  if(slo == "-")
	lo = kFloatMissing;
  else
	lo = NFmiStringTools::Convert<float>(slo);
  if(shi == "-")
	hi = kFloatMissing;
  else
	hi = NFmiStringTools::Convert<float>(shi);

  NFmiColorTools::Color color = ColorTools::checkcolor(scolor);

  if(!globals.specs.empty())
	globals.specs.back().add(ContourRange(lo,hi,color,globals.fillrule));
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "contourpattern" command
 */
// ----------------------------------------------------------------------

void do_contourpattern(istream & theInput)
{
  string slo,shi,spattern,srule;
  float alpha;
  theInput >> slo >> shi >> spattern >> srule >> alpha;

  check_errors(theInput,"contourpattern");

  float lo,hi;
  if(slo == "-")
	lo = kFloatMissing;
  else
	lo = NFmiStringTools::Convert<float>(slo);
  if(shi == "-")
	hi = kFloatMissing;
  else
	hi = NFmiStringTools::Convert<float>(shi);

  if(!globals.specs.empty())
	globals.specs.back().add(ContourPattern(lo,hi,spattern,srule,alpha));
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "contoursymbol" command
 */
// ----------------------------------------------------------------------

void do_contoursymbol(istream & theInput)
{
  string slo,shi,spattern,srule;
  float alpha;
  theInput >> slo >> shi >> spattern >> srule >> alpha;

  check_errors(theInput,"contoursymbol");

  float lo,hi;
  if(slo == "-")
	lo = kFloatMissing;
  else
	lo = NFmiStringTools::Convert<float>(slo);
  if(shi == "-")
	hi = kFloatMissing;
  else
	hi = NFmiStringTools::Convert<float>(shi);

  if(!globals.specs.empty())
	globals.specs.back().add(ContourSymbol(lo,hi,spattern,srule,alpha));
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "contourfont" command
 */
// ----------------------------------------------------------------------

void do_contourfont(istream & theInput)
{
  float value;
  int symbol;
  string scolor, font;
  theInput >> value >> symbol >> scolor >> font;

  check_errors(theInput,"contourfont");

  NFmiColorTools::Color color = ColorTools::checkcolor(scolor);

  if(!globals.specs.empty())
	globals.specs.back().add(ContourFont(value,color,symbol,font));
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "contourline" command
 */
// ----------------------------------------------------------------------

void do_contourline(istream & theInput)
{
  string svalue,scolor;
  theInput >> svalue >> scolor;

  check_errors(theInput,"contourline");

  float value;
  if(svalue == "-")
	value = kFloatMissing;
  else
	value = NFmiStringTools::Convert<float>(svalue);

  NFmiColorTools::Color color = ColorTools::checkcolor(scolor);
  if(!globals.specs.empty())
	globals.specs.back().add(ContourValue(value,color,globals.strokerule));
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "contourfills" command
 */
// ----------------------------------------------------------------------

void do_contourfills(istream & theInput)
{
  float lo,hi,step;
  string scolor1,scolor2;
  theInput >> lo >> hi >> step >> scolor1 >> scolor2;

  check_errors(theInput,"contourfills");

  int color1 = ColorTools::checkcolor(scolor1);
  int color2 = ColorTools::checkcolor(scolor2);

  int steps = static_cast<int>((hi-lo)/step);

  for(int i=0; i<steps; i++)
	{
	  float tmplo=lo+i*step;
	  float tmphi=lo+(i+1)*step;
	  int color = color1;	// in case steps=1
	  if(steps!=1)
		color = NFmiColorTools::Interpolate(color1,color2,i/(steps-1.0f));
	  if(!globals.specs.empty())
		globals.specs.back().add(ContourRange(tmplo,
											  tmphi,
											  color,
											  globals.fillrule));
	}
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "contourlines" command
 */
// ----------------------------------------------------------------------

void do_contourlines(istream & theInput)
{
  float lo,hi,step;
  string scolor1,scolor2;
  theInput >> lo >> hi >> step >> scolor1 >> scolor2;

  check_errors(theInput,"contourlines");

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
		globals.specs.back().add(ContourValue(tmplo,
											  color,
											  globals.strokerule));
	}
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "contourlabel" command
 */
// ----------------------------------------------------------------------

void do_contourlabel(istream & theInput)
{
  float value;
  theInput >> value;

  check_errors(theInput,"contourlabel");

  if(globals.specs.empty())
	throw runtime_error("Must define parameter before contourlabel");

  globals.specs.back().add(ContourLabel(value));
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "contourlabels" command
 */
// ----------------------------------------------------------------------

void do_contourlabels(istream & theInput)
{
  float lo,hi,step;
  theInput >> lo >> hi >> step;

  check_errors(theInput,"contourlabels");

  if(globals.specs.empty())
	throw runtime_error("Must define parameter before contourlabels");

  int steps = static_cast<int>((hi-lo)/step);

  for(int i=0; i<=steps; i++)
	{
	  float tmplo=lo+i*step;
	  globals.specs.back().add(ContourLabel(tmplo));
	}
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "contourlabelfont" command
 */
// ----------------------------------------------------------------------

void do_contourlabelfont(istream & theInput)
{
  string font;

  theInput >> font;

  check_errors(theInput,"contourlabelfont");

  if(globals.specs.empty())
	throw runtime_error("Must define parameter before contourlabelfont");

  globals.specs.back().contourLabelFont(font);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "contourlabelcolor" command
 */
// ----------------------------------------------------------------------

void do_contourlabelcolor(istream & theInput)
{
  string scolor;

  theInput >> scolor;

  check_errors(theInput,"contourlabelcolor");

  if(globals.specs.empty())
	throw runtime_error("Must define parameter before contourlabelcolor");

  NFmiColorTools::Color color = ColorTools::checkcolor(scolor);

  globals.specs.back().contourLabelColor(color);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "contourlabelbackground" command
 */
// ----------------------------------------------------------------------

void do_contourlabelbackground(istream & theInput)
{
  string scolor;

  theInput >> scolor;

  check_errors(theInput,"contourlabelbackground");

  if(globals.specs.empty())
	throw runtime_error("Must define parameter before contourlabelbackground");

  NFmiColorTools::Color color = ColorTools::checkcolor(scolor);

  globals.specs.back().contourLabelBackgroundColor(color);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "contourlabelmargin" command
 */
// ----------------------------------------------------------------------

void do_contourlabelmargin(istream & theInput)
{
  int dx, dy;

  theInput >> dx >> dy;

  check_errors(theInput,"contourlabelmargin");

  if(globals.specs.empty())
	throw runtime_error("Must define parameter before contourlabelmargin");

  globals.specs.back().contourLabelBackgroundXMargin(dx);
  globals.specs.back().contourLabelBackgroundYMargin(dy);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "contourlabelimagemargin" command
 */
// ----------------------------------------------------------------------

void do_contourlabelimagemargin(istream & theInput)
{
  theInput >> globals.contourlabelimagexmargin
		   >> globals.contourlabelimageymargin;

  check_errors(theInput,"contourlabelimagemargin");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "contourlabelmindistsamevalue" command
 */
// ----------------------------------------------------------------------

void do_contourlabelmindistsamevalue(istream & theInput)
{
  float dist;
  theInput >> dist;

  check_errors(theInput,"contourlabelmindistsamevalue");

  globals.labellocator.minDistanceToSameValue(dist);

}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "contourlabelmindistdifferentvalue" command
 */
// ----------------------------------------------------------------------

void do_contourlabelmindistdifferentvalue(istream & theInput)
{
  float dist;
  theInput >> dist;

  check_errors(theInput,"contourlabelmindistdifferentvalue");

  globals.labellocator.minDistanceToDifferentValue(dist);

}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "contourlabelmindistdifferentparam" command
 */
// ----------------------------------------------------------------------

void do_contourlabelmindistdifferentparam(istream & theInput)
{
  float dist;

  theInput >> dist;

  check_errors(theInput,"contourlabelmindistdifferentparam");

  globals.labellocator.minDistanceToDifferentParameter(dist);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "contourfontmindistsamevalue" command
 */
// ----------------------------------------------------------------------

void do_contourfontmindistsamevalue(istream & theInput)
{
  float dist;
  theInput >> dist;

  check_errors(theInput,"contourfontmindistsamevalue");

  globals.symbollocator.minDistanceToSameValue(dist);

}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "contourfontmindistdifferentvalue" command
 */
// ----------------------------------------------------------------------

void do_contourfontmindistdifferentvalue(istream & theInput)
{
  float dist;
  theInput >> dist;

  check_errors(theInput,"contourfontmindistdifferentvalue");

  globals.symbollocator.minDistanceToDifferentValue(dist);

}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "contourlabelmindistdifferentparam" command
 */
// ----------------------------------------------------------------------

void do_contourfontmindistdifferentparam(istream & theInput)
{
  float dist;

  theInput >> dist;

  check_errors(theInput,"contourfontmindistdifferentparam");

  globals.symbollocator.minDistanceToDifferentParameter(dist);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "highpressure" command
 */
// ----------------------------------------------------------------------

void do_highpressure(istream & theInput)
{
  theInput >> globals.highpressureimage
		   >> globals.highpressurerule
		   >> globals.highpressurefactor;

  check_errors(theInput,"highpressure");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "lowpressure" command
 */
// ----------------------------------------------------------------------

void do_lowpressure(istream & theInput)
{
  theInput >> globals.lowpressureimage
		   >> globals.lowpressurerule
		   >> globals.lowpressurefactor;
  
  check_errors(theInput,"lowpressure");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "highpressureminimum" command
 */
// ----------------------------------------------------------------------

void do_highpressureminimum(istream & theInput)
{
  theInput >> globals.highpressureminimum;

  check_errors(theInput,"highpressureminimum");

}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "lowpressuremaximum" command
 */
// ----------------------------------------------------------------------

void do_lowpressuremaximum(istream & theInput)
{
  theInput >> globals.lowpressuremaximum;

  check_errors(theInput,"lowpressuremaximum");
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "pressuremindistsame" command
 */
// ----------------------------------------------------------------------

void do_pressuremindistsame(istream & theInput)
{
  float dist;
  theInput >> dist;
  check_errors(theInput,"mindistsame");

  globals.pressurelocator.minDistanceToSame(dist);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "pressuremindistdifferent" command
 */
// ----------------------------------------------------------------------

void do_pressuremindistdifferent(istream & theInput)
{
  float dist;
  theInput >> dist;
  check_errors(theInput,"mindistdifferent");

  globals.pressurelocator.minDistanceToDifferent(dist);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "labelmarker" command
 */
// ----------------------------------------------------------------------

void do_labelmarker(istream & theInput)
{
  string filename, rule;
  float alpha;

  theInput >> filename >> rule >> alpha;

  check_errors(theInput,"labelmarker");

  if(!globals.specs.empty())
	{
	  globals.specs.back().labelMarker(filename);
	  globals.specs.back().labelMarkerRule(rule);
	  globals.specs.back().labelMarkerAlphaFactor(alpha);
	}
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "labelfont" command
 */
// ----------------------------------------------------------------------

void do_labelfont(istream & theInput)
{
  string font;
  theInput >> font;

  check_errors(theInput,"labelfont");

  if(!globals.specs.empty())
	globals.specs.back().labelFont(font);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "labelcolor" command
 */
// ----------------------------------------------------------------------

void do_labelcolor(istream & theInput)
{
  string color;
  theInput >> color;

  check_errors(theInput,"labelcolor");

  if(!globals.specs.empty())
	globals.specs.back().labelColor(ColorTools::checkcolor(color));
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "labelrule" command
 */
// ----------------------------------------------------------------------

void do_labelrule(istream & theInput)
{
  string rule;
  theInput >> rule;

  check_errors(theInput,"labelrule");

  ColorTools::checkrule(rule);

  if(!globals.specs.empty())
	globals.specs.back().labelRule(rule);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "labelalign" command
 */
// ----------------------------------------------------------------------

void do_labelalign(istream & theInput)
{
  string align;
  theInput >> align;

  check_errors(theInput,"labelalign");

  if(!globals.specs.empty())
	globals.specs.back().labelAlignment(align);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "labelformat" command
 */
// ----------------------------------------------------------------------

void do_labelformat(istream & theInput)
{
  string format;
  theInput >> format;

  check_errors(theInput,"labelformat");

  if(format == "-") format = "";
  if(!globals.specs.empty())
	globals.specs.back().labelFormat(format);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "labelmissing" command
 */
// ----------------------------------------------------------------------

void do_labelmissing(istream & theInput)
{
  string label;
  theInput >> label;

  check_errors(theInput,"labelmissing");

  if(label == "none")
	label = "";

  if(!globals.specs.empty())
	globals.specs.back().labelMissing(label);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "labeloffset" command
 */
// ----------------------------------------------------------------------

void do_labeloffset(istream & theInput)
{
  float dx,dy;
  theInput >> dx >> dy;

  check_errors(theInput,"labeloffset");

  if(!globals.specs.empty())
	{
	  globals.specs.back().labelOffsetX(dx);
	  globals.specs.back().labelOffsetY(dy);
	}
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "labelcaption" command
 */
// ----------------------------------------------------------------------

void do_labelcaption(istream & theInput)
{
  string name,align;
  float dx,dy;
  theInput >> name >> dx >> dy >> align;

  check_errors(theInput,"labelcaption");

  if(!globals.specs.empty())
	{
	  globals.specs.back().labelCaption(name);
	  globals.specs.back().labelCaptionDX(dx);
	  globals.specs.back().labelCaptionDY(dy);
	  globals.specs.back().labelCaptionAlignment(align);
	}
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "label" command
 */
// ----------------------------------------------------------------------

void do_label(istream & theInput)
{
  float lon,lat;
  theInput >> lon >> lat;

  check_errors(theInput,"label");

  if(!globals.specs.empty())
	globals.specs.back().add(NFmiPoint(lon,lat));
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "labelxy" command
 */
// ----------------------------------------------------------------------

void do_labelxy(istream & theInput)
{
  float lon,lat;
  int dx, dy;
  theInput >> lon >> lat >> dx >> dy;

  check_errors(theInput,"labelxy");

  if(!globals.specs.empty())
	globals.specs.back().add(NFmiPoint(lon,lat),NFmiPoint(dx,dy));
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "labels" command
 */
// ----------------------------------------------------------------------

void do_labels(istream & theInput)
{
  float dx,dy;
  theInput >> dx >> dy;

  check_errors(theInput,"labels");

  if(dx < 0 || dy < 0)
	throw runtime_error("labels arguments must be nonnegative");

  if(!globals.specs.empty())
	{
	  globals.specs.back().labelDX(dx);
	  globals.specs.back().labelDY(dy);
	}
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "labelsxy" command
 */
// ----------------------------------------------------------------------

void do_labelsxy(istream & theInput)
{
  float x0,y0,dx,dy;
  theInput >> x0 >> y0 >> dx >> dy;

  check_errors(theInput,"labelsxy");

  if(dx < 0 || dy < 0)
	throw runtime_error("labelsxy arguments must be nonnegative");

  if(!globals.specs.empty())
	{
	  globals.specs.back().labelXyX0(x0);
	  globals.specs.back().labelXyY0(y0);
	  globals.specs.back().labelXyDX(dx);
	  globals.specs.back().labelXyDY(dy);
	}
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "labelfile" command
 */
// ----------------------------------------------------------------------

void do_labelfile(istream & theInput)
{
  string datafilename;
  theInput >> datafilename;

  check_errors(theInput,"labelfile");

  ifstream datafile(datafilename.c_str());
  if(!datafile)
	throw runtime_error("No data file named " + datafilename);
  string datacommand;
  while( datafile >> datacommand)
	{
	  if(datacommand == "#" || datacommand == "//")
#ifdef PERKELEEN_296
		datafile.ignore(1000000,'\n');
#else
		datafile.ignore(numeric_limits<std::streamsize>::max(),'\n');
#endif
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

// ----------------------------------------------------------------------
/*!
 * \brief Handle "units" command
 */
// ----------------------------------------------------------------------

void do_units(istream & theInput)
{
  string paramname, conversion;

  theInput >> paramname >> conversion;

  check_errors(theInput,"units");

  FmiParameterName param = FmiParameterName(converter.ToEnum(paramname));
  if(param == kFmiBadParameter)
	throw runtime_error("Unknown parametername '"+paramname+"'");
  globals.unitsconverter.setConversion(param,conversion);

}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "clear" command
 */
// ----------------------------------------------------------------------

void do_clear(istream & theInput)
{
  string command;

  theInput >> command;

  check_errors(theInput,"clear");

  if(command=="contours")
	{
	  globals.specs.clear();
	  globals.labellocator.clear();
	  globals.symbollocator.clear();
	  globals.highpressureimage.clear();
	  globals.lowpressureimage.clear();
	}
  else if(command=="contourmask")
	{
	  globals.contourmaskparam = "";
	  globals.contourmasklolimit = kFloatMissing;
	  globals.contourmaskhilimit = kFloatMissing;
	}
  else if(command=="shapes")
	globals.shapespecs.clear();
  else if(command=="cache")
	{
	  globals.calculator.clearCache();
	  globals.maskcalculator.clearCache();
	}
  else if(command=="arrows")
	{
	  globals.arrowpoints.clear();
	  globals.windarrowdx = 0;
	  globals.windarrowdy = 0;
	  globals.windarrowsxydx = -1;
	  globals.windarrowsxydy = -1;
	}
  else if(command=="labels")
	{
	  list<ContourSpec>::iterator it;
	  for(it=globals.specs.begin(); it!=globals.specs.end(); ++it)
		it->clearLabels();
	}
  else if(command=="pressure")
	{
	  globals.highpressureimage.clear();
	  globals.lowpressureimage.clear();
	}
  else if(command=="units")
	globals.unitsconverter.clear();
  else
	throw runtime_error("Unknown clear target: " + command);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "draw shapes" command
 */
// ----------------------------------------------------------------------

void do_draw_shapes(istream & theInput)
{
  // The output filename

  string filename;
  theInput >> filename;

  check_errors(theInput,"draw shapes");

  auto_ptr<NFmiArea> area = globals.createArea();

  if(globals.verbose)
	report_area(*area);

  int imgwidth = static_cast<int>(area->Width()+0.5);
  int imgheight = static_cast<int>(area->Height()+0.5);

  // Initialize the background

  NFmiImage image(imgwidth, imgheight);
  globals.setImageModes(image);

  NFmiColorTools::Color erasecolor = ColorTools::checkcolor(globals.erase);
  image.Erase(erasecolor);

  // Draw all the shapes

  list<ShapeSpec>::const_iterator iter;
  list<ShapeSpec>::const_iterator begin = globals.shapespecs.begin();
  list<ShapeSpec>::const_iterator end   = globals.shapespecs.end();

  for(iter=begin; iter!=end; ++iter)
	{
	  NFmiGeoShape geo(iter->filename(),kFmiGeoShapeEsri);
	  geo.ProjectXY(*area);

	  if(iter->marker()=="")
		{
		  NFmiColorTools::NFmiBlendRule fillrule = ColorTools::checkrule(iter->fillrule());
		  NFmiColorTools::NFmiBlendRule strokerule = ColorTools::checkrule(iter->strokerule());
		  geo.Fill(image,iter->fillcolor(),fillrule);
		  geo.Stroke(image,iter->strokecolor(),strokerule);
		}
	  else
		{
		  NFmiColorTools::NFmiBlendRule markerrule = ColorTools::checkrule(iter->markerrule());

		  const NFmiImage & marker = globals.getImage(iter->marker());
		  geo.Mark(image,marker,markerrule,
				   kFmiAlignCenter,
				   iter->markeralpha());
		}
	}

  write_image(image,
			  filename+'.'+globals.format,
			  globals.format);
}

// ----------------------------------------------------------------------
/*!
 * \brief Handle "draw imagemap" command
 */
// ----------------------------------------------------------------------

void do_draw_imagemap(istream & theInput)
{
  // The relevant field name and filenames

  string fieldname, filename;
  theInput >> fieldname >> filename;

  check_errors(theInput,"draw imagemap");

  auto_ptr<NFmiArea> area = globals.createArea();

  // Generate map from all shapes in the list

  string outfile = filename + ".map";
  ofstream out(outfile.c_str());
  if(!out)
	throw runtime_error("Failed to open "+outfile+" for writing");

  if(globals.verbose)
	cout << "Writing " << outfile << endl;

  list<ShapeSpec>::const_iterator iter;
  list<ShapeSpec>::const_iterator begin = globals.shapespecs.begin();
  list<ShapeSpec>::const_iterator end   = globals.shapespecs.end();

  for(iter=begin; iter!=end; ++iter)
	{
	  NFmiGeoShape geo(iter->filename(),kFmiGeoShapeEsri);
	  geo.ProjectXY(*area);
	  geo.WriteImageMap(out,fieldname);
	}
  out.close();
}

// ----------------------------------------------------------------------
/*!
 * \brief Assign ID for parameter name
 *
 * This is needed so that Meta-functions would get an ID too
 *
 * \param theParam The parameter name
 * \return The unique ID, or kFmiBadParameter
 */
// ----------------------------------------------------------------------

int paramid(const string & theParam)
{
  if(MetaFunctions::isMeta(theParam))
	return MetaFunctions::id(theParam);
  else
	return converter.ToEnum(theParam);
}

// ----------------------------------------------------------------------
/*!
 * \brief Choose the queryinfo from the set of available datas
 */
// ----------------------------------------------------------------------

unsigned int choose_queryinfo(const string & theName,
							  int theLevel)
{
  if(globals.querystreams.size() == 0)
	throw runtime_error("No querydata has been specified");

  if(MetaFunctions::isMeta(theName))
	{
	  globals.queryinfo = globals.querystreams[0];
	  return 0;
	}
  else
	{
	  // Find the proper queryinfo to be used
	  
	  FmiParameterName param = FmiParameterName(converter.ToEnum(theName));

	  for(unsigned int qi=0; qi<globals.querystreams.size(); qi++)
		{
		  globals.queryinfo = globals.querystreams[qi];
		  globals.queryinfo->Param(param);
		  if(globals.queryinfo->IsParamUsable())
			{
			  if(set_level(*globals.queryinfo,theLevel))
				return qi;
			}
		}
	  if(theLevel < 0)
		throw runtime_error("Parameter '"+theName+"' is not available in the query files");
	  else
		throw runtime_error("Parameter '"+theName+"' on level " + NFmiStringTools::Convert(theLevel) + " is not available in the query files");
	}
}

// ----------------------------------------------------------------------
/*!
 * \brief Expand the data values
 *
 * First we try to calculate the mean from adjacent values.
 * If that fails, we try to calculate the mean from diagonal values.
 */
// ----------------------------------------------------------------------

void expand_data(NFmiDataMatrix<float> & theValues)
{
  NFmiDataModifierAvg calculator;

  NFmiDataMatrix<float> tmp(theValues);

  for(unsigned int j=0; j<theValues.NY(); j++)
	for(unsigned int i=0; i<theValues.NX(); i++)
	  {
		if(theValues[i][j] == kFloatMissing)
		  {
			calculator.Clear();
			calculator.Calculate(tmp.At(i-1,j,kFloatMissing));
			calculator.Calculate(tmp.At(i+1,j,kFloatMissing));
			calculator.Calculate(tmp.At(i,j-1,kFloatMissing));
			calculator.Calculate(tmp.At(i,j+1,kFloatMissing));
			if(calculator.CalculationResult() == kFloatMissing)
			  {
				calculator.Calculate(tmp.At(i-1,j-1,kFloatMissing));
				calculator.Calculate(tmp.At(i-1,j+1,kFloatMissing));
				calculator.Calculate(tmp.At(i+1,j-1,kFloatMissing));
				calculator.Calculate(tmp.At(i+1,j+1,kFloatMissing));
			  }
			theValues[i][j] = calculator.CalculationResult();
		  }
	  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Filter the data values
 */
// ----------------------------------------------------------------------

void filter_values(NFmiDataMatrix<float> & theValues,
				   const NFmiTime & theTime,
				   const ContourSpec & theSpec)
{
  if(globals.filter=="none")
	{
	  // The time is known to be exact
	}
  else if(globals.filter=="linear")
	{
	  NFmiTime tnow = globals.queryinfo->ValidTime();
	  bool isexact = theTime.IsEqual(tnow);
	  
	  if(!isexact)
		{
		  NFmiDataMatrix<float> tmpvals;
		  NFmiTime t2 = globals.queryinfo->ValidTime();
		  globals.queryinfo->PreviousTime();
		  NFmiTime t1 = globals.queryinfo->ValidTime();
		  if(!MetaFunctions::isMeta(theSpec.param()))
			{
			  globals.queryinfo->Values(tmpvals);
			  globals.unitsconverter.convert(FmiParameterName(globals.queryinfo->GetParamIdent()),tmpvals);
			}
		  else
			tmpvals = MetaFunctions::values(theSpec.param(), *globals.queryinfo);
		  if(theSpec.replace())
			tmpvals.Replace(theSpec.replaceSourceValue(),
							theSpec.replaceTargetValue());
		  
		  // Data from t1,t2, we want t
		  
		  long offset = theTime.DifferenceInMinutes(t1);
		  long range = t2.DifferenceInMinutes(t1);
		  
		  float weight = (static_cast<float>(offset))/range;
		  
		  theValues.LinearCombination(tmpvals,weight,1-weight);
		  
		}
	}
  else
	{
	  NFmiTime tprev = theTime;
	  tprev.ChangeByMinutes(-globals.timeinterval);

	  if(MetaFunctions::isMeta(theSpec.param()))
		throw runtime_error("Unable to filter metafunctions - use newbase parameters only");
	  

	  NFmiMetTime tnow(theTime,60);
	  NFmiDataMatrix<float> tmpvals;
	  int steps = 1;
	  for(;;)
		{

		  globals.queryinfo->Values(tmpvals,tnow);
		  globals.unitsconverter.convert(FmiParameterName(globals.queryinfo->GetParamIdent()),tmpvals);

		  if(theSpec.replace())
			tmpvals.Replace(theSpec.replaceSourceValue(),
							theSpec.replaceTargetValue());

		  if(globals.filter=="min")
			theValues.Min(tmpvals);
		  else if(globals.filter=="max")
			theValues.Max(tmpvals);
		  else if(globals.filter=="mean")
			theValues += tmpvals;
		  else if(globals.filter=="sum")
			theValues += tmpvals;
		  
		  ++steps;
		  --tnow;

		  if(tnow.IsLessThan(tprev))
			break;

		}
	  
	  if(globals.filter=="mean")
		theValues /= static_cast<float>(steps);
	}
}

// ----------------------------------------------------------------------
/*!
 * \brief Save grid values for later labelling
 */
// ----------------------------------------------------------------------

void add_label_grid_values(ContourSpec & theSpec,
						   const NFmiArea & theArea,
						   const LazyCoordinates & thePoints)
{
  const float dx = theSpec.labelDX();
  const float dy = theSpec.labelDY();

  if(dx>0 && dy>0)
	{
	  // Fast code for lattice coordinates
	  if(dx == static_cast<int>(dx) && dy == static_cast<int>(dy) )
		{
		  const int dj = static_cast<int>(dy);
		  const int di = static_cast<int>(dx);
		  for(unsigned int j=0; j<thePoints.NY(); j+=dj)
			for(unsigned int i=0; i<thePoints.NX(); i+=di)
			  theSpec.add(theArea.WorldXYToLatLon(thePoints(i,j)));
		}
	  else
		{
		  for(float y=0; y<=thePoints.NY()-1; y+=dy)
			{
			  const int j = static_cast<int>(floor(y));
			  const float dj = y-j;
			  for(float x=0; x<=thePoints.NX()-1; x+=dx)
				{
				  const int i = static_cast<int>(floor(x));
				  const float di = x-i;
				  const NFmiPoint bad(kFloatMissing,kFloatMissing);
				  const NFmiPoint xy = NFmiInterpolation::BiLinear(di, dj,
																   thePoints(i,j+1,bad),
																   thePoints(i+1,j+1,bad),
																   thePoints(i,j,bad),
																   thePoints(i+1,j,bad));
				  theSpec.add(theArea.WorldXYToLatLon(xy));
				}
			}
		}
	}
}


// ----------------------------------------------------------------------
/*!
 * \brief Save pixelgrid values for later labelling
 */
// ----------------------------------------------------------------------

void add_label_pixelgrid_values(ContourSpec & theSpec,
								const NFmiArea & theArea,
								const NFmiImage & theImage,
								const NFmiDataMatrix<float> & theValues)
{
  theSpec.clearPixelLabels();

  const float x0 = theSpec.labelXyX0();
  const float y0 = theSpec.labelXyY0();
  const float dx = theSpec.labelXyDX();
  const float dy = theSpec.labelXyDY();
  
  if(dx>0 && dy>0)
	{
	  for(float y=y0; y<=theImage.Height(); y+=dy)
		for(float x=x0; x<=theImage.Width(); x+=dx)
		  {
			NFmiPoint latlon = theArea.ToLatLon(NFmiPoint(x,y));
			NFmiPoint ij = globals.queryinfo->LatLonToGrid(latlon);
		  
			int i = static_cast<int>(ij.X()); // rounds down
			int j = static_cast<int>(ij.Y());
			float value = static_cast<float>(NFmiInterpolation::BiLinear(ij.X()-floor(ij.X()),
																		 ij.Y()-floor(ij.Y()),
																		 theValues.At(i,j+1,kFloatMissing),
																		 theValues.At(i+1,j+1,kFloatMissing),
																		 theValues.At(i,j,kFloatMissing),

																		 theValues.At(i+1,j,kFloatMissing)));
			theSpec.addPixelLabel(NFmiPoint(x,y),value);
		  }
	}
}

// ----------------------------------------------------------------------
/*!
 * \brief Save point values for later labelling
 */
// ----------------------------------------------------------------------

void add_label_point_values(ContourSpec & theSpec,
							const NFmiArea & theArea,
							const NFmiDataMatrix<float> & theValues)
{
  theSpec.clearLabelValues();
  if((theSpec.labelFormat() != "") &&
	 !theSpec.labelPoints().empty() )
	{
	  list<pair<NFmiPoint,NFmiPoint> >::const_iterator it;

	  for(it=theSpec.labelPoints().begin();
		  it!=theSpec.labelPoints().end();
		  ++it)
		{
		  NFmiPoint latlon = it->first;
		  NFmiPoint ij = globals.queryinfo->LatLonToGrid(latlon);
		  
		  int i = static_cast<int>(ij.X()); // rounds down
		  int j = static_cast<int>(ij.Y());
		  float value = static_cast<float>(NFmiInterpolation::BiLinear(ij.X()-floor(ij.X()),
																	   ij.Y()-floor(ij.Y()),
																	   theValues.At(i,j+1,kFloatMissing),
																	   theValues.At(i+1,j+1,kFloatMissing),
																	   theValues.At(i,j,kFloatMissing),
																	   theValues.At(i+1,j,kFloatMissing)));

		  theSpec.addLabelValue(value);
		}
	}
}

// ----------------------------------------------------------------------
/*!
 * \brief Draw label markers
 */
// ----------------------------------------------------------------------

void draw_label_markers(NFmiImage & theImage,
						const ContourSpec & theSpec,
						const NFmiArea & theArea)
{
  if(theSpec.labelMarker().empty())
	return;

  // Establish that something is to be done

  if(theSpec.labelPoints().empty())
	return;

  // Establish the marker specs

  const NFmiImage & marker = globals.getImage(theSpec.labelMarker());

  NFmiColorTools::NFmiBlendRule markerrule = ColorTools::checkrule(theSpec.labelMarkerRule());

  float markeralpha = theSpec.labelMarkerAlphaFactor();

  // Draw individual points

  unsigned int pointnumber = 0;
  list<pair<NFmiPoint,NFmiPoint> >::const_iterator iter;
  for(iter=theSpec.labelPoints().begin();
	  iter!=theSpec.labelPoints().end();
	  ++iter)
	{
	  // The point in question

	  NFmiPoint xy = theArea.ToXY(iter->first);

	  // Skip rendering if LabelMissing is "" and value is missing
	  if(theSpec.labelMissing().empty())
		{
		  float value = theSpec.labelValues()[pointnumber++];
		  if(value == kFloatMissing)
			continue;
		}

	  // Skip rendering if the start point is masked

	  if(IsMasked(xy, globals.mask))
		continue;

	  theImage.Composite(marker,
						 markerrule,
						 kFmiAlignCenter,
						 FmiRound(xy.X()),
						 FmiRound(xy.Y()),
						 markeralpha);
	}
}

// ----------------------------------------------------------------------
/*!
 * \brief Draw label texts
 */
// ----------------------------------------------------------------------

void draw_label_texts(NFmiImage & theImage,
					  const ContourSpec & theSpec,
					  const NFmiArea & theArea)
{
  // Establish that something is to be done

  if(theSpec.labelPoints().empty() &&
	 theSpec.pixelLabels().empty())
	return;

  // Quick exit if no labels are desired for this parameter

  if(theSpec.labelFormat() == "")
	return;

  // Create the face object to be used

  Imagine::NFmiFace face = make_face(theSpec.labelFont());
  face.Background(false);

  // Draw labels at specifing latlon points if requested

  {
	list<pair<NFmiPoint,NFmiPoint> >::const_iterator iter;
	
	int pointnumber = 0;
	for(iter=theSpec.labelPoints().begin();
		iter!=theSpec.labelPoints().end();
		++iter)
	  {
		
		// The point in question
		
		double x,y;
		if(iter->second.X() == kFloatMissing)
		  {
			NFmiPoint xy = theArea.ToXY(iter->first);
			x = xy.X();
			y = xy.Y();
		  }
		else
		  {
			x = iter->second.X();
			y = iter->second.Y();
		  }

		// Fetch the value to be rendered
		
		float value = theSpec.labelValues()[pointnumber++];
		
		// Skip rendering if the point is much too far from the image

		const int safetymargin = 50;
		if(x < -safetymargin ||
		   y < -safetymargin ||
		   x > theImage.Width()+safetymargin ||
		   y > theImage.Height()+safetymargin)
		  continue;

		// Skip rendering if the start point is masked
		
		if(IsMasked(NFmiPoint(x,y),globals.mask))
		  continue;
		
		// Convert value to string
		string strvalue = theSpec.labelMissing();
		
		if(value!=kFloatMissing)
		  {
			char tmp[20];
			sprintf(tmp,theSpec.labelFormat().c_str(),value);
			strvalue = tmp;
		  }
		
		// Don't bother drawing empty strings
		if(strvalue.empty())
		  continue;
		
		// Set new text properties
		
		face.Draw(theImage,
				  FmiRound(x + theSpec.labelOffsetX()),
				  FmiRound(y + theSpec.labelOffsetY()),
				  strvalue,
				  AlignmentValue(theSpec.labelAlignment()),
				  theSpec.labelColor(),
				  ColorTools::checkrule(theSpec.labelRule()));
		
		// Then the label caption
		
		if(!theSpec.labelCaption().empty())
		  {
			face.Draw(theImage,
					  FmiRound(x + theSpec.labelCaptionDX()),
					  FmiRound(y + theSpec.labelCaptionDY()),
					  theSpec.labelCaption(),
					  AlignmentValue(theSpec.labelCaptionAlignment()),
					  theSpec.labelColor(),
					  ColorTools::checkrule(theSpec.labelRule()));
		  }
	  }
  }

  // Draw labels at specifing pixel coordinates if requested

  {
	list<pair<NFmiPoint,float> >::const_iterator iter;
	
	for(iter=theSpec.pixelLabels().begin();
		iter!=theSpec.pixelLabels().end();
		++iter)
	  {
		
		// The point in question
		
		double x = iter->first.X();
		double y = iter->first.Y();
		float value = iter->second;
		
		// Skip rendering if the point is much too far from the image

		const int safetymargin = 50;
		if(x < -safetymargin ||
		   y < -safetymargin ||
		   x > theImage.Width()+safetymargin ||
		   y > theImage.Height()+safetymargin)
		  continue;

		// Skip rendering if the start point is masked
		
		if(IsMasked(NFmiPoint(x,y),globals.mask))
		  continue;
		
		// Convert value to string
		string strvalue = theSpec.labelMissing();
		
		if(value!=kFloatMissing)
		  {
			char tmp[20];
			sprintf(tmp,theSpec.labelFormat().c_str(),value);
			strvalue = tmp;
		  }
		
		// Don't bother drawing empty strings
		if(strvalue.empty())
		  continue;
		
		// Set new text properties
		
		face.Draw(theImage,
				  FmiRound(x + theSpec.labelOffsetX()),
				  FmiRound(y + theSpec.labelOffsetY()),
				  strvalue,
				  AlignmentValue(theSpec.labelAlignment()),
				  theSpec.labelColor(),
				  ColorTools::checkrule(theSpec.labelRule()));
		
		// Then the label caption
		
		if(!theSpec.labelCaption().empty())
		  {
			face.Draw(theImage,
					  FmiRound(x + theSpec.labelCaptionDX()),
					  FmiRound(y + theSpec.labelCaptionDY()),
					  theSpec.labelCaption(),
					  AlignmentValue(theSpec.labelCaptionAlignment()),
					  theSpec.labelColor(),
					  ColorTools::checkrule(theSpec.labelRule()));
		  }
	  }
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Draw the listed wind arrow points
 */
// ----------------------------------------------------------------------

void draw_wind_arrows_points(NFmiImage & theImage,
							 const NFmiArea & theArea,
							 const NFmiPath & theArrow)
{
  // Handle all given coordinates
  
  list<NFmiPoint>::const_iterator iter;
  
  for(iter=globals.arrowpoints.begin();
	  iter!=globals.arrowpoints.end();
	  ++iter)
	{
	  
	  // The start point
	  NFmiPoint xy0 = theArea.ToXY(*iter);
	  
	  // Skip rendering if the start point is masked
	  
	  if(IsMasked(xy0,globals.mask))
		continue;
	  
	  float dir = globals.queryinfo->InterpolatedValue(*iter);
	  dir = globals.unitsconverter.convert(FmiParameterName(globals.queryinfo->GetParamIdent()),
										   dir);

	  if(dir==kFloatMissing)	// ignore missing
		continue;
	  
	  float speed = -1;
	  
	  if(globals.queryinfo->Param(FmiParameterName(converter.ToEnum(globals.speedparam))))
		{
		  speed = globals.queryinfo->InterpolatedValue(*iter);
		  speed = globals.unitsconverter.convert(FmiParameterName(globals.queryinfo->GetParamIdent()),speed);
		}
	  globals.queryinfo->Param(FmiParameterName(converter.ToEnum(globals.directionparam)));
	  
	  // Direction calculations
	  
	  const float pi = 3.141592658979323f;
	  const float length = 0.1f;	// degrees
	  
	  double x1 = iter->X()+sin(dir*pi/180)*length;
	  double y1 = iter->Y()+cos(dir*pi/180)*length;
	  
	  NFmiPoint xy1 = theArea.ToXY(NFmiPoint(x1,y1));
	  
	  // Calculate the actual angle
	  
	  float alpha = static_cast<float>(atan2(xy1.X()-xy0.X(),
											 xy1.Y()-xy0.Y()));
	  
	  // Create a new path
	  
	  NFmiPath thispath;
	  
	  if(globals.arrowfile == "meteorological")
		thispath.Add(GramTools::metarrow(speed*globals.windarrowscaleC));
	  else
		thispath.Add(theArrow);
	  
	  if(speed>0 && speed!=kFloatMissing)
		thispath.Scale(globals.windarrowscaleA*log10(globals.windarrowscaleB*speed+1)+globals.windarrowscaleC);
	  thispath.Scale(globals.arrowscale);
	  thispath.Rotate(alpha*180/pi);
	  thispath.Translate(static_cast<float>(xy0.X()), static_cast<float>(xy0.Y()));
	  
	  // And render it
	  
	  thispath.Fill(theImage,
					ColorTools::checkcolor(globals.arrowfillcolor),
					ColorTools::checkrule(globals.arrowfillrule));
	  thispath.Stroke(theImage,
					  ColorTools::checkcolor(globals.arrowstrokecolor),
					  ColorTools::checkrule(globals.arrowstrokerule));
	}
}

// ----------------------------------------------------------------------
/*!
 * \brief Draw the data gridded wind arrow points
 */
// ----------------------------------------------------------------------

void draw_wind_arrows_grid(NFmiImage & theImage,
						   const NFmiArea & theArea,
						   const NFmiPath & theArrow)
{
  // Draw the full grid if so desired
  
  if(globals.windarrowdx<=0 || globals.windarrowdy<=0)
	return;

  NFmiDataMatrix<float> speedvalues;
  if(globals.queryinfo->Param(FmiParameterName(converter.ToEnum(globals.speedparam))))
	{
	  globals.queryinfo->Values(speedvalues);
	  globals.unitsconverter.convert(FmiParameterName(globals.queryinfo->GetParamIdent()),speedvalues);
	}
  
  NFmiDataMatrix<float> dirvalues;
  globals.queryinfo->Param(FmiParameterName(converter.ToEnum(globals.directionparam)));
  globals.queryinfo->Values(dirvalues);
  globals.unitsconverter.convert(FmiParameterName(globals.queryinfo->GetParamIdent()),dirvalues);
  
  shared_ptr<NFmiDataMatrix<NFmiPoint> > worldpts = globals.queryinfo->LocationsWorldXY(theArea);
  for(float y=0; y<=worldpts->NY()-1; y+=globals.windarrowdy)
	for(float x=0; x<=worldpts->NX()-1; x+=globals.windarrowdx)
	  {
		// The start point
		
		const int i = static_cast<int>(floor(x));
		const int j = static_cast<int>(floor(y));
		
		NFmiPoint bad(kFloatMissing,kFloatMissing);
		NFmiPoint xy = NFmiInterpolation::BiLinear(x-i,
												   y-j,
												   worldpts->At(i,j+1,bad),
												   worldpts->At(i+1,j+1,bad),
												   worldpts->At(i,j,bad),
												   worldpts->At(i+1,j,bad));
		
		NFmiPoint latlon = theArea.WorldXYToLatLon(xy);
		NFmiPoint xy0 = theArea.ToXY(latlon);
		
		// Skip rendering if the start point is masked
		if(IsMasked(xy0,globals.mask))
		  continue;
		
		double dir = NFmiInterpolation::ModBiLinear(x-i,
													y-j,
													dirvalues.At(i,j+1,kFloatMissing),
													dirvalues.At(i+1,j+1,kFloatMissing),
													dirvalues.At(i,j,kFloatMissing),
													dirvalues.At(i+1,j,kFloatMissing),
													360);
		
		if(dir==kFloatMissing)	// ignore missing
		  continue;
		
		double speed = NFmiInterpolation::BiLinear(x-i,
												   y-j,
												   speedvalues.At(i,j+1,kFloatMissing),
												   speedvalues.At(i+1,j+1,kFloatMissing),
												   speedvalues.At(i,j,kFloatMissing),
												   speedvalues.At(i+1,j,kFloatMissing));
		
		// Direction calculations
		
		const double pi = 3.141592658979323;
		const double length = 0.1;	// degrees
		
		double x0 = latlon.X();
		double y0 = latlon.Y();
		
		double x1 = x0+sin(dir*pi/180)*length;
		double y1 = y0+cos(dir*pi/180)*length;
		
		NFmiPoint xy1 = theArea.ToXY(NFmiPoint(x1,y1));
		
		// Calculate the actual angle
		
		double alpha = atan2(xy1.X()-xy0.X(),
							 xy1.Y()-xy0.Y());
		
		// Create a new path
			
		NFmiPath thispath;
		if(globals.arrowfile == "meteorological")
		  thispath.Add(GramTools::metarrow(static_cast<float>(speed*globals.windarrowscaleC)));
		else
		  thispath.Add(theArrow);
		if(speed>0 && speed != kFloatMissing)
		  thispath.Scale(static_cast<float>(globals.windarrowscaleA*log10(globals.windarrowscaleB*speed+1)+globals.windarrowscaleC));
		thispath.Scale(globals.arrowscale);
		thispath.Rotate(static_cast<float>(alpha*180/pi));
		thispath.Translate(static_cast<float>(xy0.X()), static_cast<float>(xy0.Y()));
		
		// And render it
		
		thispath.Fill(theImage,
					  ColorTools::checkcolor(globals.arrowfillcolor),
					  ColorTools::checkrule(globals.arrowfillrule));
		thispath.Stroke(theImage,
						ColorTools::checkcolor(globals.arrowstrokecolor),
						ColorTools::checkrule(globals.arrowstrokerule));
	  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Draw the wind arrows in image coordinates
 */
// ----------------------------------------------------------------------

void draw_wind_arrows_pixelgrid(NFmiImage & theImage,
								const NFmiArea & theArea,
								const NFmiPath & theArrow)
{
  // Draw the full grid if so desired

  if(globals.windarrowsxydx<=0 || globals.windarrowsxydy<=0)
	return;

  for(float y = globals.windarrowsxyy0;
	  y <= theImage.Height();
	  y += globals.windarrowsxydy)
	for(float x = globals.windarrowsxyx0;
		x <= theImage.Width();
		x += globals.windarrowsxydx)
	  {
		NFmiPoint xy0(x,y);

		// Skip the point if it is masked
		if(IsMasked(xy0,globals.mask))
		  continue;

		// Calculate the latlon value

		NFmiPoint latlon = theArea.ToLatLon(xy0);

		// Calculate the speed values
		float dir = globals.queryinfo->InterpolatedValue(latlon);
		dir = globals.unitsconverter.convert(FmiParameterName(converter.ToEnum(globals.directionparam)),dir);
		if(dir==kFloatMissing)
		  continue;
		
		float speed = -1;
		if(globals.queryinfo->Param(FmiParameterName(converter.ToEnum(globals.speedparam))))
		  {
			speed = globals.queryinfo->InterpolatedValue(latlon);
			speed = globals.unitsconverter.convert(FmiParameterName(converter.ToEnum(globals.speedparam)),speed);
		  }
		globals.queryinfo->Param(FmiParameterName(converter.ToEnum(globals.directionparam)));
		
		// Direction calculations
		
		const float pi = 3.141592658979323f;
		const float length = 0.1f;	// degrees
		
		double x1 = latlon.X()+sin(dir*pi/180)*length;
		double y1 = latlon.Y()+cos(dir*pi/180)*length;
		
		NFmiPoint xy1 = theArea.ToXY(NFmiPoint(x1,y1));
		
		// Calculate the actual angle
		
		float alpha = static_cast<float>(atan2(xy1.X()-xy0.X(),
											   xy1.Y()-xy0.Y()));
		
		// Create a new path
		
		NFmiPath thispath;
		
		if(globals.arrowfile == "meteorological")
		  thispath.Add(GramTools::metarrow(speed*globals.windarrowscaleC));
		else
		  thispath.Add(theArrow);
		
		if(speed>0 && speed!=kFloatMissing)
		  thispath.Scale(globals.windarrowscaleA*log10(globals.windarrowscaleB*speed+1)+globals.windarrowscaleC);
		thispath.Scale(globals.arrowscale);
		thispath.Rotate(alpha*180/pi);
		thispath.Translate(static_cast<float>(xy0.X()), static_cast<float>(xy0.Y()));
	  
		// And render it
		
		thispath.Fill(theImage,
					  ColorTools::checkcolor(globals.arrowfillcolor),
					  ColorTools::checkrule(globals.arrowfillrule));
		thispath.Stroke(theImage,
						ColorTools::checkcolor(globals.arrowstrokecolor),
						ColorTools::checkrule(globals.arrowstrokerule));
	  }
}


// ----------------------------------------------------------------------
/*!
 * \brief Draw wind arrows onto the image
 */
// ----------------------------------------------------------------------

void draw_wind_arrows(NFmiImage & theImage,
					  const NFmiArea & theArea)
{
  if((!globals.arrowpoints.empty() ||
	  (globals.windarrowdx>0 && globals.windarrowdy>0) ||
	  (globals.windarrowsxydx>0 && globals.windarrowsxydy>0)) &&
	 (globals.arrowfile!=""))
	{
	  FmiParameterName param = FmiParameterName(converter.ToEnum(globals.directionparam));
	  if(param==kFmiBadParameter)
		throw runtime_error("Unknown parameter "+globals.directionparam);

	  // Find the proper queryinfo to be used

	  bool ok = false;
	  for(unsigned int qi=0; qi<globals.querystreams.size(); qi++)
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
		  const string & arr = globals.itsArrowCache.find(globals.arrowfile);
		  arrowpath.Add(arr);
		}
	  
	  draw_wind_arrows_points(theImage,theArea,arrowpath);
	  draw_wind_arrows_grid(theImage,theArea,arrowpath);
	  draw_wind_arrows_pixelgrid(theImage,theArea,arrowpath);

	}

}

// ----------------------------------------------------------------------
/*!
 * \brief Draw contour fills
 */
// ----------------------------------------------------------------------

void draw_contour_fills(NFmiImage & theImage,
						const NFmiArea & theArea,
						const ContourSpec & theSpec,
						NFmiContourTree::NFmiContourInterpolation theInterpolation)
{
  list<ContourRange>::const_iterator it;
  list<ContourRange>::const_iterator begin;
  list<ContourRange>::const_iterator end;
  
  begin = theSpec.contourFills().begin();
  end   = theSpec.contourFills().end();
  
  for(it=begin ; it!=end; ++it)
	{
	  // Contour the actual data
	  
	  bool exactlo = true;
	  bool exacthi = (it->hilimit()!=kFloatMissing &&
					  theSpec.exactHiLimit()!=kFloatMissing &&
					  it->hilimit()==theSpec.exactHiLimit());
	  
	  NFmiPath path =
		globals.calculator.contour(*globals.queryinfo,
								   it->lolimit(),
								   it->hilimit(),
								   exactlo,
								   exacthi,
								   theSpec.dataLoLimit(),
								   theSpec.dataHiLimit(),
								   theInterpolation,
								   globals.contourtriangles != 0);
	  
	  if(globals.verbose && globals.calculator.wasCached())
		cout << "Using cached "
			 << it->lolimit() << " - "
			 << it->hilimit() << endl;

	  // Avoid unnecessary work if the path is empty
	  if(path.Empty())
		continue;

	  path.Project(&theArea);

	  // Augment the path with the contourmask if necessary

	  if(!theSpec.contourMaskParam().empty())
		{
		  FmiParameterName oldid = FmiParameterName(globals.maskqueryinfo->GetParamIdent());
		  FmiParameterName maskid = FmiParameterName(converter.ToEnum(theSpec.contourMaskParam()));

		  globals.maskqueryinfo->Param(maskid);

		  NFmiPath mask =
			globals.maskcalculator.contour(*globals.maskqueryinfo,
										   theSpec.contourMaskLoLimit(),
										   theSpec.contourMaskHiLimit(),
										   true,
										   true,
										   kFloatMissing,
										   kFloatMissing,
										   NFmiContourTree::kFmiContourLinear,
										   true);
		  

		  globals.maskqueryinfo->Param(oldid);

		  // Nothing to do if the mask is empty
		  if(mask.Empty())
			continue;

		  mask.Project(&theArea);

		  path = NFmiGpcTools::And(path,mask);

		}

	  NFmiColorTools::NFmiBlendRule rule = ColorTools::checkrule(it->rule());
	  path.Fill(theImage,it->color(),rule);
	  
	}
}

// ----------------------------------------------------------------------
/*!
 * \brief Draw contour patterns
 */
// ----------------------------------------------------------------------

void draw_contour_patterns(NFmiImage & theImage,
						   const NFmiArea & theArea,
						   const ContourSpec & theSpec,
						   NFmiContourTree::NFmiContourInterpolation theInterpolation)
{
  list<ContourPattern>::const_iterator it;
  list<ContourPattern>::const_iterator begin;
  list<ContourPattern>::const_iterator end;

  begin = theSpec.contourPatterns().begin();
  end   = theSpec.contourPatterns().end();

  for(it=begin ; it!=end; ++it)
	{
	  bool exactlo = true;
	  bool exacthi = (it->hilimit()!=kFloatMissing &&
					  theSpec.exactHiLimit()!=kFloatMissing &&
					  it->hilimit()==theSpec.exactHiLimit());

	  NFmiPath path =
		globals.calculator.contour(*globals.queryinfo,
								   it->lolimit(),
								   it->hilimit(),
								   exactlo, exacthi,
								   theSpec.dataLoLimit(),
								   theSpec.dataHiLimit(),
								   theInterpolation,
								   globals.contourtriangles != 0);

	  if(globals.verbose && globals.calculator.wasCached())
		cout << "Using cached "
			 << it->lolimit() << " - "
			 << it->hilimit() << endl;

	  NFmiColorTools::NFmiBlendRule rule = ColorTools::checkrule(it->rule());
	  const NFmiImage & pattern = globals.getImage(it->pattern());

	  path.Project(&theArea);
	  path.Fill(theImage,pattern,rule,it->factor());

	}
}

// ----------------------------------------------------------------------
/*!
 * \brief Draw contour strokes
 */
// ----------------------------------------------------------------------

void draw_contour_strokes(NFmiImage & theImage,
						  const NFmiArea & theArea,
						  const ContourSpec & theSpec,
						  NFmiContourTree::NFmiContourInterpolation theInterpolation)
{
  list<ContourValue>::const_iterator it;
  list<ContourValue>::const_iterator begin;
  list<ContourValue>::const_iterator end;

  begin = theSpec.contourValues().begin();
  end   = theSpec.contourValues().end();

  for(it=begin ; it!=end; ++it)
	{
	  NFmiPath path =
		globals.calculator.contour(*globals.queryinfo,
								   it->value(),
								   theSpec.dataLoLimit(),
								   theSpec.dataHiLimit(),
								   theInterpolation,
								   globals.contourtriangles != 0);

	  NFmiColorTools::NFmiBlendRule rule = ColorTools::checkrule(it->rule());
	  path.Project(&theArea);
	  path.SimplifyLines(10);
	  path.Stroke(theImage,it->color(),rule);

	}
}

// ----------------------------------------------------------------------
/*!
 * \brief Collect contour label candidate coordinates
 */
// ----------------------------------------------------------------------

void save_contour_labels(NFmiImage & theImage,
						 const NFmiArea & theArea,
						 const ContourSpec & theSpec,
						 NFmiContourTree::NFmiContourInterpolation theInterpolation)
{
  // The ID under which the coordinates will be stored

  int id = paramid(theSpec.param());
  globals.labellocator.parameter(id);

  // Start saving candindate coordinates

  list<ContourLabel>::const_iterator it;
  list<ContourLabel>::const_iterator begin;
  list<ContourLabel>::const_iterator end;
  
  begin = theSpec.contourLabels().begin();
  end   = theSpec.contourLabels().end();

  for(it=begin ; it!=end; ++it)
	{
	  NFmiPath path =
		globals.calculator.contour(*globals.queryinfo,
								   it->value(),
								   theSpec.dataLoLimit(),
								   theSpec.dataHiLimit(),
								   theInterpolation,
								   globals.contourtriangles != 0);

	  path.Project(&theArea);

	  for(NFmiPathData::const_iterator pit = path.Elements().begin();
		  pit != path.Elements().end();
		  ++pit)
		{
		  if((*pit).Oper() == kFmiLineTo)
			{
			  globals.labellocator.add(it->value(),
									   FmiRound((*pit).X()),
									   FmiRound((*pit).Y()));
			}
		}
	}
}

// ----------------------------------------------------------------------
/*!
 * \brief Draw contour labels
 */
// ----------------------------------------------------------------------

void draw_contour_labels(NFmiImage & theImage)
{
  const LabelLocator::ParamCoordinates & coords = globals.labellocator.chooseLabels();

  if(coords.empty())
	return;

  // Iterate through all parameters

  list<ContourSpec>::iterator piter;
  list<ContourSpec>::iterator pbegin = globals.specs.begin();
  list<ContourSpec>::iterator pend   = globals.specs.end();
  
  for(piter=pbegin; piter!=pend; ++piter)
	{
	  // Ignore the param if we could not assign any coordinates for it

	  const int id = paramid(piter->param());

	  LabelLocator::ParamCoordinates::const_iterator pit = coords.find(id);
	  if(pit == coords.end())
		continue;

	  // Rended the contours

	  const std::string & fontspec = piter->contourLabelFont();
	  const int fontcolor = piter->contourLabelColor();
	  const int backcolor = piter->contourLabelBackgroundColor();
	  const int xmargin = piter->contourLabelBackgroundXMargin();
	  const int ymargin = piter->contourLabelBackgroundYMargin();

	  Imagine::NFmiFace face = make_face(fontspec);
	  face.Background(true);
	  face.BackgroundColor(backcolor);
	  face.BackgroundMargin(xmargin,ymargin);

	  for(LabelLocator::ContourCoordinates::const_iterator cit = pit->second.begin();
		  cit != pit->second.end();
		  ++cit)
		{
		  const float value = cit->first;
		  const string text = NFmiStringTools::Convert(value);

		  for(LabelLocator::Coordinates::const_iterator it = cit->second.begin();
			  it != cit->second.end();
			  ++it)
			{
			  face.Draw(theImage,
						it->second.first,
						it->second.second,
						text,
						Imagine::kFmiAlignCenter,
						fontcolor);
			}
		}
	}
}

// ----------------------------------------------------------------------
/*!
 * \brief Draw contour symbols
 */
// ----------------------------------------------------------------------

void draw_contour_symbols(NFmiImage & theImage,
						  const NFmiArea & theArea,
						  const ContourSpec & theSpec,
						  const LazyCoordinates & thePoints,
						  const NFmiDataMatrix<float> & theValues)
{
  list<ContourSymbol>::const_iterator it;
  list<ContourSymbol>::const_iterator begin;
  list<ContourSymbol>::const_iterator end;
  
  begin = theSpec.contourSymbols().begin();
  end   = theSpec.contourSymbols().end();
  
  for(it=begin ; it!=end; ++it)
	{
	  NFmiColorTools::NFmiBlendRule rule = ColorTools::checkrule(it->rule());
	  const NFmiImage & symbol = globals.getImage(it->pattern());
	  const float factor = it->factor();
	  const float lo = it->lolimit();
	  const float hi = it->hilimit();

	  // Draw symbol at each grid point where necessary
	  for(unsigned int j=0; j<theValues.NY(); j++)
		for(unsigned int i=0; i<theValues.NX(); i++)
		  {
			const float z = theValues[i][j];
			bool inside = false;
			if(lo!=kFloatMissing && z<lo)
			  inside = false;
			else if(hi!=kFloatMissing && z>=hi)
			  inside = false;
			else if(lo==kFloatMissing && hi==kFloatMissing)
			  inside = (z == kFloatMissing);
			else
			  inside = true;



			if(inside)
			  {
				NFmiPoint latlon = theArea.WorldXYToLatLon(thePoints(i,j));
				NFmiPoint xy = theArea.ToXY(latlon);

				theImage.Composite(symbol,
								   rule,
								   kFmiAlignCenter,
								   FmiRound(xy.X()),
								   FmiRound(xy.Y()),
								   factor);
			  }
			
		  }
	}
}

// ----------------------------------------------------------------------
/*!
 * \brief Draw contour fonts
 */
// ----------------------------------------------------------------------

void draw_contour_fonts(NFmiImage & theImage)
{
  const LabelLocator::ParamCoordinates & paramcoords = globals.symbollocator.chooseLabels();

  if(paramcoords.empty())
	return;

  // Iterate through all parameters

  list<ContourSpec>::iterator piter;
  list<ContourSpec>::iterator pbegin = globals.specs.begin();
  list<ContourSpec>::iterator pend   = globals.specs.end();
  
  for(piter=pbegin; piter!=pend; ++piter)
	{
	  // Ignore the param if we could not assign any coordinates for it

	  const int id = paramid(piter->param());

	  LabelLocator::ParamCoordinates::const_iterator pit = paramcoords.find(id);
	  if(pit == paramcoords.end())
		continue;

	  const LabelLocator::ContourCoordinates & coords = pit->second;

	  // Loop through all the values

	  for(LabelLocator::ContourCoordinates::const_iterator cit = coords.begin();
		  cit != coords.end();
		  ++cit)
		{
		  const float value = cit->first;

		  // Find the specs for the font value

		  list<ContourFont>::const_iterator fit;
		  for(fit=piter->contourFonts().begin();
			  fit!=piter->contourFonts().end();
			  ++fit)
			{
			  if(fit->value() == value)
				break;
			}

		  // Should never happen
		  if(fit == piter->contourFonts().end())
			throw runtime_error("Internal error while contouring with fonts");

		  // Render the symbols

		  const std::string & fontspec = fit->font();
		  const int fontcolor = fit->color();
		  const int symbol = fit->symbol();

		  string text = "";
		  text += symbol;
		  
		  Imagine::NFmiFace face = make_face(fontspec);
		  face.Background(false);
		  
		  for(LabelLocator::Coordinates::const_iterator it = cit->second.begin();
			  it != cit->second.end();
			  ++it)
			{
			  
			  face.Draw(theImage,
						it->second.first,
						it->second.second,
						text,
						Imagine::kFmiAlignCenter,
						fontcolor);
			}
		}
	}
}


// ----------------------------------------------------------------------
/*!
 * \brief Save contour font coordinates
 */
// ----------------------------------------------------------------------

void save_contour_fonts(NFmiImage & theImage,
						const NFmiArea & theArea,
						const ContourSpec & theSpec,
						const LazyCoordinates & thePoints,
						const NFmiDataMatrix<float> & theValues)
{
  // The ID under which the coordinates will be stored

  int id = paramid(theSpec.param());
  globals.symbollocator.parameter(id);

  // For speed we prefer to iterate only once through the data, and
  // instead use a fast way to test if a given value is to be contoured

  set<float> okvalues;

  list<ContourFont>::const_iterator it;
  list<ContourFont>::const_iterator begin;
  list<ContourFont>::const_iterator end;
  
  begin = theSpec.contourFonts().begin();
  end   = theSpec.contourFonts().end();
  
  for(it=begin ; it!=end; ++it)
	{
	  okvalues.insert(it->value());
	}

  // Now iterate through the data once, saving candidate points

  for(unsigned int j=0; j<theValues.NY(); j++)
	for(unsigned int i=0; i<theValues.NX(); i++)
	  {
		if(okvalues.find(theValues[i][j]) != okvalues.end())
		  {
			NFmiPoint latlon = theArea.WorldXYToLatLon(thePoints(i,j));
			NFmiPoint xy = theArea.ToXY(latlon);

			globals.symbollocator.add(theValues[i][j],
									  FmiRound(xy.X()),
									  FmiRound(xy.Y()));
		  }
	  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Establish the type of extremum at the given point
 *
 * \param theValues The matrix of values
 * \param i The i-coordinate of the value
 * \param j The j-coordinate of the value
 * \param DX The search region in X-direction
 * \param DY The search region in Y-direction
 * \param mingradient Minimum required difference in area
 * \return -2/2 for absolute minima/maxima, -1/1 for minima/maxima, 0 for none
 */
// ----------------------------------------------------------------------

int extrematype(const NFmiDataMatrix<float> & theValues,
				int i,
				int j,
				int DX,
				int DY,
				float mingradient)
{
  int smaller = 0;
  int bigger = 0;

  // minimum/maximum on the frame
  float minimum = theValues[i-DX][j-DY];
  float maximum = theValues[i-DX][j-DY];

  for(int dy = -DY; dy<=DY; dy++)
	for(int dx = -DX; dx<=DX; dx++)
	  {
		// quick exit for missing values
		if(theValues[i+dx][j+dy] == kFloatMissing)
		  return 0;

		if(dx!=0 && dy!=0)
		  {
			if(theValues[i+dx][j+dy] < theValues[i][j])
			  ++smaller;
			else if(theValues[i+dx][j+dy] > theValues[i][j])
			  ++bigger;
		  }

		// quick exit for non-extrema
		if(smaller>0 && bigger>0)
		  return 0;

		// update extrema values

		if(dx == -DX || dx == DX || dy == -DY || dy == DY)
		  {
			minimum = min(minimum,theValues[i+dx][j+dy]);
			maximum = max(maximum,theValues[i+dx][j+dy]);
		  }
	  }

  // minimum change from center to rim

  float change = min(abs(theValues[i][j]-minimum),
					 abs(theValues[i][j]-maximum));

#if 0
  if(change >= mingradient)
	cout << "Change = " << change << " at " << i << ' ' << j << endl;
#endif

  if(change < mingradient)
	return 0;

  else if(smaller == (DX*2+1)*(DY*2+1)-1)
	return 2;
  else if(bigger == (DX*2+1)*(DY*2+1)-1)
	return -2;
  else if(smaller > 0)
	return 1;
  else if(bigger > 0)
	return -1;
  else
	return 0;	// should never happen due to quick exit in the loop
}


// ----------------------------------------------------------------------
/*!
 * \brief Draw high/low pressure markers
 */
// ----------------------------------------------------------------------

void draw_pressure_markers(NFmiImage & theImage,
						   const NFmiArea & theArea)
{
  // Establish which markers are to be drawn

  bool dohigh = !globals.highpressureimage.empty();
  bool dolow =  !globals.lowpressureimage.empty();

  // Exit if none

  if(!dohigh && !dolow)
	return;

  // Get the data to be analyzed

  choose_queryinfo("Pressure",0);

  shared_ptr<NFmiDataMatrix<NFmiPoint> > worldpts = globals.queryinfo->LocationsWorldXY(theArea);

  NFmiDataMatrix<float> vals;
  globals.queryinfo->Values(vals);
  globals.unitsconverter.convert(FmiParameterName(globals.queryinfo->GetParamIdent()),vals);

  // Insert candidate coordinates into the system

  // const double dx = theArea.WorldXYWidth()/theArea.Grid()->XNumber();
  // const double dy = theArea.WorldXYHeight()/theArea.Grid()->YNumber();

  const int DX = 7; // ceil(500*1000/dx/2);	// 500km radius required
  const int DY = 7; // ceil(500*1000/dx/2);
  const float required_gradient = 1.0;

  for(unsigned int j=DY; j<vals.NY()-DY; j++)
	for(unsigned int i=DX; i<vals.NX()-DX; i++)
	  {
		int extrem = extrematype(vals,i,j,DX,DY,required_gradient);
		if(extrem != 0)
		  {
			// the point in kilometer units
			NFmiPoint point((*worldpts)[i][j].X()/1000,
							(*worldpts)[i][j].Y()/1000);
			
			if(extrem < 0)
			  {
				if(dolow)
				  globals.pressurelocator.add(ExtremaLocator::Minimum,
											  point.X(),
											  point.Y());
			  }
			else
			  {
				if(dohigh)
				  globals.pressurelocator.add(ExtremaLocator::Maximum,
											  point.X(),
											  point.Y());
			  }
		  }
	  }

  // Now choose the marker positions and draw them

  const ExtremaLocator::ExtremaCoordinates & extrema = globals.pressurelocator.chooseCoordinates();

  NFmiColorTools::NFmiBlendRule lowrule = ColorTools::checkrule(globals.lowpressurerule);
  NFmiColorTools::NFmiBlendRule highrule = ColorTools::checkrule(globals.highpressurerule);

  for(ExtremaLocator::ExtremaCoordinates::const_iterator eit = extrema.begin();
	  eit != extrema.end();
	  ++eit)
	{
	  for(ExtremaLocator::Coordinates::const_iterator it = eit->second.begin();
		  it != eit->second.end();
		  ++it)
		{
		  NFmiPoint wxy(it->first*1000,it->second*1000);
		  NFmiPoint latlon = theArea.WorldXYToLatLon(wxy);
		  NFmiPoint xy = theArea.ToXY(latlon);

		  switch(eit->first)
			{
			case ExtremaLocator::Minimum:
			  theImage.Composite(globals.lowpressureimage,
								 lowrule,
								 kFmiAlignCenter,
								 FmiRound(xy.X()),
								 FmiRound(xy.Y()),
								 globals.lowpressurefactor);
			  break;
			case ExtremaLocator::Maximum:
			  theImage.Composite(globals.highpressureimage,
								 highrule,
								 kFmiAlignCenter,
								 FmiRound(xy.X()),
								 FmiRound(xy.Y()),
								 globals.highpressurefactor);
			  break;
			}
		}
	}
}

// ----------------------------------------------------------------------
/*!
 * \brief Draw the foreground onto the image
 */
// ----------------------------------------------------------------------

void draw_foreground(NFmiImage & theImage)
{
  if(globals.foreground.empty())
	return;

  NFmiColorTools::NFmiBlendRule rule = ColorTools::checkrule(globals.foregroundrule);

  theImage.Composite(globals.getImage(globals.foreground),
					 rule,
					 kFmiAlignNorthWest,
					 0,0,1);
}



// ----------------------------------------------------------------------
/*!
 * \brief Handle "draw contours" command
 */
// ----------------------------------------------------------------------

void do_draw_contours(istream & theInput)
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

  globals.labellocator.clear();
  globals.pressurelocator.clear();
  globals.symbollocator.clear();

  if(globals.querystreams.empty())
	throw runtime_error("No query data has been read!");

  auto_ptr<NFmiArea> area = globals.createArea();

  // This message intentionally ignores globals.verbose

  if(!globals.background.empty())
	cout << "Contouring for background " << globals.background << endl;

  if(globals.verbose)
	report_area(*area);

  // Establish querydata timelimits and initialize
  // the XY-coordinates simultaneously.

  // Note that we use world-coordinates when smoothing
  // so that we can use meters as the smoothing radius.
  // Also, this means the contours are independent of
  // the image size.

  NFmiTime time1, time2;

  NFmiDataMatrix<float> vals;
  NFmiDataMatrix<float> maskvalues;

  unsigned int qi;
  for(qi=0; qi<globals.querystreams.size(); qi++)
	{
	  // Establish time limits

	  globals.queryinfo = globals.querystreams[qi];

	  globals.queryinfo->LastTime();
	  NFmiTime t2 = globals.queryinfo->ValidTime();

	  globals.queryinfo->FirstTime();
	  NFmiTime t1 = globals.queryinfo->ValidTime();

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
	  if(!globals.force && globals.isOutdated())
		{
		  if(globals.verbose)
			cout << "Aborting 'draw contour' since querydata has been updated" << endl;
		  break;
		}

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
			  NFmiTime loc = globals.queryinfo->ValidTime();
			  if(!loc.IsLessThan(t))
				break;
			}
		  NFmiTime tnow = globals.queryinfo->ValidTime();

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
			  NFmiTime tstamp = TimeTools::ToUTC(secs);
			  filename += "_" + tstamp.ToStr(kYYYYMMDDHHMM);
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

	  int imgwidth = static_cast<int>(area->Width()+0.5);
	  int imgheight = static_cast<int>(area->Height()+0.5);

	  NFmiColorTools::Color erasecolor = ColorTools::checkcolor(globals.erase);

	  auto_ptr<NFmiImage> image;
	  if(globals.background.empty())
		image.reset(new NFmiImage(imgwidth,imgheight,erasecolor));
	  else
		{
		  image.reset(new NFmiImage(globals.getImage(globals.background)));
		  if(imgwidth != image->Width() ||
			 imgheight != image->Height())
			{
			  throw runtime_error("Background image size does not match area size");
			}
		}

	  if(image.get()==0)
		throw runtime_error("Failed to allocate a new image for rendering");

	  globals.setImageModes(*image);

	  // Initialize label locator bounding box

	  globals.labellocator.boundingBox(globals.contourlabelimagexmargin,
									   globals.contourlabelimageymargin,
									   image->Width()-globals.contourlabelimagexmargin,
									   image->Height()-globals.contourlabelimageymargin);

	  // Initialize symbol locator bounding box with reasonably safety
	  // for large symbols

	  globals.symbollocator.boundingBox(-30,-30,image->Width()+30,image->Height()+30);

	  // Loop over all parameters
	  // The loop collects all contour label information, but
	  // does not render it yet

	  list<ContourSpec>::iterator piter;
	  list<ContourSpec>::iterator pbegin = globals.specs.begin();
	  list<ContourSpec>::iterator pend   = globals.specs.end();

	  for(piter=pbegin; piter!=pend; ++piter)
		{
		  // Establish the parameter

		  string name = piter->param();
		  int level = piter->level();

		  qi = choose_queryinfo(name,level);

		  if(globals.verbose)
			report_queryinfo(name,qi);

		  // Establish the contour method

		  string interpname = piter->contourInterpolation();
		  NFmiContourTree::NFmiContourInterpolation interp
			= NFmiContourTree::ContourInterpolationValue(interpname);
		  if(interp==NFmiContourTree::kFmiContourMissingInterpolation)
			throw runtime_error("Unknown contour interpolation method " + interpname);

		  // Get the values.

		  if(!MetaFunctions::isMeta(name))
			{
			  globals.queryinfo->Values(vals);
			  globals.unitsconverter.convert(FmiParameterName(globals.queryinfo->GetParamIdent()),vals);
			}
		  else
			vals = MetaFunctions::values(piter->param(),
										 *globals.queryinfo);

		  // Replace values if so requested

		  if(piter->replace())
			vals.Replace(piter->replaceSourceValue(),piter->replaceTargetValue());

		  // Filter the values if so requested

		  filter_values(vals,t,*piter);

		  // Expand the data if so requested

		  if(globals.expanddata) expand_data(vals);

		  // Call smoother only if necessary to avoid LazyCoordinates dereferencing

		  LazyCoordinates worldpts(*area);

		  if(piter->smoother() != "None")
			{
			  NFmiSmoother smoother(piter->smoother(),
									piter->smootherFactor(),
									piter->smootherRadius());

			  vals = smoother.Smoothen(*worldpts,vals);
			}

		  // Setup the contourer with the values

		  globals.calculator.data(vals);


		  if(!piter->contourMaskParam().empty())
			{
			  // Find the data with the mask parameter
			  unsigned int masknro = 0;
			  bool foundmask = false;
			  FmiParameterName param = FmiParameterName(converter.ToEnum(piter->contourMaskParam()));
			  for(masknro=0; masknro<globals.querystreams.size(); masknro++)
				{
				  FmiParameterName oldid = FmiParameterName(globals.querystreams[masknro]->GetParamIdent());
				  globals.querystreams[masknro]->Param(param);
				  foundmask = globals.querystreams[masknro]->IsParamUsable();
				  if(foundmask)
					{
					  globals.querystreams[masknro]->Values(maskvalues);
					  globals.unitsconverter.convert(FmiParameterName(globals.querystreams[masknro]->GetParamIdent()),maskvalues);
					  globals.querystreams[masknro]->Param(oldid);
					  break;
					}
				  globals.querystreams[masknro]->Param(oldid);
				}
			  if(!foundmask)
				throw runtime_error("Could not find data with mask parameter '"+piter->contourMaskParam()+"'");

			  globals.maskcalculator.data(maskvalues);
			  globals.maskqueryinfo = globals.querystreams[masknro];
			}

		  // Save the data values at desired points for later
		  // use, this lets us avoid using InterpolatedValue()
		  // which does not use smoothened values.

		  // First, however, if this is the first image, we add
		  // the grid points to the set of points, if so requested

		  if(!labeldxdydone)
			add_label_grid_values(*piter,*area,worldpts);

		  // For pixelgrids we must repeat the process for all new
		  // background images, since the pixel spacing changes
		  // every time. Note! We assume the following calling order!

		  add_label_point_values(*piter,*area,vals);
		  add_label_pixelgrid_values(*piter,*area,*image,vals);

		  // Fill the contours

		  draw_contour_fills(*image,*area,*piter,interp);

		  // Pattern fill the contours

		  draw_contour_patterns(*image,*area,*piter,interp);

		  // Stroke the contours

		  draw_contour_strokes(*image,*area,*piter,interp);

		  // Symbol fill the contours

		  draw_contour_symbols(*image,*area,*piter,worldpts,vals);

		  // Save symbol fill coordinates

		  save_contour_fonts(*image,*area,*piter,worldpts,vals);

		  // Save contour label coordinates

		  save_contour_labels(*image,*area,*piter,interp);

		}

	  // Bang the foreground

	  draw_foreground(*image);

	  // Draw wind arrows if so requested

	  draw_wind_arrows(*image,*area);

	  // Draw contour fonts

	  draw_contour_fonts(*image);

	  // Label the contours

	  draw_contour_labels(*image);

	  // Draw labels

	  for(piter=pbegin; piter!=pend; ++piter)
		{
		  draw_label_markers(*image,*piter,*area);
		  draw_label_texts(*image,*piter,*area);
		}

	  // Draw high/low pressure markers

	  draw_pressure_markers(*image,*area);

	  // Bang the combine image (legend, logo, whatever)

	  globals.drawCombine(*image);

	  // Finally, draw a time stamp on the image if so
	  // requested

	  const string stamp = globals.getImageStampText(t);
	  globals.drawImageStampText(*image,stamp);

	  // dx and dy labels have now been extracted into a list,
	  // disable adding them again and again and again..

	  labeldxdydone = true;

	  // Save

	  write_image(*image,filename,globals.format);

	  // Advance in time

	  globals.labellocator.nextTime();
	  globals.pressurelocator.nextTime();
	  globals.symbollocator.nextTime();

	}
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

	  istringstream in(text);
      string cmd;
      while( in >> cmd)
		{
		  // Handle comments

		  if(cmd == "#")							do_comment(in);
		  else if(cmd[0] == '#')					do_comment(in);
		  else if(cmd == "//")						do_comment(in);
		  else if(cmd == "cache")					do_cache(in);
		  else if(cmd == "querydata")				do_querydata(in);
		  else if(cmd == "filter")					do_filter(in);
		  else if(cmd == "timestepskip")			do_timestepskip(in);
		  else if(cmd == "timestep")				do_timestep(in);
		  else if(cmd == "timeinterval")			do_timeinterval(in);
		  else if(cmd == "timesteps")				do_timesteps(in);
		  else if(cmd == "timestamp")				do_timestamp(in);
		  else if(cmd == "timestampzone")			do_timestampzone(in);
		  else if(cmd == "timesteprounding")		do_timesteprounding(in);
		  else if(cmd == "timestampimage")			do_timestampimage(in);
		  else if(cmd == "timestampimagexy")		do_timestampimagexy(in);
		  else if(cmd == "timestampimageformat")	do_timestampimageformat(in);
		  else if(cmd == "timestampimagefont")		do_timestampimagefont(in);
		  else if(cmd == "timestampimagecolor")		do_timestampimagecolor(in);
		  else if(cmd == "timestampimagebackground")	do_timestampimagebackground(in);
		  else if(cmd == "timestampimagemargin")	do_timestampimagemargin(in);
		  else if(cmd == "projection")				do_projection(in);
		  else if(cmd == "erase")					do_erase(in);
		  else if(cmd == "fillrule")				do_fillrule(in);
		  else if(cmd == "strokerule")				do_strokerule(in);
		  else if(cmd == "directionparam")			do_directionparam(in);
		  else if(cmd == "speedparam")				do_speedparam(in);
		  else if(cmd == "arrowscale")				do_arrowscale(in);
		  else if(cmd == "windarrowscale")			do_windarrowscale(in);
		  else if(cmd == "arrowfill")				do_arrowfill(in);
		  else if(cmd == "arrowstroke")				do_arrowstroke(in);
		  else if(cmd == "arrowpath")				do_arrowpath(in);
		  else if(cmd == "windarrow")				do_windarrow(in);
		  else if(cmd == "windarrows")				do_windarrows(in);
		  else if(cmd == "windarrowsxy")			do_windarrowsxy(in);
		  else if(cmd == "background")				do_background(in);
		  else if(cmd == "foreground")				do_foreground(in);
		  else if(cmd == "mask")					do_mask(in);
		  else if(cmd == "combine")					do_combine(in);
		  else if(cmd == "foregroundrule")			do_foregroundrule(in);
		  else if(cmd == "savepath")				do_savepath(in);
		  else if(cmd == "prefix")					do_prefix(in);
		  else if(cmd == "suffix")					do_suffix(in);
		  else if(cmd == "format")					do_format(in);
		  else if(cmd == "gamma")					do_gamma(in);
		  else if(cmd == "intent")					do_intent(in);
		  else if(cmd == "pngquality")				do_pngquality(in);
		  else if(cmd == "jpegquality")				do_jpegquality(in);
		  else if(cmd == "savealpha")				do_savealpha(in);
		  else if(cmd == "reducecolors")			do_reducecolors(in);
		  else if(cmd == "wantpalette")				do_wantpalette(in);
		  else if(cmd == "forcepalette")			do_forcepalette(in);
		  else if(cmd == "alphalimit")				do_alphalimit(in);
		  else if(cmd == "hilimit")					do_hilimit(in);
		  else if(cmd == "datalolimit")				do_datalolimit(in);
		  else if(cmd == "datahilimit")				do_datahilimit(in);
		  else if(cmd == "datareplace")				do_datareplace(in);
		  else if(cmd == "expanddata")				do_expanddata(in);
		  else if(cmd == "contourdepth")			do_contourdepth(in);
		  else if(cmd == "contourinterpolation")	do_contourinterpolation(in);
		  else if(cmd == "contourtriangles")		do_contourtriangles(in);
		  else if(cmd == "smoother")				do_smoother(in);
		  else if(cmd == "smootherradius")			do_smootherradius(in);
		  else if(cmd == "smootherfactor")			do_smootherfactor(in);
		  else if(cmd == "level")					do_level(in);
		  else if(cmd == "param")					do_param(in);
		  else if(cmd == "shape")					do_shape(in);
		  else if(cmd == "contourmask")				do_contourmask(in);
		  else if(cmd == "contourfill")				do_contourfill(in);
		  else if(cmd == "contourpattern")			do_contourpattern(in);
		  else if(cmd == "contoursymbol")			do_contoursymbol(in);
		  else if(cmd == "contourfont")				do_contourfont(in);
		  else if(cmd == "contourline")				do_contourline(in);
		  else if(cmd == "contourfills")			do_contourfills(in);
		  else if(cmd == "contourlines")			do_contourlines(in);
		  	
		  else if(cmd == "contourlabel")			do_contourlabel(in);
		  else if(cmd == "contourlabels")			do_contourlabels(in);
		  else if(cmd == "contourlabelfont")		do_contourlabelfont(in);
		  else if(cmd == "contourlabelcolor")		do_contourlabelcolor(in);
		  else if(cmd == "contourlabelbackground")	do_contourlabelbackground(in);
		  else if(cmd == "contourlabelmargin")		do_contourlabelmargin(in);
		  else if(cmd == "contourlabelimagemargin")	do_contourlabelimagemargin(in);
		  else if(cmd == "contourlabelmindistsamevalue") do_contourlabelmindistsamevalue(in);
		  else if(cmd == "contourlabelmindistdifferentvalue") do_contourlabelmindistdifferentvalue(in);
		  else if(cmd == "contourlabelmindistdifferentparam") do_contourlabelmindistdifferentparam(in);
		  else if(cmd == "contourfontmindistsamevalue") do_contourfontmindistsamevalue(in);
		  else if(cmd == "contourfontmindistdifferentvalue") do_contourfontmindistdifferentvalue(in);
		  else if(cmd == "contourfontmindistdifferentparam") do_contourfontmindistdifferentparam(in);

		  else if(cmd == "highpressure")			do_highpressure(in);
		  else if(cmd == "lowpressure")				do_lowpressure(in);
		  else if(cmd == "lowpressuremaximum")		do_lowpressuremaximum(in);
		  else if(cmd == "highpressureminimum")		do_highpressureminimum(in);
		  else if(cmd == "pressuremindistsame")		do_pressuremindistsame(in);
		  else if(cmd == "pressuremindistdifferent") do_pressuremindistdifferent(in);
		  else if(cmd == "labelmarker")				do_labelmarker(in);
		  else if(cmd == "labelfont")				do_labelfont(in);
		  else if(cmd == "labelcolor")				do_labelcolor(in);
		  else if(cmd == "labelrule")				do_labelrule(in);
		  else if(cmd == "labelalign")				do_labelalign(in);
		  else if(cmd == "labelformat")				do_labelformat(in);
		  else if(cmd == "labelmissing")			do_labelmissing(in);
		  else if(cmd == "labeloffset")				do_labeloffset(in);
		  else if(cmd == "labelcaption")			do_labelcaption(in);
		  else if(cmd == "label")					do_label(in);
		  else if(cmd == "labelxy")					do_labelxy(in);
		  else if(cmd == "labels")					do_labels(in);
		  else if(cmd == "labelsxy")				do_labelsxy(in);
		  else if(cmd == "labelfile")				do_labelfile(in);
		  else if(cmd == "units")					do_units(in);
		  else if(cmd == "clear")					do_clear(in);

		  else if(cmd == "draw")
			{
			  in >> cmd;

			  if(cmd == "shapes")				do_draw_shapes(in);
			  else if(cmd == "imagemap")		do_draw_imagemap(in);
			  else if(cmd == "contours")		do_draw_contours(in);
			  else
				throw runtime_error("draw " + cmd + " not implemented");
			}
		  else
			throw runtime_error("Unknown command " + cmd);
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
