// ======================================================================
/*!
 * \file
 * \brief Definition of global variables
 */
// ======================================================================

#include "Globals.h"
#include "LazyQueryData.h"

#include "NFmiSettings.h"

#include <string>

using namespace std;
using NFmiSettings::Optional;

// ----------------------------------------------------------------------
/*!
 * \brief Constructor for global variables
 */
// ----------------------------------------------------------------------

Globals::Globals()
  : verbose(false)
  , force(false)
  , cmdline_querydata()
  , cmdline_files()
  , datapath(Optional<string>("qdcontour::querydata_path","."))
  , mapspath(Optional<string>("qdcontour::maps_path","."))
  , projection()
  , filter("none")
  , erase("transparent")
  , fillrule("Atop")
  , strokerule("Atop")
  , directionparam("WindDirection")
  , speedparam("WindSpeedMS")
  , arrowscale(1)
  , queryfilelist()
  , queryfilenames()
  , queryinfo(0)
  , querydatalevel(1)
  , timesteps(24)
  , timestep(0)
  , timeinterval(0)
  , timestepskip(0)
  , timesteprounding(1)
  , timestampflag(1)
  , timestampzone("local")
  , timestampimagex(0)
  , timestampimagey(0)
  , calculator()
  , querystreams()
  , shapespecs()
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Destructor
 */
// ----------------------------------------------------------------------

Globals::~Globals()
{
  clear_querystreams();
}

// ----------------------------------------------------------------------
/*!
 * \brief Delete all active querystreams
 */
// ----------------------------------------------------------------------

void Globals::clear_querystreams()
{
  for(vector<LazyQueryData *>::iterator it = querystreams.begin();
	  it != querystreams.end();
	  ++it)
	{
	  delete *it;
	}
  querystreams.resize(0);
  queryinfo = 0;
}

// ======================================================================
