// ======================================================================
//
// qdcontour
//
// A program to draw the desired querydata grids as contour fields.
//
// -v = verbose mode
// -f = force contour image output even if same file already exists
//
// Logiikka aikaleimojen valintaan:
// --------------------------------
//
// Querydatan aikarajat t1, t2, luonnollinen aika-askel DT
// Haluttu skip tskip, voi olla 0, jolloin tarkoitetaan dt = DT
// Haluttu kuvam‰‰r‰ n
// Haluttu askel dt, voi olla 0
//
// t = t1 + tskip
// jos t % dt <> 0
//    t = t + dt - (t % dt )   eli seuraava dt monikerta
// for (i=0; i<n; i++)
//    laske hila ajalle t k‰ytt‰en asetettua funktiota
//    piirr‰ aika t
//    t = t + dt
// 
// Funktiot:
//
// Kun dt < DT, tulee k‰ytt‰‰ interpolointifunktiota, esim. linear
// Kun dt = DT, on funktio merkityksetˆn
// Kun dt > DT, tulee k‰yt‰‰ interpolointifunktiota tai aika-askeleesta
//              dt riippuvaista funktiota (max,min,mean,sum jne)
//
// ======================================================================

#include "NFmiColorTools.h"
#include "NFmiSmoother.h"		// for smoothing data
#include "NFmiContourTree.h"	// for contouring
#include "NFmiImage.h"			// for rendering
#include "NFmiGeoShape.h"		// for esri data
#include "NFmiText.h"			// for labels
#include "NFmiFontHershey.h"	// for Hershey fonts

#include "NFmiCmdLine.h"			// command line options
#include "NFmiDataModifierClasses.h"
#include "NFmiEnumConverter.h"		// FmiParameterName<-->string
#include "NFmiFileSystem.h"			// FileExists()
#include "NFmiLatLonArea.h"			// Geographic projection
#include "NFmiSettings.h"			// Configuration
#include "NFmiStereographicArea.h"	// Stereographic projection
#include "NFmiStreamQueryData.h"
#include "NFmiPreProcessor.h"

#include <fstream>
#include <string>
#include <list>
#ifdef OLDGCC
  #include <strstream>
#else
  #include <sstream>
#endif

// ----------------------------------------------------------------------
// Usage
// ----------------------------------------------------------------------

void Usage(void)
{
  cout << "Usage: qdcontour [conffiles]" << endl << endl;
  cout << "Commands in configuration files:" << endl << endl;
}

// ----------------------------------------------------------------------
// Read a file into the input string
// ----------------------------------------------------------------------

