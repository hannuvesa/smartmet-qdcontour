// ======================================================================
/*!
 * \file
 * \brief Implementation of class LazyQueryData
 */
// ======================================================================

#include "LazyQueryData.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiFileSystem.h"
#include "NFmiGrid.h"
#include "NFmiQueryData.h"
#include <fstream>
#include <stdexcept>

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
  itsInfo.reset(0);
  itsData.reset(0);

  string filename = theDataFile;

  if(DirectoryExists(filename))
	{
	  const string & dirname = filename;
	  list<string> files = DirectoryFiles(filename);
	  if(files.empty())
		throw runtime_error("Directory "+filename+" is empty");
	  string newestfile;
	  time_t newesttime = 0;

	  for(list<string>::const_iterator it=files.begin(); it!=files.end(); ++it)
		{
		  string tmpfile = dirname + '/' + *it;
		  if(FileReadable(tmpfile))
			{
			  time_t modtime = FileModificationTime(tmpfile);
			  if(modtime > newesttime)
				{
				  newesttime = modtime;
				  newestfile = tmpfile;
				}
			}
		}
	  filename = newestfile;
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

void LazyQueryData::Locations(NFmiDataMatrix<NFmiPoint> & theMatrix) const
{
  requireInfo();
  itsInfo->Locations(theMatrix);
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void LazyQueryData::LocationsWorldXY(NFmiDataMatrix<NFmiPoint> & theMatrix,
									 const NFmiArea & theArea) const
{
  requireInfo();
  itsInfo->LocationsWorldXY(theMatrix, theArea);
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void LazyQueryData::LocationsXY(NFmiDataMatrix<NFmiPoint> & theMatrix,
								const NFmiArea & theArea) const
{
  requireInfo();
  itsInfo->LocationsXY(theMatrix, theArea);
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

// ======================================================================

