// ======================================================================
/*!
 * \file
 * \brief Declaration of global variables
 */
// ======================================================================

#ifndef GLOBALS_H
#define GLOBALS_H

#include <string>

struct Globals
{
  Globals();

  bool verbose;						// -v option
  bool force;						// -f option
  std::string cmdline_querydata;	// -q option

  std::string datapath;				// default searchpath for data
  std::string mapspath;				// default searchpath for maps

};

#endif // GLOBALS_H

// ======================================================================
