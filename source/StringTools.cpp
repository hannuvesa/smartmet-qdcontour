// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace StringTools
 */
// ======================================================================

#include "StringTools.h"
#include <iostream>

using namespace std;

namespace StringTools
{
  // ----------------------------------------------------------------------
  /*!
   * \brief Read an input stream into a string
   *
   * \param is The input stream to read from
   * \return The read string
   */
  // ----------------------------------------------------------------------

  std::string readfile(std::istream & is)
  {
	string ret;
	const int bufsize = 1024;
	char buffer[bufsize];
	while(!is.eof() && !is.fail())
	  {
		is.read(buffer,bufsize);
		ret.append(buffer,is.gcount());
	  }
	return ret;
  }

} // namespace StringTools

// ======================================================================
