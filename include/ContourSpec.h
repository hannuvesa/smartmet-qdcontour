// ----------------------------------------------------------------------
// Yksittäisen parametrin piirto-ohjeet
// ----------------------------------------------------------------------

#ifndef CONTOURSPEC_H
#define CONTOURSPEC_H

#include "ContourPattern.h"
#include "ContourRange.h"
#include "ContourValue.h"

#include "newbase/NFmiPoint.h"

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
			  float theHiLimit=kFloatMissing);
  
  const std::list<ContourRange> & contourFills(void) const;
  const std::list<ContourPattern> & contourPatterns(void) const;
  const std::list<ContourValue> & contourValues(void) const;
  
  const std::string & param(void) const;
  const std::string & contourInterpolation(void) const;
  const std::string & smoother(void) const;
  float smootherRadius(void) const;
  int smootherFactor(void) const;
  float exactHiLimit(void) const;
  int contourDepth(void) const;
  float dataHiLimit(void) const;
  float dataLoLimit(void) const;
  
  void contourInterpolation(const std::string & theValue);
  void smoother(const std::string & theValue);
  void smootherRadius(float theRadius);
  void smootherFactor(int theFactor);
  void exactHiLimit(float theLimit);
  void contourDepth(int theDepth);
  
  void dataLoLimit(float theLimit);
  void dataHiLimit(float theLimit);
  
  void add(ContourRange theRange);
  void add(ContourValue theValue);
  void add(ContourPattern theValue);
  
  // This was done to replace 32700 with -1 in PrecipitationForm
  
  bool replace(void) const;
  float replaceSourceValue(void) const;
  float replaceTargetValue(void) const;
  
  void replace(float theSrcValue, float theDstValue);

  // Label specific methods
  
  const std::list<std::pair<NFmiPoint,NFmiPoint> > & labelPoints(void) const;
  
  void add(const NFmiPoint & thePoint,
		   const NFmiPoint theXY = NFmiPoint(kFloatMissing,kFloatMissing));
  
  const std::vector<float> & labelValues(void) const;
  
  void addLabelValue(float theValue);
  void clearLabelValues(void);
  void clearLabels(void);
  
  const std::string & labelMarker(void) const;
  const std::string & labelMarkerRule(void) const;
  float labelMarkerAlphaFactor(void) const;
  const std::string & labelFont(void) const;
  float labelSize(void) const;
  int labelStrokeColor(void) const;
  const std::string & labelStrokeRule(void) const;
  int labelFillColor(void) const;
  const std::string & labelFillRule(void) const;
  const std::string & labelAlignment(void) const;
  const std::string & labelFormat(void) const;
  const std::string & labelMissing(void) const;
  float labelAngle(void) const;
  float labelOffsetX(void) const;
  float labelOffsetY(void) const;
  int labelDX(void) const;
  int labelDY(void) const;
  
  std::string labelCaption(void) const;
  float labelCaptionDX(void) const;
  float labelCaptionDY(void) const;
  std::string labelCaptionAlignment(void) const;
  
  void labelMarker(const std::string & theValue);
  void labelMarkerRule(const std::string & theValue);
  void labelMarkerAlphaFactor(float theValue);
  void labelFont(const std::string & theValue);
  void labelSize(float theValue);
  void labelStrokeColor(int theValue);
  void labelStrokeRule(const std::string & theValue);
  void labelFillColor(int theValue);
  void labelFillRule(const std::string & theValue);
  void labelAlignment(const std::string & theValue);
  void labelFormat(const std::string & theValue);
  void labelMissing(const std::string & theValue);
  void labelAngle(float theValue);
  void labelOffsetX(float theValue);
  void labelOffsetY(float theValue);
  void labelDX(int theValue);
  void labelDY(int theValue);
  
  void labelCaption(const std::string & theValue);
  void labelCaptionDX(float theValue);
  void labelCaptionDY(float theValue);
  void labelCaptionAlignment(const std::string & theValue);
  
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
  
  bool itHasReplace;
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

#endif // CONTOURSPEC_H

// ======================================================================
