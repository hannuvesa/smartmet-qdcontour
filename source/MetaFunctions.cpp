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

#include <boost/shared_ptr.hpp>

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

  NFmiDataMatrix<float> elevation_angle_values(LazyQueryData & theQI)
  {
	NFmiDataMatrix<float> values;

	boost::shared_ptr<NFmiDataMatrix<NFmiPoint> > pts = theQI.Locations();
	values.Resize(pts->NX(),pts->NY(),kFloatMissing);

	for(unsigned int j=0; j<pts->NY(); j++)
	  for(unsigned int i=0; i<pts->NX(); i++)
		{
		  NFmiLocation loc((*pts)[i][j]);
		  NFmiMetTime t(theQI.ValidTime());
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

  NFmiDataMatrix<float> wind_chill_values(LazyQueryData & theQI)
  {
	NFmiDataMatrix<float> t2m;
	NFmiDataMatrix<float> wspd;

	theQI.Param(kFmiTemperature);
	theQI.Values(t2m);
	theQI.Param(kFmiWindSpeedMS);
	theQI.Values(wspd);

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

  NFmiDataMatrix<float> dew_difference_values(LazyQueryData & theQI)
  {
	NFmiDataMatrix<float> tdew;
	NFmiDataMatrix<float> troad;

	theQI.Param(kFmiRoadTemperature);
	theQI.Values(troad);
	theQI.Param(kFmiDewPoint);
	theQI.Values(tdew);

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
   * \brief Return DewDifferenceAir matrix from given query info
   *
   * \param theQI The query info
   * \return The values in a matrix
   */
  // ----------------------------------------------------------------------

  NFmiDataMatrix<float> air_dew_difference_values(LazyQueryData & theQI)
  {
	NFmiDataMatrix<float> tdew;
	NFmiDataMatrix<float> t2m;

	theQI.Param(kFmiTemperature);
	theQI.Values(t2m);
	theQI.Param(kFmiDewPoint);
	theQI.Values(tdew);

	// overwrite troad with troad-tdew

	for(unsigned int j=0; j<t2m.NY(); j++)
	  for(unsigned int i=0; i<t2m.NX(); i++)
		{
		  if(t2m[i][j] == kFloatMissing)
			;
		  else if(tdew[i][j] == kFloatMissing)
			t2m[i][j] = kFloatMissing;
		  else
			t2m[i][j] -= tdew[i][j];
		}
	return t2m;
  }

  // ----------------------------------------------------------------------
  /*!
   * \brief Return N matrix from given query info
   *
   * \param theQI The query info
   * \return The values in a matrix
   */
  // ----------------------------------------------------------------------

  NFmiDataMatrix<float> n_cloudiness(LazyQueryData & theQI)
  {
	NFmiDataMatrix<float> n;
	theQI.Param(kFmiTotalCloudCover);
	theQI.Values(n);

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

  NFmiDataMatrix<float> nn_cloudiness(LazyQueryData & theQI)
  {
	NFmiDataMatrix<float> nn;
	theQI.Param(kFmiMiddleAndLowCloudCover);
	theQI.Values(nn);

	for(unsigned int j=0; j<nn.NY(); j++)
	  for(unsigned int i=0; i<nn.NX(); i++)
		nn[i][j] = eights(nn[i][j]);

	return nn;
  }

  // ----------------------------------------------------------------------
  /*!
   * \brief Calculate nabla of the given parameter
   *
   * \param theF The data to nabla
   * \param theDX The grid x-resolution
   * \param theDY The grid y-resolution
   * \param theNablaX The X-part of the nabla
   * \param theNablaY The Y-part of the nabla
   */
  // ----------------------------------------------------------------------

  void matrix_nabla(const NFmiDataMatrix<float> & theF,
					float theDX,
					float theDY,
					NFmiDataMatrix<float> & theNablaX,
					NFmiDataMatrix<float> & theNablaY)
  {
	theNablaX.Resize(theF.NX(),theF.NY(),kFloatMissing);
	theNablaY.Resize(theF.NX(),theF.NY(),kFloatMissing);

	for(unsigned int j=0; j<theF.NY(); j++)
	  for(unsigned int i=0; i<theF.NX(); i++)
		{
		  bool allok = theF[i][j] != kFloatMissing;
		  if(i>0)           allok &= theF[i-1][j] != kFloatMissing;
		  if(i<theF.NX()-1) allok &= theF[i+1][j] != kFloatMissing;
		  if(j>0)           allok &= theF[i][j-1] != kFloatMissing;
		  if(j<theF.NY()-1) allok &= theF[i][j+1] != kFloatMissing;
		  
		  if(allok)
			{
			  if(i==0)
				theNablaX[i][j] = (theF[i+1][j]-theF[i][j])/theDX;		// forward difference
			  else if(i==theF.NX()-1)
				theNablaX[i][j] = (theF[i][j]-theF[i-1][j])/theDX;		// backward difference
			  else
				theNablaX[i][j] = (theF[i+1][j]-theF[i-1][j])/(2*theDX);	// centered difference
			  
			  if(j==0)
				theNablaY[i][j] = (theF[i][j+1]-theF[i][j])/theDY;
			  else if(j==theF.NY()-1)
				theNablaY[i][j] = (theF[i][j]-theF[i][j-1])/theDY;
			  else
				theNablaY[i][j] = (theF[i][j+1]-theF[i][j-1])/(2*theDY);
			  
			}
		  else
			{
			  theNablaX[i][j] = kFloatMissing;
			  theNablaY[i][j] = kFloatMissing;
			}
		}
  }

  // ----------------------------------------------------------------------
  /*!
   * \brief Calculate absolute value of a vector
   *
   * \param theFX The X-part of the data
   * \param theFY The Y-part of the data
   * \param theResult The absolute value
   */
  // ----------------------------------------------------------------------

  void matrix_abs(const NFmiDataMatrix<float> & theX,
				  const NFmiDataMatrix<float> & theY,
				  NFmiDataMatrix<float> & theResult)
  {
	theResult.Resize(theX.NX(),theY.NY(),kFloatMissing);

	for(unsigned int j=0; j<theX.NY(); j++)
	  for(unsigned int i=0; i<theX.NX(); i++)
		{
		  const float x = theX[i][j];
		  const float y = theY[i][j];

		  if(x == kFloatMissing || y == kFloatMissing)
			theResult[i][j] = kFloatMissing;
		  else
			theResult[i][j] = sqrt(x*x+y*y);
		}
  }


  // ----------------------------------------------------------------------
  /*!
   * \brief Return T2m advection field
   *
   * \param theQI The query info
   * \return The values in a matrix
   */
  // ----------------------------------------------------------------------

  NFmiDataMatrix<float> t2m_advection(LazyQueryData & theQI)
  {
	NFmiDataMatrix<float> wspd;
	NFmiDataMatrix<float> wdir;
	NFmiDataMatrix<float> t2m;
	
	theQI.Param(kFmiTemperature);
	theQI.Values(t2m);
	theQI.Param(kFmiWindSpeedMS);
	theQI.Values(wspd);
	theQI.Param(kFmiWindDirection);
	theQI.Values(wdir);

	// advection = v dot nabla(t)
	// we overwrite wspd with the results
	
	// grid resolution in meters for difference formulas
	const float dx = (theQI.Area()->WorldXYWidth()) / (theQI.Grid()->XNumber());
	const float dy = (theQI.Area()->WorldXYHeight()) / (theQI.Grid()->YNumber());

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

  // ----------------------------------------------------------------------
  /*!
   * \brief Return Thermal Front Parameter
   *
   * \param theQI The query info
   * \return The values in a matrix
   */
  // ----------------------------------------------------------------------

  NFmiDataMatrix<float> thermal_front(LazyQueryData & theQI)
  {
	NFmiDataMatrix<float> t2m;
	theQI.Param(kFmiTemperature);
	theQI.Values(t2m);

	NFmiDataMatrix<float> tfp;
	tfp.Resize(t2m.NX(),t2m.NY(),kFloatMissing);

	// thermal front parameter = (-nabla |nabla T|) dot (nabla T /|nabla T|)

	// grid resolution in meters for difference formulas
	const float dx = (theQI.Area()->WorldXYWidth()) / (theQI.Grid()->XNumber());
	const float dy = (theQI.Area()->WorldXYHeight()) / (theQI.Grid()->YNumber());

	NFmiDataMatrix<float> nablatx, nablaty;
	matrix_nabla(t2m,dx,dy,nablatx,nablaty);

	NFmiDataMatrix<float> nablat;
	matrix_abs(nablatx,nablaty,nablat);

	NFmiDataMatrix<float> nablanablatx,nablanablaty;
	matrix_nabla(nablat,dx,dy,nablanablatx,nablanablaty);

	for(unsigned int j=0; j<t2m.NY(); j++)
 	  for(unsigned int i=0; i<t2m.NX(); i++)
		{
		  const float nntx = nablanablatx[i][j];
		  const float nnty = nablanablaty[i][j];
		  const float ntx = nablatx[i][j];
		  const float nty = nablaty[i][j];
		  const float nt = nablat[i][j];

		  if(nntx != kFloatMissing && nnty != kFloatMissing &&
			 ntx != kFloatMissing && nty != kFloatMissing &&
			 nt != kFloatMissing)
			{
			  // The 1e9 factor is there just to get a convenient scale

			  if(nt != 0)
				tfp[i][j] = - 1e9 * (nntx*ntx+nnty*nty)/nt;
			  else
				tfp[i][j] = 0;
			}
		}
	return tfp;
  }

  // ----------------------------------------------------------------------
  /*!
   * \brief Probability of snow according to the Gospel of Elina Saltikoff
   *
   * \param theQI The queryinfo
   * \return The valeus in a matrix
   */
  // ----------------------------------------------------------------------

  NFmiDataMatrix<float> snowprob(LazyQueryData & theQI)
  {
	NFmiDataMatrix<float> t2m;
	NFmiDataMatrix<float> rh;

	theQI.Param(kFmiTemperature);
	theQI.Values(t2m);
	theQI.Param(kFmiHumidity);
	theQI.Values(rh);

	// overwrite t2m with snowprob

	for(unsigned int j=0; j<t2m.NY(); j++)
	  for(unsigned int i=0; i<t2m.NX(); i++)
		{
		  if(t2m[i][j] == kFloatMissing)
			;
		  else if(rh[i][j] == kFloatMissing)
			t2m[i][j] = kFloatMissing;
		  else
			t2m[i][j] = 100*(1-1/(1+exp(22-2.7*t2m[i][j]-0.2*rh[i][j])));
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
	if(theFunction == "MetaThermalFront")
	  return 10006;
	if(theFunction == "MetaDewDifferenceAir")
	  return 10007;
	if(theFunction == "MetaSnowProb")
	  return 10008;
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
							   LazyQueryData & theQI)
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
	if(theFunction == "MetaThermalFront")
	  return thermal_front(theQI);
	if(theFunction == "MetaDewDifferenceAir")
	  return air_dew_difference_values(theQI);
	if(theFunction == "MetaSnowProb")
	  return snowprob(theQI);

	throw runtime_error("Unrecognized meta function " + theFunction);

  }

} // namespace MetaFunctions

// ======================================================================
