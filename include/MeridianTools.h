// ======================================================================
/*!
 * \brief Tools for handling meridian shifting near 180th meridian
 */
// ======================================================================

#ifndef MERIDIANTOOLS_H
#define MERIDIANTOOLS_H

#include "NFmiArea.h"
#include "NFmiPath.h"

namespace MeridianTools
{
  NFmiPoint Relocate(const NFmiPoint & thePoint,
					 const NFmiArea & theArea);

  void Relocate(Imagine::NFmiPath & thePath,
				const NFmiArea & theArea);

} // MeridianTools

#endif // MERIDIANTOOLS_H

// ======================================================================
