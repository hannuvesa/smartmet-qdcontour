// ======================================================================
/*!
 * \file
 * \brief Interface of class LabelLocator
 */
// ======================================================================

#ifndef LABELLOCATOR_H
#define LABELLOCATOR_H

class LabelLocator
{
public:

  ~LabelLocator();
  LabelLocator();

  void clear();

  void boundingBox(int theX1, int theY1, int theX2, int theY2);
  void minDistanceToSameValue(float theDistance);
  void minDistanceToDifferentValue(float theDistance);
  void minDistanceToDifferentParameter(float theDistance);

private:

  // Intentionally disabled:

  LabelLocator(const LabelLocator & theLocator);
  LabelLocator & operator=(const LabelLocator & theLocator);

  bool itHasBBox;
  int itsBBoxX1;
  int itsBBoxY1;
  int itsBBoxX2;
  int itsBBoxY2;

  float itsMinDistanceToSameValue;
  float itsMinDistanceToDifferentValue;
  float itsMinDistanceToDifferentParameter;

}; // class LabelLocator

#endif // LABELLOCATOR_H

// ======================================================================
