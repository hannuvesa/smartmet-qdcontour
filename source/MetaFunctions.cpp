// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace MetaFunctions
 */
// ======================================================================

#include "MetaFunctions.h"
#include "newbase/NFmiLocation.h"
#include "newbase/NFmiMetMath.h"
#include "newbase/NFmiMetTime.h"
#include "newbase/NFmiPoint.h"

#include "boost/shared_ptr.hpp"

#include <iostream>
#include <stdexcept>

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

  NFmiDataMatrix<float> elevation_angle_values(LazyQueryData * theQI)
  {
	NFmiDataMatrix<float> values;

	boost::shared_ptr<NFmiDataMatrix<NFmiPoint> > pts = theQI->Locations();
	values.Resize(pts->NX(),pts->NY(),kFloatMissing);

	for(unsigned int j=0; j<pts->NY(); j++)
	  for(unsigned int i=0; i<pts->NX(); i++)
		{
		  NFmiLocation loc((*pts)[i][j]);
		  NFmiMetTime t(theQI->ValidTime());
		  double angle = loc.ElevationAngle(t);
		  values[i][j] = static_cast<float>(angle);
		}
	return values;
  }

  // ----------------------------------------------------------------------
  /*!
   * \brief Return WindChill matrix from given query info
   *
   * \param theQI The query info
   * \return The values in a matrix
   */
  // ----------------------------------------------------------------------

  NFmiDataMatrix<float> wind_chill_values(LazyQueryData * theQI)
  {
	NFmiDataMatrix<float> t2m;
	NFmiDataMatrix<float> wspd;

	theQI->Param(kFmiTemperature);
	theQI->Values(t2m);
	theQI->Param(kFmiWindSpeedMS);
	theQI->Values(wspd);

	// overwrite t2m with wind chill

	for(unsigned int j=0; j<t2m.NY(); j++)
	  for(unsigned int i=0; i<t2m.NX(); i++)
		{
		  t2m[i][j] = FmiWindChill(wspd[i][j],t2m[i][j]);
		}
	return t2m;
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
	if(theFunction == "MetaWindChill")
	  return true;
	return false;
  }

  // ----------------------------------------------------------------------
  /*!
   * \brief Return the function values for the given meta function
   *
   * An exception is thrown if the name is not recognized. One should
   * always test with isMeta first.
   *
   * \param theFunction The function name
   * \param theQI The query info
   * \return A matrix of function values
   */
  // ----------------------------------------------------------------------

  NFmiDataMatrix<float> values(const std::string & theFunction,
							   LazyQueryData * theQI)
  {
	if(theFunction == "MetaElevationAngle")
	  return elevation_angle_values(theQI);
	if(theFunction == "MetaWindChill")
	  return wind_chill_values(theQI);

	throw runtime_error("Unrecognized meta function " + theFunction);

  }

} // namespace MetaFunctions

// ======================================================================
