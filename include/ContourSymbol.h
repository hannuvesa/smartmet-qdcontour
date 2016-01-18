// ======================================================================
/*!
 * \file
 * \brief Interface of class ContourSymbol
 */
// ======================================================================
/*!
 * \class ContourSymbol
 * \brief Storage for a single contour symbol
 *
 */
// ======================================================================

#ifndef CONTOURSYMBOL_H
#define CONTOURSYMBOL_H

#include "ContourPattern.h"

// The interface is exactly the same, no sense in wasting code
// However, a typedef is not enough to get proper operator overloading,
// instead we use inheritance to get a new type

class ContourSymbol : public ContourPattern
{
 public:
  ContourSymbol(float theLoLimit,
                float theHiLimit,
                const std::string &thePattern,
                const std::string &theRule,
                float theFactor = 1.0)
      : ContourPattern(theLoLimit, theHiLimit, thePattern, theRule, theFactor)
  {
  }
};

#endif  // CONTOURSYMBOL_H

// ======================================================================
