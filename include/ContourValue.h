// ======================================================================
/*!
 * \file
 * \brief Interface of class ContourValue
 */
// ======================================================================
/*!
 * \class ContourValue
 * \brief Storage for a single contour-value
 *
 */
// ======================================================================

#ifndef CONTOURVALUE_H
#define CONTOURVALUE_H

#include <string>

class ContourValue
{
public:
  
#ifdef COMPILER_GENERATED
  ~ContourValue();
  ContourValue(const ContourValue & theValue);
  ContourValue & operator=(const ContourValue & theValue);
#endif

  ContourValue(float theValue,
			   int theColor,
			   const std::string & theRule="Atop");
  
  float value(void) const;
  int color(void) const;
  const std::string & rule(void) const;
private:

  ContourValue(void);

  float itsValue;
  int itsColor;
  std::string itsRule;
};


#endif // CONTOURVALUE_H

// ======================================================================
