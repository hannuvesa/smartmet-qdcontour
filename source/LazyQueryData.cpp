// ======================================================================
/*!
 * \file
 * \brief Implementation of class LazyQueryData
 */
// ======================================================================

#include "LazyQueryData.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiFileSystem.h"
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

  NFmiFastQueryInfo * oldinfo = itsInfo.release();

  ifstream in(itsDataFile.c_str(), ios::in|ios::binary);
  if(!in) throw runtime_error("File "+itsDataFile+" was lost");

  NFmiQueryData * qdata = new NFmiQueryData;
  in >> *qdata;
  in.close();

  itsData.reset(qdata);
  itsInfo.reset(new NFmiFastQueryInfo(itsData.get()));
  itsInfo->SetDescriptors(oldinfo,false);
  delete oldinfo;

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
			  time_t modtime = FileModificationTime(filename);
			  if(modtime > newesttime)
				{
				  newesttime = modtime;
				  newestfile = tmpfile;
				}
			}
		}
	  filename = newestfile;
	}

  NFmiQueryInfo qinfo;
  ifstream in(filename.c_str(), ios::in|ios::binary);
  if(!in) throw runtime_error("Could not open "+filename);
  in >> qinfo;
  in.close();

  itsDataFile = filename;

  itsInfo.reset(new NFmiFastQueryInfo(qinfo));
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
  throw runtime_error("LatLonToGrid not implemented yet");
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

