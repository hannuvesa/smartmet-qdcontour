// ======================================================================
/*!
 * \file
 * \brief Interface of class ContourCalculator
 */
// ======================================================================
/*!
 * \class ContourCalculator
 * \brief Calculates and caches contours
 *
 * The ContourCalculator is able to calculate contours from the
 * given data, and then to cache the result in case the same
 * contour is calculator again.
 *
 */
// ======================================================================

#ifndef CONTOURCALCULATOR_H
#define CONTOURCALCULATOR_H

#include <memory>
#include "imagine/NFmiContourTree.h"

template <typename T> class NFmiDataMatrix;

class ContourCalculatorPimple;
class LazyQueryData;

namespace Imagine
{
  class NFmiPath;
}

class ContourCalculator
{
public:
  ~ContourCalculator();
  ContourCalculator();

  Imagine::NFmiPath contour(const LazyQueryData & theData,
							float theLoLimit, float theHiLimit,
							bool theLoIsExact, bool theHiIsExact,
							float theDataLoLimit, float theDataHiLimit,
							Imagine::NFmiContourTree::NFmiContourInterpolation theInterpolation,
							bool theContourTrianglesOn);

  Imagine::NFmiPath contour(const LazyQueryData & theData,
							float theValue,
							float theDataLoLimit, float theDataHiLimit,
							Imagine::NFmiContourTree::NFmiContourInterpolation theInterpolation,
							bool theContourTrianglesOn);

  void data(const NFmiDataMatrix<float> & theData);
  void clearCache();
  void cache(bool);
  bool wasCached(void) const;
  
private:

  ContourCalculator(const ContourCalculator & theCalc);
  ContourCalculator & operator=(const ContourCalculator & theCalc);

  std::auto_ptr<ContourCalculatorPimple> itsPimple;

}; // class ContourCalculator

#endif // CONTOURCALCULATOR_H

// ======================================================================
