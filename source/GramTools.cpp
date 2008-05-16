// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace GramTools
 */
// ======================================================================

#include "GramTools.h"

namespace GramTools
{

  // ----------------------------------------------------------------------
  /*!
   * \brief Return a meteorological arrow for the given wind speed
   *
   * The arrow is suitable only for stroking, not filling.
   * If the speed is negative or missing, and empty path is returned.
   *
   * Note the following details
   * \code
   *  1.25- 3.75 = 1 short on the side, plus on upward extra segment
   *  3.75- 6.25 = 1 long
   *  6.25- 8.75 = 1 long, 1 short
   *  8.75-11.25 = 2 long
   * 11.25-13.75 = 2 long, 1 short
   * \endcode
   *
   * \param theSpeed The wind speed
   * \return The arrow as a strokable path object
   *
   * \todo Speeds over 50 are not properly supported
   */
  // ----------------------------------------------------------------------

  Imagine::NFmiPath metarrow(float theSpeed)
  {
	Imagine::NFmiPath path;

	// Handle bad cases
	if(theSpeed<0 || theSpeed==kFloatMissing)
	  return path;
	
	// The details of the flag
	
	const float spot_size = 0.75;		// size of the spot at the origin
	const float initial_length = 14;	// length of the initial line for speed 0
	const float flag_interval = 4;		// the spacing between the speed indicators
	const float flag_length = 12;		// the length of a full speed indicator
	
	const float flag_angle_deg = 60;	// angle from the direction the wind is coming from
	
	const float flag_angle = flag_angle_deg/180*3.14159265358979323846f;
	
	// Mark the spot with a small dot
	path.MoveTo(spot_size,spot_size);
	path.LineTo(spot_size,-spot_size);
	path.LineTo(-spot_size,-spot_size);
	path.LineTo(-spot_size,spot_size);
	path.LineTo(spot_size,spot_size);
	
	// Start rendering
	
	const float full_unit = 5;
	const float half_unit = full_unit/2;
	const float quarter_unit = full_unit/4;	// limit between rounding up and down
	
	// Note: Must use theSpeed+quarter_unit in both formulas in the same order,
	// otherwise rounding errors may occur (causing a negative underflow).
	// Using a separate intermediate variable makes the requirement explicit.

	const float tmpspeed = theSpeed+quarter_unit;
	const unsigned int long_segments = static_cast<unsigned int>(::floor(tmpspeed/full_unit));
	const unsigned int short_segments = static_cast<unsigned int>(::floor((tmpspeed-long_segments*full_unit)/half_unit));
	
	const bool has_extra_up = (theSpeed >= quarter_unit && theSpeed < full_unit - quarter_unit);
	
	// The long streak upwards
	
	path.MoveTo(0,spot_size);
	path.LineTo(0,spot_size
				+ initial_length
				+ (std::max(1u,long_segments+short_segments)-1)*flag_interval
				+ (has_extra_up ? flag_interval : 0));
	
	// First the short segments (should be only one, according to present settings)
	
	float y = spot_size + initial_length;
	
	if(short_segments>0)
	  for(unsigned int i=0; i<short_segments; i++)
		{
		  path.MoveTo(0,y);
		  path.LineTo(-flag_length/2*::sin(flag_angle),y+flag_length/2*::cos(flag_angle));
		  y += flag_interval;
		}
	
	// Then the long ones
	
	if(long_segments>0)
	  for(unsigned int j=0; j<long_segments; j++)
		{
		  path.MoveTo(0,y);
		  path.LineTo(-flag_length*::sin(flag_angle),y+flag_length*::cos(flag_angle));
		  y += flag_interval;
		}

	return path;
  }

} // namespace GramTools

// ======================================================================
