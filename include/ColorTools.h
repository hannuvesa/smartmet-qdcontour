// ======================================================================
/*!
 * \file
 * \brief Interface of namespace ColorTools
 */
// ======================================================================
/*!
 * \namespace ColorTools
 * \brief Various tools related to colours
 *
 */
// ======================================================================

#ifndef COLORTOOLS_H
#define COLORTOOLS_H

#include "NFmiColorTools.h"

namespace ColorTools
{

  NFmiColorTools::Color parsecolor(const std::string & theColor);

} // namespace ColorTools

#endif // COLORTOOLS_H

// ======================================================================