void StringReader(std::istream & is, string & theString)
{
  theString.resize(0);

  const int bufsize = 1024;
  char buffer[bufsize];
  while(!is.eof() && !is.fail())
    {
      is.read(buffer,bufsize);
      theString.append(buffer,is.gcount());
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
// Yksitt‰isen contour-arvon s‰ilytysluokka
// ----------------------------------------------------------------------

class ContourValue
{
public:
  
  ContourValue(float value, int color, string rule="Atop")
    : itsValue(value)
    , itsColor(color)
    , itsRule(rule)
  {}
  
  float Value(void) const { return itsValue; }
  int Color(void) const { return itsColor; }
  const string & Rule(void) const { return itsRule; }
private:
  ContourValue(void);
  float itsValue;
  int itsColor;
  string itsRule;
};

// ----------------------------------------------------------------------
// Yksitt‰isen contour-intervallin s‰ilytysluokka
// ----------------------------------------------------------------------

class ContourRange
{
public:
  
  ContourRange(float lolimit,float hilimit,int color, string rule="Atop")
    : itsLoLimit(lolimit)
    , itsHiLimit(hilimit)
    , itsColor(color)
    , itsRule(rule)
  {}
  float LoLimit(void) const { return itsLoLimit; }
  float HiLimit(void) const { return itsHiLimit; }
  int   Color(void)   const { return itsColor; }
  const string & Rule(void) const { return itsRule; }
  
private:
  ContourRange(void);
  float itsLoLimit;
  float itsHiLimit;
  int   itsColor;
  string itsRule;
};

// ----------------------------------------------------------------------
// Yksitt‰isen contour-patternin s‰ilytysluokka
// ----------------------------------------------------------------------

class ContourPattern
{
public:
  
  ContourPattern(float lolimit,float hilimit,
				 const string & thePattern,
				 const string & theRule,
				 float theFactor=1.0)
    : itsLoLimit(lolimit)
    , itsHiLimit(hilimit)
    , itsPattern(thePattern)
    , itsRule(theRule)
    , itsFactor(theFactor)
  {}
  float LoLimit(void) const		{ return itsLoLimit; }
  float HiLimit(void) const		{ return itsHiLimit; }
  const string & Pattern(void) const	{ return itsPattern; }
  const string & Rule(void) const	{ return itsRule; }
  float Factor(void) const		{ return itsFactor; }
  
private:
  ContourPattern(void);
  float itsLoLimit;
  float itsHiLimit;
  string itsPattern;
  string itsRule;
  float itsFactor;
};

// ----------------------------------------------------------------------
// Yksitt‰isen parametrin piirto-ohjeet
// ----------------------------------------------------------------------

class ContourSpec
{
public:
  ContourSpec(const string & param,
			  const string & interpolation,
			  const string & smoother,
			  int depth=0,
			  float smootherradius=1.0,
			  int smootherfactor=1,
			  float hilimit=kFloatMissing)
    : itsParam(param)
    , itsContourInterpolation(interpolation)
    , itsSmoother(smoother)
    , itsSmootherRadius(smootherradius)
    , itsSmootherFactor(smootherfactor)
    , itsExactHiLimit(hilimit)
    , itsContourDepth(depth)
    , itsDataLoLimit(kFloatMissing)
    , itsDataHiLimit(kFloatMissing)
    , itHasReplace(false)
    , itsLabelMarker("")
    , itsLabelMarkerRule("Copy")
    , itsLabelMarkerAlphaFactor(1.0)
    , itsLabelFont("TimesRoman")
    , itsLabelSize(12)
    , itsLabelStrokeColor(NFmiColorTools::NoColor)
    , itsLabelStrokeRule("Copy")
    , itsLabelFillColor(NFmiColorTools::NoColor)
    , itsLabelFillRule("Copy")
    , itsLabelAlignment("Center")
    , itsLabelFormat("%.1f")
    , itsLabelAngle(0)
    , itsLabelOffsetX(0)
    , itsLabelOffsetY(0)
    , itsLabelDX(0)
    , itsLabelDY(0)
    , itsLabelCaption("")
    , itsLabelCaptionDX(0)
    , itsLabelCaptionDY(0)
    , itsLabelCaptionAlignment("West")
  {}
  
  const list<ContourRange> & ContourFills(void) const { return itsContourFills; }
  const list<ContourPattern> & ContourPatterns(void) const { return itsContourPatterns; }
  const list<ContourValue> & ContourValues(void) const { return itsContourValues; }
  
  const string & Param(void) const		{ return itsParam; }
  const string & ContourInterpolation(void) const	{ return itsContourInterpolation; }
  const string & Smoother(void) const		{ return itsSmoother; }
  float SmootherRadius(void) const		{ return itsSmootherRadius; }
  int SmootherFactor(void) const		{ return itsSmootherFactor; }
  float ExactHiLimit(void) const		{ return itsExactHiLimit; }
  int ContourDepth(void) const			{ return itsContourDepth; }
  float DataHiLimit(void) const			{ return itsDataHiLimit; }
  float DataLoLimit(void) const			{ return itsDataLoLimit; }
  
  void ContourInterpolation(const string & val)	{ itsContourInterpolation = val; }
  void Smoother(const string & val)		{ itsSmoother = val; }
  void SmootherRadius(float radius)		{ itsSmootherRadius = radius; }
  void SmootherFactor(int factor)		{ itsSmootherFactor = factor; }
  void ExactHiLimit(float limit)		{ itsExactHiLimit = limit; }
  void ContourDepth(int depth)			{ itsContourDepth = depth; }
  
  void DataLoLimit(float limit)			{ itsDataLoLimit = limit; }
  void DataHiLimit(float limit)			{ itsDataHiLimit = limit; }
  
  void Add(ContourRange range) { itsContourFills.push_back(range); }
  void Add(ContourValue value) { itsContourValues.push_back(value); }
  void Add(ContourPattern value) { itsContourPatterns.push_back(value); }
  
  // This was done to replace 32700 with -1 in PrecipitationForm
  
  bool Replace(void) const		{ return itHasReplace; }
  float ReplaceSourceValue(void) const	{ return itsReplaceSourceValue; }
  float ReplaceTargetValue(void) const	{ return itsReplaceTargetValue; }
  
  void Replace(float src, float dst)
  {
    itHasReplace = true;
    itsReplaceSourceValue = src;
    itsReplaceTargetValue = dst;
  }
  
  // Label specific methods
  
  const list<pair<NFmiPoint,NFmiPoint> > & LabelPoints(void) const { return itsLabelPoints; }
  
  void Add(const NFmiPoint & point,
		   const NFmiPoint xy = NFmiPoint(kFloatMissing,kFloatMissing))
  { itsLabelPoints.push_back(make_pair(point,xy)); }
  
  const vector<float> & LabelValues(void) const
  { return itsLabelValues; }
  
  void AddLabelValue(float value)
  { itsLabelValues.push_back(value); }
  
  void ClearLabelValues(void)
  {
    itsLabelValues.clear();
  }
  
  void ClearLabels(void)
  {
    itsLabelPoints.clear();
    itsLabelValues.clear();
  }
  
  const string & LabelMarker(void) const	{ return itsLabelMarker; }
  const string & LabelMarkerRule(void) const 	{ return itsLabelMarkerRule; }
  float LabelMarkerAlphaFactor(void) const 	{ return itsLabelMarkerAlphaFactor; }
  const string & LabelFont(void) const		{ return itsLabelFont; }
  float LabelSize(void) const			{ return itsLabelSize; }
  int LabelStrokeColor(void) const		{ return itsLabelStrokeColor; }
  const string & LabelStrokeRule(void) const	{ return itsLabelStrokeRule; }
  int LabelFillColor(void) const		{ return itsLabelFillColor; }
  const string & LabelFillRule(void) const	{ return itsLabelFillRule; }
  const string & LabelAlignment(void) const	{ return itsLabelAlignment; }
  const string & LabelFormat(void) const	{ return itsLabelFormat; }
  float LabelAngle(void) const			{ return itsLabelAngle; }
  float LabelOffsetX(void) const		{ return itsLabelOffsetX; }
  float LabelOffsetY(void) const		{ return itsLabelOffsetY; }
  int LabelDX(void) const			{ return itsLabelDX; }
  int LabelDY(void) const			{ return itsLabelDY; }
  
  string LabelCaption(void) const		{ return itsLabelCaption; }
  float LabelCaptionDX(void) const		{ return itsLabelCaptionDX; }
  float LabelCaptionDY(void) const		{ return itsLabelCaptionDY; }
  string LabelCaptionAlignment(void) const	{ return itsLabelCaptionAlignment; }
  
  void LabelMarker(const string & value)	{ itsLabelMarker = value; }
  void LabelMarkerRule(const string & value)	{ itsLabelMarkerRule = value; }
  void LabelMarkerAlphaFactor(float value) 	{ itsLabelMarkerAlphaFactor = value; }
  void LabelFont(const string & value)		{ itsLabelFont = value; }
  void LabelSize(float value)			{ itsLabelSize = value; }
  void LabelStrokeColor(int value)		{ itsLabelStrokeColor = value; }
  void LabelStrokeRule(const string & value)	{ itsLabelStrokeRule = value; }
  void LabelFillColor(int value)		{ itsLabelFillColor = value; }
  void LabelFillRule(const string & value)	{ itsLabelFillRule = value; }
  void LabelAlignment(const string & value)	{ itsLabelAlignment = value; }
  void LabelFormat(const string & value)	{ itsLabelFormat = value; }
  void LabelAngle(float value)			{ itsLabelAngle = value; }
  void LabelOffsetX(float value)		{ itsLabelOffsetX = value; }
  void LabelOffsetY(float value)		{ itsLabelOffsetY = value; }
  void LabelDX(int value)			{ itsLabelDX = value; }
  void LabelDY(int value)			{ itsLabelDY = value; }
  
  void LabelCaption(const string & value)	{ itsLabelCaption = value; }
  void LabelCaptionDX(float value)		{ itsLabelCaptionDX = value; }
  void LabelCaptionDY(float value)		{ itsLabelCaptionDY = value; }
  void LabelCaptionAlignment(const string & value) { itsLabelCaptionAlignment = value; }
  
private:
  ContourSpec(void);
  string itsParam;
  string itsContourInterpolation;
  string itsSmoother;
  float itsSmootherRadius;
  int itsSmootherFactor;
  
  list<ContourRange> itsContourFills;
  list<ContourValue> itsContourValues;
  list<ContourPattern> itsContourPatterns;
  
  float itsExactHiLimit;
  int itsContourDepth;
  float itsDataLoLimit;
  float itsDataHiLimit;
  
  bool itHasReplace;;
  float itsReplaceSourceValue;
  float itsReplaceTargetValue;
  
  // LatLon, optional label location pairs in pixels
  list<pair<NFmiPoint,NFmiPoint> > itsLabelPoints;
  // Respective values calculated while contouring
  vector<float> itsLabelValues;
  
  string itsLabelMarker;
  string itsLabelMarkerRule;
  float itsLabelMarkerAlphaFactor;
  string itsLabelFont;
  float itsLabelSize;
  int itsLabelStrokeColor;
  string itsLabelStrokeRule;
  int itsLabelFillColor;
  string itsLabelFillRule;
  string itsLabelAlignment;
  string itsLabelFormat;
  float itsLabelAngle;
  float itsLabelOffsetX;
  float itsLabelOffsetY;
  int itsLabelDX;
  int itsLabelDY;
  
  string itsLabelCaption;
  float itsLabelCaptionDX;
  float itsLabelCaptionDY;
  string itsLabelCaptionAlignment;
  
};

// ----------------------------------------------------------------------
// Yksitt‰isen shapen piirto-ohjeet
// ----------------------------------------------------------------------

class ShapeSpec
{
public:
  ShapeSpec(const string & shapefile,
			NFmiColorTools::Color fill = NFmiColorTools::MakeColor(0,0,0,127),
			NFmiColorTools::Color stroke = NFmiColorTools::MakeColor(0,0,0,127),
			const string & fillrule = "Copy",
			const string & strokerule = "Copy")
    : itsShapeFileName(shapefile)
    , itsFillRule(fillrule)
    , itsStrokeRule(strokerule)
    , itsFillColor(fill)
    , itsStrokeColor(stroke)
    , itsMarker("")
    , itsMarkerRule("Over")
    , itsMarkerAlpha(1.0)
  {}
  
  // Data-access
  
  const string & FileName(void) const		{ return itsShapeFileName; }
  const string & FillRule(void) const		{ return itsFillRule; }
  const string & StrokeRule(void) const		{ return itsStrokeRule; }
  NFmiColorTools::Color FillColor(void) const	{ return itsFillColor; }
  NFmiColorTools::Color StrokeColor(void) const	{ return itsStrokeColor; }
  
  void FillRule(const string & rule)		{ itsFillRule = rule; }
  void StrokeRule(const string & rule)		{ itsStrokeRule = rule; }
  void FillColor(NFmiColorTools::Color color)	{ itsFillColor = color; }
  void StrokeColor(NFmiColorTools::Color color)	{ itsStrokeColor = color; }
  
  void Marker(const string & marker, const string & rule, float alpha)
  {
    itsMarker = marker;
    itsMarkerRule = rule;
    itsMarkerAlpha = alpha;
  }
  
  const string & Marker(void) const		{ return itsMarker; }
  const string & MarkerRule(void) const		{ return itsMarkerRule; }
  float MarkerAlpha(void) const			{ return itsMarkerAlpha; }
private:
  ShapeSpec(void);
  string	itsShapeFileName;
  string	itsFillRule;
  string	itsStrokeRule;
  NFmiColorTools::Color	itsFillColor;
  NFmiColorTools::Color	itsStrokeColor;
  string	itsMarker;
  string	itsMarkerRule;
  float		itsMarkerAlpha;
  
};

// ----------------------------------------------------------------------
// Main program.
// ----------------------------------------------------------------------

int main(int argc, char *argv[])
{
  // Ymp‰ristˆn konfigurointi

  string datapath = NFmiSettings::instance().value("qdcontour::querydata_path",".");

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
  string theFillRule	= "Atop";
  string theStrokeRule	= "Atop";
  
  string theForegroundRule = "Over";
  
  string theFilter = "none";
  
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
  
  // This holds a vector of querydatastreams
  
  vector<NFmiStreamQueryData *> theQueryStreams;
  string theQueryStreamNames = "";
  
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
						  if(!tmp->ReadLatestData(filename))
							exit(1);
						  theQueryStreams.push_back(tmp);
						}
					}
				}
			}
		  
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
				theShapeSpecs.back().FillRule(theFillRule);
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
				theShapeSpecs.back().StrokeRule(theStrokeRule);
			}
		  
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
			  theBackgroundImage.Read(theBackground);
			}
		  
		  else if(command == "foreground")
			{
			  input >> theForeground;
			  theForegroundImage.Read(theForeground);
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
			input >> theSavePath;
		  
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
				  spec.Marker(marker,markerrule,markeralpha);
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
				  // if(verbose)
				  // cout << "Value " << tmplo
				  // << " colour is "
				  // << NFmiColorTools::GetRed(color) << ","
				  // << NFmiColorTools::GetGreen(color) << ","
				  // << NFmiColorTools::GetBlue(color) << ","
				  // << NFmiColorTools::GetAlpha(color)
				  // << endl;
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
						  float thelo = citer->LoLimit();
						  float thehi = citer->HiLimit();
						  NFmiColorTools::NFmiBlendRule rule = NFmiColorTools::BlendValue(citer->Rule());
						  
						  if(thelo==kFloatMissing) thelo=-1e6;
						  if(thehi==kFloatMissing) thehi= 1e6;
						  
						  NFmiPath path;
						  path.MoveTo(0,height*(1-(thelo-lolimit)/(hilimit-lolimit)));
						  path.LineTo(width,height*(1-(thelo-lolimit)/(hilimit-lolimit)));
						  path.LineTo(width,height*(1-(thehi-lolimit)/(hilimit-lolimit)));
						  path.LineTo(0,height*(1-(thehi-lolimit)/(hilimit-lolimit)));
						  path.CloseLineTo();
						  
						  path.Fill(legend,citer->Color(),rule);
						}
					  
					  list<ContourValue>::const_iterator liter;
					  list<ContourValue>::const_iterator lbegin;
					  list<ContourValue>::const_iterator lend;
					  
					  lbegin = theSpecs.back().ContourValues().begin();
					  lend   = theSpecs.back().ContourValues().end();
					  
					  for(liter=lbegin ; liter!=lend; ++liter)
						{
						  float thevalue = liter->Value();
						  
						  if(thevalue==kFloatMissing)
							continue;
						  
						  NFmiPath path;
						  path.MoveTo(0,height*(1-(thevalue-lolimit)/(hilimit-lolimit)));
						  path.LineTo(width,height*(1-(thevalue-lolimit)/(hilimit-lolimit)));
						  path.Stroke(legend,liter->Color());
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
					  NFmiGeoShape geo(iter->FileName(),kFmiGeoShapeEsri);
					  geo.ProjectXY(theArea);
					  
					  if(iter->Marker()=="")
						{
						  NFmiColorTools::NFmiBlendRule fillrule = NFmiColorTools::BlendValue(iter->FillRule());
						  NFmiColorTools::NFmiBlendRule strokerule = NFmiColorTools::BlendValue(iter->StrokeRule());
						  geo.Fill(theImage,iter->FillColor(),fillrule);
						  geo.Stroke(theImage,iter->StrokeColor(),strokerule);
						}
					  else
						{
						  NFmiColorTools::NFmiBlendRule markerrule = NFmiColorTools::BlendValue(iter->MarkerRule());
						  
						  NFmiImage marker;
						  marker.Read(iter->Marker());
						  geo.Mark(theImage,marker,markerrule,
								   kFmiAlignCenter,
								   iter->MarkerAlpha());
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
					  NFmiGeoShape geo(iter->FileName(),kFmiGeoShapeEsri);
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
					  
					  // Establish time limits
					  theQueryInfo->LastTime();
					  utctime = theQueryInfo->ValidTime();
					  NFmiTime t2 = NFmiMetTime(utctime,1).LocalTime();
					  theQueryInfo->FirstTime();
					  utctime = theQueryInfo->ValidTime();
					  NFmiTime t1 = NFmiMetTime(utctime,1).LocalTime();
					  
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
							  NFmiTime loc = NFmiMetTime(utc,1).LocalTime();
							  if(!loc.IsLessThan(t))
								break;
							}
						  NFmiTime utc = theQueryInfo->ValidTime();
						  NFmiTime tnow = NFmiMetTime(utc,1).LocalTime();
						  
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
						  for(qi=0; qi<theQueryStreams.size(); qi++)
							{
							  theQueryInfo = theQueryStreams[qi]->QueryInfoIter();
							  NFmiTime futctime = theQueryInfo->OriginTime(); 
							  NFmiTime tfor = NFmiMetTime(futctime,1).LocalTime();
							  filename += "_" + tfor.ToStr(kDDHHMM);
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
						  FmiParameterName param = FmiParameterName(NFmiEnumConverter().ToEnum(name));
						  
						  if(param==kFmiBadParameter)
							{
							  cerr << "Error: Unknown parameter " << name << endl;
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
						  
						  theQueryInfo->Values(vals);
						  
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
							  NFmiTime tnow = NFmiMetTime(utc,1).LocalTime();
							  bool isexact = t.IsEqual(tnow);
							  
							  if(!isexact)
								{
								  NFmiDataMatrix<float> tmpvals;
								  NFmiTime t2utc = theQueryInfo->ValidTime();
								  NFmiTime t2 = NFmiMetTime(t2utc,1).LocalTime();
								  theQueryInfo->PreviousTime();
								  NFmiTime t1utc = theQueryInfo->ValidTime();
								  NFmiTime t1 = NFmiMetTime(t1utc,1).LocalTime();
								  theQueryInfo->Values(tmpvals);
								  if(piter->Replace())
									tmpvals.Replace(piter->ReplaceSourceValue(),
													piter->ReplaceTargetValue());
								  
								  // Data from t1,t2, we want t
								  
								  long offset = t.DifferenceInMinutes(t1);
								  long range = t2.DifferenceInMinutes(t1);
								  
								  float weight = ((float) offset)/range;
								  
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
								  NFmiTime tnow = NFmiMetTime(utc,1).LocalTime();
								  if(tnow.IsLessThan(tprev))
									break;
								  
								  steps++;
								  theQueryInfo->Values(tmpvals);
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
						  
						  smoother.Smoothen(worldpts[qi],vals);
						  
						  // ofstream out("values.dat");
						  // out << vals;
						  // out.close();
						  
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
						  // use, this lets ups avoid using InterpolatedValue()
						  // which does not use smoothened values.
						  
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
									value = vals[static_cast<int>(ij.X())][static_cast<int>(ij.Y())];
								  else
									{
									  int i = static_cast<int>(ij.X());
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
								  if(citer->LoLimit()!=kFloatMissing &&
									 citer->HiLimit()!=kFloatMissing)
									continue;
								}
							  else
								{
								  if(citer->LoLimit()!=kFloatMissing &&
									 valmax<citer->LoLimit())
									continue;
								  if(citer->HiLimit()!=kFloatMissing &&
									 valmin>citer->HiLimit())
									continue;
								}
							  
							  bool exactlo = true;
							  bool exacthi = (citer->HiLimit()!=kFloatMissing &&
											  piter->ExactHiLimit()!=kFloatMissing &&
											  citer->HiLimit()==piter->ExactHiLimit());
							  NFmiContourTree tree(citer->LoLimit(),
												   citer->HiLimit(),
												   exactlo,exacthi);
							  
							  if(piter->DataLoLimit()!=kFloatMissing)
								tree.DataLoLimit(piter->DataLoLimit());
							  if(piter->DataHiLimit()!=kFloatMissing)
								tree.DataHiLimit(piter->DataHiLimit());
							  
							  NFmiColorTools::NFmiBlendRule rule = NFmiColorTools::BlendValue(citer->Rule());
							  
							  tree.Contour(pts[qi],vals,interp,piter->ContourDepth());
							  tree.Fill(theImage,citer->Color(),rule);
							  
							  // NFmiPath path = tree.Path();
							  // path.Fill(theImage,citer->Color(),rule);
							  // cout << "<path style=\"fill=rgb("
							  // << NFmiColorTools::GetRed(citer->Color()) << ","
							  // << NFmiColorTools::GetGreen(citer->Color()) << ","
							  // << NFmiColorTools::GetBlue(citer->Color())
							  // << ")\" d=\""
							  // << path.SVG()
							  // << "\">"
							  // << endl;
							  
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
								  if(patiter->LoLimit()!=kFloatMissing &&
									 patiter->HiLimit()!=kFloatMissing)
									continue;
								}
							  else
								{
								  if(patiter->LoLimit()!=kFloatMissing &&
									 valmax<patiter->LoLimit())
									continue;
								  if(patiter->HiLimit()!=kFloatMissing &&
									 valmin>patiter->HiLimit())
									continue;
								}
							  
							  bool exactlo = true;
							  bool exacthi = (patiter->HiLimit()!=kFloatMissing &&
											  piter->ExactHiLimit()!=kFloatMissing &&
											  patiter->HiLimit()==piter->ExactHiLimit());
							  NFmiContourTree tree(patiter->LoLimit(),
												   patiter->HiLimit(),
												   exactlo,exacthi);
							  
							  if(piter->DataLoLimit()!=kFloatMissing)
								tree.DataLoLimit(piter->DataLoLimit());
							  if(piter->DataHiLimit()!=kFloatMissing)
								tree.DataHiLimit(piter->DataHiLimit());
							  
							  NFmiColorTools::NFmiBlendRule rule = NFmiColorTools::BlendValue(patiter->Rule());
							  
							  tree.Contour(pts[qi],vals,interp,piter->ContourDepth());
							  NFmiImage pattern(patiter->Pattern());
							  
							  tree.Fill(theImage,pattern,rule,patiter->Factor());
							  
							  // NFmiPath path = tree.Path();
							  // path.Fill(theImage,citer->Color(),rule);
							  // cout << "<path style=\"fill=rgb("
							  // << NFmiColorTools::GetRed(citer->Color()) << ","
							  // << NFmiColorTools::GetGreen(citer->Color()) << ","
							  // << NFmiColorTools::GetBlue(citer->Color())
							  // << ")\" d=\""
							  // << path.SVG()
							  // << "\">"
							  // << endl;
							  
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
								  if(liter->Value()!=kFloatMissing &&
									 valmax<liter->Value())
									continue;
								  if(liter->Value()!=kFloatMissing &&
									 valmin>liter->Value())
									continue;
								}
							  
							  NFmiContourTree tree(liter->Value(),kFloatMissing);
							  if(piter->DataLoLimit()!=kFloatMissing)
								tree.DataLoLimit(piter->DataLoLimit());
							  if(piter->DataHiLimit()!=kFloatMissing)
								tree.DataHiLimit(piter->DataHiLimit());
							  
							  NFmiColorTools::NFmiBlendRule rule = NFmiColorTools::BlendValue(liter->Rule());
							  tree.Contour(pts[qi],vals,interp,piter->ContourDepth());
							  NFmiPath path = tree.Path();
							  path.SimplifyLines(10);
							  path.Stroke(theImage,liter->Color(),rule);
							  
							  // cout << path << endl;
							  // cout << "Value = " << liter->LoLimit() << endl;
							  // cout << "<path style=\"fill=rgb("
							  // << NFmiColorTools::GetRed(citer->Color()) << ","
							  // << NFmiColorTools::GetGreen(citer->Color()) << ","
							  // << NFmiColorTools::GetBlue(citer->Color())
							  // << ")\" d=\""
							  // << path.SVG()
							  // << "\">"
							  // << endl;
							  
							  
							}
						  
						}
					  
					  
					  
					  // Bang the foreground
					  
					  if(theForeground != "")
						{
						  NFmiColorTools::NFmiBlendRule rule = NFmiColorTools::BlendValue(theForegroundRule);
						  
						  theImage.Composite(theForegroundImage,rule,kFmiAlignNorthWest,0,0,1);
						  
						}
					  
					  // Draw labels
					  
					  for(piter=pbegin; piter!=pend; ++piter)
						{
						  // Establish the parameter
						  
						  string name = piter->Param();
						  FmiParameterName param = FmiParameterName(NFmiEnumConverter().ToEnum(name));
						  
						  if(param==kFmiBadParameter)
							{
							  cerr << "Error: Unknown parameter " << name << endl;
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
							  cerr << "Error: The parameter is not usable: " << name << endl;
							  exit(1);
							}
						  
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
								  list<pair<NFmiPoint,NFmiPoint> >::const_iterator iter;
								  for(iter=piter->LabelPoints().begin();
									  iter!=piter->LabelPoints().end();
									  ++iter)
									{
									  // The point in question
									  
									  NFmiPoint xy = theArea.ToXY(iter->first);
									  
									  theImage.Composite(marker,
														 markerrule,
														 kFmiAlignCenter,
														 static_cast<int>(xy.X()),
														 static_cast<int>(xy.Y()),
														 markeralpha);
									}
								}
							  
							  // Draw grid
							  
							  if(piter->LabelDX()!=0 && piter->LabelDY()!=0)
								{
								  for(unsigned int j=0; j<pts[qi].NY(); j+=piter->LabelDY())
									for(unsigned int i=0; i<pts[qi].NX(); i+=piter->LabelDX())
									  theImage.Composite(marker,
														 markerrule,
														 kFmiAlignCenter,
														 static_cast<int>(pts[qi][i][j].X()),
														 static_cast<int>(pts[qi][i][j].Y()),
														 markeralpha);
								}
							}
						  
						  // Label markers now drawn, only label texts remain
						  
						  // Quick exit from loop if no labels are
						  // desired for this parameter
						  
						  if(piter->LabelPoints().empty() &&
							 !(piter->LabelDX()!=0 && piter->LabelDY()!=0))
							continue;
						  
						  // Draw markers if so requested
						  
						  // Draw labels at specifing latlon points if requested
						  
						  if( (piter->LabelFormat() != "") && !piter->LabelPoints().empty())
							{
							  
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
							  
							  list<pair<NFmiPoint,NFmiPoint> >::const_iterator iter;
							  
							  int pointnumber = 0;
							  for(iter=piter->LabelPoints().begin();
								  iter!=piter->LabelPoints().end();
								  ++iter)
								{
								  float value = piter->LabelValues()[pointnumber++];
								  
								  // Convert value to string
								  
								  string strvalue("-");
								  
								  if(value!=kFloatMissing)
									{
									  char tmp[20];
									  sprintf(tmp,piter->LabelFormat().c_str(),value);
									  strvalue = tmp;
									}
								  
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
							  
							  // Grid would be labeled here, but it's not implemented
							  // yet. I have to decide whether to smoothen as in
							  // contouring or whether to use the original data value
							  // at the grid point.
							  
							}
						}
					  
					  
					  // Draw wind arrows if so requested
					  
					  
					  if(theQueryInfo->Param().GetParamIdent()==kFmiWindDirection &&
						 (!theArrowPoints.empty() || (theWindArrowDX!=0 && theWindArrowDY!=0)) &&
						 (theArrowFile!=""))
						{
						  // Read the arrow definition
						  
						  ifstream arrow(theArrowFile.c_str());
						  if(!arrow)
							{
							  cerr << "Error: Could not open " << theArrowFile << endl;
							  exit(1);
							}
						  // Read in the entire file
						  
						  string pathstring;
						  StringReader(arrow,pathstring);
						  arrow.close();
						  
						  // Convert to a path
						  
						  NFmiPath arrowpath;
						  arrowpath.Add(pathstring);
						  
						  // Handle all given coordinates
						  
						  list<NFmiPoint>::const_iterator iter;
						  
						  for(iter=theArrowPoints.begin();
							  iter!=theArrowPoints.end();
							  ++iter)
							{
							  float dir = theQueryInfo->InterpolatedValue(*iter);
							  if(dir==kFloatMissing)	// ignore missing
								continue;
							  
							  float speed = -1;
							  if(theQueryInfo->Param(kFmiWindSpeedMS))
								speed = theQueryInfo->InterpolatedValue(*iter);
							  theQueryInfo->Param(kFmiWindDirection);
							  
							  // The start point
							  
							  NFmiPoint xy0 = theArea.ToXY(*iter);
							  
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
							  if(theQueryInfo->Param(kFmiWindSpeedMS))
								theQueryInfo->Values(speedvalues);
							  theQueryInfo->Param(kFmiWindDirection);
							  
							  for(unsigned int j=0; j<pts[qi].NY(); j+=theWindArrowDY)
								for(unsigned int i=0; i<pts[qi].NX(); i+=theWindArrowDX)
								  {
									float dir = vals[i][j];
									if(dir==kFloatMissing)	// ignore missing
									  continue;
									
									float speed = speedvalues[i][j];

									// The start point
									
									NFmiPoint xy0 = theArea.ToXY(latlons[i][j]);
									
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
							NFmiTime tlocal = NFmiMetTime(futctime,1).LocalTime();
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

