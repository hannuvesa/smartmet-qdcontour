// ======================================================================
/*!
 * \brief Interface of class ImageCache
 */
// ======================================================================

#ifndef IMAGECACHE_H
#define IMAGECACHE_H

#include "NFmiImage.h"
#include <map>
#include <string>

class ImageCache
{
public:

  const Imagine::NFmiImage & getImage(const std::string & theFile) const;
  void clear() const;

private:

  typedef std::map<std::string,Imagine::NFmiImage> storage_type;
  mutable storage_type itsCache;

};

#endif // IMAGECACHE_H

// ======================================================================

