// ======================================================================
/*!
 * \file
 * \brief Interface of namespace StringTools
 */
// ======================================================================
/*!
 * \namespace StringTools
 * \brief A collection of various string handling tools
 */
// ======================================================================

#ifndef STRINGTOOLS_H
#define STRINGTOOLS_H

#include <iosfwd>
#include <list>
#include <string>

namespace StringTools
{
  std::string readfile(std::istream & is);
  std::list<std::string> splitwords(const std::string & theString,
									const char theSeparator);

} // namespace StringTools

#endif // STRINGTOOLS_H

// ======================================================================
