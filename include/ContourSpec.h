// ----------------------------------------------------------------------
// Yksittäisen parametrin piirto-ohjeet
// ----------------------------------------------------------------------

#include "ContourPattern.h"
#include "ContourRange.h"
#include "ContourValue.h"

#include "NFmiColorTools.h"
#include "NFmiPoint.h"

#include <list>
#include <string>
#include <vector>

class ContourSpec
{
public:

#ifdef COMPILER_GENERATED
  ~ContourSpec();
  ContourSpec(const ContourSpec & theValue);
  ContourSpec & operator=(const ContourSpec & theValue);
#endif

  ContourSpec(const std::string & theParam,
			  const std::string & theInterpolation,
			  const std::string & theSmoother,
			  int theDepth=0,
			  float theSmootherRadius=1.0,
			  int theSmootherFactor=1,
			  float theHiLimit=kFloatMissing)
    : itsParam(theParam)
    , itsContourInterpolation(theInterpolation)
    , itsSmoother(theSmoother)
    , itsSmootherRadius(theSmootherRadius)
    , itsSmootherFactor(theSmootherFactor)
    , itsExactHiLimit(theHiLimit)
    , itsContourDepth(theDepth)
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
    , itsLabelMissing("-")
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
  
  const std::list<ContourRange> & contourFills(void) const { return itsContourFills; }
  const std::list<ContourPattern> & contourPatterns(void) const { return itsContourPatterns; }
  const std::list<ContourValue> & contourValues(void) const { return itsContourValues; }
  
  const std::string & param(void) const		{ return itsParam; }
  const std::string & contourInterpolation(void) const	{ return itsContourInterpolation; }
  const std::string & smoother(void) const		{ return itsSmoother; }
  float smootherRadius(void) const		{ return itsSmootherRadius; }
  int smootherFactor(void) const		{ return itsSmootherFactor; }
  float exactHiLimit(void) const		{ return itsExactHiLimit; }
  int contourDepth(void) const			{ return itsContourDepth; }
  float dataHiLimit(void) const			{ return itsDataHiLimit; }
  float dataLoLimit(void) const			{ return itsDataLoLimit; }
  
  void contourInterpolation(const std::string & val)	{ itsContourInterpolation = val; }
  void smoother(const std::string & val)		{ itsSmoother = val; }
  void smootherRadius(float radius)		{ itsSmootherRadius = radius; }
  void smootherFactor(int factor)		{ itsSmootherFactor = factor; }
  void exactHiLimit(float limit)		{ itsExactHiLimit = limit; }
  void contourDepth(int depth)			{ itsContourDepth = depth; }
  
  void dataLoLimit(float limit)			{ itsDataLoLimit = limit; }
  void dataHiLimit(float limit)			{ itsDataHiLimit = limit; }
  
  void add(ContourRange range) { itsContourFills.push_back(range); }
  void add(ContourValue value) { itsContourValues.push_back(value); }
  void add(ContourPattern value) { itsContourPatterns.push_back(value); }
  
  // This was done to replace 32700 with -1 in PrecipitationForm
  
  bool replace(void) const		{ return itHasReplace; }
  float replaceSourceValue(void) const	{ return itsReplaceSourceValue; }
  float replaceTargetValue(void) const	{ return itsReplaceTargetValue; }
  
  void replace(float src, float dst)
  {
    itHasReplace = true;
    itsReplaceSourceValue = src;
    itsReplaceTargetValue = dst;
  }
  
  // Label specific methods
  
  const std::list<std::pair<NFmiPoint,NFmiPoint> > & labelPoints(void) const { return itsLabelPoints; }
  
  void add(const NFmiPoint & point,
		   const NFmiPoint xy = NFmiPoint(kFloatMissing,kFloatMissing))
  { itsLabelPoints.push_back(std::make_pair(point,xy)); }
  
  const std::vector<float> & labelValues(void) const
  { return itsLabelValues; }
  
  void addLabelValue(float value)
  { itsLabelValues.push_back(value); }
  
  void clearLabelValues(void)
  {
    itsLabelValues.clear();
  }
  
  void clearLabels(void)
  {
    itsLabelPoints.clear();
    itsLabelValues.clear();
	itsLabelDX = 0;
	itsLabelDY = 0;
  }
  
