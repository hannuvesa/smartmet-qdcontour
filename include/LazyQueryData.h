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
#include <memory>
#include <string>

class NFmiArea;
class NFmiFastQueryInfo;
class NFmiGrid;
class NFmiMetTime;
class NFmiPoint;
class NFmiQueryData;

class LazyQueryData
{
public:
  ~LazyQueryData();
  LazyQueryData();

  const std::string & Filename() const { return itsDataFile; }

  std::string GetParamName() const;
  unsigned long GetParamIdent() const;

  // These do not require the data values

  void Read(const std::string & theDataFile);

  void ResetTime();
  bool FirstLevel();
  bool FirstTime();
  bool LastTime();
  bool NextLevel();
  bool NextTime();
  bool PreviousTime();

  bool Param(FmiParameterName theParam);

  // LastTime();
  const NFmiMetTime & ValidTime() const;
  const NFmiMetTime & OriginTime() const;

  bool IsParamUsable() const;

  void Locations(NFmiDataMatrix<NFmiPoint> & theMatrix) const;
  void LocationsWorldXY(NFmiDataMatrix<NFmiPoint> & theMatrix, const NFmiArea & theArea) const;
  void LocationsXY(NFmiDataMatrix<NFmiPoint> & theMatrix, const NFmiArea & theArea) const;

  bool BiLinearInterpolation(double x, double y, float & theValue,
							 float topLeftValue, float topRightValue,
							 float bottomLeftValue, float bottomRightValue);
  
  // Grid()

  NFmiPoint LatLonToGrid(const NFmiPoint & theLatLonPoint);
  const NFmiGrid * Grid(void) const;

  // These require the data values

  float InterpolatedValue(const NFmiPoint & theLatLonPoint);

  void Values(NFmiDataMatrix<float> & theValues);

private:

  LazyQueryData(const LazyQueryData & theQD);
  LazyQueryData & operator=(const LazyQueryData & theQD);

  void requireInfo() const;
  void requireData();

  std::string itsDataFile;
  std::auto_ptr<NFmiFastQueryInfo> itsInfo;
  std::auto_ptr<NFmiQueryData> itsData;

}; // class LazyQueryData

#endif // LAZYQUERYDATA_H

// ======================================================================

