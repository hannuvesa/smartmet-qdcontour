// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace MetaFunctions
 */
// ======================================================================

#include "MetaFunctions.h"
#include "NFmiLocation.h"
#include "NFmiMetTime.h"
#include "NFmiPoint.h"
#include <iostream>

using namespace std;

namespace
{
  // ----------------------------------------------------------------------
  /*!
   * \brief Return ElevationAngle matrix from given query info
   *
   * \param theQI The query info
   * \return The values in a matrix
   */
  // ----------------------------------------------------------------------

  NFmiDataMatrix<float> elevation_angle_values(NFmiFastQueryInfo * theQI)
  {
	NFmiDataMatrix<float> values;
	NFmiDataMatrix<NFmiPoint> pts;
	theQI->Locations(pts);
	values.Resize(pts.NX(),pts.NY(),kFloatMissing);

	for(unsigned int j=0; j<pts.NY(); j++)
	  for(unsigned int i=0; i<pts.NX(); i++)
		{
		  NFmiLocation loc(pts[i][j]);
		  NFmiMetTime t(theQI->ValidTime());
		  double angle = loc.ElevationAngle(t);
		  values[i][j] = static_cast<float>(angle);
		}
	return values;
  }

} // namespace anonymous


namespace MetaFunctions
{

  // ----------------------------------------------------------------------
  /*!
   * \brief Test if the given function name is a meta function
   *
   * \param theFunction The function name
   * \return True, if the function is a meta function
   */
  // ----------------------------------------------------------------------
  
  bool isMeta(const std::string & theFunction)
  {
	if(theFunction == "MetaElevationAngle")
	  return true;
	return false;
  }

  // ----------------------------------------------------------------------
  /*!
   * \brief Return the function values for the given meta function
   *
   * \param theFunction The function name
   * \param theQI The query info
   * \return A matrix of function values
   */
  // ----------------------------------------------------------------------

  NFmiDataMatrix<float> values(const std::string & theFunction,
							   NFmiFastQueryInfo * theQI)
  {
	if(theFunction == "MetaElevationAngle")
	  return elevation_angle_values(theQI);

	cerr << "Error: Unrecognized meta function " << theFunction << endl;
	exit(1);
  }

} // namespace MetaFunctions

// ======================================================================
