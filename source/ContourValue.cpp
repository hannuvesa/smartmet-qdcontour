// ======================================================================
/*!
 * \file
 * \brief Implementation of class ContourValue
 */
// ======================================================================

#include "ContourValue.h"

using namespace std;

// ----------------------------------------------------------------------
/*!
 * \brief The constructor
 *
 * \param theValue The value on the contour
 * \param theColor The color to draw the contour with
 * \param theRule The blending rule for the color
 */
// ----------------------------------------------------------------------

ContourValue::ContourValue(float theValue,
						   int theColor,
						   const std::string & theRule)
  : itsValue(theValue)
  , itsColor(theColor)
  , itsRule(theRule)
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the value on the contour line
 *
 * \return The value
 */
// ----------------------------------------------------------------------

float ContourValue::value() const
{
  return itsValue;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the color of the contour line
 *
 * \return The color
 */
// ----------------------------------------------------------------------

int ContourValue::color() const
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

const std::string & ContourValue::rule() const
{
  return itsRule;
}

// ======================================================================
