// ----------------------------------------------------------------------
// Yksittäisen parametrin piirto-ohjeet
// ----------------------------------------------------------------------

#include "ContourValue.h"

#include "NFmiColorTools.h"
#include "NFmiPoint.h"

#include <list>
#include <string>
#include <vector>

class ContourSpec
{
public:
  ContourSpec(const std::string & param,
			  const std::string & interpolation,
			  const std::string & smoother,
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
  
  const std::list<ContourRange> & ContourFills(void) const { return itsContourFills; }
  const std::list<ContourPattern> & ContourPatterns(void) const { return itsContourPatterns; }
  const std::list<ContourValue> & ContourValues(void) const { return itsContourValues; }
  
  const std::string & Param(void) const		{ return itsParam; }
  const std::string & ContourInterpolation(void) const	{ return itsContourInterpolation; }
  const std::string & Smoother(void) const		{ return itsSmoother; }
  float SmootherRadius(void) const		{ return itsSmootherRadius; }
  int SmootherFactor(void) const		{ return itsSmootherFactor; }
  float ExactHiLimit(void) const		{ return itsExactHiLimit; }
  int ContourDepth(void) const			{ return itsContourDepth; }
  float DataHiLimit(void) const			{ return itsDataHiLimit; }
  float DataLoLimit(void) const			{ return itsDataLoLimit; }
  
  void ContourInterpolation(const std::string & val)	{ itsContourInterpolation = val; }
  void Smoother(const std::string & val)		{ itsSmoother = val; }
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
  
  const std::list<std::pair<NFmiPoint,NFmiPoint> > & LabelPoints(void) const { return itsLabelPoints; }
  
  void Add(const NFmiPoint & point,
		   const NFmiPoint xy = NFmiPoint(kFloatMissing,kFloatMissing))
  { itsLabelPoints.push_back(std::make_pair(point,xy)); }
  
  const std::vector<float> & LabelValues(void) const
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
	itsLabelDX = 0;
	itsLabelDY = 0;
  }
  
  const std::string & LabelMarker(void) const	{ return itsLabelMarker; }
  const std::string & LabelMarkerRule(void) const 	{ return itsLabelMarkerRule; }
  float LabelMarkerAlphaFactor(void) const 	{ return itsLabelMarkerAlphaFactor; }
  const std::string & LabelFont(void) const		{ return itsLabelFont; }
  float LabelSize(void) const			{ return itsLabelSize; }
  int LabelStrokeColor(void) const		{ return itsLabelStrokeColor; }
  const std::string & LabelStrokeRule(void) const	{ return itsLabelStrokeRule; }
  int LabelFillColor(void) const		{ return itsLabelFillColor; }
  const std::string & LabelFillRule(void) const	{ return itsLabelFillRule; }
  const std::string & LabelAlignment(void) const	{ return itsLabelAlignment; }
  const std::string & LabelFormat(void) const	{ return itsLabelFormat; }
  const std::string & LabelMissing(void) const	{ return itsLabelMissing; }
  float LabelAngle(void) const			{ return itsLabelAngle; }
  float LabelOffsetX(void) const		{ return itsLabelOffsetX; }
  float LabelOffsetY(void) const		{ return itsLabelOffsetY; }
  int LabelDX(void) const			{ return itsLabelDX; }
  int LabelDY(void) const			{ return itsLabelDY; }
  
  std::string LabelCaption(void) const		{ return itsLabelCaption; }
  float LabelCaptionDX(void) const		{ return itsLabelCaptionDX; }
  float LabelCaptionDY(void) const		{ return itsLabelCaptionDY; }
  std::string LabelCaptionAlignment(void) const	{ return itsLabelCaptionAlignment; }
  
  void LabelMarker(const std::string & value)	{ itsLabelMarker = value; }
  void LabelMarkerRule(const std::string & value)	{ itsLabelMarkerRule = value; }
  void LabelMarkerAlphaFactor(float value) 	{ itsLabelMarkerAlphaFactor = value; }
  void LabelFont(const std::string & value)		{ itsLabelFont = value; }
  void LabelSize(float value)			{ itsLabelSize = value; }
  void LabelStrokeColor(int value)		{ itsLabelStrokeColor = value; }
  void LabelStrokeRule(const std::string & value)	{ itsLabelStrokeRule = value; }
  void LabelFillColor(int value)		{ itsLabelFillColor = value; }
  void LabelFillRule(const std::string & value)	{ itsLabelFillRule = value; }
  void LabelAlignment(const std::string & value)	{ itsLabelAlignment = value; }
  void LabelFormat(const std::string & value)	{ itsLabelFormat = value; }
  void LabelMissing(const std::string & value)	{ itsLabelMissing = value; }
  void LabelAngle(float value)			{ itsLabelAngle = value; }
  void LabelOffsetX(float value)		{ itsLabelOffsetX = value; }
  void LabelOffsetY(float value)		{ itsLabelOffsetY = value; }
  void LabelDX(int value)			{ itsLabelDX = value; }
  void LabelDY(int value)			{ itsLabelDY = value; }
  
  void LabelCaption(const std::string & value)	{ itsLabelCaption = value; }
  void LabelCaptionDX(float value)		{ itsLabelCaptionDX = value; }
  void LabelCaptionDY(float value)		{ itsLabelCaptionDY = value; }
  void LabelCaptionAlignment(const std::string & value) { itsLabelCaptionAlignment = value; }
  
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

