// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace TimeTools
 */
// ======================================================================

#include "TimeTools.h"
#include "newbase/NFmiMetTime.h"
#include <stdexcept>

namespace TimeTools
{

  // ----------------------------------------------------------------------
  /*!
   * \brief Convert time to another time zone
   *
   * The recognized zones are:
   *
   *  - <em>local</em> for local time
   *  - <em>utc</em> for UTC time (no conversion is done)
   *
   * An exception is thrown if the time zone is not recognized
   *
   * \param theTime The UTC time to convert
   * \param theZone Textual description of the zone
   * \return The converted time
   */
  // ----------------------------------------------------------------------

  NFmiTime ConvertZone(const NFmiTime & theTime,
					   const std::string & theZone)
  {
	if(theZone == "utc")
	  {
		return theTime;
	  }
	else if(theZone == "local")
	  {
		return NFmiMetTime(theTime,1).CorrectLocalTime();
	  }
	else
	  throw std::runtime_error("ConvertZone: Unrecognized time zone "+theZone);
  }

} // namespace TimeTools

// ======================================================================
