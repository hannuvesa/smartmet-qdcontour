// ======================================================================
/*!
 * \brief Tools for handling meridian shifting near 180th meridian
 */
// ======================================================================

#include "MeridianTools.h"
#include "NFmiEdgeTree.h"
#include <set>

using namespace Imagine;

// Local utilities
namespace
{

  // ----------------------------------------------------------------------
  /*!
   * \brief Test whether the area implies a shifted meridian
   */
  // ----------------------------------------------------------------------

  bool centralmeridian(const NFmiArea & theArea)
  {
	// the area longitudes

	double x1 = theArea.BottomLeftLatLon().X();
	double x2 = theArea.TopRightLatLon().X();

	// make sure the corners are in incremental order
	if(x2 < x1) x2+=360;

	if(x1 < -180 && x2 >= -180)
	  return -180;
	if(x1 <= 180 && x2 > 180)
	  return 180;
	return 0;
  }

  // ----------------------------------------------------------------------
  /*!
   * \brief Calculate the required shift for a polygon
   */
  // ----------------------------------------------------------------------

  float centralmeridian(const NFmiPath & thePath, const NFmiArea & theArea)
  {
	if(thePath.Empty())
	  return 0;

	return centralmeridian(theArea);
  }

} // anonymous namespace

namespace MeridianTools
{
  // ----------------------------------------------------------------------
  /*!
   * \brief Handle a point
   */
  // ----------------------------------------------------------------------

  NFmiPoint Relocate(const NFmiPoint & thePoint, const NFmiArea & theArea)
  {
	const float meridian = centralmeridian(theArea);
	if(meridian == 0)
	  return thePoint;

	if(std::abs(thePoint.X()-meridian) < 180)
	  return thePoint;

	const float shift = (meridian < 0 ? -360 : 360);
	return NFmiPoint(thePoint.X()+shift,thePoint.Y());

  }

  // ----------------------------------------------------------------------
  /*!
   * \brief Handle a path object
   */
  // ----------------------------------------------------------------------

  void Relocate(NFmiPath & thePath, const NFmiArea & theArea)
  {
	const float meridian = centralmeridian(thePath,theArea);
	if(meridian == 0)
	  return;

	NFmiPath path;
	const float shift = (meridian < 0 ? -360 : 360);

	for(NFmiPathData::const_iterator it = thePath.Elements().begin();
		it!=thePath.Elements().end();
		++it)
	  {
		switch(it->Oper())
		  {
		  case kFmiMoveTo:
			path.MoveTo(it->X()+shift,it->Y());
			break;
		  case kFmiLineTo:
			path.LineTo(it->X()+shift,it->Y());
			break;
		  case kFmiGhostLineTo:
			path.GhostLineTo(it->X()+shift,it->Y());
			break;
		  default:
			break;
		  }
	  }
	thePath.Add(path);
  }

} // MeridianTools

// ======================================================================
