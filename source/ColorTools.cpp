// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace ColorTools
 */
// ======================================================================

#include "ColorTools.h"
#include <stdexcept>
#include <vector>

using namespace std;

namespace
{
  // ----------------------------------------------------------------------
  /*!
   * \brief Parse a hexstring into an integer
   * \return The integer, or -1 on failure
   */
  // ----------------------------------------------------------------------

  int hex2int(const string & theHex)
  {
	int value=0;
	for(unsigned int i=0; i<theHex.length(); i++)
	  {
		value *= 16;
		if(theHex[i]>='0' && theHex[i]<='9')
		  value += theHex[i]-'0';
		else if(theHex[i]>='A' && theHex[i]<='F')
		  value += 10+theHex[i]-'A';
		else if(theHex[i]>='a' && theHex[i]<='f')
		  value += 10+theHex[i]-'a';
		else
		  return -1;
	  }
	return value;
  }

  // ----------------------------------------------------------------------
  /*!
   * \brief Parse a hexadecimal color
   *
   * Allowed formats are AARRGGBB and RRGGBB
   *
   * \param theColor The color as a hexadecimal string
   * \return The color, or NFmiColorTools::MissingColor
   */
  // ----------------------------------------------------------------------

  NFmiColorTools::Color parse_hex_color(const std::string & theColor)
  {
	int a,r,g,b;
	if(theColor.length()==6)
	  {
		a = 0;
		r = hex2int(theColor.substr(0,2));
		g = hex2int(theColor.substr(2,2));
		b = hex2int(theColor.substr(4,2));
	  }
	else if(theColor.length()==8)
	  {
		a = hex2int(theColor.substr(0,2));
		r = hex2int(theColor.substr(2,2));
		g = hex2int(theColor.substr(4,2));
		b = hex2int(theColor.substr(6,2));
	  }
	if(r>=0 && g>=0 && b>=0 && a>=0)
	  return NFmiColorTools::MakeColor(r,g,b,a);

	return NFmiColorTools::MissingColor;
  }

  // ----------------------------------------------------------------------
  /*!
   * \brief Parse a decimal color
   *
   * Allowed formats are R,G,B and R,G,B,A
   *
   * \param theColor The string to parse
   * \return The color, or NFmiColorTools::MissingColor
   */
  // ----------------------------------------------------------------------

  NFmiColorTools::Color parse_dec_color(const string & theColor)
  {
	vector<int> tmp;
	int value=-1;
	for(unsigned int i=0; i<theColor.length(); i++)
	  {
		const char ch = theColor[i];
		if(ch>='0' && ch<='9')
		  {
			if(value<0)
			  value = ch-'0';
			else
			  value = value*10+ch-'0';
			}
		else if(ch==',')
		  {
			tmp.push_back(value);
			value = -1;
		  }
		else
		  return NFmiColorTools::MissingColor;
	  }
	if(value>=0)
	  tmp.push_back(value);
	  
	if(tmp.size()==3)
	  return NFmiColorTools::MakeColor(tmp[0],tmp[1],tmp[2],0);
	else if(tmp.size()==4)
	  return NFmiColorTools::MakeColor(tmp[0],tmp[1],tmp[2],tmp[3]);
	else
	  return NFmiColorTools::MissingColor;
    }

  // ----------------------------------------------------------------------
  /*!
   * \brief Parse a named color
   *
   * Allowed formats are name and name,alpha. (+none and transparent)
   *
   * \param theColor The string to parse
   * \return The color, or NFmiColorTools::MissingColor
   */
  // ----------------------------------------------------------------------

  NFmiColorTools::Color parse_named_color(const string & theColor)
  {
	unsigned int pos = theColor.find(",");
	if(pos == string::npos)
	  return NFmiColorTools::ColorValue(theColor);

	int value = -1;
	for(unsigned int i=pos+1; i<theColor.length(); i++)
	  {
		const char ch = theColor[i];
		if(ch>='0' && ch<='9')
		  {
			if(value<0)
			  value = ch-'0';
			else
			  value = value*10+ch-'0';
		  }
		else
		  return NFmiColorTools::MissingColor;
	  }
	if(value<0)
	  return NFmiColorTools::MissingColor;

	NFmiColorTools::Color tmp = NFmiColorTools::ColorValue(theColor.substr(0,pos));
	return NFmiColorTools::ReplaceAlpha(tmp,value);
  }


} // namespace anonymous

namespace ColorTools
{

  // ----------------------------------------------------------------------
  /*!
   * \brief Parse a text description of a color
   *
   * Returns NFmiColorTools::MissingColor on failure
   *
   * \param theColor The color string
   * \return The color
   */
  // ----------------------------------------------------------------------

  NFmiColorTools::Color parsecolor(const string & theColor)
  {

	if(theColor.empty())
	  return NFmiColorTools::MissingColor;

	// Handle hex format number AARRGGBB or RRGGBB

	const char ch1 = theColor[0];
	if(ch1=='#')
	  return parse_hex_color(theColor.substr(1));
  
	// Handle decimal number format R,G,B or R,G,B,A

	if(ch1>='0' && ch1<='9')
	  return parse_dec_color(theColor);

	// Handle ascii format
  
	return parse_named_color(theColor);
  }

  // ----------------------------------------------------------------------
  /*!
   * \brief Check a textual color parses ok
   *
   * Throws an exception if the color is not recognized
   *
   * \param theColor The color string
   * \return The color
   */
  // ----------------------------------------------------------------------

  NFmiColorTools::Color checkcolor(const std::string & theColor)
  {
	NFmiColorTools::Color c = parsecolor(theColor);
	if(c != NFmiColorTools::MissingColor)
	  return c;

	throw runtime_error("Unrecognized color " + theColor);
  }

  // ----------------------------------------------------------------------
  /*!
   * \brief Parse a text description of a blending rule
   *
   * Return NFmiColorTools::kFmiColorRuleMissing on failure
   *
   * \param theRule The string to parse
   * \return The blending rule
   */
  // ----------------------------------------------------------------------

  NFmiColorTools::NFmiBlendRule parserule(const std::string & theRule)
  {
	return NFmiColorTools::BlendValue(theRule);
  }

  // ----------------------------------------------------------------------
  /*!
   * \brief Check the blending rule parses ok
   *
   * Throws an exception for unknown blending rules
   *
   * \param theRule The string to parse
   * \return The blending rule
   */
  // ----------------------------------------------------------------------

  NFmiColorTools::NFmiBlendRule checkrule(const std::string & theRule)
  {
	NFmiColorTools::NFmiBlendRule rule = parserule(theRule);
	if(rule != NFmiColorTools::kFmiColorRuleMissing)
	  return rule;
	throw runtime_error("Unknown blending rule "+theRule);
  }


} // namespace ColorTools

// ======================================================================
