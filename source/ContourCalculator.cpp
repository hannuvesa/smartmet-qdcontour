// ======================================================================
/*!
 * \file
 * \brief Implementation of class ContourCalculator
 */
// ======================================================================

#include "ContourCalculator.h"
#include "ContourCache.h"
#include "LazyQueryData.h"

#include "NFmiContourTree.h"
#include "NFmiPath.h"
#include "NFmiContourDataHelper.h"

#include "NFmiDataMatrix.h"

#include <memory>
#include <stdexcept>

// ----------------------------------------------------------------------
/*!
 * \brief Implementation hiding pimple for ContourCalculator
 */
// ----------------------------------------------------------------------

class ContourCalculatorPimple
{
public:
  ContourCalculatorPimple()
	: itsAreaCache()
	, itsLineCache()
	, isCacheOn(false)
	, itWasCached(false)
	, itsData(0)
  { }

  ContourCache itsAreaCache;
  ContourCache itsLineCache;
  bool isCacheOn;
  bool itWasCached;
  const NFmiDataMatrix<float> * itsData;			// does not own!
  std::auto_ptr<Imagine::NFmiContourDataHelper> itsHelper;
  
}; // class ContourCalculatorPimple

// ----------------------------------------------------------------------
/*!
 * \brief Destructor
 */
// ----------------------------------------------------------------------

ContourCalculator::~ContourCalculator()
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 */
// ----------------------------------------------------------------------

ContourCalculator::ContourCalculator()
{
  itsPimple.reset(new ContourCalculatorPimple());
}

// ----------------------------------------------------------------------
/*!
 * \brief Clear the cache
 */
// ----------------------------------------------------------------------

void ContourCalculator::clearCache()
{
  itsPimple->itsAreaCache.clear();
  itsPimple->itsLineCache.clear();
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the cache on or off
 *
 * \param theFlag True if cache is to be set on
 */
// ----------------------------------------------------------------------

void ContourCalculator::cache(bool theFlag)
{
  itsPimple->isCacheOn = theFlag;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return whether the last contour was cached
 *
 * \return True, if last returned contour was cached
 */
// ----------------------------------------------------------------------

bool ContourCalculator::wasCached() const
{
  return itsPimple->itWasCached;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set new active data on
 */
// ----------------------------------------------------------------------

void ContourCalculator::data(const NFmiDataMatrix<float> & theData)
{
  itsPimple->itsData = &theData;
  itsPimple->itsHelper.reset(new Imagine::NFmiContourDataHelper(theData));
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the desired contour
 *
 * \return The path object
 */
// ----------------------------------------------------------------------

Imagine::NFmiPath ContourCalculator::contour(const LazyQueryData & theData,
											 float theLoLimit, float theHiLimit,
											 bool theLoIsExact, bool theHiIsExact,
											 float theDataLoLimit, float theDataHiLimit,
											 Imagine::NFmiContourTree::NFmiContourInterpolation theInterpolation,
											 bool theContourTrianglesOn)
{
  if(itsPimple->itsData == 0)
	throw std::runtime_error("ContourCalculator:: No data set before calling contour");

  if(itsPimple->isCacheOn &&
	 itsPimple->itsAreaCache.contains(theLoLimit, theHiLimit, theData))
	{
	  itsPimple->itWasCached = true;
	  return itsPimple->itsAreaCache.find(theLoLimit, theHiLimit, theData);
	}

  Imagine::NFmiContourTree tree(theLoLimit, theHiLimit, theLoIsExact, theHiIsExact);
  tree.SubTriangleMode(theContourTrianglesOn);

  if(theDataLoLimit != kFloatMissing)
	tree.DataLoLimit(theDataLoLimit);

  if(theDataHiLimit != kFloatMissing)
	tree.DataHiLimit(theDataHiLimit);

  tree.Contour(*(itsPimple->itsData), *(itsPimple->itsHelper), theInterpolation);

  Imagine::NFmiPath path = tree.Path();
  path.InvGrid(theData.Grid());

  if(itsPimple->isCacheOn)
	itsPimple->itsAreaCache.insert(path, theLoLimit, theHiLimit, theData);

  itsPimple->itWasCached = false;
  return path;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the desired contour line
 *
 * \return The path object
 */
// ----------------------------------------------------------------------

Imagine::NFmiPath ContourCalculator::contour(const LazyQueryData & theData,
											 float theValue,
											 float theDataLoLimit, float theDataHiLimit,
											 Imagine::NFmiContourTree::NFmiContourInterpolation theInterpolation,
											 bool theContourTrianglesOn)
{
  if(itsPimple->itsData == 0)
	throw std::runtime_error("ContourCalculator:: No data set before calling contour");

  if(itsPimple->isCacheOn &&
	 itsPimple->itsLineCache.contains(theValue, kFloatMissing, theData))
	{
	  itsPimple->itWasCached = true;
	  return itsPimple->itsLineCache.find(theValue, kFloatMissing, theData);
	}

  Imagine::NFmiContourTree tree(theValue, kFloatMissing, true, false);
  tree.LinesOnly(true);
  tree.SubTriangleMode(theContourTrianglesOn);

  if(theDataLoLimit != kFloatMissing)
	tree.DataLoLimit(theDataLoLimit);

  if(theDataHiLimit != kFloatMissing)
	tree.DataHiLimit(theDataHiLimit);

  tree.Contour(*(itsPimple->itsData), *(itsPimple->itsHelper), theInterpolation);

  Imagine::NFmiPath path = tree.Path();
  path.InvGrid(theData.Grid());

  if(itsPimple->isCacheOn)
	itsPimple->itsLineCache.insert(path, theValue, kFloatMissing, theData);

  itsPimple->itWasCached = false;
  return path;
}

// ======================================================================

