// ======================================================================

#include "ContourInterpolation.h"

ContourInterpolation ContourInterpolationValue(const std::string & theName)
{
  if(theName=="Nearest")
	return Nearest;
  else if(theName=="Linear")
	return Linear;
  else if(theName=="LogLinear")
	return LogLinear;
  else if(theName=="Discrete")
	return Discrete;
  else
	return Missing;
}
