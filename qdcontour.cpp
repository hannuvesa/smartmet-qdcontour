// ======================================================================
/*!
 * \file
 * \brief Main program for qdcontour
 */
// ======================================================================

// internal
#include "ContourPattern.h"
#include "ContourRange.h"
#include "ContourSpec.h"
#include "ContourValue.h"
#include "ShapeSpec.h"
#include "StringTools.h"
// imagine
#include "NFmiColorTools.h"
#include "NFmiSmoother.h"		// for smoothing data
#include "NFmiContourTree.h"	// for contouring
#include "NFmiImage.h"			// for rendering
#include "NFmiGeoShape.h"		// for esri data
#include "NFmiText.h"			// for labels
#include "NFmiFontHershey.h"	// for Hershey fonts
// newbase
#include "NFmiCmdLine.h"			// command line options
#include "NFmiDataModifierClasses.h"
#include "NFmiEnumConverter.h"		// FmiParameterName<-->string
#include "NFmiFileSystem.h"			// FileExists()
#include "NFmiLatLonArea.h"			// Geographic projection
#include "NFmiSettings.h"			// Configuration
#include "NFmiStereographicArea.h"	// Stereographic projection
#include "NFmiStreamQueryData.h"
#include "NFmiPreProcessor.h"
// system
#include <fstream>
#include <string>
#include <list>
#ifdef OLDGCC
  #include <strstream>
#else
  #include <sstream>
#endif

using namespace std;

// ----------------------------------------------------------------------
// Usage
// ----------------------------------------------------------------------

void Usage(void)
{
  cout << "Usage: qdcontour [conffiles]" << endl << endl;
  cout << "Commands in configuration files:" << endl << endl;
}

// ----------------------------------------------------------------------
// Return true if the given name is a recognized meta function
// ----------------------------------------------------------------------

bool is_meta_parameter(const string & name)
{
  if(name == "MetaElevationAngle")
	return true;
  else
	return false;
}

// ----------------------------------------------------------------------
// Return meta function values
// ----------------------------------------------------------------------

void meta_values(const string & name,
				 NFmiFastQueryInfo * theQI,
				 NFmiDataMatrix<float> & vals)
{
  if(name == "MetaElevationAngle")
	{
	  NFmiDataMatrix<NFmiPoint> pts;
	  theQI->Locations(pts);
	  vals.Resize(pts.NX(),pts.NY(),kFloatMissing);

	  for(unsigned int j=0; j<pts.NY(); j++)
		for(unsigned int i=0; i<pts.NX(); i++)
		  {
			NFmiLocation loc(pts[i][j]);
			NFmiMetTime t(theQI->ValidTime());
			double angle = loc.ElevationAngle(t);
			vals[i][j] = static_cast<float>(angle);
		  }
	}
  else
	{
	  cerr << "Error: Unrecognized parameter name " << name << endl;
	  exit(1);
	}
}

// ----------------------------------------------------------------------
// Convert textual description of color to internal color value.
// Accepted formats:
//
// none			--> 127,0,0,0
// transparent		--> 127,0,0,0
// #rrggbb		-->   0,r,g,b
// #aarrggbb		-->   a,r,g,b
// r,g,b		-->   0,r,g,b
// a,r,g,b		-->   a,r,g,b
//
// Returns MissingColor for invalid colors.
// ----------------------------------------------------------------------

// Utility for parsing hex strings

int HexToInt(const string & theHex)
{
  int value=0;
  for(unsigned int i=0; i<theHex.length(); i++)
    {
      value *= 16;
      if(theHex[i]>='0' && theHex[i]<='9')
		value += theHex[i]-'0';
      else if(theHex[i]>='A' && theHex[i]<='F')
		value += 10+theHex[i]-'A';
      else if(theHex[i]>='a' && theHex[i]<='f')
		value += 10+theHex[i]-'a';
      else
		return -1;
    }
  return value;
}

NFmiColorTools::Color ToColor(const string & theColor)
{
  // Handle hex format number
  
  if(theColor[0]=='#')
    {
      int a,r,g,b;
      if(theColor.length()==7)
		{
		  a = 0;
		  r = HexToInt(theColor.substr(1,2));
		  g = HexToInt(theColor.substr(3,2));
		  b = HexToInt(theColor.substr(5,2));
		}
      else if(theColor.length()==9)
		{
		  a = HexToInt(theColor.substr(1,2));
		  r = HexToInt(theColor.substr(3,2));
		  g = HexToInt(theColor.substr(5,2));
		  b = HexToInt(theColor.substr(7,2));
		}
      if(r>=0 && g>=0 && b>=0 && a>=0)
		return NFmiColorTools::MakeColor(r,g,b,a);
      else
		return NFmiColorTools::MissingColor;
    }
  
  // Handle ascii format
  
  else if(theColor[0]<'0' || theColor[0]>'9')
    {
      unsigned int pos = theColor.find(",");
      if(pos == string::npos)
		return NFmiColorTools::ColorValue(theColor);
      else
		{
		  int value = -1;
		  for(unsigned int i=pos+1; i<theColor.length(); i++)
			{
			  if(theColor[i]>='0' && theColor[i]<='9')
				{
				  if(value<0)
					value = theColor[i]-'0';
				  else
					value = value*10+theColor[i]-'0';
				}
			  else
				return NFmiColorTools::MissingColor;
			}
		  if(value<0)
			return NFmiColorTools::MissingColor;
		  NFmiColorTools::Color tmp = NFmiColorTools::ColorValue(theColor.substr(0,pos));
		  return NFmiColorTools::ReplaceAlpha(tmp,value);
		}
    }
  
  
  // Handle decimal format number
  else
    {
      vector<int> tmp;
      int value=-1;
      for(unsigned int i=0; i<theColor.length(); i++)
		{
		  if(theColor[i]>='0' && theColor[i]<='9')
			{
			  if(value<0)
				value = theColor[i]-'0';
			  else
				value = value*10+theColor[i]-'0';
			}
		  else if(theColor[i]==',')
			{
			  tmp.push_back(value);
			  value = -1;
			}
		  else
			return NFmiColorTools::MissingColor;
		}
      if(value>=0)
		tmp.push_back(value);
	  
      if(tmp.size()==3)
		return NFmiColorTools::MakeColor(tmp[0],tmp[1],tmp[2],0);
      else if(tmp.size()==4)
		return NFmiColorTools::MakeColor(tmp[0],tmp[1],tmp[2],tmp[3]);
      else
		return NFmiColorTools::MissingColor;
    }
}

// ----------------------------------------------------------------------
/*!
 * This function returns the meteorological arrow for the given wind
 * speed as a NFmiPath. The arrow is suitable for stroking but not
 * for filling.
 *
 * If the speed is negative or kFloatMissing, an empty path is returned.
 *
 * Note the following details
 * \code
 *  1.25- 3.75 = 1 short on the side, plus on upward extra segment
 *  3.75- 6.25 = 1 long
 *  6.25- 8.75 = 1 long, 1 short
 *  8.75-11.25 = 2 long
 * 11.25-13.75 = 2 long, 1 short
 * \endcode
 *
 * \param theSpeed The wind speed.
 * \return The path of the meteorological arrow.
 * \todo Speeds over 50 are not supported yet
 */
// ----------------------------------------------------------------------

