// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace MetaFunctions
 */
// ======================================================================

#include "MetaFunctions.h"
#include "newbase/NFmiArea.h"
#include "newbase/NFmiGrid.h"
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
   * \brief Convert cloudiness value in range 0-100 to value 0-8
   */
  // ----------------------------------------------------------------------

  inline
  float eights(float theCloudiness)
  {
	if(theCloudiness == kFloatMissing)
	  return kFloatMissing;
	else
	  return FmiRound(theCloudiness/100*8);
  }

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

  // ----------------------------------------------------------------------
  /*!
   * \brief Return DewDifference matrix from given query info
   *
   * \param theQI The query info
   * \return The values in a matrix
   */
  // ----------------------------------------------------------------------

  NFmiDataMatrix<float> dew_difference_values(LazyQueryData * theQI)
  {
	NFmiDataMatrix<float> tdew;
	NFmiDataMatrix<float> troad;

	theQI->Param(kFmiRoadTemperature);
	theQI->Values(troad);
	theQI->Param(kFmiDewPoint);
	theQI->Values(tdew);

	// overwrite troad with troad-tdew

	for(unsigned int j=0; j<troad.NY(); j++)
	  for(unsigned int i=0; i<troad.NX(); i++)
		{
		  if(troad[i][j] == kFloatMissing)
			;
		  else if(tdew[i][j] == kFloatMissing)
			troad[i][j] = kFloatMissing;
		  else
			troad[i][j] -= tdew[i][j];
		}
	return troad;
  }

  // ----------------------------------------------------------------------
  /*!
   * \brief Return N matrix from given query info
   *
   * \param theQI The query info
   * \return The values in a matrix
   */
  // ----------------------------------------------------------------------

  NFmiDataMatrix<float> n_cloudiness(LazyQueryData * theQI)
  {
	NFmiDataMatrix<float> n;
	theQI->Param(kFmiTotalCloudCover);
	theQI->Values(n);

	for(unsigned int j=0; j<n.NY(); j++)
	  for(unsigned int i=0; i<n.NX(); i++)
		n[i][j] = eights(n[i][j]);
	return n;
  }

  // ----------------------------------------------------------------------
  /*!
   * \brief Return NN matrix from given query info
   *
   * \param theQI The query info
   * \return The values in a matrix
   */
  // ----------------------------------------------------------------------

  NFmiDataMatrix<float> nn_cloudiness(LazyQueryData * theQI)
  {
	NFmiDataMatrix<float> nn;
	theQI->Param(kFmiMiddleAndLowCloudCover);
	theQI->Values(nn);

	for(unsigned int j=0; j<nn.NY(); j++)
	  for(unsigned int i=0; i<nn.NX(); i++)
		nn[i][j] = eights(nn[i][j]);

	return nn;
  }

  // ----------------------------------------------------------------------
  /*!
   * \brief Return T2m advection field
   *
   * \param theQI The query info
   * \return The values in a matrix
   */
  // ----------------------------------------------------------------------

  NFmiDataMatrix<float> t2m_advection(LazyQueryData * theQI)
  {
	NFmiDataMatrix<float> wspd;
	NFmiDataMatrix<float> wdir;
	NFmiDataMatrix<float> t2m;
	
	theQI->Param(kFmiTemperature);
	theQI->Values(t2m);
	theQI->Param(kFmiWindSpeedMS);
	theQI->Values(wspd);
	theQI->Param(kFmiWindDirection);
	theQI->Values(wdir);

	// advection = v dot nabla(t)
	// we overwrite wspd with the results
	
	// grid resolution in meters for difference formulas
	const float dx = (theQI->Area()->WorldXYWidth()) / (theQI->Grid()->XNumber());
	const float dy = (theQI->Area()->WorldXYHeight()) / (theQI->Grid()->YNumber());

	const float pirad=3.14159265358979323/360;

	for(unsigned int j=0; j<t2m.NY(); j++)
	  for(unsigned int i=0; i<t2m.NX(); i++)
		{
		  const float ff = wspd[i][j];
		  const float fd = wdir[i][j];

		  wspd[i][j] = kFloatMissing;

		  if(ff != kFloatMissing && fd != kFloatMissing)
			{
			  bool allok = t2m[i][j] != kFloatMissing;
			  if(i>0)          allok &= t2m[i-1][j] != kFloatMissing;
			  if(i<t2m.NX()-1) allok &= t2m[i+1][j] != kFloatMissing;
			  if(j>0)          allok &= t2m[i][j-1] != kFloatMissing;
			  if(j<t2m.NY()-1) allok &= t2m[i][j+1] != kFloatMissing;

			  if(allok)
				{
				  float tx, ty;
				  if(i==0)
					tx = (t2m[i+1][j]-t2m[i][j])/dx;		// forward difference
				  else if(i==t2m.NX()-1)
					tx = (t2m[i][j]-t2m[i-1][j])/dx;		// backward difference
				  else
					tx = (t2m[i+1][j]-t2m[i-1][j])/(2*dx);	// centered difference

				  if(j==0)
					ty = (t2m[i][j+1]-t2m[i][j])/dy;
				  else if(j==t2m.NY()-1)
					ty = (t2m[i][j]-t2m[i][j-1])/dy;
				  else
					ty = (t2m[i][j+1]-t2m[i][j-1])/(2*dy);

				  const float adv = -ff*(cos(fd*pirad)*tx + sin(fd*pirad)*ty)*3600;	// degrees/hour

				  wspd[i][j] = adv;
				}
			}

		}
	return wspd;
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
	return (id(theFunction) != 0);
  }

  // ----------------------------------------------------------------------
  /*!
   * \brief Assign ID for meta functions
   *
   * \param theFunction The function
   * \return The ID, or 0 for a bad parameter
   */
  // ----------------------------------------------------------------------

  int id(const std::string & theFunction)
  {
	if(theFunction == "MetaElevationAngle")
	  return 10000;
	if(theFunction == "MetaWindChill")
	  return 10001;
	if(theFunction == "MetaDewDifference")
	  return 10002;
	if(theFunction == "MetaN")
	  return 10003;
	if(theFunction == "MetaNN")
	  return 10004;
	if(theFunction == "MetaT2mAdvection")
	  return 10005;
	return 0;
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
	if(theFunction == "MetaDewDifference")
	  return dew_difference_values(theQI);
	if(theFunction == "MetaN")
	  return n_cloudiness(theQI);
	if(theFunction == "MetaNN")
	  return nn_cloudiness(theQI);
	if(theFunction == "MetaT2mAdvection")
	  return t2m_advection(theQI);

	throw runtime_error("Unrecognized meta function " + theFunction);

  }

} // namespace MetaFunctions

// ======================================================================
