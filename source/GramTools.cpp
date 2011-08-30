// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace GramTools
 */
// ======================================================================

#include "GramTools.h"
#include "UnitsConverter.h"

namespace GramTools
{

  // ----------------------------------------------------------------------
  /*!
   * \brief Return a meteorological arrow for the given wind speed
   *
   * The arrow is suitable only for stroking, not filling.
   * If the speed is negative or missing, and empty path is returned.
   *
   * \param theSpeed The wind speed in meters/second
   * \return The arrow as a strokable path object
   */
  // ----------------------------------------------------------------------

  Imagine::NFmiPath metarrow(float theSpeed)
  {
	Imagine::NFmiPath path;

	// Handle bad cases
	if(theSpeed<0 || theSpeed==kFloatMissing)
	  return path;

	// size of the spot at the origin
	const float spot_size = 0.75;

	// length of the initial line before flags are added
	const float stem_length = 18;

	// Separation between barbs
	const float barb_interval = stem_length/5;

	// Barb length
	const float barb_length = 12;

	// Barb angle
	const float barb_angle = 45.f / 180.f*3.14159265358979323846f;

	// Flag side length
	const float flag_length = 7;

	// The actual speed in knots
	const float speed = theSpeed / 0.5144444444f;

	// The respective number of flags
	const int flags = static_cast<int>(floor(speed/50));

	// The number of long barbs
	const int long_barbs = static_cast<int>(floor((speed-flags*50.0)/10.0));

	// The number of short barbs
	const int short_barbs = static_cast<int>(floor((speed-flags*50.0-long_barbs*10.0)/5.0));

	// The full length of the stem
	const float full_stem_length = stem_length + (flags == 0.0 ? 0.0 : flags * flag_length + barb_interval);


	// Mark the spot with a small dot
	path.MoveTo(spot_size,spot_size);
	path.LineTo(spot_size,-spot_size);
	path.LineTo(-spot_size,-spot_size);
	path.LineTo(-spot_size,spot_size);
	path.LineTo(spot_size,spot_size);

	path.MoveTo(0,spot_size);
	
	// The stem
	
	float y = spot_size + full_stem_length;

	if(flags>0 || long_barbs>0 || short_barbs>0)
	  {
		path.LineTo(0,y);
	  }
	
	// Flags

	for(int i=0; i<flags; i++)
	  {
		path.LineTo(-barb_length*cos(barb_angle),
					y - flag_length + barb_length*sin(barb_angle));
		y -= flag_length;
		path.LineTo(0,y);
	  }

	// Long barbs

	if(flags > 0)
	  {
		y -= barb_interval;
	  }

	for(int i=0; i<long_barbs; i++)
	  {
		path.LineTo(0,y);
		path.LineTo(-barb_length*cos(barb_angle),
					y + barb_length*sin(barb_angle));
		path.MoveTo(0,y);
		y -= barb_interval;
	  }

	// Short barbs. We use factor 1.5 to make 5 knot arrow more readable

	if(long_barbs==0 && flags==0 && short_barbs>0)
	  y -= 1.5*barb_interval;

	for(int i=0; i<short_barbs; i++)
	  {
		path.LineTo(0,y);
		path.LineTo(-0.5*barb_length*cos(barb_angle),
					y + 0.5*barb_length*sin(barb_angle));
		path.MoveTo(0,y);
		y -= barb_interval;
	  }

	// path.LineTo(0,0);

	return path;
  }

} // namespace GramTools

// ======================================================================
