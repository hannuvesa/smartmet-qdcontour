// ======================================================================
/*!
 * \file
 * \brief Implementation of class ContourLabel
 */
// ======================================================================

#include "ContourLabel.h"

using namespace std;

// ----------------------------------------------------------------------
/*!
 * \brief The constructor
 *
 * \param theValue The value on the contour
 */
// ----------------------------------------------------------------------

ContourLabel::ContourLabel(float theValue)
  : itsValue(theValue)
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the value on the contour line
 *
 * \return The value
 */
// ----------------------------------------------------------------------

float ContourLabel::value() const
{
  return itsValue;
}

// ======================================================================
