// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace ProjectionFactory
 */
// ======================================================================

#include "ProjectionFactory.h"
// newbase
#include "NFmiPoint.h"
#include "NFmiStereographicArea.h"
// system
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
											NFmiPoint & theBottomLeft,
											NFmiPoint & theTopRight,
											int & theWidth,
											int & theHeight)
  {

	if(theBottomLeft.X()==kFloatMissing ||
	   theBottomLeft.Y()==kFloatMissing ||
	   theTopRight.X()==kFloatMissing ||
	   theTopRight.Y()==kFloatMissing)
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
	  throw runtime_error("Image width & height unspecified");
	
	// The actual area we wanted
	
	NFmiStereographicArea theArea(theBottomLeft,
								  theTopRight,
								  theCentralLongitude,
								  NFmiPoint(0,0),
								  NFmiPoint(theWidth,theHeight),
								  theCentralLatitude,
								  theTrueLatitude);

	return theArea;
  }

} // namespace ProjectionFactory

// ======================================================================
