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
#include <string>

namespace StringTools
{
  std::string readfile(std::istream & is);

} // namespace StringTools

#endif // STRINGTOOLS_H

// ======================================================================
