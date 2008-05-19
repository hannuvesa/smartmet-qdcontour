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
 * \param theLineWidth The line width
 * \param theColor The color to draw the contour with
 * \param theRule The blending rule for the color
 */
// ----------------------------------------------------------------------

ContourValue::ContourValue(float theValue,
						   float theLineWidth,
						   int theColor,
						   const std::string & theRule)
  : itsValue(theValue)
  , itsLineWidth(theLineWidth)
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
 * \brief Return the line width
 *
 * \return The width
 */
// ----------------------------------------------------------------------

float ContourValue::linewidth() const
{
  return itsLineWidth;
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
