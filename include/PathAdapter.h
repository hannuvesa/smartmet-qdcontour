// ======================================================================
/*!
 * \brief Tron adapter for NFmiPath
 */
// ======================================================================

#ifndef PATHADAPTER_H
#define PATHADAPTER_H

#include "NFmiPath.h"

class PathAdapter
{
public:

  void moveto(float x, float y) { itsPath.MoveTo(x,y); }
  void lineto(float x, float y) { itsPath.LineTo(x,y); }
  void closepath() { itsPath.CloseLineTo(); }
  const Imagine::NFmiPath & path() const { return itsPath; }

private:

  Imagine::NFmiPath itsPath;

}; // class PathAdapter

#endif // PATHADAPTER_H

// ======================================================================
