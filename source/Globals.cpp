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
  , savepath(".")
  , prefix()
  , suffix()
  , format("png")
  , gamma(-1)
  , intent()
  , alphalimit(-1)
  , pngquality(-1)
  , jpegquality(-1)
  , savealpha(true)
  , wantpalette(false)
  , forcepalette(false)
  , contourdepth(0)
  , contourinterpolation("Linear")
  , contourtriangles(1)
  , smoother("None")
  , smootherradius(1)
  , smootherfactor(1)
  , projection()
  , filter("none")
  , foregroundrule("Over")
  , background()
  , foreground()
  , mask()
  , combine()
  , backgroundimage()
  , foregroundimage()
  , maskimage()
  , combineimage()
  , combinex(0)
  , combiney(0)
  , combinerule("Over")
  , combinefactor(1)
  , erase("transparent")
  , fillrule("Atop")
  , strokerule("Atop")
  , directionparam("WindDirection")
  , speedparam("WindSpeedMS")
  , arrowscale(1)
  , arrowfillcolor("white")
  , arrowstrokecolor("black")
  , arrowfillrule("Over")
  , arrowstrokerule("Over")
  , arrowfile("")
  , windarrowscaleA(0)
  , windarrowscaleB(0)
  , windarrowscaleC(1)
  , windarrowdx(0)
  , windarrowdy(0)
  , arrowpoints()
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
  , specs()
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
