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
 * Note that all the relevant settings should be made before adding
 * any candidate coordinates to the object. In particular, the bounding
 * box filtering is used to immediately discard coordinates outside
 * the target image. The code enforces this by throwing an exception
 * when the settings are changed while coordinates have been added.
 *
 * The algorithm for choosing the labels for a new timestep is
 *
 *   -# discard all points not within the bounding box
 *   -# for all candidate labels
 *      -# remove those too close to labels of different value
 *      -# remove those too close to labels of another parameter
 *   -# sort the candidate coordinates based on minimum distances to
 *      previous timestep coordinates
 *   -# for all remaining candidate labels until there are none
 *      -# remove those too close to same value labels chosen earlier
 *      -# choose the first label (the one closest to the earlier timestep)
 *
 */
// ======================================================================

#include "LabelLocator.h"
#include <stdexcept>

using namespace std;

//! Bad parameter value

const int badparameter = 0;

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
  , itsActiveParameter(0)
  , itsPreviousCoordinates()
  , itsCurrentCoordinates()
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Test if the locator contains no coordinates
 *
 * \return True, if there are no coordinates in the locator
 */
// ----------------------------------------------------------------------

bool LabelLocator::empty() const
{
  return (itsPreviousCoordinates.empty() && itsCurrentCoordinates.empty());
}

// ----------------------------------------------------------------------
/*!
 * \brief Clear all earlier label locations
 */
// ----------------------------------------------------------------------

void LabelLocator::clear()
{
  itsActiveParameter = badparameter;
  itsPreviousCoordinates.clear();
  itsCurrentCoordinates.clear();
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
  if(!empty())
	throw runtime_error("LabelLocator: Cannot change bounding box once coordinates have been added");

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
  if(!empty())
	throw runtime_error("LabelLocator: Cannot change minimum distances once coordinates have been added");

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
  if(!empty())
	throw runtime_error("LabelLocator: Cannot change minimum distances once coordinates have been added");

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
  if(!empty())
	throw runtime_error("LabelLocator: Cannot change minimum distances once coordinates have been added");

  itsMinDistanceToDifferentParameter = theDistance;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the parameter id of the active parameter
 *
 * Parameter number 0 is reserved for indicating no active parameter.
 * This co-incides with the newbase value kFmiBadParameter. However,
 * one should not use this method to set parameter value 0, instead
 * one should use the clear method.
 *
 * \param theParameter The parameter value
 */
// ----------------------------------------------------------------------

void LabelLocator::parameter(int theParameter)
{
  if(theParameter == badparameter)
	throw runtime_error("Cannot activate parameter number 0, must be nonnegative");

  itsActiveParameter = theParameter;
}

// ----------------------------------------------------------------------
/*!
 * \brief Initialize next time step
 *
 * This must be called before coordinates are added for any new timestep.
 * It should be called also before the first timestep, although at this
 * point it could be omitted. This is not however guaranteed in the
 * future.
 */
// ----------------------------------------------------------------------

void LabelLocator::nextTime()
{
  itsPreviousCoordinates.clear();
  swap(itsPreviousCoordinates,itsCurrentCoordinates);
}

// ----------------------------------------------------------------------
/*!
 * \brief Test if the point is within the bounding box
 *
 * Note that the bounding box may not be set, in which case all
 * points are considered inside.
 *
 * \param theX The X-coordinate
 * \param theY The Y-coordinate
 * \return True, if the point is inside the bounding box
 */
// ----------------------------------------------------------------------

bool LabelLocator::inside(int theX, int theY) const
{
  if(!itHasBBox)
	return true;

  return (theX >= itsBBoxX1 &&
		  theX < itsBBoxX2 &&
		  theY >= itsBBoxY1 &&
		  theY < itsBBoxY2);
}

// ----------------------------------------------------------------------
/*!
 * \brief Add a new coordinate for the active parameter
 *
 * \param theContour The contour value
 * \param theX The X-coordinate
 * \param theY The Y-coordinate
 */
// ----------------------------------------------------------------------

void LabelLocator::add(float theContour, int theX, int theY)
{
  // Ignore the point if it is not within the bounding box

  if(!inside(theX,theY))
	return;

  // Cannot add any coordinates for non-existent parameter

  if(itsActiveParameter == badparameter)
	throw runtime_error("LabelLocator: Cannot add label location before setting the parameter");


  // Default constructed values are a desired side-effect in here
  // This is much simpler than using find + insert with checking

  ContourCoordinates & cc = itsCurrentCoordinates[itsActiveParameter];
  Coordinates & c = cc[theContour];

  c.push_back(Coordinates::value_type(theX,theY));

}

// ======================================================================