  const std::string & labelMarker(void) const	{ return itsLabelMarker; }
  const std::string & labelMarkerRule(void) const 	{ return itsLabelMarkerRule; }
  float labelMarkerAlphaFactor(void) const 	{ return itsLabelMarkerAlphaFactor; }
  const std::string & labelFont(void) const		{ return itsLabelFont; }
  float labelSize(void) const			{ return itsLabelSize; }
  int labelStrokeColor(void) const		{ return itsLabelStrokeColor; }
  const std::string & labelStrokeRule(void) const	{ return itsLabelStrokeRule; }
  int labelFillColor(void) const		{ return itsLabelFillColor; }
  const std::string & labelFillRule(void) const	{ return itsLabelFillRule; }
  const std::string & labelAlignment(void) const	{ return itsLabelAlignment; }
  const std::string & labelFormat(void) const	{ return itsLabelFormat; }
  const std::string & labelMissing(void) const	{ return itsLabelMissing; }
  float labelAngle(void) const			{ return itsLabelAngle; }
  float labelOffsetX(void) const		{ return itsLabelOffsetX; }
  float labelOffsetY(void) const		{ return itsLabelOffsetY; }
  int labelDX(void) const			{ return itsLabelDX; }
  int labelDY(void) const			{ return itsLabelDY; }
  
  std::string labelCaption(void) const		{ return itsLabelCaption; }
  float labelCaptionDX(void) const		{ return itsLabelCaptionDX; }
  float labelCaptionDY(void) const		{ return itsLabelCaptionDY; }
  std::string labelCaptionAlignment(void) const	{ return itsLabelCaptionAlignment; }
  
  void labelMarker(const std::string & value)	{ itsLabelMarker = value; }
  void labelMarkerRule(const std::string & value)	{ itsLabelMarkerRule = value; }
  void labelMarkerAlphaFactor(float value) 	{ itsLabelMarkerAlphaFactor = value; }
  void labelFont(const std::string & value)		{ itsLabelFont = value; }
  void labelSize(float value)			{ itsLabelSize = value; }
  void labelStrokeColor(int value)		{ itsLabelStrokeColor = value; }
  void labelStrokeRule(const std::string & value)	{ itsLabelStrokeRule = value; }
  void labelFillColor(int value)		{ itsLabelFillColor = value; }
  void labelFillRule(const std::string & value)	{ itsLabelFillRule = value; }
  void labelAlignment(const std::string & value)	{ itsLabelAlignment = value; }
  void labelFormat(const std::string & value)	{ itsLabelFormat = value; }
  void labelMissing(const std::string & value)	{ itsLabelMissing = value; }
  void labelAngle(float value)			{ itsLabelAngle = value; }
  void labelOffsetX(float value)		{ itsLabelOffsetX = value; }
  void labelOffsetY(float value)		{ itsLabelOffsetY = value; }
  void labelDX(int value)			{ itsLabelDX = value; }
  void labelDY(int value)			{ itsLabelDY = value; }
  
  void labelCaption(const std::string & value)	{ itsLabelCaption = value; }
  void labelCaptionDX(float value)		{ itsLabelCaptionDX = value; }
  void labelCaptionDY(float value)		{ itsLabelCaptionDY = value; }
  void labelCaptionAlignment(const std::string & value) { itsLabelCaptionAlignment = value; }
  
private:

  ContourSpec(void);

  std::string itsParam;
  std::string itsContourInterpolation;
  std::string itsSmoother;
  float itsSmootherRadius;
  int itsSmootherFactor;
  
  std::list<ContourRange> itsContourFills;
  std::list<ContourValue> itsContourValues;
  std::list<ContourPattern> itsContourPatterns;
  
  float itsExactHiLimit;
  int itsContourDepth;
  float itsDataLoLimit;
  float itsDataHiLimit;
  
  bool itHasReplace;;
  float itsReplaceSourceValue;
  float itsReplaceTargetValue;
  
  // LatLon, optional label location pairs in pixels
  std::list<std::pair<NFmiPoint,NFmiPoint> > itsLabelPoints;
  // Respective values calculated while contouring
  std::vector<float> itsLabelValues;
  
  std::string itsLabelMarker;
  std::string itsLabelMarkerRule;
  float itsLabelMarkerAlphaFactor;
  std::string itsLabelFont;
  float itsLabelSize;
  int itsLabelStrokeColor;
  std::string itsLabelStrokeRule;
  int itsLabelFillColor;
  std::string itsLabelFillRule;
  std::string itsLabelAlignment;
  std::string itsLabelFormat;
  std::string itsLabelMissing;
  float itsLabelAngle;
  float itsLabelOffsetX;
  float itsLabelOffsetY;
  int itsLabelDX;
  int itsLabelDY;
  
  std::string itsLabelCaption;
  float itsLabelCaptionDX;
  float itsLabelCaptionDY;
  std::string itsLabelCaptionAlignment;
  
};

