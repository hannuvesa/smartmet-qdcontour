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

  // ----------------------------------------------------------------------
  /*!
   * \brief Split string into list of strings
   *
   * \param theString The string to split into parts
   * \param theSeparator The separator character
   * \return List of strings
   */
  // ----------------------------------------------------------------------

  std::list<std::string> splitwords(const std::string & theString,
									char theSeparator)
  {
	std::list<std::string> ret;
	unsigned int pos1 = 0;
	while(pos1<theString.size())
	  {
		unsigned int pos2 = theString.find(theSeparator,pos1);
		if(pos2==std::string::npos)
		  pos2 = theString.size();
		ret.push_back(theString.substr(pos1,pos2-pos1));
		pos1 = pos2 + 1;
	  }
	return ret;
  }


} // namespace StringTools

// ======================================================================

