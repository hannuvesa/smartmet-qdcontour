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

  bool verbose;
  bool force;

  std::string datapath;
  std::string mapspath;

};

#endif // GLOBALS_H

// ======================================================================
