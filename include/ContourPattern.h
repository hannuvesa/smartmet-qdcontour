// ======================================================================
/*!
 * \file
 * \brief Interface of class ContourPattern
 */
// ======================================================================
/*!
 * \class ContourPattern
 * \brief Storage for a single contour pattern
 *
 */
// ======================================================================

#ifndef CONTOURPATTERN_H
#define CONTOURPATTERN_H

#include <string>

class ContourPattern
{
public:

#ifdef COMPILER_GENERATED
  ContourPattern(const ContourPattern & theValue);
  ContourPattern & operator=(const ContourPattern & theValue);
#endif
  
  virtual ~ContourPattern() { }
  ContourPattern(float theLoLimit,
				 float theHiLimit,
				 const std::string & thePattern,
				 const std::string & theRule,
				 float theFactor=1.0);

  float lolimit(void) const;
  float hilimit(void) const;
  const std::string & pattern(void) const;
  const std::string & rule(void) const;
  float factor(void) const;
  
private:

  ContourPattern(void);

  float itsLoLimit;
  float itsHiLimit;
  std::string itsPattern;
  std::string itsRule;
  float itsFactor;

}; // class ContourPattern

#endif // CONTOURPATTERN_H

// ======================================================================

