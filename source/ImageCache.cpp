// ======================================================================
/*!
 * \brief Implementation of class ImageCache
 */
// ======================================================================

#include "ImageCache.h"

using namespace Imagine;
using namespace std;

// ----------------------------------------------------------------------
/*!
 * \brief Clear the cache
 */
// ----------------------------------------------------------------------

void ImageCache::clear() const
{
  itsCache.clear();
}

// ----------------------------------------------------------------------
/*!
 * \brief Find image from cache (or read it if necessary)
 */
// ----------------------------------------------------------------------

#include <iostream>

const NFmiImage & ImageCache::getImage(const string & theFile) const
{
  storage_type::const_iterator it = itsCache.find(theFile);
  if(it != itsCache.end())
	return it->second;

  pair<storage_type::const_iterator,bool> ret = itsCache.insert(storage_type::value_type(theFile,NFmiImage(theFile)));

  if(ret.second)
	return ret.first->second;

  throw runtime_error("ImageCache failed to store '"+theFile+"'");

}
