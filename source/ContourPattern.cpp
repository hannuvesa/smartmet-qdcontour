// ======================================================================
/*!
 * \file
 * \brief Implementation of class ContourPattern
 */
// ======================================================================

#include "ContourPattern.h"

using namespace std;

// ----------------------------------------------------------------------
/*!
 * \brief The constructor
 *
 * \param theLoLimit The lower limit of the contour range
 * \param theHiLimit The upper limit of the contour range
 * \param thePattern The filename of the pattern image
 * \param theRule The blending rule for the color
 * \param theFactor The alpha blending factor for the pattern
 */
// ----------------------------------------------------------------------

ContourPattern::ContourPattern(float theLoLimit,
                               float theHiLimit,
                               const std::string &thePattern,
                               const std::string &theRule,
                               float theFactor)
    : itsLoLimit(theLoLimit),
      itsHiLimit(theHiLimit),
      itsPattern(thePattern),
      itsRule(theRule),
      itsFactor(theFactor)
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the lower limit of the contour range
 *
 * \return The lower limit
 */
// ----------------------------------------------------------------------

float ContourPattern::lolimit() const { return itsLoLimit; }
// ----------------------------------------------------------------------
/*!
 * \brief Return the upper limit of the contour range
 *
 * \return The upper limit
 */
// ----------------------------------------------------------------------

float ContourPattern::hilimit() const { return itsHiLimit; }
// ----------------------------------------------------------------------
/*!
 * \brief Return the pattern image file of the contour range
 *
 * \return The pattern image filename
 */
// ----------------------------------------------------------------------

const std::string &ContourPattern::pattern() const { return itsPattern; }
// ----------------------------------------------------------------------
/*!
 * \brief Return the blending rule for the pattern
 *
 * \return The blending rule
 */
// ----------------------------------------------------------------------

const std::string &ContourPattern::rule() const { return itsRule; }
// ----------------------------------------------------------------------
/*!
 * \brief Return the alpha blending factor
 *
 * \return The factor
 */
// ----------------------------------------------------------------------

float ContourPattern::factor() const { return itsFactor; }
// ======================================================================
