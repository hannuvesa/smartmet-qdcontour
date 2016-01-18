// ======================================================================

#pragma once

#include <string>

enum ContourInterpolation
{
  Missing,
  Nearest,
  Linear,
  Discrete,
  LogLinear
};

ContourInterpolation ContourInterpolationValue(const std::string &theName);
