// ======================================================================
/*!
 * \file
 * \brief Implementation of class LazyQueryData
 */
// ======================================================================

#include "LazyQueryData.h"
#include "newbase/NFmiFastQueryInfo.h"
#include "newbase/NFmiFileSystem.h"
#include "newbase/NFmiGrid.h"
#include "newbase/NFmiQueryData.h"
#include <fstream>
#include <stdexcept>
#include <sstream>

using namespace std;

// ----------------------------------------------------------------------
/*!
 * \brief Destructor
 */
// ----------------------------------------------------------------------

LazyQueryData::~LazyQueryData()
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 */
// ----------------------------------------------------------------------

LazyQueryData::LazyQueryData()
  : itsInfo()
  , itsData()
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Require info has been read
 */
// ----------------------------------------------------------------------

void LazyQueryData::requireInfo() const
{
  if(itsInfo.get()!=0) return;
  throw runtime_error("Trying to querydata before reading any");
}

// ----------------------------------------------------------------------
/*!
 * \brief Require data has been read
 */
// ----------------------------------------------------------------------

void LazyQueryData::requireData()
{
  if(itsInfo.get()==0)
	throw runtime_error("Trying to access data before reading anything");
  if(itsData.get()!=0)
	return;

  // Now we must read the data so that the query info remains valid.

  throw runtime_error("Error: requireData() is no longer needed, see Read()");
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the parameter name
 *
 * \return The parameter name
 */
// ----------------------------------------------------------------------

std::string LazyQueryData::GetParamName() const
{
  requireInfo();
  return (itsInfo->Param().GetParamName().CharPtr());
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the parameter ID number
 *
 * \return The number
 */
// ----------------------------------------------------------------------

unsigned long LazyQueryData::GetParamIdent() const
{
  requireInfo();
  return (itsInfo->Param().GetParamIdent());
}

// ----------------------------------------------------------------------
/*!
 * \brief Lazy-read the given query data file
 *
 * Throws if an error occurs.
 *
 * \param theDataFile The filename (or directory) to read
 */
// ----------------------------------------------------------------------

void LazyQueryData::Read(const std::string & theDataFile)
{
  itsInputName = theDataFile;

  itsInfo.reset(0);
  itsData.reset(0);

  string filename = theDataFile;

  if(NFmiFileSystem::DirectoryExists(filename))
	{
	  filename = NFmiFileSystem::NewestFile(filename);
	  if(filename.empty())
		throw runtime_error("Directory "+filename+" is missing or empty");
	}

  NFmiQueryData * qdata = new NFmiQueryData;
  ifstream in(filename.c_str(), ios::in|ios::binary);
  if(!in) throw runtime_error("Could not open "+filename);
  in >> *qdata;
  in.close();

  itsDataFile = filename;

  itsData.reset(qdata);
  itsInfo.reset(new NFmiFastQueryInfo(itsData.get()));
}

// ----------------------------------------------------------------------
/*!
 * \brief Return true, if the original querydata is outdated
 */
// ----------------------------------------------------------------------

bool LazyQueryData::IsOutdated() const
{
  // Not outdated if not read yet
  if(itsInputName.empty())
	return false;

  // Fixed files never become outdated

  if(itsInputName == itsDataFile)
	return false;

  // Weird case - but makes this check safer
  if(!NFmiFileSystem::DirectoryExists(itsInputName))
	return false;

  // And finally the actual case of a changed datafile

  string filename = NFmiFileSystem::NewestFile(filename);
  return (filename != itsDataFile);

}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void LazyQueryData::ResetTime()
{
  requireInfo();
  itsInfo->ResetTime();
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void LazyQueryData::ResetLevel()
{
  requireInfo();
  itsInfo->ResetLevel();
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

bool LazyQueryData::FirstLevel()
{
  requireInfo();
  return itsInfo->FirstLevel();
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

bool LazyQueryData::FirstTime()
{
  requireInfo();
  return itsInfo->FirstTime();
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

bool LazyQueryData::LastTime()
{
  requireInfo();
  return itsInfo->LastTime();
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

bool LazyQueryData::NextLevel()
{
  requireInfo();
  return itsInfo->NextLevel();
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

const NFmiLevel * LazyQueryData::Level() const
{
  requireInfo();
  return itsInfo->Level();
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

bool LazyQueryData::NextTime()
{
  requireInfo();
  return itsInfo->NextTime();
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

bool LazyQueryData::PreviousTime()
{
  requireInfo();
  return itsInfo->PreviousTime();
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

bool LazyQueryData::Param(FmiParameterName theParam)
{
  requireInfo();
  return itsInfo->Param(theParam);
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

const NFmiMetTime & LazyQueryData::ValidTime() const
{
  requireInfo();
  return itsInfo->ValidTime();
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

const NFmiMetTime & LazyQueryData::OriginTime() const
{
  requireInfo();
  return itsInfo->OriginTime();
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

bool LazyQueryData::IsParamUsable() const
{
  requireInfo();
  return itsInfo->IsParamUsable();
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

boost::shared_ptr<LazyQueryData::Coordinates>
LazyQueryData::Locations() const
{
  requireInfo();
  if(itsLocations.get() == 0)
	{
	  itsLocations.reset(new Coordinates);
	  itsInfo->Locations(*itsLocations);
	}
  return itsLocations;
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

boost::shared_ptr<LazyQueryData::Coordinates>
LazyQueryData::LocationsWorldXY(const NFmiArea & theArea) const
{
  requireInfo();

  ostringstream os;
  os << theArea;

  if(itsLocationsWorldXY.get() == 0 || os.str() != itsLocationsArea)
	{
	  itsLocationsArea = os.str();
	  itsLocationsWorldXY.reset(new Coordinates);
	  itsInfo->LocationsWorldXY(*itsLocationsWorldXY, theArea);
	}
  return itsLocationsWorldXY;
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

boost::shared_ptr<LazyQueryData::Coordinates>
LazyQueryData::LocationsXY(const NFmiArea & theArea) const
{
  requireInfo();

  ostringstream os;
  os << theArea;

  if(itsLocationsXY.get() == 0 || os.str() != itsLocationsArea)
	{
	  itsLocationsArea = os.str();
	  itsLocationsXY.reset(new Coordinates);
	  itsInfo->LocationsXY(*itsLocationsXY, theArea);
	}
  return itsLocationsXY;
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

bool LazyQueryData::BiLinearInterpolation(double x, double y, float & theValue,
										  float topLeftValue, float topRightValue,
										  float bottomLeftValue, float bottomRightValue)
{
  requireInfo();
  return itsInfo->BiLinearInterpolation(x,y,theValue,
										topLeftValue, topRightValue,
										bottomLeftValue, bottomRightValue);
}
  
// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

NFmiPoint LazyQueryData::LatLonToGrid(const NFmiPoint & theLatLonPoint)
{
  requireInfo();
  return itsInfo->Grid()->LatLonToGrid(theLatLonPoint);
}


// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

const NFmiGrid * LazyQueryData::Grid(void) const
{
  requireInfo();
  return itsInfo->Grid();
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

float LazyQueryData::InterpolatedValue(const NFmiPoint & theLatLonPoint)
{
  requireData();
  return itsInfo->InterpolatedValue(theLatLonPoint);
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void LazyQueryData::Values(NFmiDataMatrix<float> & theValues)
{
  requireData();
  itsInfo->Values(theValues);
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void LazyQueryData::Values(NFmiDataMatrix<float> & theValues,
						   const NFmiMetTime & theTime)
{
  requireData();
  itsInfo->Values(theValues,theTime);
}

// ======================================================================