NFmiPath MetArrow(float theSpeed)
{
  NFmiPath path;

  // Handle bad cases
  if(theSpeed<0 || theSpeed==kFloatMissing)
	return path;

  // The details of the flag

  const float spot_size = 0.75;			// size of the spot at the origin
  const float initial_length = 14;		// length of the initial line for speed 0
  const float flag_interval = 4;		// the spacing between the speed indicators
  const float flag_length = 10;			// the length of a full speed indicator

  const float flag_angle_deg = 60;		// angle from the direction the wind is coming from

  const float flag_angle = flag_angle_deg/180*3.14159265358979323846;

  // Mark the spot with a small dot
  path.MoveTo(spot_size,spot_size);
  path.LineTo(spot_size,-spot_size);
  path.LineTo(-spot_size,-spot_size);
  path.LineTo(-spot_size,spot_size);
  path.LineTo(spot_size,spot_size);

  // Start rendering

  const float full_unit = 5;
  const float half_unit = full_unit/2;
  const float quarter_unit = full_unit/4;	// limit between rounding up and down

  const unsigned int long_segments = static_cast<unsigned int>(::floor((theSpeed+quarter_unit)/full_unit));
  const unsigned int short_segments = static_cast<unsigned int>(::floor((theSpeed-long_segments*full_unit+quarter_unit)/half_unit));

  const bool has_extra_up = (theSpeed >= quarter_unit && theSpeed < full_unit - quarter_unit);

  // The long streak upwards

  path.MoveTo(0,spot_size);
  path.LineTo(0,spot_size
			  + initial_length
			  + (std::max(1u,long_segments+short_segments)-1)*flag_interval
			  + (has_extra_up ? flag_interval : 0));

  // First the short segments (should be only one, according to present settings)

  float y = spot_size + initial_length;

  if(short_segments>0)
	for(unsigned int i=0; i<short_segments; i++)
	  {
		path.MoveTo(0,y);
		path.LineTo(flag_length/2*::sin(flag_angle),y+flag_length/2*::cos(flag_angle));
		y += flag_interval;
	  }
  
  // Then the long ones
  
  if(long_segments>0)
	for(unsigned int j=0; j<long_segments; j++)
	  {
		path.MoveTo(0,y);
		path.LineTo(flag_length*::sin(flag_angle),y+flag_length*::cos(flag_angle));
		y += flag_interval;
	  }
  
  return path;

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
// Main program.
// ----------------------------------------------------------------------

int main(int argc, const char *argv[])
{
  // Ymp‰ristˆn konfigurointi

  string datapath = NFmiSettings::instance().value("qdcontour::querydata_path",".");
  string mapspath = NFmiSettings::instance().value("qdcontour::maps_path",".");

  // Lista komentitiedostoista
  
  list<string> theFiles;
  
  // Aktiiviset contour-speksit (ja label speksit)
  
  list<ContourSpec> theSpecs;
  
  // Aktiiviset shape-speksit
  
  list<ShapeSpec> theShapeSpecs;
  
  // Aktiiviset tuulinuolet
  
  list<NFmiPoint> theArrowPoints;
  
  // Komentotiedostosta luettavat optiot
  
  string theParam = "";
  string theShapeFileName = "";
  string theContourInterpolation = "Linear";
  string theSmoother = "None";
  float theSmootherRadius = 1.0;
  int theTimeStepRoundingFlag = 1;
  int theTimeStampFlag	= 1;
  int theSmootherFactor = 1;
  int theTimeStepSkip	= 0;	// skipattava minuuttim‰‰r‰
  int theTimeStep	= 0;	// aika-askel
  int theTimeInterval   = 0;	// inklusiivinen aikam‰‰r‰
  int theTimeSteps	= 24;	// max kuvien lukum‰‰r‰
  
  int theTimeStampImageX = 0;
  int theTimeStampImageY = 0;
  string theTimeStampImage = "none";
  
  // Alueen m‰‰ritelm‰
  
  NFmiPoint theBottomLeft	= NFmiPoint(kFloatMissing,kFloatMissing);
  NFmiPoint theTopRight		= NFmiPoint(kFloatMissing,kFloatMissing);
  NFmiPoint theCenter		= NFmiPoint(kFloatMissing,kFloatMissing);

  float theCentralLongitude	= 25.0;
  float theCentralLatitude	= 90.0;
  float theTrueLatitude		= 60.0;
  
  int theWidth		= -1;
  int theHeight		= -1;
  float theScale	= -1;
  
  string theSavePath	= ".";
  string thePrefix	= "";
  string theSuffix	= "";
  string theFormat	= "png";
  bool   theSaveAlphaFlag = true;
  bool   theWantPaletteFlag = false;
  bool   theForcePaletteFlag = false;
  string theLegendErase = "white";
  string theErase	= "#7F000000";
  string theBackground	= "";
  string theForeground	= "";
  string theMask = "";
  string theFillRule	= "Atop";
  string theStrokeRule	= "Atop";
  
  string theForegroundRule = "Over";

  string theCombine = "";
  int theCombineX;
  int theCombineY;
  string theCombineRule = "Over";
  float theCombineFactor = 1.0;
  
  string theFilter = "none";

  string theDirectionParameter = "WindDirection";
  string theSpeedParameter = "WindSpeedMS";
  
  float theArrowScale = 1.0;

  float theWindArrowScaleA = 0.0;	// a*log10(b*x+1)+c = 0*log10(0+1)+1 = 1
  float theWindArrowScaleB = 0.0;
  float theWindArrowScaleC = 1.0;

  string theArrowFillColor = "white";
  string theArrowStrokeColor = "black";
  string theArrowFillRule = "Over";
  string theArrowStrokeRule = "Over";
  string theArrowFile = "";
  
  unsigned int theWindArrowDX = 0;
  unsigned int theWindArrowDY = 0;
  
  int theContourDepth	= 0;
  
  int thePngQuality = -1;
  int theJpegQuality = -1;
  int theAlphaLimit = -1;
  float theGamma = -1.0;
  string theIntent = "";
  
  // Related variables
  
  NFmiImage theBackgroundImage;
  NFmiImage theForegroundImage;
  NFmiImage theMaskImage;
  NFmiImage theCombineImage;
  
  // This holds a vector of querydatastreams
  
  vector<NFmiStreamQueryData *> theQueryStreams;
  int theQueryDataLevel = 1;
  string theQueryStreamNames = "";
  vector<string> theFullQueryFileNames;
  
  // These will hold the querydata for the active parameter
  
  NFmiQueryData *theQueryData = 0;
  NFmiFastQueryInfo *theQueryInfo = 0;
  
  bool verbose	= false;	// verbose mode off
  bool force	= false;	// overwrite disabled
  
  // Read command line options
  // ~~~~~~~~~~~~~~~~~~~~~~~~~
  
  NFmiCmdLine cmdline(argc,argv,"vf");
  
  // Check for parsing errors
  
  if(cmdline.Status().IsError())
    {
      cerr << "Error: Invalid command line:" << endl
		   << cmdline.Status().ErrorLog().CharPtr() << endl;
      Usage();
      return 1;
	  
    }
  
  // Read -v option
  
  if(cmdline.isOption('v'))
    verbose = true;
  
  // Read -f option
  
  if(cmdline.isOption('f'))
    force = true;
  
  // Read command filenames
  
  if(cmdline.NumberofParameters() == 0)
    {
      cerr << "Error: Expecting atleast one command file argument\n\n";
      return 1;
    }
  
  for(int i=1; i<=cmdline.NumberofParameters(); i++)
	theFiles.push_back(cmdline.Parameter(i));
  
  // Process all command files
  // ~~~~~~~~~~~~~~~~~~~~~~~~~
  
  list<string>::const_iterator fileiter = theFiles.begin();
  for( ; fileiter!=theFiles.end(); ++fileiter)
    {
      const string & cmdfilename = *fileiter;
	  
      if(verbose)
		cout << "Processing file: " << cmdfilename << endl;
	  
      // Open command file for reading

	  const bool strip_pound = true;
	  NFmiPreProcessor processor(strip_pound);
	  processor.SetIncluding("include", "", "");
	  if(!processor.ReadAndStripFile(cmdfilename))
		{
		  cerr << "Error: Could not parse " << cmdfilename << endl;
		  return 1;
		}
	  // Extract the assignments
	  string text = processor.GetString();

#ifdef OLDGCC
	  istrstream input(text.c_str());
#else
	  istringstream input(text);
#endif
	  
      // Process the commands
      string command;
      while( input >> command)
		{
		  // Handle comments
		  
		  if(command == "#" || command == "//" || command[0]=='#')
			{
			  // Should use numeric_limits<int>::max() to by definition
			  // skip to end of line, but numeric_limits does not exist
			  // in g++ v2.95
			  
			  input.ignore(1000000,'\n');
			}
		  
		  else if(command == "querydata")
			{
			  string newnames;
			  input >> newnames;
			  
			  if(theQueryStreamNames != newnames)
				{
				  theQueryStreamNames = newnames;
				  
				  // Delete possible old infos
				  
				  for(unsigned int i=0; i<theQueryStreams.size(); i++)
					delete theQueryStreams[i];
				  theQueryStreams.resize(0);
				  theQueryInfo = 0;
				  theQueryData = 0;
				  
				  // Split the comma separated list into a real list
				  
				  list<string> qnames;
				  unsigned int pos1 = 0;
				  while(pos1<theQueryStreamNames.size())
					{
					  unsigned int pos2 = theQueryStreamNames.find(',',pos1);
					  if(pos2==std::string::npos)
						pos2 = theQueryStreamNames.size();
					  qnames.push_back(theQueryStreamNames.substr(pos1,pos2-pos1));
					  pos1 = pos2 + 1;
					}
			  
				  // Read the queryfiles
				  
					{
					  list<string>::const_iterator iter;
					  for(iter=qnames.begin(); iter!=qnames.end(); ++iter)
						{
						  NFmiStreamQueryData * tmp = new NFmiStreamQueryData();
						  string filename = FileComplete(*iter,datapath);
						  theFullQueryFileNames.push_back(filename);
						  if(!tmp->ReadLatestData(filename))
							exit(1);
						  theQueryStreams.push_back(tmp);
						}
					}
				}
			}

		  else if(command == "querydatalevel")
			input >> theQueryDataLevel;
		  
		  else if(command == "filter")
			input >> theFilter;
		  
		  else if(command == "timestepskip")
			input >> theTimeStepSkip;
		  
		  else if(command == "timestep")
			{
			  input >> theTimeStep;
			  theTimeInterval = theTimeStep;
			}
		  
		  else if(command == "timeinterval")
			input >> theTimeInterval;
		  
		  else if(command == "timesteps")
			input >> theTimeSteps;
		  
		  else if(command == "timestamp")
			input >> theTimeStampFlag;
		  
		  else if(command == "timesteprounding")
			input >> theTimeStepRoundingFlag;
		  
		  else if(command == "timestampimage")
			input >> theTimeStampImage;
		  
		  else if(command == "timestampimagexy")
			input >> theTimeStampImageX >> theTimeStampImageY;
		  
		  else if(command == "bottomleft")
			{
			  float lon,lat;
			  input >> lon >> lat;
			  theBottomLeft.Set(lon,lat);
			}
		  
		  else if(command == "topright")
			{
			  float lon,lat;
			  input >> lon >> lat;
			  theTopRight.Set(lon,lat);
			}
		  
		  else if(command == "center")
			{
			  float lon,lat;
			  input >> lon >> lat;
			  theCenter.Set(lon,lat);
			}

		  else if(command == "stereographic")
			input >> theCentralLongitude
				   >> theCentralLatitude
				   >> theTrueLatitude;
		  
		  else if(command == "size")
			{
			  input >> theWidth >> theHeight;
			  theBackground = "";
			}
		  
		  else if(command == "width")
			input >> theWidth;
		  
		  else if(command == "height")
			input >> theHeight;
		  
		  else if(command == "scale")
			input >> theScale;

		  else if(command == "erase")
			{
			  input >> theErase;
			  if(ToColor(theErase)==NFmiColorTools::MissingColor)
				{
				  cerr << "Error: Invalid color " << theErase << endl;
				  exit(1);
				}
			}
		  
		  else if(command == "legenderase")
			{
			  input >> theLegendErase;
			  if(ToColor(theLegendErase)==NFmiColorTools::MissingColor)
				{
				  cerr << "Error: Invalid color " << theLegendErase << endl;
				  exit(1);
				}
			}
		  
		  else if(command == "fillrule")
			{
			  input >> theFillRule;
			  if(NFmiColorTools::BlendValue(theFillRule)==NFmiColorTools::kFmiColorRuleMissing)
				{
				  cerr << "Error: Unknown blending rule " << theFillRule << endl;
				  exit(1);
				}
			  if(!theShapeSpecs.empty())
				theShapeSpecs.back().fillrule(theFillRule);
			}
		  else if(command == "strokerule")
			{
			  input >> theStrokeRule;
			  if(NFmiColorTools::BlendValue(theStrokeRule)==NFmiColorTools::kFmiColorRuleMissing)
				{
				  cerr << "Error: Unknown blending rule " << theStrokeRule << endl;
				  exit(1);
				}
			  if(!theShapeSpecs.empty())
				theShapeSpecs.back().strokerule(theStrokeRule);
			}
		  
		  else if(command == "directionparam")
		    input >> theDirectionParameter;

		  else if(command == "speedparam")
		    input >> theSpeedParameter;

		  else if(command == "arrowscale")
			input >> theArrowScale;

		  else if(command == "windarrowscale")
			input >> theWindArrowScaleA >> theWindArrowScaleB >> theWindArrowScaleC;
		  
		  else if(command == "arrowfill")
			{
			  input >> theArrowFillColor >> theArrowFillRule;
			  if(ToColor(theArrowFillColor)==NFmiColorTools::MissingColor)
				{
				  cerr << "Error: Invalid color " << theArrowFillColor << endl;
				  exit(1);
				}
			  if(NFmiColorTools::BlendValue(theArrowFillRule)==NFmiColorTools::kFmiColorRuleMissing)
				{
				  cerr << "Error: Unknown blending rule " << theArrowFillRule << endl;
				  exit(1);
				}
			}
		  else if(command == "arrowstroke")
			{
			  input >> theArrowStrokeColor >> theArrowStrokeRule;
			  if(ToColor(theArrowStrokeColor)==NFmiColorTools::MissingColor)
				{
				  cerr << "Error: Invalid color " << theArrowStrokeColor << endl;
				  exit(1);
				}
			  if(NFmiColorTools::BlendValue(theArrowStrokeRule)==NFmiColorTools::kFmiColorRuleMissing)
				{
				  cerr << "Error: Unknown blending rule " << theArrowStrokeRule << endl;
				  exit(1);
				}
			}
		  
		  else if(command == "arrowpath")
			input >> theArrowFile;
		  
		  else if(command == "windarrow")
			{
			  float lon,lat;
			  input >> lon >> lat;
			  theArrowPoints.push_back(NFmiPoint(lon,lat));
			}
		  
		  else if(command == "windarrows")
			input >> theWindArrowDX >> theWindArrowDY;
		  
		  else if(command == "background")
			{
			  input >> theBackground;
			  if(theBackground != "none")
				theBackgroundImage.Read(FileComplete(theBackground,mapspath));
			  else
				theBackground = "";
			}
		  
		  else if(command == "foreground")
			{
			  input >> theForeground;
			  if(theForeground != "none")
				theForegroundImage.Read(FileComplete(theForeground,mapspath));
			  else
				theForeground = "";
			}
		  
		  else if(command == "mask")
			{
			  input >> theMask;
			  if(theMask != "none")
				theMaskImage.Read(FileComplete(theMask,mapspath));
			  else
				theMask = "";
			}
		  else if(command == "combine")
			{
			  input >> theCombine;
			  if(theCombine != "none")
				{
				  input >> theCombineX >> theCombineY;
				  input >> theCombineRule >> theCombineFactor;
				  if(NFmiColorTools::BlendValue(theCombineRule)==NFmiColorTools::kFmiColorRuleMissing)
					{
					  cerr << "Error: Unknown blending rule " << theCombineRule << endl;
					  exit(1);
					}
				  theCombineImage.Read(FileComplete(theCombine,mapspath));
				}
			  else
				theCombine = "";
			}

		  else if(command == "foregroundrule")
			{
			  input >> theForegroundRule;
			  
			  if(NFmiColorTools::BlendValue(theForegroundRule)==NFmiColorTools::kFmiColorRuleMissing)
				{
				  cerr << "Error: Unknown blending rule " << theForegroundRule << endl;
				  exit(1);
				}
			}
		  
		  else if(command == "savepath")
			{
			  input >> theSavePath;
			  if(!DirectoryExists(theSavePath))
				{
				  cerr << "savepath " << theSavePath << " does not exist!" << endl;
				  return 1;
				}
			}
		  
		  else if(command == "prefix")
			input >> thePrefix;
		  
		  else if(command == "suffix")
			input >> theSuffix;
		  
		  else if(command == "format")
			input >> theFormat;
		  
		  else if(command == "gamma")
			input >> theGamma;
		  
		  else if(command == "intent")
			input >> theIntent;
		  
		  else if(command == "pngquality")
			input >> thePngQuality;
		  
		  else if(command == "jpegquality")
			input >> theJpegQuality;
		  
		  else if(command == "savealpha")
			input >> theSaveAlphaFlag;
		  
		  else if(command == "wantpalette")
			input >> theWantPaletteFlag;
		  
		  else if(command == "forcepalette")
			input >> theForcePaletteFlag;
		  
		  else if(command == "alphalimit")
			input >> theAlphaLimit;
		  
		  else if(command == "hilimit")
			{
			  float limit;
			  input >> limit;
			  if(!theSpecs.empty())
				theSpecs.back().ExactHiLimit(limit);
			}
		  else if(command == "datalolimit")
			{
			  float limit;
			  input >> limit;
			  if(!theSpecs.empty())
				theSpecs.back().DataLoLimit(limit);
			}
		  else if(command == "datahilimit")
			{
			  float limit;
			  input >> limit;
			  if(!theSpecs.empty());
			  theSpecs.back().DataHiLimit(limit);
			}
		  else if(command == "datareplace")
			{
			  float src,dst;
			  input >> src >> dst;
			  if(!theSpecs.empty())
				theSpecs.back().Replace(src,dst);
			}
		  else if(command == "contourdepth")
			{
			  input >> theContourDepth;
			  if(!theSpecs.empty())
				theSpecs.back().ContourDepth(theContourDepth);
			}
		  
		  else if(command == "contourinterpolation")
			{
			  input >> theContourInterpolation;
			  if(!theSpecs.empty())
				theSpecs.back().ContourInterpolation(theContourInterpolation);
			}
		  else if(command == "smoother")
			{
			  input >> theSmoother;
			  if(!theSpecs.empty())
				theSpecs.back().Smoother(theSmoother);
			}
		  else if(command == "smootherradius")
			{
			  input >> theSmootherRadius;
			  if(!theSpecs.empty())
				theSpecs.back().SmootherRadius(theSmootherRadius);
			}
		  else if(command == "smootherfactor")
			{
			  input >> theSmootherFactor;
			  if(!theSpecs.empty())
				theSpecs.back().SmootherFactor(theSmootherFactor);
			}
		  else if(command == "param")
			{
			  input >> theParam;
			  theSpecs.push_back(ContourSpec(theParam,
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
				  
				  if(NFmiColorTools::BlendValue(markerrule)==NFmiColorTools::kFmiColorRuleMissing)
					{
					  cerr << "Error: Unknown blending rule " << markerrule << endl;
					  exit(1);
					}
				  ShapeSpec spec(theShapeFileName);
				  spec.marker(marker,markerrule,markeralpha);
				  theShapeSpecs.push_back(spec);
				}
			  else
				{
				  string fillcolor = arg1;
				  string strokecolor;
				  input >> strokecolor;
				  NFmiColorTools::Color fill = ToColor(fillcolor);
				  NFmiColorTools::Color stroke = ToColor(strokecolor);
				  if(fill == NFmiColorTools::MissingColor)
					{
					  cerr << "Error: fillcolor " << fillcolor << " unrecognized" << endl;
					  exit(1);
					}
				  if(stroke == NFmiColorTools::MissingColor)
					{
					  cerr << "Error: strokecolor " << strokecolor << " unrecognized" << endl;
					  exit(1);
					}
				  theShapeSpecs.push_back(ShapeSpec(theShapeFileName,
													fill,stroke,
													theFillRule,theStrokeRule));
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
			  
			  NFmiColorTools::Color color = ToColor(scolor);
			  
			  if(!theSpecs.empty())
				theSpecs.back().Add(ContourRange(lo,hi,color,theFillRule));
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
			  
			  if(!theSpecs.empty())
				theSpecs.back().Add(ContourPattern(lo,hi,spattern,srule,alpha));
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
			  
			  NFmiColorTools::Color color = ToColor(scolor);
			  if(!theSpecs.empty())
				theSpecs.back().Add(ContourValue(value,color,theStrokeRule));
			}
		  
		  else if(command == "contourfills")
			{
			  float lo,hi,step;
			  string scolor1,scolor2;
			  input >> lo >> hi >> step >> scolor1 >> scolor2;
			  
			  int color1 = ToColor(scolor1);
			  int color2 = ToColor(scolor2);
			  
			  int steps = static_cast<int>((hi-lo)/step);
			  
			  for(int i=0; i<steps; i++)
				{
				  float tmplo=lo+i*step;
				  float tmphi=lo+(i+1)*step;
				  int color = color1;	// in case steps=1
				  if(steps!=1)
					color = NFmiColorTools::Interpolate(color1,color2,i/(steps-1.0));
				  if(!theSpecs.empty())
					theSpecs.back().Add(ContourRange(tmplo,tmphi,color,theFillRule));
				  // if(verbose)
				  // cout << "Interval " << tmplo << "," << tmphi
				  // << " colour is "
				  // << NFmiColorTools::GetRed(color) << ","
				  // << NFmiColorTools::GetGreen(color) << ","
				  // << NFmiColorTools::GetBlue(color) << ","
				  // << NFmiColorTools::GetAlpha(color)
				  // << endl;
				}
			}
	      
		  else if(command == "contourlines")
			{
			  float lo,hi,step;
			  string scolor1,scolor2;
			  input >> lo >> hi >> step >> scolor1 >> scolor2;
			  
			  int color1 = ToColor(scolor1);
			  int color2 = ToColor(scolor2);
			  
			  int steps = static_cast<int>((hi-lo)/step);
			  
			  for(int i=0; i<=steps; i++)
				{
				  float tmplo=lo+i*step;
				  int color = color1;	// in case steps=1
				  if(steps!=0)
					color = NFmiColorTools::Interpolate(color1,color2,i/steps);
				  if(!theSpecs.empty())
					theSpecs.back().Add(ContourValue(tmplo,color,theStrokeRule));
				}
			}
	      
		  else if(command == "clear")
			{
			  input >> command;
			  if(command=="contours")
				theSpecs.clear();
			  else if(command=="shapes")
				theShapeSpecs.clear();
			  else if(command=="arrows")
				{
				  theArrowPoints.clear();
				  theWindArrowDX = 0;
				  theWindArrowDY = 0;
				}
			  else if(command=="labels")
				{
				  list<ContourSpec>::iterator piter;
				  for(piter=theSpecs.begin(); piter!=theSpecs.end(); ++piter)
					piter->ClearLabels();
				}
			  else if(command=="corners")
				{
				  theBottomLeft = NFmiPoint(kFloatMissing,kFloatMissing);
				  theTopRight = NFmiPoint(kFloatMissing,kFloatMissing);
				}
			  else
				{
				  cerr << "Error: Unknown clear target: " << command << endl;
				  exit(1);
				}
			}
		  
		  else if(command == "labelmarker")
			{
			  string filename, rule;
			  float alpha;
			  
			  input >> filename >> rule >> alpha;
			  
			  if(!theSpecs.empty())
				{
				  theSpecs.back().LabelMarker(filename);
				  theSpecs.back().LabelMarkerRule(rule);
				  theSpecs.back().LabelMarkerAlphaFactor(alpha);
				}
			}
		  
		  else if(command == "labelfont")
			{
			  string font;
			  input >> font;
			  if(!theSpecs.empty())
				theSpecs.back().LabelFont(font);
			}
		  
		  else if(command == "labelsize")
			{
			  float size;
			  input >> size;
			  if(!theSpecs.empty())
				theSpecs.back().LabelSize(size);
			}
		  
		  else if(command == "labelstroke")
			{
			  string color,rule;
			  input >> color >> rule;
			  if(!theSpecs.empty())
				{
				  theSpecs.back().LabelStrokeColor(ToColor(color));
				  theSpecs.back().LabelStrokeRule(rule);
				}
			}
		  
		  else if(command == "labelfill")
			{
			  string color,rule;
			  input >> color >> rule;
			  if(!theSpecs.empty())
				{
				  theSpecs.back().LabelFillColor(ToColor(color));
				  theSpecs.back().LabelFillRule(rule);
				}
			}
		  
		  else if(command == "labelalign")
			{
			  string align;
			  input >> align;
			  if(!theSpecs.empty())
				theSpecs.back().LabelAlignment(align);
			}
		  
		  else if(command == "labelformat")
			{
			  string format;
			  input >> format;
			  if(format == "-") format = "";
			  if(!theSpecs.empty())
				theSpecs.back().LabelFormat(format);
			}
		  
		  else if(command == "labelmissing")
			{
			  string label;
			  input >> label;
			  if(label == "none") label = "";
			  if(!theSpecs.empty())
				theSpecs.back().LabelMissing(label);
			}
		  
		  else if(command == "labelangle")
			{
			  float angle;
			  input >> angle;
			  if(!theSpecs.empty())
				theSpecs.back().LabelAngle(angle);
			}
		  
		  else if(command == "labeloffset")
			{
			  float dx,dy;
			  input >> dx >> dy;
			  if(!theSpecs.empty())
				{
				  theSpecs.back().LabelOffsetX(dx);
				  theSpecs.back().LabelOffsetY(dy);
				}
			}
		  
		  else if(command == "labelcaption")
			{
			  string name,align;
			  float dx,dy;
			  input >> name >> dx >> dy >> align;
			  if(!theSpecs.empty())
				{
				  theSpecs.back().LabelCaption(name);
				  theSpecs.back().LabelCaptionDX(dx);
				  theSpecs.back().LabelCaptionDY(dy);
				  theSpecs.back().LabelCaptionAlignment(align);
				}
			}
		  
		  else if(command == "label")
			{
			  float lon,lat;
			  input >> lon >> lat;
			  if(!theSpecs.empty())
				theSpecs.back().Add(NFmiPoint(lon,lat));
			}
		  
		  else if(command == "labelxy")
			{
			  float lon,lat;
			  input >> lon >> lat;
			  int dx, dy;
			  input >> dx >> dy;
			  if(!theSpecs.empty())
				theSpecs.back().Add(NFmiPoint(lon,lat),NFmiPoint(dx,dy));
			}
		  
		  else if(command == "labels")
			{
			  int dx,dy;
			  input >> dx >> dy;
			  if(!theSpecs.empty())
				{
				  theSpecs.back().LabelDX(dx);
				  theSpecs.back().LabelDY(dy);
				}

			}
		  
		  else if(command == "labelfile")
			{
			  string datafilename;
			  input >> datafilename;
			  ifstream datafile(datafilename.c_str());
			  if(!datafile)
				{
				  cerr << "Error: No data file named " << datafilename << endl;
				  exit(1);
				}
			  string datacommand;
			  while( datafile >> datacommand)
				{
				  if(datacommand == "#" || datacommand == "//")
					datafile.ignore(1000000,'\n');
				  else if(datacommand == "label")
					{
					  float lon,lat;
					  datafile >> lon >> lat;
					  if(!theSpecs.empty())
						theSpecs.back().Add(NFmiPoint(lon,lat));
					}
				  else
					{
					  cerr << "Error: Unknown datacommand " << datacommand << endl;
					  exit(1);
					}
				}
			  datafile.close();
			}
		  
		  else if(command == "draw")
			{
			  // Draw what?
			  
			  input >> command;
			  
			  // --------------------------------------------------
			  // Draw legend
			  // --------------------------------------------------
			  
			  if(command == "legend")
				{
				  string legendname;
				  int width, height;
				  float lolimit, hilimit;
				  input >> legendname >> lolimit >> hilimit >> width >> height;
				  
				  if(!theSpecs.empty())
					{
					  NFmiImage legend(width,height);
					  
					  NFmiColorTools::Color color = ToColor(theLegendErase);
					  legend.Erase(color);
					  
					  list<ContourRange>::const_iterator citer;
					  list<ContourRange>::const_iterator cbegin;
					  list<ContourRange>::const_iterator cend;
					  
					  cbegin = theSpecs.back().ContourFills().begin();
					  cend   = theSpecs.back().ContourFills().end();
					  
					  for(citer=cbegin ; citer!=cend; ++citer)
						{
						  float thelo = citer->lolimit();
						  float thehi = citer->hilimit();
						  NFmiColorTools::NFmiBlendRule rule = NFmiColorTools::BlendValue(citer->rule());
						  
						  if(thelo==kFloatMissing) thelo=-1e6;
						  if(thehi==kFloatMissing) thehi= 1e6;
						  
						  NFmiPath path;
						  path.MoveTo(0,height*(1-(thelo-lolimit)/(hilimit-lolimit)));
						  path.LineTo(width,height*(1-(thelo-lolimit)/(hilimit-lolimit)));
						  path.LineTo(width,height*(1-(thehi-lolimit)/(hilimit-lolimit)));
						  path.LineTo(0,height*(1-(thehi-lolimit)/(hilimit-lolimit)));
						  path.CloseLineTo();
						  
						  path.Fill(legend,citer->color(),rule);
						}
					  
					  list<ContourValue>::const_iterator liter;
					  list<ContourValue>::const_iterator lbegin;
					  list<ContourValue>::const_iterator lend;
					  
					  lbegin = theSpecs.back().ContourValues().begin();
					  lend   = theSpecs.back().ContourValues().end();
					  
					  for(liter=lbegin ; liter!=lend; ++liter)
						{
						  float thevalue = liter->value();
						  
						  if(thevalue==kFloatMissing)
							continue;
						  
						  NFmiPath path;
						  path.MoveTo(0,height*(1-(thevalue-lolimit)/(hilimit-lolimit)));
						  path.LineTo(width,height*(1-(thevalue-lolimit)/(hilimit-lolimit)));
						  path.Stroke(legend,liter->color());
						}
					  
					  legend.WritePng(legendname+".png");
					}
				  
				}
			  
			  // --------------------------------------------------
			  // Render shapes
			  // --------------------------------------------------
			  
			  else if(command == "shapes")
				{
				  // The output filename
				  
				  string filename;
				  input >> filename;
				  
				  if(theBottomLeft.X()==kFloatMissing ||
					 theBottomLeft.Y()==kFloatMissing ||
					 theTopRight.X()==kFloatMissing ||
					 theTopRight.Y()==kFloatMissing)
					{

					  if(theCenter.X()==kFloatMissing || theCenter.Y()==kFloatMissing)
						{
						  cerr << "Error: Area corner coordinates not given" << endl;
						  return 1;
						}
					  
					  if(theScale<0 || theWidth<0 || theHeight<0)
						{
						  cerr << "Error: scale, width and height must be given along with center coordinates" << endl;
						  return 1;
						}

					  NFmiStereographicArea area(theCenter,theCenter,
												 theCentralLongitude,
												 NFmiPoint(0,0),
												 NFmiPoint(1,1),
												 theCentralLatitude,
												 theTrueLatitude);

					  NFmiPoint c = area.LatLonToWorldXY(theCenter);
					  
					  NFmiPoint bl(c.X()-theScale*1000*theWidth, c.Y()-theScale*1000*theHeight);
					  NFmiPoint tr(c.X()+theScale*1000*theWidth, c.Y()+theScale*1000*theHeight);

					  theBottomLeft = area.WorldXYToLatLon(bl);
					  theTopRight = area.WorldXYToLatLon(tr);

					  if(verbose)
						{
						  cout << "Calculated corner points to be"
							   << endl
							   << "bottomleft\t= " 
							   << theBottomLeft.X()
							   << ','
							   << theBottomLeft.Y()
							   << endl
							   << "topright\t= "
							   << theTopRight.X()
							   << ','
							   << theTopRight.Y()
							   << endl;
						}

					}

				  
				  // Initialize XY-coordinates
				  
				  NFmiStereographicArea area(theBottomLeft,
											 theTopRight,
											 theCentralLongitude,
											 NFmiPoint(0,0),
											 NFmiPoint(1,1),
											 theCentralLatitude,
											 theTrueLatitude);
				  
				  // Calculate world coordinates
				  
				  NFmiPoint bl = area.LatLonToWorldXY(theBottomLeft);
				  NFmiPoint tr = area.LatLonToWorldXY(theTopRight);
				  
				  if(theWidth<=0 && theHeight>0)
					{
					  // Calculate width from height
					  theWidth = static_cast<int>((tr.X()-bl.X())/(tr.Y()-bl.Y())*theHeight);
					}
				  else if(theHeight<=0 && theWidth>0)
					{
					  // Calculate height from width
					  theHeight = static_cast<int>((tr.Y()-bl.Y())/(tr.X()-bl.X())*theWidth);
					}
				  else if(theWidth<=0 && theHeight<=0)
					{
					  cerr << "Error: Image width & height unspecified"
						   << endl;
					  exit(1);
					}
				  
				  // The actual area we wanted
				  
				  NFmiStereographicArea theArea(theBottomLeft,
												theTopRight,
												theCentralLongitude,
												NFmiPoint(0,0),
												NFmiPoint(theWidth,theHeight),
												theCentralLatitude,
												theTrueLatitude);
				  
				  // Initialize the background
				  
				  NFmiImage theImage(theWidth,theHeight);
				  theImage.SaveAlpha(theSaveAlphaFlag);
				  theImage.WantPalette(theWantPaletteFlag);
				  theImage.ForcePalette(theForcePaletteFlag);
				  if(theGamma>0) theImage.Gamma(theGamma);
				  if(!theIntent.empty()) theImage.Intent(theIntent);
				  if(thePngQuality>=0) theImage.PngQuality(thePngQuality);
				  if(theJpegQuality>=0) theImage.JpegQuality(theJpegQuality);
				  if(theAlphaLimit>=0) theImage.AlphaLimit(theAlphaLimit);
				  
				  NFmiColorTools::Color erasecolor = ToColor(theErase);
				  theImage.Erase(erasecolor);
				  
				  // Draw all the shapes
				  
				  list<ShapeSpec>::const_iterator iter;
				  list<ShapeSpec>::const_iterator begin = theShapeSpecs.begin();
				  list<ShapeSpec>::const_iterator end   = theShapeSpecs.end();
				  
				  for(iter=begin; iter!=end; ++iter)
					{
					  NFmiGeoShape geo(iter->filename(),kFmiGeoShapeEsri);
					  geo.ProjectXY(theArea);
					  
					  if(iter->marker()=="")
						{
						  NFmiColorTools::NFmiBlendRule fillrule = NFmiColorTools::BlendValue(iter->fillrule());
						  NFmiColorTools::NFmiBlendRule strokerule = NFmiColorTools::BlendValue(iter->strokerule());
						  geo.Fill(theImage,iter->fillcolor(),fillrule);
						  geo.Stroke(theImage,iter->strokecolor(),strokerule);
						}
					  else
						{
						  NFmiColorTools::NFmiBlendRule markerrule = NFmiColorTools::BlendValue(iter->markerrule());
						  
						  NFmiImage marker;
						  marker.Read(iter->marker());
						  geo.Mark(theImage,marker,markerrule,
								   kFmiAlignCenter,
								   iter->markeralpha());
						}
					}
				  
				  string outfile = filename + "." + theFormat;
				  if(verbose)
					cout << "Writing " << outfile << endl;
				  if(theFormat=="png")
					theImage.WritePng(outfile);
				  else if(theFormat=="jpg" || theFormat=="jpeg")
					theImage.WriteJpeg(outfile);
				  else if(theFormat=="gif")
					theImage.WriteGif(outfile);
				  else
					{
					  cerr << "Error: Image format " << theFormat << " is not supported" << endl;
					  return 1;
					}
				}
			  
			  // --------------------------------------------------
			  // Generate imagemap data
			  // --------------------------------------------------
			  
			  else if(command == "imagemap")
				{
				  // The relevant field name and filenames
				  
				  string fieldname, filename;
				  input >> fieldname >> filename;
				  
				  if(theBottomLeft.X()==kFloatMissing ||
					 theBottomLeft.Y()==kFloatMissing ||
					 theTopRight.X()==kFloatMissing ||
					 theTopRight.Y()==kFloatMissing)
					{
					  cerr << "Error: Area corner coordinates not given"
						   << endl;
					  return 1;


					}
				  
				  // Initialize XY-coordinates
				  
				  NFmiStereographicArea area(theBottomLeft,
											 theTopRight,
											 theCentralLongitude,
											 NFmiPoint(0,0),
											 NFmiPoint(1,1),
											 theCentralLatitude,
											 theTrueLatitude);
				  
				  // Calculate world coordinates
				  
				  NFmiPoint bl = area.LatLonToWorldXY(theBottomLeft);
				  NFmiPoint tr = area.LatLonToWorldXY(theTopRight);
				  
				  if(theWidth<=0 && theHeight>0)
					{
					  // Calculate width from height
					  theWidth = static_cast<int>((tr.X()-bl.X())/(tr.Y()-bl.Y())*theHeight);
					}
				  else if(theHeight<=0 && theWidth>0)
					{
					  // Calculate height from width
					  theHeight = static_cast<int>((tr.Y()-bl.Y())/(tr.X()-bl.X())*theWidth);
					}
				  else if(theWidth<=0 && theHeight<=0)
					{
					  cerr << "Error: Image width & height unspecified"
						   << endl;
					  exit(1);
					}
				  
				  // The actual area we wanted
				  
				  NFmiStereographicArea theArea(theBottomLeft,
												theTopRight,
												theCentralLongitude,
												NFmiPoint(0,0),
												NFmiPoint(theWidth,theHeight),
												theCentralLatitude,
												theTrueLatitude);
				  
				  // Generate map from all shapes in the list
				  
				  list<ShapeSpec>::const_iterator iter;
				  list<ShapeSpec>::const_iterator begin = theShapeSpecs.begin();
				  list<ShapeSpec>::const_iterator end   = theShapeSpecs.end();
				  
				  string outfile = filename + ".map";
				  ofstream out(outfile.c_str());
				  if(!out)
					{
					  cerr << "Error: Failed to open "
						   << outfile
						   << " for writing"
						   << endl;
					  exit(1);
					}
				  if(verbose)
					cout << "Writing " << outfile << endl;
				  
				  for(iter=begin; iter!=end; ++iter)
					{
					  NFmiGeoShape geo(iter->filename(),kFmiGeoShapeEsri);
					  geo.ProjectXY(theArea);
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
				  
				  if(theQueryStreams.empty())
					{
					  cerr << "Error: No query data has been read!\n";
					  return 1;
					}
				  
				  // Check map image width & height
				  
				  if(theBackground != "")
					{
					  theWidth = theBackgroundImage.Width();
					  theHeight = theBackgroundImage.Height();
					}
				  
				  if(theBottomLeft.X()==kFloatMissing ||
					 theBottomLeft.Y()==kFloatMissing ||
					 theTopRight.X()==kFloatMissing ||
					 theTopRight.Y()==kFloatMissing)
					{

					  // Duplicate code as in "draw shapes"
					  if(theCenter.X()==kFloatMissing || theCenter.Y()==kFloatMissing)
						{
						  cerr << "Error: Area corner coordinates not given" << endl;
						  return 1;
						}
					  
					  if(theScale<0 || theWidth<0 || theHeight<0)
						{
						  cerr << "Error: scale, width and height must be given along with center coordinates" << endl;
						  return 1;
						}

					  NFmiStereographicArea area(theCenter,theCenter,
												 theCentralLongitude,
												 NFmiPoint(0,0),
												 NFmiPoint(1,1),
												 theCentralLatitude,
												 theTrueLatitude);

					  NFmiPoint c = area.LatLonToWorldXY(theCenter);
					  
					  NFmiPoint bl(c.X()-theScale*1000*theWidth, c.Y()-theScale*1000*theHeight);
					  NFmiPoint tr(c.X()+theScale*1000*theWidth, c.Y()+theScale*1000*theHeight);

					  theBottomLeft = area.WorldXYToLatLon(bl);
					  theTopRight = area.WorldXYToLatLon(tr);
					  
					}
				  
				  // Initialize XY-coordinates
				  
				  NFmiStereographicArea area(theBottomLeft,
											 theTopRight,
											 theCentralLongitude,
											 NFmiPoint(0,0),
											 NFmiPoint(1,1),
											 theCentralLatitude,
											 theTrueLatitude);
				  
				  // Calculate world coordinates
				  
				  NFmiPoint bl = area.LatLonToWorldXY(theBottomLeft);
				  NFmiPoint tr = area.LatLonToWorldXY(theTopRight);
				  
				  if(theWidth<=0 && theHeight>0)
					{
					  // Calculate width from height
					  theWidth = static_cast<int>((tr.X()-bl.X())/(tr.Y()-bl.Y())*theHeight);
					}
				  else if(theHeight<=0 && theWidth>0)
					{
					  // Calculate height from width
					  theHeight = static_cast<int>((tr.Y()-bl.Y())/(tr.X()-bl.X())*theWidth);
					}
				  else if(theWidth<=0 && theHeight<=0)
					{
					  cerr << "Error: Image width & height unspecified"
						   << endl;
					  exit(1);
					}
				  
				  // The actual area we wanted
				  
				  NFmiStereographicArea theArea(theBottomLeft,
												theTopRight,
												theCentralLongitude,
												NFmiPoint(0,0),
												NFmiPoint(theWidth,theHeight),
												theCentralLatitude,
												theTrueLatitude);
				  
				  // Establish querydata timelimits and initialize
				  // the XY-coordinates simultaneously.
				  
				  // Note that we use world-coordinates when smoothing
				  // so that we can use meters as the smoothing radius.
				  // Also, this means the contours are independent of
				  // the image size.
				  
				  NFmiTime utctime, time1, time2;
				  
				  vector<NFmiDataMatrix<NFmiPoint> > worldpts(theQueryStreams.size());
				  vector<NFmiDataMatrix<NFmiPoint> > pts(theQueryStreams.size());
				  NFmiDataMatrix<float> vals;
				  
				  unsigned int qi;
				  for(qi=0; qi<theQueryStreams.size(); qi++)
					{
					  // Initialize the queryinfo
					  
					  theQueryInfo = theQueryStreams[qi]->QueryInfoIter();
					  theQueryInfo->FirstLevel();
					  if(theQueryDataLevel>0)
						{
						  int level = theQueryDataLevel;
						  while(--level > 0)
							theQueryInfo->NextLevel();
						}
					  
					  // Establish time limits
					  theQueryInfo->LastTime();
					  utctime = theQueryInfo->ValidTime();
					  NFmiTime t2 = NFmiMetTime(utctime,1).CorrectLocalTime();
					  theQueryInfo->FirstTime();
					  utctime = theQueryInfo->ValidTime();
					  NFmiTime t1 = NFmiMetTime(utctime,1).CorrectLocalTime();
					  
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
					  
					  // Establish coordinates
					  
					  theQueryInfo->LocationsWorldXY(worldpts[qi],theArea);
					  
					  theQueryInfo->LocationsXY(pts[qi],theArea);
					  
					}
				  
				  if(verbose)
					{
					  cout << "Data start time " << time1 << endl
						   << "Data end time " << time2 << endl;
					}
				  
				  // Skip to first time
				  
				  NFmiMetTime tmptime(time1,
									  theTimeStepRoundingFlag ?
									  (theTimeStep>0 ? theTimeStep : 1) :
									  1);
				  
				  tmptime.ChangeByMinutes(theTimeStepSkip);
				  if(theTimeStepRoundingFlag)
					tmptime.PreviousMetTime();
				  NFmiTime t = tmptime;
				  
				  // Loop over all times
				  
				  int imagesdone = 0;
				  while(true)
					{
					  if(imagesdone>=theTimeSteps)
						break;
					  
					  // Skip to next time to be drawn
					  
					  t.ChangeByMinutes(theTimeStep > 0 ? theTimeStep : 1);
					  
					  cout << t << endl;
					  
					  // If the time is after time2, we're done
					  
					  if(time2.IsLessThan(t))
						break;
					  
					  // Search first time >= the desired time
					  // This is quaranteed to succeed since we've
					  // already tested against time2, the last available
					  // time.
					  
					  bool ok = true;
					  for(qi=0; ok && qi<theQueryStreams.size(); qi++)
						{
						  theQueryInfo = theQueryStreams[qi]->QueryInfoIter();
						  theQueryInfo->ResetTime();
						  while(theQueryInfo->NextTime())
							{
							  NFmiTime utc = theQueryInfo->ValidTime();
							  NFmiTime loc = NFmiMetTime(utc,1).CorrectLocalTime();
							  if(!loc.IsLessThan(t))
								break;
							}
						  NFmiTime utc = theQueryInfo->ValidTime();
						  NFmiTime tnow = NFmiMetTime(utc,1).CorrectLocalTime();
						  
						  // we wanted
						  
						  if(theTimeStep==0)
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
						  tprev.ChangeByMinutes(-theTimeInterval);
						  
						  bool hasprevious = !tprev.IsLessThan(time1);
						  
						  // Skip this image if we are unable to render it
						  
						  if(theFilter=="none")
							{
							  // Cannot draw time with filter none
							  // if time is not exact.
							  
							  ok = isexact;
							  
							}
						  else if(theFilter=="linear")
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
					  
					  if(verbose)
						cout << "Time is " << datatimestr.CharPtr() << endl;

					  string filename =
						theSavePath
						+ "/"
						+ thePrefix
						+ datatimestr.CharPtr();
					  
					  if(theTimeStampFlag)
						{
						  for(qi=0; qi<theFullQueryFileNames.size(); qi++)
							{
							  time_t secs = FileModificationTime(theFullQueryFileNames[qi]);
							  NFmiTime tlocal(secs);
							  filename += "_" + tlocal.ToStr(kDDHHMM);
							}
						}
					  
					  filename +=
						theSuffix
						+ "."
						+ theFormat;
					  
					  // In force-mode we always write, but otherwise
					  // we first check if the output image already
					  // exists. If so, we assume it is up to date
					  // and skip to the next time stamp.
					  
					  if(!force && !FileEmpty(filename))
						{
						  if(verbose)
							cout << "Not overwriting " << filename << endl;
						  continue;
						}
					  
					  // Initialize the background
					  
					  NFmiImage theImage(theWidth,theHeight);
					  theImage.SaveAlpha(theSaveAlphaFlag);
					  theImage.WantPalette(theWantPaletteFlag);
					  theImage.ForcePalette(theForcePaletteFlag);
					  if(theGamma>0) theImage.Gamma(theGamma);
					  if(!theIntent.empty()) theImage.Intent(theIntent);
					  if(thePngQuality>=0) theImage.PngQuality(thePngQuality);
					  if(theJpegQuality>=0) theImage.JpegQuality(theJpegQuality);
					  if(theAlphaLimit>=0) theImage.AlphaLimit(theAlphaLimit);
					  
					  NFmiColorTools::Color erasecolor = ToColor(theErase);
					  theImage.Erase(erasecolor);
					  
					  if(theBackground != "")
						theImage = theBackgroundImage;
					  
					  // Loop over all parameters
					  
					  list<ContourSpec>::iterator piter;
					  list<ContourSpec>::iterator pbegin = theSpecs.begin();
					  list<ContourSpec>::iterator pend   = theSpecs.end();
					  
					  for(piter=pbegin; piter!=pend; ++piter)
						{
						  // Establish the parameter
						  
						  string name = piter->Param();

						  bool ismeta = false;
						  ok = false;
						  FmiParameterName param = FmiParameterName(NFmiEnumConverter().ToEnum(name));
						  
						  if(param==kFmiBadParameter)
							{
							  if(!is_meta_parameter(name))
								{
								  cerr << "Error: Unknown parameter " << name << endl;
								  return 1;
								}
							  ismeta = true;
							  ok = true;
							  // We always assume the first querydata is ok
							  qi = 0;
							  theQueryInfo = theQueryStreams[0]->QueryInfoIter();
							}
						  else
							{
							  // Find the proper queryinfo to be used
							  // Note that qi will be used later on for
							  // getting the coordinate matrices
							  
							  for(qi=0; qi<theQueryStreams.size(); qi++)
								{
								  theQueryInfo = theQueryStreams[qi]->QueryInfoIter();
								  theQueryInfo->Param(param);
								  ok = theQueryInfo->IsParamUsable();
								  if(ok) break;
								}
							}
						  
						  if(!ok)
							{
							  cerr << "Error: The parameter is not usable: " << name << endl;
							  exit(1);
							}
						  
						  if(verbose)
							{
							  cout << "Param " << name << " from queryfile number "
								   << (qi+1) << endl;
							}
						  
						  // Establish the contour method
						  
						  string interpname = piter->ContourInterpolation();
						  NFmiContourTree::NFmiContourInterpolation interp
							= NFmiContourTree::ContourInterpolationValue(interpname);
						  if(interp==NFmiContourTree::kFmiContourMissingInterpolation)
							{
							  cerr << "Error: Unknown contour interpolation method " << interpname << endl;
							  exit(1);
							}
						  
						  // Get the values. 
						  
						  if(!ismeta)
							theQueryInfo->Values(vals);
						  else
							meta_values(piter->Param(), theQueryInfo, vals);
						  
						  // Replace values if so requested
						  
						  if(piter->Replace())
							vals.Replace(piter->ReplaceSourceValue(),piter->ReplaceTargetValue());
						  
						  if(theFilter=="none")
							{
							  // The time is known to be exact
							}
						  else if(theFilter=="linear")
							{
							  NFmiTime utc = theQueryInfo->ValidTime();
							  NFmiTime tnow = NFmiMetTime(utc,1).CorrectLocalTime();
							  bool isexact = t.IsEqual(tnow);
							  
							  if(!isexact)
								{
								  NFmiDataMatrix<float> tmpvals;
								  NFmiTime t2utc = theQueryInfo->ValidTime();
								  NFmiTime t2 = NFmiMetTime(t2utc,1).CorrectLocalTime();
								  theQueryInfo->PreviousTime();
								  NFmiTime t1utc = theQueryInfo->ValidTime();
								  NFmiTime t1 = NFmiMetTime(t1utc,1).CorrectLocalTime();
								  if(!ismeta)
									theQueryInfo->Values(tmpvals);
								  else
									meta_values(piter->Param(), theQueryInfo, tmpvals);
								  if(piter->Replace())
									tmpvals.Replace(piter->ReplaceSourceValue(),
													piter->ReplaceTargetValue());
								  
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
							  tprev.ChangeByMinutes(-theTimeInterval);
							  
							  NFmiDataMatrix<float> tmpvals;
							  int steps = 1;
							  while(true)
								{
								  theQueryInfo->PreviousTime();
								  NFmiTime utc = theQueryInfo->ValidTime();
								  NFmiTime tnow = NFmiMetTime(utc,1).CorrectLocalTime();
								  if(tnow.IsLessThan(tprev))
									break;
								  
								  steps++;
								  if(!ismeta)
									theQueryInfo->Values(tmpvals);
								  else
									meta_values(piter->Param(), theQueryInfo, tmpvals);
								  if(piter->Replace())
									tmpvals.Replace(piter->ReplaceSourceValue(),
													piter->ReplaceTargetValue());
								  
								  if(theFilter=="min")
									vals.Min(tmpvals);
								  else if(theFilter=="max")
									vals.Max(tmpvals);
								  else if(theFilter=="mean")
									vals += tmpvals;
								  else if(theFilter=="sum")
									vals += tmpvals;
								}
							  
							  if(theFilter=="mean")
								vals /= steps;
							}
						  
						  
						  // Smoothen the values
						  
						  NFmiSmoother smoother(piter->Smoother(),
												piter->SmootherFactor(),
												piter->SmootherRadius());
						  
						  vals = smoother.Smoothen(worldpts[qi],vals);
						  
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
						  
						  if(verbose)
							cout << "Data range for " << name << " is " << valmin << "," << valmax << endl;
						  
						  // Save the data values at desired points for later
						  // use, this lets us avoid using InterpolatedValue()
						  // which does not use smoothened values.

						  // First, however, if this is the first image, we add
						  // the grid points to the set of points, if so requested

						  if(piter->LabelDX() > 0 && piter->LabelDY() > 0)
							{
							  for(unsigned int j=0; j<pts[qi].NY(); j+=piter->LabelDY())
								for(unsigned int i=0; i<pts[qi].NX(); i+=piter->LabelDX())
								  piter->Add(area.WorldXYToLatLon(worldpts[qi][i][j]));
							}

						  piter->ClearLabelValues();
						  if((piter->LabelFormat() != "") &&
							 !piter->LabelPoints().empty() )
							{
							  list<pair<NFmiPoint,NFmiPoint> >::const_iterator iter;
							  
							  for(iter=piter->LabelPoints().begin();
								  iter!=piter->LabelPoints().end();
								  ++iter)
								{
								  NFmiPoint latlon = iter->first;
								  NFmiPoint ij = theQueryInfo->Grid()->LatLonToGrid(latlon);
								  
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
									  if(!theQueryInfo->BiLinearInterpolation(ij.X(),
																			  ij.Y(),
																			  value,
																			  v00,v10,
																			  v01,v11))
										value = kFloatMissing;

									}
								  piter->AddLabelValue(value);
								}
							}
					  
						  // Fill the contours
						  
						  list<ContourRange>::const_iterator citer;
						  list<ContourRange>::const_iterator cbegin;
						  list<ContourRange>::const_iterator cend;
						  
						  cbegin = piter->ContourFills().begin();
						  cend   = piter->ContourFills().end();
						  
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
											  piter->ExactHiLimit()!=kFloatMissing &&
											  citer->hilimit()==piter->ExactHiLimit());
							  NFmiContourTree tree(citer->lolimit(),
												   citer->hilimit(),
												   exactlo,exacthi);
							  
							  if(piter->DataLoLimit()!=kFloatMissing)
								tree.DataLoLimit(piter->DataLoLimit());
							  if(piter->DataHiLimit()!=kFloatMissing)
								tree.DataHiLimit(piter->DataHiLimit());
							  
							  NFmiColorTools::NFmiBlendRule rule = NFmiColorTools::BlendValue(citer->rule());
							  
							  tree.Contour(pts[qi],vals,interp,piter->ContourDepth());
							  tree.Fill(theImage,citer->color(),rule);
							  
							}
						  
						  // Fill the contours with patterns
						  
						  list<ContourPattern>::const_iterator patiter;
						  list<ContourPattern>::const_iterator patbegin;
						  list<ContourPattern>::const_iterator patend;
						  
						  patbegin = piter->ContourPatterns().begin();
						  patend   = piter->ContourPatterns().end();
						  
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
											  piter->ExactHiLimit()!=kFloatMissing &&
											  patiter->hilimit()==piter->ExactHiLimit());
							  NFmiContourTree tree(patiter->lolimit(),
												   patiter->hilimit(),
												   exactlo,exacthi);
							  
							  if(piter->DataLoLimit()!=kFloatMissing)
								tree.DataLoLimit(piter->DataLoLimit());
							  if(piter->DataHiLimit()!=kFloatMissing)
								tree.DataHiLimit(piter->DataHiLimit());
							  
							  NFmiColorTools::NFmiBlendRule rule = NFmiColorTools::BlendValue(patiter->rule());
							  
							  tree.Contour(pts[qi],vals,interp,piter->ContourDepth());
							  NFmiImage pattern(patiter->pattern());
							  
							  tree.Fill(theImage,pattern,rule,patiter->factor());
							  
							}
						  
						  // Stroke the contours
						  
						  list<ContourValue>::const_iterator liter;
						  list<ContourValue>::const_iterator lbegin;
						  list<ContourValue>::const_iterator lend;
						  
						  lbegin = piter->ContourValues().begin();
						  lend   = piter->ContourValues().end();
						  
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
							  
							  NFmiContourTree tree(liter->value(),kFloatMissing);
							  if(piter->DataLoLimit()!=kFloatMissing)
								tree.DataLoLimit(piter->DataLoLimit());
							  if(piter->DataHiLimit()!=kFloatMissing)
								tree.DataHiLimit(piter->DataHiLimit());
							  
							  NFmiColorTools::NFmiBlendRule rule = NFmiColorTools::BlendValue(liter->rule());
							  tree.Contour(pts[qi],vals,interp,piter->ContourDepth());
							  NFmiPath path = tree.Path();
							  path.SimplifyLines(10);
							  path.Stroke(theImage,liter->color(),rule);
							  
							  
							}
						  
						}
					  
					  
					  
					  // Bang the foreground
					  
					  if(theForeground != "")
						{
						  NFmiColorTools::NFmiBlendRule rule = NFmiColorTools::BlendValue(theForegroundRule);
						  
						  theImage.Composite(theForegroundImage,rule,kFmiAlignNorthWest,0,0,1);
						  
						}

					  // Draw wind arrows if so requested
					  
					  NFmiEnumConverter converter;
					  if((!theArrowPoints.empty() || (theWindArrowDX!=0 && theWindArrowDY!=0)) &&
						 (theArrowFile!=""))
						{
						  
						  FmiParameterName param = FmiParameterName(NFmiEnumConverter().ToEnum(theDirectionParameter));
						  if(param==kFmiBadParameter)
							{
							  cerr << "Error: Unknown parameter " << theDirectionParameter << endl;
							  return 1;
							}
						  
						  // Find the proper queryinfo to be used
						  // Note that qi will be used later on for
						  // getting the coordinate matrices
						  
						  ok = false;
						  for(qi=0; qi<theQueryStreams.size(); qi++)
							{
							  theQueryInfo = theQueryStreams[qi]->QueryInfoIter();
							  theQueryInfo->Param(param);
							  ok = theQueryInfo->IsParamUsable();
							  if(ok) break;
							}
						  
						  if(!ok)
							{
							  cerr << "Error: The parameter is not usable: " << theDirectionParameter << endl;
							  exit(1);
							}

						  // Read the arrow definition
						  
						  NFmiPath arrowpath;
						  if(theArrowFile != "meteorological")
							{
							  ifstream arrow(theArrowFile.c_str());
							  if(!arrow)
								{
								  cerr << "Error: Could not open " << theArrowFile << endl;
								  exit(1);
								}
							  // Read in the entire file
							  string pathstring = StringTools::readfile(arrow);
							  arrow.close();

							  // Convert to a path
							  
							  arrowpath.Add(pathstring);
							}
						  
						  // Handle all given coordinates
						  
						  list<NFmiPoint>::const_iterator iter;
						  
						  for(iter=theArrowPoints.begin();
							  iter!=theArrowPoints.end();
							  ++iter)
							{

							  // The start point
							  NFmiPoint xy0 = theArea.ToXY(*iter);

							  // Skip rendering if the start point is masked

							  if(IsMasked(xy0,theMask,theMaskImage))
								continue;

							  float dir = theQueryInfo->InterpolatedValue(*iter);
							  if(dir==kFloatMissing)	// ignore missing
								continue;
							  
							  float speed = -1;
							  
							  if(theQueryInfo->Param(FmiParameterName(converter.ToEnum(theSpeedParameter))))
								speed = theQueryInfo->InterpolatedValue(*iter);
							  theQueryInfo->Param(FmiParameterName(converter.ToEnum(theDirectionParameter)));
							  
						  
							  // Direction calculations
							  
							  const float pi = 3.141592658979323;
							  const float length = 0.1;	// degrees
							  
							  float x1 = iter->X()+sin(dir*pi/180)*length;
							  float y1 = iter->Y()+cos(dir*pi/180)*length;
							  
							  NFmiPoint xy1 = theArea.ToXY(NFmiPoint(x1,y1));
							  
							  // Calculate the actual angle
							  
							  float alpha = atan2(xy1.X()-xy0.X(),
												  xy1.Y()-xy0.Y());
							  
							  // Create a new path
							  
							  NFmiPath thispath;

							  if(theArrowFile == "meteorological")
								thispath.Add(MetArrow(speed*theWindArrowScaleC));
							  else
								thispath.Add(arrowpath);

							  if(speed>0 && speed!=kFloatMissing)
								thispath.Scale(theWindArrowScaleA*log10(theWindArrowScaleB*speed+1)+theWindArrowScaleC);
							  thispath.Scale(theArrowScale);
							  thispath.Rotate(alpha*180/pi);
							  thispath.Translate(xy0.X(),xy0.Y());
							  
							  // And render it
							  
							  thispath.Fill(theImage,
											ToColor(theArrowFillColor),
											NFmiColorTools::BlendValue(theArrowFillRule));
							  thispath.Stroke(theImage,
											  ToColor(theArrowStrokeColor),
											  NFmiColorTools::BlendValue(theArrowStrokeRule));
							}
						  
						  // Draw the full grid if so desired
						  
						  if(theWindArrowDX!=0 && theWindArrowDY!=0)
							{
							  NFmiDataMatrix<NFmiPoint> latlons;
							  theQueryInfo->Locations(latlons);

							  NFmiDataMatrix<float> speedvalues(vals.NX(),vals.NY(),-1);
							  if(theQueryInfo->Param(FmiParameterName(converter.ToEnum(theSpeedParameter))))
								theQueryInfo->Values(speedvalues);
							  theQueryInfo->Param(FmiParameterName(converter.ToEnum(theDirectionParameter)));
							  
							  for(unsigned int j=0; j<pts[qi].NY(); j+=theWindArrowDY)
								for(unsigned int i=0; i<pts[qi].NX(); i+=theWindArrowDX)
								  {
									// The start point
									
									NFmiPoint xy0 = theArea.ToXY(latlons[i][j]);

									// Skip rendering if the start point is masked

									if(IsMasked(xy0,theMask,theMaskImage))
									  continue;

									float dir = vals[i][j];
									if(dir==kFloatMissing)	// ignore missing
									  continue;
									
									float speed = speedvalues[i][j];

									// Direction calculations
									
									const float pi = 3.141592658979323;
									const float length = 0.1;	// degrees
									
									float x0 = latlons[i][j].X();
									float y0 = latlons[i][j].Y();
									
									float x1 = x0+sin(dir*pi/180)*length;
									float y1 = y0+cos(dir*pi/180)*length;
									
									NFmiPoint xy1 = theArea.ToXY(NFmiPoint(x1,y1));
									
									// Calculate the actual angle
									
									float alpha = atan2(xy1.X()-xy0.X(),
														xy1.Y()-xy0.Y());
									
									// Create a new path
									
									NFmiPath thispath;
									if(theArrowFile == "meteorological")
									  thispath.Add(MetArrow(speed*theWindArrowScaleC));
									else
									  thispath.Add(arrowpath);
									if(speed>0 && speed != kFloatMissing)
									  thispath.Scale(theWindArrowScaleA*log10(theWindArrowScaleB*speed+1)+theWindArrowScaleC);
									thispath.Scale(theArrowScale);
									thispath.Rotate(alpha*180/pi);
									thispath.Translate(xy0.X(),xy0.Y());
									
									// And render it
									
									thispath.Fill(theImage,
												  ToColor(theArrowFillColor),
												  NFmiColorTools::BlendValue(theArrowFillRule));
									thispath.Stroke(theImage,
													ToColor(theArrowStrokeColor),
													NFmiColorTools::BlendValue(theArrowStrokeRule));
								  }
							}
						}
					  
					  // Draw labels
					  
					  for(piter=pbegin; piter!=pend; ++piter)
						{
						  
						  // Draw label markers first
						  
						  if(!piter->LabelMarker().empty())
							{
							  // Establish that something is to be done
							  
							  if(piter->LabelPoints().empty() &&
								 !(piter->LabelDX()==0 || piter->LabelDX()==0))
								continue;
							  
							  // Establish the marker specs
							  
							  NFmiImage marker;
							  marker.Read(piter->LabelMarker());
							  
							  NFmiColorTools::NFmiBlendRule markerrule = NFmiColorTools::BlendValue(piter->LabelMarkerRule());
							  
							  float markeralpha = piter->LabelMarkerAlphaFactor();
							  
							  // Draw individual points
							  
							  if(!piter->LabelPoints().empty())
								{
								  unsigned int pointnumber = 0;
								  list<pair<NFmiPoint,NFmiPoint> >::const_iterator iter;
								  for(iter=piter->LabelPoints().begin();
									  iter!=piter->LabelPoints().end();
									  ++iter)
									{
									  // The point in question
									  
									  NFmiPoint xy = theArea.ToXY(iter->first);
									  
									  // Skip rendering if the start point is masked
									  
									  if(IsMasked(xy,theMask,theMaskImage))
										continue;

                                      // Skip rendering if LabelMissing is "" and value is missing
                                      if(piter->LabelMissing().empty())
                                        {
                                          float value = piter->LabelValues()[pointnumber++];
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
							  
							}
						  
						  // Label markers now drawn, only label texts remain
						  
						  // Quick exit from loop if no labels are
						  // desired for this parameter
						  
						  if(piter->LabelPoints().empty() &&
							 !(piter->LabelDX()!=0 && piter->LabelDY()!=0))
							continue;
						  
						  if(piter->LabelFormat() == "")
							continue;

						  // Draw markers if so requested
						  
						  
						  // Create the font object to be used
						  
						  NFmiFontHershey font(piter->LabelFont());
							  
						  // Create the text object to be used
						  
						  NFmiText text("",
										font,
										piter->LabelSize(),
										0.0,	// x
										0.0,	// y
										AlignmentValue(piter->LabelAlignment()),
										piter->LabelAngle());
						  
						  
						  NFmiText caption(piter->LabelCaption(),
										   font,
										   piter->LabelSize(),
										   0.0,
										   0.0,
										   AlignmentValue(piter->LabelCaptionAlignment()),
										   piter->LabelAngle());
						  
						  // The rules
						  
						  NFmiColorTools::NFmiBlendRule fillrule
							= NFmiColorTools::BlendValue(piter->LabelFillRule());
						  
						  NFmiColorTools::NFmiBlendRule strokerule
							= NFmiColorTools::BlendValue(piter->LabelStrokeRule());
						  
						  // Draw labels at specifing latlon points if requested
						  
						  list<pair<NFmiPoint,NFmiPoint> >::const_iterator iter;
						  
						  int pointnumber = 0;
						  for(iter=piter->LabelPoints().begin();
							  iter!=piter->LabelPoints().end();
							  ++iter)
							{

							  // The point in question
							  
							  float x,y;
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

							  // Skip rendering if the start point is masked
							  
							  if(IsMasked(NFmiPoint(x,y),theMask,theMaskImage))
								continue;

							  float value = piter->LabelValues()[pointnumber++];
							  
							  // Convert value to string
							  string strvalue = piter->LabelMissing();
							  
							  if(value!=kFloatMissing)
								{
								  char tmp[20];
								  sprintf(tmp,piter->LabelFormat().c_str(),value);
								  strvalue = tmp;
								}

							  // Don't bother drawing empty strings
							  if(strvalue.empty())
								continue;
							  
							  // Set new text properties
							  
							  text.Text(strvalue);
							  text.X(x + piter->LabelOffsetX());
							  text.Y(y + piter->LabelOffsetY());
							  
							  // And render the text
							  
							  text.Fill(theImage,piter->LabelFillColor(),fillrule);
							  text.Stroke(theImage,piter->LabelStrokeColor(),strokerule);
							  
							  // Then the label caption
							  
							  if(!piter->LabelCaption().empty())
								{
								  caption.X(text.X() + piter->LabelCaptionDX());
								  caption.Y(text.Y() + piter->LabelCaptionDY());
								  caption.Fill(theImage,piter->LabelFillColor(),fillrule);
								  caption.Stroke(theImage,piter->LabelStrokeColor(),strokerule);
								}
							  
							}
							  
						}
		  
  
					  
					  // Bang the combine image (legend, logo, whatever)
					  
					  if(theCombine != "")
						{
						  NFmiColorTools::NFmiBlendRule rule = NFmiColorTools::BlendValue(theCombineRule);
						  
						  theImage.Composite(theCombineImage,rule,kFmiAlignNorthWest,theCombineX,theCombineY,theCombineFactor);
						  
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
						for(qi=0; qi<theQueryStreams.size(); qi++)
						  {
							theQueryInfo = theQueryStreams[qi]->QueryInfoIter();
							NFmiTime futctime = theQueryInfo->OriginTime(); 
							NFmiTime tlocal = NFmiMetTime(futctime,1).CorrectLocalTime();
							if(qi==0 || tlocal.IsLessThan(tfor))
							  tfor = tlocal;
						  }
						
						int foryy = tfor.GetYear();
						int formm = tfor.GetMonth();
						int fordd = tfor.GetDay();
						int forhh = tfor.GetHour();
						int formi = tfor.GetMin();
						
						char buffer[100];
						
						if(theTimeStampImage == "obs")
						  {
							// hh:mi dd.mm.yyyy
							sprintf(buffer,"%02d:%02d %02d.%02d.%04d",
									obshh,obsmi,obsdd,obsmm,obsyy);
							thestamp = buffer;
						  }
						else if(theTimeStampImage == "for")
						  {
							// hh:mi dd.mm.yyyy
							sprintf(buffer,"%02d:%02d %02d.%02d.%04d",
									forhh,formi,fordd,formm,foryy);
							thestamp = buffer;
						  }
						else if(theTimeStampImage == "forobs")
						  {
							// hh:mi dd.mm.yyyy +hh
							long diff = t.DifferenceInMinutes(tfor);
							if(diff%60==0 && theTimeStep%60==0)
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
						  
						  int x = theTimeStampImageX;
						  int y = theTimeStampImageY;
						  
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
					  
					  // Save
					  
					  if(verbose)
						cout << "Writing " << filename << endl;
					  if(theFormat=="png")
						theImage.WritePng(filename);
					  else if(theFormat=="jpg" || theFormat=="jpeg")
						theImage.WriteJpeg(filename);
					  else if(theFormat=="gif")
						theImage.WriteGif(filename);
					  else
						{
						  cerr << "Error: Image format " << theFormat << " is not supported" << endl;
						  return 1;
						}
					}
				}
			  
			  else
				{
				  cerr << "Error: draw " << command << " not implemented\n";
				  return 1;
				}
			}
		  else
			{
			  cerr << "Error: Unknown command " << command << endl;
			  return 1;
			}
		}
    }
}

// ======================================================================

