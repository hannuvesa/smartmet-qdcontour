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
  , queryfilelist()
  , queryfilenames()
  , calculator()
  , querystreams()
  , queryinfo(0)
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
