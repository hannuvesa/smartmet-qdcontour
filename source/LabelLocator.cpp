// ======================================================================
/*!
 * \file
 * \brief Implementation of class LabelLocator
 */
// ======================================================================
/*!
 * \class LabelLocator
 *
 * \brief Finds suitable locations for contour labels
 *
 * The LabelLocator class provides means for finding suitable locations
 * for contour labels based on
 *
 *   - the bounding box for the coordinates
 *   - the previous timestep locations
 *   - the minimum allowed distance to nearest same value label
 *   - the minimum allowed distance to nearest different value label
 *   - the minimum allowed distance to nearest label of another parameter
 *
 */
// ======================================================================

#include "LabelLocator.h"
#include <stdexcept>

using namespace std;

// ----------------------------------------------------------------------
/*!
 * \brief Destructor
 */
// ----------------------------------------------------------------------

LabelLocator::~LabelLocator()
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Default constructor
 */
// ----------------------------------------------------------------------

LabelLocator::LabelLocator()
  : itHasBBox(false)
  , itsBBoxX1(0)
  , itsBBoxY1(0)
  , itsBBoxX2(0)
  , itsBBoxY2(0)
  , itsMinDistanceToSameValue(100)
  , itsMinDistanceToDifferentValue(50)
  , itsMinDistanceToDifferentParameter(50)
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Clear all earlier label locations
 */
// ----------------------------------------------------------------------

void LabelLocator::clear()
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the bounding box to which all label coordinates are clipped
 *
 * Note that normally the bounding box does not correspond directly to
 * the image dimensions, but one substracts some margin width first
 * to prevent labels from being place too close to the borders where
 * the label text might be rendered outside the image.
 *
 * \param theX1 The minimum X-coordinate
 * \param theY1 The minimum Y-coordinate
 * \param theX2 The maximum X-coordinate + 1
 * \param theY2 The maximum Y-coordinate + 1
 */
// ----------------------------------------------------------------------

void LabelLocator::boundingBox(int theX1, int theY1, int theX2, int theY2)
{
  if(theX2 <= theX1 || theY2 <= theY1)
	throw runtime_error("Empty bounding box not allowed in LabelLocator");

  itHasBBox = true;
  itsBBoxX1 = theX1;
  itsBBoxY1 = theY1;
  itsBBoxX2 = theX2;
  itsBBoxY2 = theY2;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the minimum distance to label of same value
 *
 * \param theDistance The minimum distance
 */
// ----------------------------------------------------------------------

void LabelLocator::minDistanceToSameValue(float theDistance)
{
  itsMinDistanceToSameValue = theDistance;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the minimum distance to label of different value
 *
 * \param theDistance The minimum distance
 */
// ----------------------------------------------------------------------

void LabelLocator::minDistanceToDifferentValue(float theDistance)
{
  itsMinDistanceToDifferentValue = theDistance;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the minimum distance to label of parameter
 *
 * \param theDistance The minimum distance
 */
// ----------------------------------------------------------------------

void LabelLocator::minDistanceToDifferentParameter(float theDistance)
{
  itsMinDistanceToDifferentParameter = theDistance;
}

// ======================================================================
