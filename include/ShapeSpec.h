// ======================================================================
/*!
 * \file
 * \brief Interface of class ShapeSpec
 */
// ======================================================================
/*!
 * \class ShapeSpec
 * \brief The specifications for rendering a shapefile
 *
 */
// ======================================================================

#ifndef SHAPESPEC_H
#define SHAPESPEC_H

#include "imagine/NFmiColorTools.h"

class ShapeSpec
{
public:

#ifdef COMPILER_GENERATED
  ~ShapeSpec();
  ShapeSpec(const ShapeSpec & theValue);
  ShapeSpec & operator=(const ShapeSpec & theValue);
#endif

  ShapeSpec(const std::string & theShapeFile,
			Imagine::NFmiColorTools::Color theFill = Imagine::NFmiColorTools::MakeColor(0,0,0,127),
			Imagine::NFmiColorTools::Color theStroke = Imagine::NFmiColorTools::MakeColor(0,0,0,127),
			const std::string & theFillRule = "Copy",
			const std::string & theStrokeRule = "Copy");
  
  // Data-access
  
  const std::string & filename(void) const;
  const std::string & fillrule(void) const;
  const std::string & strokerule(void) const;
  Imagine::NFmiColorTools::Color fillcolor(void) const;
  Imagine::NFmiColorTools::Color strokecolor(void) const;
  
  void fillrule(const std::string & theRule);
  void strokerule(const std::string & theRule);
  void fillcolor(Imagine::NFmiColorTools::Color theColor);
  void strokecolor(Imagine::NFmiColorTools::Color theColor);
  
  void marker(const std::string & theMarker,
			  const std::string & theRule,
			  float theAlpha);
  
  const std::string & marker(void) const;
  const std::string & markerrule(void) const;
  float markeralpha(void) const;

private:

  ShapeSpec(void);

  std::string	itsShapeFileName;
  std::string	itsFillRule;
  std::string	itsStrokeRule;
  Imagine::NFmiColorTools::Color	itsFillColor;
  Imagine::NFmiColorTools::Color	itsStrokeColor;
  std::string	itsMarker;
  std::string	itsMarkerRule;
  float		itsMarkerAlpha;
  
}; // class ShapeSpec

#endif // SHAPESPEC_H

// ======================================================================
