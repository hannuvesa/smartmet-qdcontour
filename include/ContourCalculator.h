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
#include "NFmiContourTree.h"
#include "NFmiDataMatrix.h"

class NFmiPath;
class ContourCalculatorPimple;
class LazyQueryData;

class ContourCalculator
{
public:
  ~ContourCalculator();
  ContourCalculator();

  NFmiPath contour(const NFmiDataMatrix<float> & theValues,
				   const LazyQueryData & theData,
				   float theLoLimit, float theHiLimit,
				   bool theLoIsExact, bool theHiIsExact,
				   float theDataLoLimit, float theDataHiLimit,
				   int theContourDepth,
				   NFmiContourTree::NFmiContourInterpolation theInterpolation);

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
