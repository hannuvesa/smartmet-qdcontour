// ======================================================================
/*!
 * \file
 * \brief Declaration of global variables
 */
// ======================================================================

#ifndef GLOBALS_H
#define GLOBALS_H

#include "ContourCalculator.h"
#include "ShapeSpec.h"

#include "NFmiPoint.h"

#include <list>
#include <string>
#include <vector>

class LazyQueryData;

struct Globals
{
  ~Globals();
  Globals();

  void clear_querystreams();

  // Command line options

  bool verbose;								// -v option
  bool force;								// -f option
  std::string cmdline_querydata;			// -q option
  std::list<std::string> cmdline_files;		// command line parameters

  // Status variables

  std::string datapath;				// default searchpath for data
  std::string mapspath;				// default searchpath for maps

  std::string projection;			// projection definition
  std::string filter;				// filtering mode

  std::string erase;				// background color
  std::string fillrule;				// normal filling rule
  std::string strokerule;			// normal stroking rule

  std::string directionparam;		// direction parameter for arrows
  std::string speedparam;			// speed parameter for arrows
  float arrowscale;					// scale factor for arrows

  std::string arrowfillcolor;
  std::string arrowstrokecolor;
  std::string arrowfillrule;
  std::string arrowstrokerule;
  std::string arrowfile;

  float windarrowscaleA;			// a*log10(b*x+1)
  float windarrowscaleB;			// default:
  float windarrowscaleC;			// 0*log10(0+1)+1 = 1

  std::list<NFmiPoint> arrowpoints;	// Active wind arrows

  std::string queryfilelist;		// querydata files in use
  std::vector<std::string> queryfilenames;	// querydata files in use

  LazyQueryData * queryinfo;		// active data, does not own pointer
  int querydatalevel;				// level index
  int timesteps;					// how many images to draw?
  int timestep;						// timestep, 0 = all valid
  int timeinterval;					// inclusive time interval
  int timestepskip;					// initial time to skip in minutes
  int timesteprounding;				// rounding flag
  int timestampflag;				// put timestamp into image name?
  std::string timestampzone;		// timezone for the timestamp
  std::string timestampimage;		// image timestamping mode
  int timestampimagex;
  int timestampimagey;

  // Active storage

  ContourCalculator calculator;		// data contourer
  std::vector<LazyQueryData *> querystreams;

  std::list<ShapeSpec> shapespecs;

};

#endif // GLOBALS_H

// ======================================================================
