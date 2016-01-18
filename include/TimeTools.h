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

#include <ctime>
#include <string>
class NFmiTime;

namespace TimeTools
{
NFmiTime ConvertZone(const NFmiTime &theTime, const std::string &theZone);

NFmiTime ToUTC(::time_t theTime);

}  // namespace TimeTools

#endif  // TIMETOOLS_H

// ======================================================================
