// ======================================================================
/*!
 * \file
 * \brief Interface of class LabelLocator
 */
// ======================================================================

#ifndef LABELLOCATOR_H
#define LABELLOCATOR_H

#include <list>
#include <map>

class LabelLocator
{
public:

  ~LabelLocator();
  LabelLocator();

  bool empty() const;
  void clear();

  void boundingBox(int theX1, int theY1, int theX2, int theY2);
  void minDistanceToSameValue(float theDistance);
  void minDistanceToDifferentValue(float theDistance);
  void minDistanceToDifferentParameter(float theDistance);

  void parameter(int theParameter);
  void nextTime();

private:

  // Intentionally disabled:

  LabelLocator(const LabelLocator & theLocator);
  LabelLocator & operator=(const LabelLocator & theLocator);

  bool inside(int theX1, int theY1) const;

  bool itHasBBox;
  int itsBBoxX1;
  int itsBBoxY1;
  int itsBBoxX2;
  int itsBBoxY2;

  float itsMinDistanceToSameValue;
  float itsMinDistanceToDifferentValue;
  float itsMinDistanceToDifferentParameter;

  typedef std::pair<int,int> XY;
  typedef std::list<XY> Coordinates;
  typedef std::map<int,Coordinates> ParamCoordinates;

  int itsActiveParameter;
  ParamCoordinates itsPreviousCoordinates;
  ParamCoordinates itsCurrentCoordinates;

}; // class LabelLocator

#endif // LABELLOCATOR_H

// ======================================================================
