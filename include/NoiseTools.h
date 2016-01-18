// ======================================================================
/*!
 * \file
 * \brief Interface of namespace NoiseTools
 */
// ======================================================================

#ifndef NOISETOOLS_H
#define NOISETOOLS_H

#include "NFmiDataMatrix.h"

namespace NoiseTools
{
// weighted median filter
void despeckle(NFmiDataMatrix<float> &theValues,
               float theLoLimit,
               float theHiLimit,
               int theRadius,
               float theWeight,
               int theIterations);

}  // namespace NoiseTools

#endif  // NOISETOOLS_H

// ======================================================================
