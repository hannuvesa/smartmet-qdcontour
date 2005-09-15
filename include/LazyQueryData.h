// ======================================================================
/*!
 * \file
 * \brief Interface of class LazyQueryData
 */
// ======================================================================
/*!
 * \class LazyQueryData
 * \brief Non-greedy access to querydata
 *
 * The purpose of LazuQueryData is to provide access to the query data
 * in a specific file without reading the full file until absolutely
 * necessary.
 *
 * The basic idea is to always read the header, but the data part
 * only when it is required.
 *
 */
// ======================================================================

#ifndef LAZYQUERYDATA_H
#define LAZYQUERYDATA_H

#include "newbase/NFmiDataMatrix.h"
#include "newbase/NFmiParameterName.h"
#include <boost/shared_ptr.hpp>
#include <memory>
#include <string>

class NFmiArea;
class NFmiFastQueryInfo;
class NFmiGrid;
class NFmiLevel;
class NFmiMetTime;
class NFmiPoint;
class NFmiQueryData;

class LazyQueryData
{
public:
  ~LazyQueryData();
  LazyQueryData();

  bool IsOutdated() const;

  const std::string & Filename() const { return itsDataFile; }

  std::string GetParamName() const;
  unsigned long GetParamIdent() const;

  // These do not require the data values

  void Read(const std::string & theDataFile);

  void ResetTime();
  void ResetLevel();
  bool FirstLevel();
  bool FirstTime();
  bool LastTime();
  bool NextLevel();
  bool NextTime();
  bool PreviousTime();
  const NFmiLevel * Level() const;

  bool Param(FmiParameterName theParam);

  // LastTime();
  const NFmiMetTime & ValidTime() const;
  const NFmiMetTime & OriginTime() const;

  bool IsParamUsable() const;

  typedef NFmiDataMatrix<NFmiPoint> Coordinates;

  boost::shared_ptr<Coordinates> Locations() const;
  boost::shared_ptr<Coordinates> LocationsWorldXY(const NFmiArea & theArea) const;
  boost::shared_ptr<Coordinates> LocationsXY(const NFmiArea & theArea) const;

  bool BiLinearInterpolation(double x, double y, float & theValue,
							 float topLeftValue, float topRightValue,
							 float bottomLeftValue, float bottomRightValue);
  
  // Grid()

  NFmiPoint LatLonToGrid(const NFmiPoint & theLatLonPoint);
  const NFmiGrid * Grid(void) const;
  const NFmiArea * Area(void) const;

  // These require the data values

  float InterpolatedValue(const NFmiPoint & theLatLonPoint);

  void Values(NFmiDataMatrix<float> & theValues);
  void Values(NFmiDataMatrix<float> & theValues, const NFmiMetTime & theTime);

private:

  LazyQueryData(const LazyQueryData & theQD);
  LazyQueryData & operator=(const LazyQueryData & theQD);

  void requireInfo() const;
  void requireData();

  std::string itsInputName;
  std::string itsDataFile;
  std::auto_ptr<NFmiFastQueryInfo> itsInfo;
  std::auto_ptr<NFmiQueryData> itsData;

  mutable boost::shared_ptr<Coordinates> itsLocations;
  mutable boost::shared_ptr<Coordinates> itsLocationsWorldXY;
  mutable boost::shared_ptr<Coordinates> itsLocationsXY;
  mutable std::string itsLocationsArea;

}; // class LazyQueryData

#endif // LAZYQUERYDATA_H

// ======================================================================

