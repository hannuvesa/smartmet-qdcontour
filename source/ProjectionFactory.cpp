// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace ProjectionFactory
 */
// ======================================================================

#include "ProjectionFactory.h"
#include "newbase/NFmiPoint.h"
#include "newbase/NFmiStereographicArea.h"
#include <stdexcept>

using namespace std;

namespace ProjectionFactory
{

  // ----------------------------------------------------------------------
  /*!
   * \brief Create a stereographic projection
   *
   */
  // ----------------------------------------------------------------------

  NFmiStereographicArea createStereographic(double theCentralLongitude,
											double theCentralLatitude,
											double theTrueLatitude,
											const NFmiPoint & theCenter,
											float theScale,
											const NFmiPoint & theBottomLeft,
											const NFmiPoint & theTopRight,
											int & theWidth,
											int & theHeight)
  {

	NFmiPoint bottomleft = theBottomLeft;
	NFmiPoint topright = theTopRight;

	if(bottomleft.X()==kFloatMissing ||
	   bottomleft.Y()==kFloatMissing ||
	   topright.X()==kFloatMissing ||
	   topright.Y()==kFloatMissing)
	  {
		
		if(theCenter.X()==kFloatMissing || theCenter.Y()==kFloatMissing)
		  throw runtime_error("Area corner coordinates not given");
		
		if(theScale<0 || theWidth<0 || theHeight<0)
		  throw runtime_error("scale, width and height must be given along with center coordinates");
		
		NFmiStereographicArea area(theCenter,theCenter,
								   theCentralLongitude,
								   NFmiPoint(0,0),
								   NFmiPoint(1,1),
								   theCentralLatitude,
								   theTrueLatitude);
		
		NFmiPoint c = area.LatLonToWorldXY(theCenter);
		
		NFmiPoint bl(c.X()-theScale*1000*theWidth, c.Y()-theScale*1000*theHeight);
		NFmiPoint tr(c.X()+theScale*1000*theWidth, c.Y()+theScale*1000*theHeight);
		
		bottomleft = area.WorldXYToLatLon(bl);
		topright = area.WorldXYToLatLon(tr);

	  }		

	// Initialize XY-coordinates
	
	NFmiStereographicArea area(bottomleft,
							   topright,
							   theCentralLongitude,
							   NFmiPoint(0,0),
							   NFmiPoint(1,1),
							   theCentralLatitude,
							   theTrueLatitude);
	
	// Calculate world coordinates
	
	NFmiPoint bl = area.LatLonToWorldXY(bottomleft);
	NFmiPoint tr = area.LatLonToWorldXY(topright);
	
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
	  throw runtime_error("Image width & height unspecified");
	
	// The actual area we wanted
	
	NFmiStereographicArea theArea(bottomleft,
								  topright,
								  theCentralLongitude,
								  NFmiPoint(0,0),
								  NFmiPoint(theWidth,theHeight),
								  theCentralLatitude,
								  theTrueLatitude);

	return theArea;
  }

} // namespace ProjectionFactory

// ======================================================================
