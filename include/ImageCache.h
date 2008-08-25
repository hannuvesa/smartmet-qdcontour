// ======================================================================
/*!
 * \brief Interface of class ImageCache
 */
// ======================================================================

#ifndef IMAGECACHE_H
#define IMAGECACHE_H

#ifdef IMAGINE_WITH_CAIRO
# include "ImagineXr.h"
#else
# include "NFmiImage.h"
#endif

#include <map>
#include <string>

class ImageCache
{
public:
  const ImagineXr_or_NFmiImage & getImage(const std::string & theFile) const;
  void clear() const;

private:
  typedef std::map< std::string, ImagineXr_or_NFmiImage > storage_type;
  mutable storage_type itsCache;

};

#endif // IMAGECACHE_H

// ======================================================================

