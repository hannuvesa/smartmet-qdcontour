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
#include "imagine/NFmiFontHershey.h"	// for Hershey fonts
#include "imagine/NFmiPath.h"
#include "imagine/NFmiText.h"			// for labels

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
  , wantpalette(false)
  , forcepalette(false)
  , contourinterpolation("Linear")
  , contourtriangles(1)
  , smoother("None")
  , smootherradius(1)
  , smootherfactor(1)
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
  , calculator()
  , querystreams()
  , shapespecs()
  , specs()
{
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
	  // hh:mi dd.mm.yyyy
	  sprintf(buffer,"%02d:%02d %02d.%02d.%04d",
			  obshh,obsmi,obsdd,obsmm,obsyy);
	  stamp = buffer;
	}
  else if(timestampimage == "for")
	{
	  // hh:mi dd.mm.yyyy
	  sprintf(buffer,"%02d:%02d %02d.%02d.%04d",
			  forhh,formi,fordd,formm,foryy);
	  stamp = buffer;
	}
  else if(timestampimage == "forobs")
	{
	  // hh:mi dd.mm.yyyy +hh
	  const long diff = theTime.DifferenceInMinutes(tfor);
	  if(diff%60==0 && timestep%60==0)
		sprintf(buffer,"%02d.%02d.%04d %02d:%02d %s%ldh",
				fordd,formm,foryy,forhh,formi,
				(diff<0 ? "" : "+"), diff/60);
	  else
		sprintf(buffer,"%02d.%02d.%04d %02d:%02d %s%ldm",
				fordd,formm,foryy,forhh,formi,
				(diff<0 ? "" : "+"), diff);
	  stamp = buffer;
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

  NFmiFontHershey font("TimesRoman-Bold");
  
  int x = timestampimagex;
  int y = timestampimagey;
  
  if(x<0) x+= theImage.Width();
  if(y<0) y+= theImage.Height();
		  
  NFmiText text(theText,font,14,x,y,kFmiAlignNorthWest,0.0);

  // And render the text

  NFmiPath path = text.Path();

  NFmiEsriBox box = path.BoundingBox();

  NFmiPath rect;
  int w = 4;
  rect.MoveTo(box.Xmin()-w,box.Ymin()-w);
  rect.LineTo(box.Xmax()+w,box.Ymin()-w);
  rect.LineTo(box.Xmax()+w,box.Ymax()+w);
  rect.LineTo(box.Xmin()-w,box.Ymax()+w);
  rect.CloseLineTo();

  rect.Fill(theImage,
			NFmiColorTools::MakeColor(180,180,180,32),
			NFmiColorTools::kFmiColorOver);

  path.Stroke(theImage,
			  NFmiColorTools::Black,
			  NFmiColorTools::kFmiColorCopy);

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

// ======================================================================
