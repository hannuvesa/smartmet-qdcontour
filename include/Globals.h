// ======================================================================
/*!
 * \file
 * \brief Declaration of global variables
 */
// ======================================================================

#ifndef GLOBALS_H
#define GLOBALS_H

#include "ContourCalculator.h"

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

  std::string filter;				// filtering mode

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

};

#endif // GLOBALS_H

// ======================================================================
