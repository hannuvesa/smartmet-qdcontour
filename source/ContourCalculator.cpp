// ======================================================================
/*!
 * \file
 * \brief Implementation of class ContourCalculator
 */
// ======================================================================

#include "ContourCalculator.h"
#include "ContourCache.h"
#include "LazyQueryData.h"

#include "imagine/NFmiContourTree.h"
#include "imagine/NFmiPath.h"

#include "newbase/NFmiDataMatrix.h"

// ----------------------------------------------------------------------
/*!
 * \brief Implementation hiding pimple for ContourCalculator
 */
// ----------------------------------------------------------------------

class ContourCalculatorPimple
{
public:
  ContourCalculatorPimple()
	: itsCache()
	, isCacheOn(false)
	, itWasCached(false)
  { }

  ContourCache itsCache;
  bool isCacheOn;
  bool itWasCached;
  
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
  itsPimple->itsCache.clear();
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
 * \brief Return the desired contour
 *
 * \return The path object
 */
// ----------------------------------------------------------------------

Imagine::NFmiPath ContourCalculator::contour(const NFmiDataMatrix<float> & theValues,
											 const LazyQueryData & theData,
											 float theLoLimit, float theHiLimit,
											 bool theLoIsExact, bool theHiIsExact,
											 float theDataLoLimit, float theDataHiLimit,
											 int theContourDepth,
											 Imagine::NFmiContourTree::NFmiContourInterpolation theInterpolation,
											 bool theContourTrianglesOn)
{
  if(itsPimple->isCacheOn &&
	 itsPimple->itsCache.contains(theLoLimit, theHiLimit, theData))
	{
	  itsPimple->itWasCached = true;
	  return itsPimple->itsCache.find(theLoLimit, theHiLimit, theData);
	}

  Imagine::NFmiContourTree tree(theLoLimit, theHiLimit, theLoIsExact, theHiIsExact);
  tree.SubTriangleMode(theContourTrianglesOn);

  if(theDataLoLimit != kFloatMissing)
	tree.DataLoLimit(theDataLoLimit);

  if(theDataHiLimit != kFloatMissing)
	tree.DataHiLimit(theDataHiLimit);

  tree.Contour(theValues, theInterpolation, theContourDepth);

  Imagine::NFmiPath path = tree.Path();
  path.InvGrid(theData.Grid());

  if(itsPimple->isCacheOn)
	itsPimple->itsCache.insert(path, theLoLimit, theHiLimit, theData);

  itsPimple->itWasCached = false;
  return path;
}

// ======================================================================

