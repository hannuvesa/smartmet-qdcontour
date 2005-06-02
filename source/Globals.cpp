// ======================================================================
/*!
 * \file
 * \brief Definition of global variables
 */
// ======================================================================

#include "ColorTools.h"
#include "Globals.h"
#include "LazyQueryData.h"
#include "TimeTools.h"

#include "imagine/NFmiEsriBox.h"
#include "imagine/NFmiFace.h"
#include "imagine/NFmiFreeType.h"
#include "imagine/NFmiPath.h"

#include "newbase/NFmiAreaFactory.h"
#include "newbase/NFmiSettings.h"
#include "newbase/NFmiTime.h"

#include <string>

using NFmiSettings::Optional;
using namespace Imagine;
using namespace std;

// ----------------------------------------------------------------------
/*!
 * \brief Constructor for global variables
 */
// ----------------------------------------------------------------------

Globals::Globals()
  : verbose(false)
  , force(false)
  , cmdline_querydata()
  , cmdline_files()
  , datapath(Optional<string>("qdcontour::querydata_path","."))
  , mapspath(Optional<string>("qdcontour::maps_path","."))
  , savepath(".")
  , prefix()
  , suffix()
  , format("png")
  , gamma(-1)
  , intent()
  , alphalimit(-1)
  , pngquality(-1)
  , jpegquality(-1)
  , savealpha(true)
  , reducecolors(false)
  , wantpalette(false)
  , forcepalette(false)
  , contourinterpolation("Linear")
  , contourtriangles(1)
  , smoother("None")
  , smootherradius(1)
  , smootherfactor(1)
  , expanddata(false)
  , projection()
  , filter("none")
  , foregroundrule("Over")
  , background()
  , foreground()
  , mask()
  , combine()
  , backgroundimage()
  , foregroundimage()
  , maskimage()
  , combineimage()
  , combinex(0)
  , combiney(0)
  , combinerule("Over")
  , combinefactor(1)
  , erase("transparent")
  , fillrule("Atop")
  , strokerule("Atop")
  , directionparam("WindDirection")
  , speedparam("WindSpeedMS")
  , arrowscale(1)
  , arrowfillcolor("white")
  , arrowstrokecolor("black")
  , arrowfillrule("Over")
  , arrowstrokerule("Over")
  , arrowfile("")
  , windarrowscaleA(0)
  , windarrowscaleB(0)
  , windarrowscaleC(1)
  , windarrowdx(0)
  , windarrowdy(0)
  , arrowpoints()
  , queryfilelist()
  , queryfilenames()
  , queryinfo(0)
  , querydatalevel(-1)
  , timesteps(24)
  , timestep(0)
  , timeinterval(0)
  , timestepskip(0)
  , timesteprounding(1)
  , timestampflag(1)
  , timestampzone("local")
  , timestampimagex(0)
  , timestampimagey(0)
  , timestampimageformat("hourdateyear")
  , timestampimagefont("misc/6x13B.pcf.gz:6x13")
  , timestampimagecolor(NFmiColorTools::Black)
  , timestampimagebackground(NFmiColorTools::NoColor)
  , timestampimagexmargin(2)
  , timestampimageymargin(2)
  , contourlabelimagexmargin(20)
  , contourlabelimageymargin(20)
  , highpressureimage()
  , highpressurerule("Over")
  , highpressurefactor(1)
  , highpressureminimum(980)
  , lowpressureimage()
  , lowpressurerule("Over")
  , lowpressurefactor(1)
  , lowpressuremaximum(1020)
  , pressurelocator()
  , labellocator()
  , symbollocator()
  , calculator()
  , querystreams()
  , shapespecs()
  , specs()
{
  symbollocator.minDistanceToDifferentParameter(8);
  symbollocator.minDistanceToDifferentValue(8);
  symbollocator.minDistanceToSameValue(8);
}

// ----------------------------------------------------------------------
/*!
 * \brief Destructor
 */
// ----------------------------------------------------------------------

Globals::~Globals()
{
  clear_querystreams();
}

// ----------------------------------------------------------------------
/*!
 * \brief Delete all active querystreams
 */
// ----------------------------------------------------------------------

