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

#include "imagine/NFmiColorTools.h"

namespace ColorTools
{

  NFmiColorTools::Color parsecolor(const std::string & theColor);
  NFmiColorTools::Color checkcolor(const std::string & theColor);

  NFmiColorTools::NFmiBlendRule parserule(const std::string & theRule);
  NFmiColorTools::NFmiBlendRule checkrule(const std::string & theRule);

} // namespace ColorTools

#endif // COLORTOOLS_H

// ======================================================================
