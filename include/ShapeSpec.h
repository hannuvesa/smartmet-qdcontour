// ----------------------------------------------------------------------
// Yksittäisen shapen piirto-ohjeet
// ----------------------------------------------------------------------

#include <string>

class ShapeSpec
{
public:
  ShapeSpec(const std::string & shapefile,
			NFmiColorTools::Color fill = NFmiColorTools::MakeColor(0,0,0,127),
			NFmiColorTools::Color stroke = NFmiColorTools::MakeColor(0,0,0,127),
			const std::string & fillrule = "Copy",
			const std::string & strokerule = "Copy")
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
  
  const std::string & FileName(void) const		{ return itsShapeFileName; }
  const std::string & FillRule(void) const		{ return itsFillRule; }
  const std::string & StrokeRule(void) const		{ return itsStrokeRule; }
  NFmiColorTools::Color FillColor(void) const	{ return itsFillColor; }
  NFmiColorTools::Color StrokeColor(void) const	{ return itsStrokeColor; }
  
  void FillRule(const std::string & rule)		{ itsFillRule = rule; }
  void StrokeRule(const std::string & rule)		{ itsStrokeRule = rule; }
  void FillColor(NFmiColorTools::Color color)	{ itsFillColor = color; }
  void StrokeColor(NFmiColorTools::Color color)	{ itsStrokeColor = color; }
  
  void Marker(const std::string & marker, const std::string & rule, float alpha)
  {
    itsMarker = marker;
    itsMarkerRule = rule;
    itsMarkerAlpha = alpha;
  }
  
  const std::string & Marker(void) const		{ return itsMarker; }
  const std::string & MarkerRule(void) const		{ return itsMarkerRule; }
  float MarkerAlpha(void) const			{ return itsMarkerAlpha; }
private:
  ShapeSpec(void);
  std::string	itsShapeFileName;
  std::string	itsFillRule;
  std::string	itsStrokeRule;
  NFmiColorTools::Color	itsFillColor;
  NFmiColorTools::Color	itsStrokeColor;
  std::string	itsMarker;
  std::string	itsMarkerRule;
  float		itsMarkerAlpha;
  
};
