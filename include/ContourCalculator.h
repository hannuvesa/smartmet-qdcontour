// ======================================================================
/*!
 * \file
 * \brief Interface of class ContourCalculator
 */
// ======================================================================
/*!
 * \class ContourCalculator
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

#include "NFmiContourTree.h"

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
							Imagine::NFmiContourTree::NFmiContourInterpolation theInterpolation);

  Imagine::NFmiPath contour(const LazyQueryData & theData,
							float theValue,
							Imagine::NFmiContourTree::NFmiContourInterpolation theInterpolation);

  void data(const NFmiDataMatrix<float> & theData);
  void clearCache();
  void cache(bool);
  bool wasCached(void) const;
  
private:

  ContourCalculator(const ContourCalculator & theCalc);
  ContourCalculator & operator=(const ContourCalculator & theCalc);

  boost::shared_ptr<ContourCalculatorPimple> itsPimple;

}; // class ContourCalculator

#endif // CONTOURCALCULATOR_H

// ======================================================================
