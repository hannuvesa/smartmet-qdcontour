// ======================================================================
/*!
 * \file
 * \brief Interface of namespace TimeTools
 */
// ======================================================================
/*!
 * \namespace TimeTools
 * \brief Various tools related to time
 *
 */
// ======================================================================

#ifndef TIMETOOLS_H
#define TIMETOOLS_H

#include <string>
class NFmiTime;

namespace TimeTools
{
  NFmiTime ConvertZone(const NFmiTime & theTime,
					   const std::string & theZone);

} // namespace TimeTools

#endif // TIMETOOLS_H

// ======================================================================
