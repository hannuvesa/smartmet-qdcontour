// ======================================================================
/*!
 * \file
 * \brief Implementation of class ContourRange
 */
// ======================================================================

#include "ContourRange.h"

using namespace std;

// ----------------------------------------------------------------------
/*!
 * \brief The constructor
 *
 * \param theLoLimit The lower limit of the contour range
 * \param theHiLimit The upper limit of the contour range
 * \param theColor The color to draw the contour with
 * \param theRule The blending rule for the color
 */
// ----------------------------------------------------------------------

ContourRange::ContourRange(float theLoLimit,
						   float theHiLimit,
						   int theColor,
						   const std::string & theRule)
  : itsLoLimit(theLoLimit)
  , itsHiLimit(theHiLimit)
  , itsColor(theColor)
  , itsRule(theRule)
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the lower limit of the contour range
 *
 * \return The lower limit
 */
// ----------------------------------------------------------------------

float ContourRange::lolimit() const
{
  return itsLoLimit;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the upper limit of the contour range
 *
 * \return The upper limit
 */
// ----------------------------------------------------------------------

float ContourRange::hilimit() const
{
  return itsHiLimit;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the color of the contour range
 *
 * \return The color
 */
// ----------------------------------------------------------------------

int ContourRange::color() const
{
  return itsColor;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the blending rule for the color
 *
 * \return The blending rule
 */
// ----------------------------------------------------------------------

const std::string & ContourRange::rule() const
{
  return itsRule;
}

// ======================================================================
