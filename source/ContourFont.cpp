// ======================================================================
/*!
 * \file
 * \brief Implementation of class ContourFont
 */
// ======================================================================

#include "ContourFont.h"

using namespace std;

// ----------------------------------------------------------------------
/*!
 * \brief The constructor
 *
 * \param theValue The value on the contour
 * \param theColor The color to draw the symbol with
 * \param theSymbol The symbol character number
 * \param theFont The font specification
 */
// ----------------------------------------------------------------------

ContourFont::ContourFont(float theValue,
						 int theColor,
						 int theSymbol,
						 const std::string & theFont)
  : itsValue(theValue)
  , itsColor(theColor)
  , itsSymbol(theSymbol)
  , itsFont(theFont)
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the value on the contour line
 *
 * \return The value
 */
// ----------------------------------------------------------------------

float ContourFont::value() const
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

int ContourFont::color() const
{
  return itsColor;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the symbol number
 *
 * \return The symbol number
 */
// ----------------------------------------------------------------------

int ContourFont::symbol() const
{
  return itsSymbol;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the font specification
 *
 * \return The font specification
 */
// ----------------------------------------------------------------------

const std::string & ContourFont::font() const
{
  return itsFont;
}

// ======================================================================