void Globals::clear_querystreams()
{
  for(vector<LazyQueryData *>::iterator it = querystreams.begin();
	  it != querystreams.end();
	  ++it)
	{
	  delete *it;
	}
  querystreams.resize(0);
  queryinfo = 0;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set image modes
 */
// ----------------------------------------------------------------------

void Globals::setImageModes(NFmiImage & theImage) const
{
  theImage.SaveAlpha(savealpha);
  theImage.WantPalette(wantpalette);
  theImage.ForcePalette(forcepalette);
  if(gamma>0) theImage.Gamma(gamma);
  if(!intent.empty()) theImage.Intent(intent);
  if(pngquality>=0) theImage.PngQuality(pngquality);
  if(jpegquality>=0) theImage.JpegQuality(jpegquality);
  if(alphalimit>=0) theImage.AlphaLimit(alphalimit);
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the area object
 */
// ----------------------------------------------------------------------

std::auto_ptr<NFmiArea> Globals::createArea() const
{
  if(projection.empty())
	throw runtime_error("A projection specification is required");

  return NFmiAreaFactory::Create(projection);
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the time stamp string to be rendered in the image
 */
// ----------------------------------------------------------------------

const std::string Globals::getImageStampText(const NFmiTime & theTime) const
{
  const int obsyy = theTime.GetYear();
  const int obsmm = theTime.GetMonth();
  const int obsdd = theTime.GetDay();
  const int obshh = theTime.GetHour();
  const int obsmi = theTime.GetMin();

  // Interpretation: The age of the forecast is the age
  // of the oldest forecast
  
  NFmiTime tfor;
  
  for(unsigned int qi=0; qi<querystreams.size(); qi++)
	{
	  NFmiTime futctime = querystreams[qi]->OriginTime();
	  NFmiTime tlocal = TimeTools::ConvertZone(futctime,timestampzone);
	  if(qi==0 || tlocal.IsLessThan(tfor))
		tfor = tlocal;
	}

  const int foryy = tfor.GetYear();
  const int formm = tfor.GetMonth();
  const int fordd = tfor.GetDay();
  const int forhh = tfor.GetHour();
  const int formi = tfor.GetMin();

  char buffer[100];

  string stamp;
  if(timestampimage == "obs")
	{
	  if(timestampimageformat == "hour") // hh:mi
		sprintf(buffer,"%02d:%02d",obshh,obsmi);
	  
	  else if(timestampimageformat == "hourdate") // hh:mi dd.mm.
		sprintf(buffer,"%02d:%02d %02d.%02d.",obshh,obsmi,obsdd,obsmm);

	  else // hh:mi dd.mm.yyyy
		sprintf(buffer,"%02d:%02d %02d.%02d.%04d",obshh,obsmi,obsdd,obsmm,obsyy);
	  stamp = buffer;
	}
  else if(timestampimage == "for")
	{
	  if(timestampimageformat == "hour") // hh:mi
		sprintf(buffer,"%02d:%02d",forhh,formi);
	  else if(timestampimageformat == "hourdate") // hh:mi dd.mm.
		sprintf(buffer,"%02d:%02d %02d.%02d.",forhh,formi,fordd,formm);
	  else // hh:mi dd.mm.yyyy
		sprintf(buffer,"%02d:%02d %02d.%02d.%04d",forhh,formi,fordd,formm,foryy);
	  stamp = buffer;
	}
  else if(timestampimage == "forobs")
	{
	  if(timestampimageformat == "hour") // hh:mi +hh
		sprintf(buffer,"%02d:%02d",forhh,formi);
	  else if(timestampimageformat == "hourdate") // dd.mm. hh:mi + hh
		sprintf(buffer,"%02d.%02d. %02d:%02d",fordd,formm,forhh,formi);
	  else // dd.mm.yy hh:mi +hh
		sprintf(buffer,"%02d.%02d.%04d %02d:%02d",
				fordd,formm,foryy,forhh,formi);
	  
	  stamp = buffer;

	  const long diff = theTime.DifferenceInMinutes(tfor);
	  if(diff%60==0 && timestep%60==0)
		sprintf(buffer," %s%ldh",(diff<0 ? "" : "+"), diff/60);
	  else
		sprintf(buffer," %s%ldm",(diff<0 ? "" : "+"), diff);

	  stamp += buffer;

	}
  return stamp;
}

// ----------------------------------------------------------------------
/*!
 * \brief Draw the given text into the time stamp position in the image
 */
// ----------------------------------------------------------------------

void Globals::drawImageStampText(NFmiImage & theImage,
								 const std::string & theText) const
{
  if(theText.empty())
	return;

  NFmiFace face(timestampimagefont);
  face.Background(true);
  
  int x = timestampimagex;
  int y = timestampimagey;
  
  if(x<0) x+= theImage.Width();
  if(y<0) y+= theImage.Height();

  if(timestampimagebackground != NFmiColorTools::NoColor)
	{
	  face.Background(true);
	  face.BackgroundMargin(timestampimagexmargin,timestampimageymargin);
	  face.BackgroundColor(timestampimagebackground);
	}

  face.Draw(theImage,
			x,y,
			theText,
			kFmiAlignNorthWest,
			timestampimagecolor);
}

// ----------------------------------------------------------------------
/*!
 * \brief Draw the "combine" image over the given image
 */
// ----------------------------------------------------------------------

void Globals::drawCombine(NFmiImage & theImage) const
{
  if(combine.empty())
	return;

  NFmiColorTools::NFmiBlendRule rule = ColorTools::checkrule(combinerule);

  theImage.Composite(combineimage,
					 rule,
					 kFmiAlignNorthWest,
					 combinex,
					 combiney,
					 combinefactor);
}

// ----------------------------------------------------------------------
/*!
 * \brief Check if any queryfile is outdated
 */
// ----------------------------------------------------------------------

bool Globals::isOutdated() const
{
  for(vector<LazyQueryData *>::const_iterator it = querystreams.begin();
	  it != querystreams.end();
	  ++it)
	{
	  if(*it == 0)
		continue;
	  if((*it)->IsOutdated())
		return true;
	}
  return false;
}

// ======================================================================
