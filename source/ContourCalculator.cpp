// ======================================================================
/*!
 * \file
 * \brief Implementation of class ContourCalculator
 */
// ======================================================================

#include "ContourCalculator.h"
#include "ContourCache.h"
#include "LazyQueryData.h"
#include "DataMatrixAdapter.h"

#include "NFmiDataMatrix.h"

#include "PathAdapter.h"

#include "Tron.h"

#include <memory>
#include <stdexcept>

typedef Tron::Traits<float,float,Tron::FmiMissing> MyTraits;

typedef Tron::Contourer<DataMatrixAdapter,
						PathAdapter,
						MyTraits,
						Tron::LinearInterpolation> MyLinearContourer;

typedef Tron::Contourer<DataMatrixAdapter,
						PathAdapter,
						MyTraits,
						Tron::LogLinearInterpolation> MyLogLinearContourer;

typedef Tron::Contourer<DataMatrixAdapter,
						PathAdapter,MyTraits,
						Tron::NearestNeighbourInterpolation> MyNearestContourer;

typedef Tron::Contourer<DataMatrixAdapter,
						PathAdapter,
						MyTraits,
						Tron::DiscreteInterpolation> MyDiscreteContourer;

// typedef MyLinearContourer::hints_type MyHints;
typedef Tron::Hints<DataMatrixAdapter,MyTraits> MyHints;

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
	, itsData()
	, itsHintsOK(false)
  { }

  ContourCache itsAreaCache;
  ContourCache itsLineCache;
  bool isCacheOn;
  bool itWasCached;
  boost::shared_ptr<DataMatrixAdapter> itsData;	// does not own!
  bool itsHintsOK;
  boost::shared_ptr<MyHints> itsHints;

  void require_hints();
  
}; // class ContourCalculatorPimple

// ----------------------------------------------------------------------
/*!
 * \brief Require the pimple to be up to date
 */
// ----------------------------------------------------------------------

void ContourCalculatorPimple::require_hints()
{
  if(itsHintsOK)
	return;

  itsHints.reset(new MyHints(*itsData));
  itsHintsOK = true;
}

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
  itsPimple->itsData.reset(new DataMatrixAdapter(theData));
  itsPimple->itsHintsOK = false;
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
											 ContourInterpolation theInterpolation)
{
  if(itsPimple->itsData.get() == 0)
	throw std::runtime_error("ContourCalculator:: No data set before calling contour");

  if(itsPimple->isCacheOn &&
	 itsPimple->itsAreaCache.contains(theLoLimit, theHiLimit, theData))
	{
	  itsPimple->itWasCached = true;
	  return itsPimple->itsAreaCache.find(theLoLimit, theHiLimit, theData);
	}

  itsPimple->require_hints();

  PathAdapter adapter;
  
  switch(theInterpolation)
	{
	case Linear:
	case Missing:
	  {
		MyLinearContourer::fill(adapter,
								*(itsPimple->itsData),
								theLoLimit,theHiLimit,
								*(itsPimple->itsHints));
		break;
	  }
	case LogLinear:
	  {
		MyLogLinearContourer::fill(adapter,
								   *(itsPimple->itsData),
								   theLoLimit,theHiLimit,
								   *(itsPimple->itsHints));
		break;
	  }
	case Nearest:
	  {
		MyNearestContourer::fill(adapter,
								 *(itsPimple->itsData),
								 theLoLimit,theHiLimit,
								 *(itsPimple->itsHints));
		break;
	  }
	case Discrete:
	  {
		MyDiscreteContourer::fill(adapter,
								  *(itsPimple->itsData),
								  theLoLimit,theHiLimit,
								  *(itsPimple->itsHints));
		break;
	  }
	}

  Imagine::NFmiPath path = adapter.path();
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
											 ContourInterpolation theInterpolation)
{

  if(itsPimple->itsData.get() == 0)
	throw std::runtime_error("ContourCalculator:: No data set before calling contour");

  if(itsPimple->isCacheOn &&
	 itsPimple->itsLineCache.contains(theValue, kFloatMissing, theData))
	{
	  itsPimple->itWasCached = true;
	  return itsPimple->itsLineCache.find(theValue, kFloatMissing, theData);
	}

  itsPimple->require_hints();

  PathAdapter adapter;
  
  switch(theInterpolation)
	{
	case Linear:
	case Missing:
	  {
#if 0
		MyLinearContourer::line(adapter,
								*(itsPimple->itsData),
								theValue,
								*(itsPimple->itsHints));
#endif
		MyLinearContourer::line(adapter,
								*(itsPimple->itsData),
								theValue);
		break;
	  }
	case LogLinear:
	  {
		MyLogLinearContourer::line(adapter,
								   *(itsPimple->itsData),
								   theValue);
		break;
	  }
	case Nearest:
	  {
		throw std::runtime_error("Contour lines not supported for nearest neighbour interpolation");
	  }
	case Discrete:
	  {
		throw std::runtime_error("Contour lines not supported for discrete neighbour interpolation");
		break;
	  }
	}

  Imagine::NFmiPath path = adapter.path();
  path.InvGrid(theData.Grid());

  if(itsPimple->isCacheOn)
	itsPimple->itsLineCache.insert(path, theValue, kFloatMissing, theData);

  itsPimple->itWasCached = false;
  return path;
}

// ======================================================================

