// ======================================================================
/*!
 * \file
 * \brief Implementation of class ContourSpec
 */
// ======================================================================

#include "ContourSpec.h"
#include "imagine/NFmiColorTools.h"

// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 *
 * \param theParam The parameter
 * \param theInterpolation The interpolation method
 * \param theSmoother The smoothing method
 * \param theDepth The contouring recursion depth
 * \param theSmootherRadius The smoother search radius
 * \param theSmootherFactor The smoother sharpness factor
 * \param theHiLimit The absolute high limit of the data
 */
// ----------------------------------------------------------------------

ContourSpec::ContourSpec(const std::string & theParam,
						 const std::string & theInterpolation,
						 const std::string & theSmoother,
						 int theDepth,
						 float theSmootherRadius,
						 int theSmootherFactor,
						 float theHiLimit)
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
  , itsLabelStrokeColor(Imagine::NFmiColorTools::NoColor)
  , itsLabelStrokeRule("Copy")
  , itsLabelFillColor(Imagine::NFmiColorTools::NoColor)
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
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Return ContourRange specifications
 *
 * \return Reference to the internal list of ContourRange objects
 */
// ----------------------------------------------------------------------

const std::list<ContourRange> & ContourSpec::contourFills(void) const
{
  return itsContourFills;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return ContourPattern specifications
 *
 * \return Reference to the internal list of ContourPattern objects
 */
// ----------------------------------------------------------------------

const std::list<ContourPattern> & ContourSpec::contourPatterns(void) const
{
  return itsContourPatterns;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return ContourValue specifications
 *
 * \return Reference to the internal list of ContourValue objects
 */
// ----------------------------------------------------------------------

const std::list<ContourValue> & ContourSpec::contourValues(void) const
{
  return itsContourValues;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return ContourSymbol specifications
 *
 * \return Reference to the internal list of ContourSymbol objects
 */
// ----------------------------------------------------------------------

const std::list<ContourSymbol> & ContourSpec::contourSymbols(void) const
{
  return itsContourSymbols;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the parameter name
 *
 * \return The parameter name
 */
// ----------------------------------------------------------------------

const std::string & ContourSpec::param(void) const
{
  return itsParam;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the interpolation method
 *
 * \return The interpolation method
 */
// ----------------------------------------------------------------------

const std::string & ContourSpec::contourInterpolation(void) const
{
  return itsContourInterpolation;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the smoother name
 *
 * \return The smoother
 */
// ----------------------------------------------------------------------

const std::string & ContourSpec::smoother(void) const
{
  return itsSmoother;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the smoother radius
 *
 * \return The radius
 */
// ----------------------------------------------------------------------

float ContourSpec::smootherRadius(void) const
{
  return itsSmootherRadius;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the smoother sharpness factor
 *
 * \return The factor
 */
// ----------------------------------------------------------------------

int ContourSpec::smootherFactor(void) const
{
  return itsSmootherFactor;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the upper limit for data values
 *
 * \return The upper limit
 */
// ----------------------------------------------------------------------

float ContourSpec::exactHiLimit(void) const
{
  return itsExactHiLimit;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the contour recursion depth
 *
 * \return The depth
 */
// ----------------------------------------------------------------------

int ContourSpec::contourDepth(void) const
{
  return itsContourDepth;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the data upper limit
 *
 * \return The limit
 */
// ----------------------------------------------------------------------

float ContourSpec::dataHiLimit(void) const
{
  return itsDataHiLimit;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the data lower limit
 *
 * \return The limit
 */
// ----------------------------------------------------------------------

float ContourSpec::dataLoLimit(void) const
{
  return itsDataLoLimit;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the interpolation method
 *
 * \param theValue The method
 */
// ----------------------------------------------------------------------

void ContourSpec::contourInterpolation(const std::string & theValue)
{
  itsContourInterpolation = theValue;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the smoother
 *
 * \param theValue The method
 */
// ----------------------------------------------------------------------

void ContourSpec::smoother(const std::string & theValue)
{
  itsSmoother = theValue;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the smoother radius
 *
 * \param theRadius The radius
 */
// ----------------------------------------------------------------------

void ContourSpec::smootherRadius(float theRadius)
{
  itsSmootherRadius = theRadius;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the smoother sharpness factor
 *
 * \param theFactor The factor
 */
// ----------------------------------------------------------------------

void ContourSpec::smootherFactor(int theFactor)
{
  itsSmootherFactor = theFactor;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the data upper limit
 *
 * \param theLimit The upper limit
 */
// ----------------------------------------------------------------------

void ContourSpec::exactHiLimit(float theLimit)
{
  itsExactHiLimit = theLimit;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the contouring recursion depth
 *
 * \param theDepth The recursion depth
 */
// ----------------------------------------------------------------------

void ContourSpec::contourDepth(int theDepth)
{
  itsContourDepth = theDepth;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the data lower limit
 *
 * \param theLimit The lower limit
 */
// ----------------------------------------------------------------------

void ContourSpec::dataLoLimit(float theLimit)
{
  itsDataLoLimit = theLimit;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the data upper limit
 *
 * \param theLimit The upper limit
 */
// ----------------------------------------------------------------------

void ContourSpec::dataHiLimit(float theLimit)
{
  itsDataHiLimit = theLimit;
}

// ----------------------------------------------------------------------
/*!
 * \brief Add a new ContourRange specification
 *
 * \param theRange The specification to add
 */
// ----------------------------------------------------------------------

void ContourSpec::add(ContourRange theRange)
{
  itsContourFills.push_back(theRange);
}

// ----------------------------------------------------------------------
/*!
 * \brief Add a new ContourValue specification
 *
 * \param theValue The specification to add
 */
// ----------------------------------------------------------------------

void ContourSpec::add(ContourValue theValue)
{
  itsContourValues.push_back(theValue);
}

// ----------------------------------------------------------------------
/*!
 * \brief Add a new ContourPattern specification
 *
 * \param theValue The specification to add
 */
// ----------------------------------------------------------------------

void ContourSpec::add(ContourPattern theValue)
{
  itsContourPatterns.push_back(theValue);
}

// ----------------------------------------------------------------------
/*!
 * \brief Add a new ContourSymbol specification
 *
 * \param theValue The specification to add
 */
// ----------------------------------------------------------------------

void ContourSpec::add(ContourSymbol theValue)
{
  itsContourSymbols.push_back(theValue);
}

// ----------------------------------------------------------------------
/*!
 * \brief Return whether a data replacement command is set or not
 *
 * \return True if a replacement has been set
 */
// ----------------------------------------------------------------------

bool ContourSpec::replace(void) const
{
  return itHasReplace;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the replacement source value
 *
 * \return The value being replaced
 */
// ----------------------------------------------------------------------

float ContourSpec::replaceSourceValue(void) const
{
  return itsReplaceSourceValue;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the replacement destination value
 *
 * \return The new value
 */
// ----------------------------------------------------------------------

float ContourSpec::replaceTargetValue(void) const
{
  return itsReplaceTargetValue;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set a replacement
 *
 * \param theSrcValue The old value
 * \param theDstValue The new value
 */
// ----------------------------------------------------------------------

void ContourSpec::replace(float theSrcValue, float theDstValue)
{
  itHasReplace = true;
  itsReplaceSourceValue = theSrcValue;
  itsReplaceTargetValue = theDstValue;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return label points
 *
 * \return A reference to the internal list of points
 */
// ----------------------------------------------------------------------

const std::list<std::pair<NFmiPoint,NFmiPoint> > & ContourSpec::labelPoints(void) const
{
  return itsLabelPoints;
}

// ----------------------------------------------------------------------
/*!
 * \brief Add a new label point
 *
 * \param thePoint The geographic coordinates
 * \param theXY The xy-adjustment on the pixmap
 */
// ----------------------------------------------------------------------

void ContourSpec::add(const NFmiPoint & thePoint,
					  const NFmiPoint theXY)
{
  itsLabelPoints.push_back(std::make_pair(thePoint,theXY));
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the label values
 *
 * \return The label values
 */
// ----------------------------------------------------------------------

const std::vector<float> & ContourSpec::labelValues(void) const
{
  return itsLabelValues;
}

// ----------------------------------------------------------------------
/*!
 * \brief Add a new label value
 *
 * \param theValue The new value
 */
// ----------------------------------------------------------------------

void ContourSpec::addLabelValue(float theValue)
{
  itsLabelValues.push_back(theValue);
}

// ----------------------------------------------------------------------
/*!
 * \brief Clear all label values
 *
 */
// ----------------------------------------------------------------------

void ContourSpec::clearLabelValues(void)
{
  itsLabelValues.clear();
}

// ----------------------------------------------------------------------
/*!
 * \brief Clear all labels
 *
 */
// ----------------------------------------------------------------------

void ContourSpec::clearLabels(void)
{
  itsLabelPoints.clear();
  itsLabelValues.clear();
  itsLabelDX = 0;
  itsLabelDY = 0;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the label marker image
 *
 * \return The marker image filename
 */
// ----------------------------------------------------------------------

const std::string & ContourSpec::labelMarker(void) const
{
  return itsLabelMarker;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the label marker blending rule
 *
 * \return The blending rule
 */
// ----------------------------------------------------------------------

const std::string & ContourSpec::labelMarkerRule(void) const
{
  return itsLabelMarkerRule;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the label marker alpha blending factor
 *
 * \return The blending factor
 */
// ----------------------------------------------------------------------

float ContourSpec::labelMarkerAlphaFactor(void) const
{
  return itsLabelMarkerAlphaFactor;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the label font
 *
 * \return The font name
 */
// ----------------------------------------------------------------------

const std::string & ContourSpec::labelFont(void) const
{
  return itsLabelFont;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the label size
 *
 * \return The label size
 */
// ----------------------------------------------------------------------

float ContourSpec::labelSize(void) const
{
  return itsLabelSize;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the label stroke color
 *
 * \return The color
 */
// ----------------------------------------------------------------------

int ContourSpec::labelStrokeColor(void) const
{
  return itsLabelStrokeColor;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the label stroke rule
 *
 * \return The stroke rule
 */
// ----------------------------------------------------------------------

const std::string & ContourSpec::labelStrokeRule(void) const
{
  return itsLabelStrokeRule;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the label fill color
 *
 * \return The fill color
 */
// ----------------------------------------------------------------------

int ContourSpec::labelFillColor(void) const
{
  return itsLabelFillColor;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the label fill rule
 *
 * \return The fill rule
 */
// ----------------------------------------------------------------------

const std::string & ContourSpec::labelFillRule(void) const
{
  return itsLabelFillRule;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the label alignment
 *
 * \return The alignment
 */
// ----------------------------------------------------------------------

const std::string & ContourSpec::labelAlignment(void) const
{
  return itsLabelAlignment;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the label number format
 *
 * \return The number format
 */
// ----------------------------------------------------------------------

const std::string & ContourSpec::labelFormat(void) const
{
  return itsLabelFormat;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the label for missing values
 *
 * \return The missing value text
 */
// ----------------------------------------------------------------------

const std::string & ContourSpec::labelMissing(void) const
{
  return itsLabelMissing;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the label angle
 *
 * \return The label angle
 */
// ----------------------------------------------------------------------

float ContourSpec::labelAngle(void) const
{
  return itsLabelAngle;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the label X-offset
 *
 * \return The offset
 */
// ----------------------------------------------------------------------

float ContourSpec::labelOffsetX(void) const
{
  return itsLabelOffsetX;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the label Y-offset
 *
 * \return The offset
 */
// ----------------------------------------------------------------------

float ContourSpec::labelOffsetY(void) const
{
  return itsLabelOffsetY;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the label X-step
 *
 * \return The step
 */
// ----------------------------------------------------------------------

int ContourSpec::labelDX(void) const
{
  return itsLabelDX;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the label Y-step
 *
 * \return The step
 */
// ----------------------------------------------------------------------

int ContourSpec::labelDY(void) const
{
  return itsLabelDY;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the label caption
 *
 * \return The caption
 */
// ----------------------------------------------------------------------

std::string ContourSpec::labelCaption(void) const
{
  return itsLabelCaption;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the label caption X-offset
 *
 * \return The offset
 */
// ----------------------------------------------------------------------

float ContourSpec::labelCaptionDX(void) const
{
  return itsLabelCaptionDX;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the label caption Y-offset
 *
 * \return The offset
 */
// ----------------------------------------------------------------------

float ContourSpec::labelCaptionDY(void) const
{
  return itsLabelCaptionDY;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the label caption alignment
 *
 * \return The alignment
 */
// ----------------------------------------------------------------------

std::string ContourSpec::labelCaptionAlignment(void) const
{
  return itsLabelCaptionAlignment;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the label marker filename
 *
 * \param theValue The filename
 */
// ----------------------------------------------------------------------

void ContourSpec::labelMarker(const std::string & theValue)
{
  itsLabelMarker = theValue;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the label marker blending rule
 *
 * \param theValue The rule
 */
// ----------------------------------------------------------------------

void ContourSpec::labelMarkerRule(const std::string & theValue)
{
  itsLabelMarkerRule = theValue;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the label marker alpha blending factor
 *
 * \param theValue The factor
 */
// ----------------------------------------------------------------------

void ContourSpec::labelMarkerAlphaFactor(float theValue)
{
  itsLabelMarkerAlphaFactor = theValue;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the label font
 *
 * \param theValue The font name
 */
// ----------------------------------------------------------------------

void ContourSpec::labelFont(const std::string & theValue)
{
  itsLabelFont = theValue;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the label size
 *
 * \param theValue The label size
 */
// ----------------------------------------------------------------------

void ContourSpec::labelSize(float theValue)
{
  itsLabelSize = theValue;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the label stroke color
 *
 * \param theValue The color
 */
// ----------------------------------------------------------------------

void ContourSpec::labelStrokeColor(int theValue)
{
  itsLabelStrokeColor = theValue;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the label stroke blending rule
 *
 * \param theValue The rule
 */
// ----------------------------------------------------------------------

void ContourSpec::labelStrokeRule(const std::string & theValue)
{
  itsLabelStrokeRule = theValue;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the label fill color
 *
 * \param theValue The color
 */
// ----------------------------------------------------------------------

void ContourSpec::labelFillColor(int theValue)
{
  itsLabelFillColor = theValue;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the label fill blending rule
 *
 * \param theValue The rule
 */
// ----------------------------------------------------------------------

void ContourSpec::labelFillRule(const std::string & theValue)
{
  itsLabelFillRule = theValue;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the label alignment
 *
 * \param theValue The alignment
 */
// ----------------------------------------------------------------------

void ContourSpec::labelAlignment(const std::string & theValue)
{
  itsLabelAlignment = theValue;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the label number format
 *
 * \param theValue The format
 */
// ----------------------------------------------------------------------

void ContourSpec::labelFormat(const std::string & theValue)
{
  itsLabelFormat = theValue;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the label for missing values
 *
 * \param theValue The label
 */
// ----------------------------------------------------------------------

void ContourSpec::labelMissing(const std::string & theValue)
{
  itsLabelMissing = theValue;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the label angle
 *
 * \param theValue The angle
 */
// ----------------------------------------------------------------------

void ContourSpec::labelAngle(float theValue)
{
  itsLabelAngle = theValue;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the label X-offset
 *
 * \param theValue The offset
 */
// ----------------------------------------------------------------------

void ContourSpec::labelOffsetX(float theValue)
{
  itsLabelOffsetX = theValue;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the label Y-offset
 *
 * \param theValue The offset
 */
// ----------------------------------------------------------------------

void ContourSpec::labelOffsetY(float theValue)
{
  itsLabelOffsetY = theValue;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the label X-step
 *
 * \param theValue The step
 */
// ----------------------------------------------------------------------

void ContourSpec::labelDX(int theValue)
{
  itsLabelDX = theValue;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the label Y-step
 *
 * \param theValue The step
 */
// ----------------------------------------------------------------------

void ContourSpec::labelDY(int theValue)
{
  itsLabelDY = theValue;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the label caption
 *
 * \param theValue The caption
 */
// ----------------------------------------------------------------------

void ContourSpec::labelCaption(const std::string & theValue)
{
 itsLabelCaption = theValue;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the label caption X-offset
 *
 * \param theValue The offset
 */
// ----------------------------------------------------------------------

void ContourSpec::labelCaptionDX(float theValue)
{
  itsLabelCaptionDX = theValue;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the label caption Y-offset
 *
 * \param theValue The offset
 */
// ----------------------------------------------------------------------

void ContourSpec::labelCaptionDY(float theValue)
{
  itsLabelCaptionDY = theValue;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the label caption alignment
 *
 * \param theValue The alignment
 */
// ----------------------------------------------------------------------

void ContourSpec::labelCaptionAlignment(const std::string & theValue)
{
  itsLabelCaptionAlignment = theValue;
}

// ======================================================================

