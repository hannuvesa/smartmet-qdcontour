// ======================================================================
/*!
 * \file
 * \brief Interface of namespace ProjectionFactory
 */
// ======================================================================
/*!
 * \namespace ProjectionFactory
 *
 * \brief Tools for creating valid NFmiArea objects.
 *
 */
// ======================================================================

#ifndef PROJECTIONFACTORY_H
#define PROJECTIONFACTORY_H

class NFmiPoint;
class NFmiStereographicArea;

namespace ProjectionFactory
{
NFmiStereographicArea createStereographic(double theCentralLongitude,
                                          double theCentralLatitude,
                                          double theTrueLatitude,
                                          const NFmiPoint &theCenter,
                                          float theScale,
                                          const NFmiPoint &theBottomLeft,
                                          const NFmiPoint &theTopRight,
                                          int &theWidth,
                                          int &theHeight);

}  // namespace ProjectionFactory

#endif

// ======================================================================
