// ======================================================================
/*!
 * \file
 * \brief Main program for qdcontour
 */
// ======================================================================
/*!
 * \mainpage qdcontour - A querydata contouring application
 *
 * <ol>
 *  <li>\ref intro_section</li>
 *  <li>\ref cmdline_section</li>
 *  <li>\ref cmd_section</li>
 *  <ol>
 *    <li>\ref variables_section</li>
 *    <li>\ref flowcontrol_section</li>
 *    <li>\ref comment_section</li>
 *  </ol>
 *  <li>\ref renderingshapefiles_section</li>
 *  <ol>
 *    <li>\ref projection_section</li>
 *    <li>\ref shape_section</li>
 *    <li>\ref format_section</li>
 *    <li>\ref savingshape_section</li>
 *  </ol>
 *  <li>\ref renderingcontours_section</li>
 *  <ol>
 *    <li>\ref querydata_section</li>
 *    <li>\ref filename_section</li>
 *    <li>\ref contour_section</li>
 *    <li>\ref arrow_section</li>
 *    <li>\ref label_section</li>
 *    <li>\ref rendercontour_section</li>
 *    <li>\ref filtering_section</li>
 *    <li>\ref smoothing_section</li>
 *    <li>\ref timestamp_section</li>
 *  </ol>
 *  <li>\ref appendix_section</li>
 *  <ol>
 *    <li>\ref color_section</li>
 *    <li>\ref blending_section</li>
 *    <li>\ref shapesample_section</li>
 *    <li>\ref sample_section</li>
 *    <li>\ref deprecated_section</li>
 *  </ol>
 * </ol>
 *
 * \section intro_section Introduction
 *
 * The qdcontour program takes as input a control file, and depending
 * on the contents of the control file renders either contour images
 * from querydata, or maps from shapefiles.
 *
 * \section cmdline_section Command line options
 *
 * The program is used as follows:
 * \code
 * qdcontour [-v] [-f] [controlfile]
 * \endcode
 *
 * The options are
 * <dl>
 * <dt>-v</dt>
 * <dd>Verbose output. For example output files are printed.</dd>
 * <dt>-f</dt>
 * <dd>Force output. Normally qdcontour may decide not to draw
 *     some contour image since an image by that name already
 *     exists. This greatly speeds up contouring by omitting
 *     unnecessary redrawing. However, occasionally one
 *     changes the control file somehow, and a redraw must be
 *     done by using the -f option.
 * </dd>
 * </dl>
 *
 *
 * \section cmd_section Control file syntax
 *
 * \subsection comment_section Comments
 *
 * Normal comments of the following form are supported.
 * \code
 * # comment
 * // comment
 * command # comment
 * command // comment
 * \endcode
 *
 *
 * \subsection variables_section Variables
 *
 * The control file parser does not at the moment provide support
 * for user defined variables. However, this may change in the
 * future.
 *
 * \subsection flowcontrol_section Flow control
 *
 * The control file parser provides for very little flow control,
 * in particular there are no loop structures.
 *
 * \htmlonly
 * <dl>
 * <dt>include [filename]</dt>
 * <dd>
 * The include command works via the NFmiPreProcessor class in
 * the newbase library.
 * </dd>
 * </dl>
 * \endhtmlonly
 *
 * It would be possible to implement various extra conditional
 * directives in the \#if - \#else - \#endif style. For example,
 * one could automatically enable seasonal contouring colors
 * this way.
 *
 *
 * \section renderingshapefiles_section Rendering shapefiles
 *
 * A complete sample control file can be found in the
 * appendix at \ref shapesample_section.
 *
 * \subsection projection_section Specifying the projection
 *
 * Before an image containing possibly multiple individual
 * shapefiles can be rendered, one must specify the
 * projection and the bounding box of the area to be rendered.
 * There are multiple ways to do this, each one automatically
 * calculating the missing information.
 *
 * At the moment qdcontour supports only one projection,
 * the stereographic one. It is specified using the command
 * \code
 * stereographic [centrallongitude] [centrallatitude] [truelatitude]
 * \endcode
 * where <em>centrallongitude</em> is usually 25 when rendering Finland,
 * 20 for Skandinavia and 10 for Europe. <em>centrallatitude</em>
 * is almost always 90 to indicate the polar stereographic projection.
 * The true latitude is often chosen to be 60, so that the distortion
 * would be minimal at the latitudes of Finland.
 *
 * Once the projection has been specified, one must specify the
 * geographic bounding box as well as the image size. Below
 * we go through various ways of doing this.
 *
 * \code
 * bottomleft [longitude] [latitude]
 * topright [longitude] [latitude]
 * size [width] [height]
 * \endcode
 * In above we specify the bounding box of the geographic area
 * by giving the corner coordinates in geographic units (degrees).
 * Then we fix the size of the produced image in both X and Y
 * directions. In effect this means we can force a geographic
 * area to be rendered at some fixed size, which may distort
 * the map.
 *
 * A better alternative is
 * \code
 * bottomleft [longitude] [latitude]
 * topright [longitude] [latitude]
 * width [width]
 * \endcode
 * or
 * \code
 * bottomleft [longitude] [latitude]
 * topright [longitude] [latitude]
 * height [height]
 * \endcode
 * In these cases the missing image dimension is calculated
 * automatically so that the map will not be distorted.
 *
 * Often however specifying the corner points is very
 * inconvenient, one may have to perform several trials
 * before finding a reasonable area. Overall the best
 * strategy may be to use
 * \code
 * center [longitude] [latitude]
 * scale [scale]
 * size [width] [height]
 * \endcode
 * In this case one specifies the location to be placed
 * at the center of the map, the desired size of the image
 * and then adjusts the scale so that the desired area
 * is completely covered.
 *
 * The scale can be approximately interpreted to mean
 * how many kilometers one pixel is, but only roughly.
 *
 * \subsection shape_section Drawing a single shapefile
 *
 * There are two ways to render a shape file, either by filling
 * or stroking polyline or polygon data, or by placing markers
 * at point data.
 *
 * Polyline or polygon data rendering is controlled using the
 * command
 * \code
 * shape [shapefilename] [fillcolor] [strokecolor]
 * \endcode
 * where <em>shapefilename</em> is the name of the shape, without
 * any suffix such as .shp or .dbf. If the filename cannot
 * be found directly, the environmental <em>imagine::shapes_path</em>
 * is used to locate the file.
 *
 * The fillcolor and strokecolor can be in the form given
 * in \ref color_section. Often one of the colors is specified
 * to be <em>none</em>, indicating that either filling or
 * stroking is not to be done.
 *
 * Point data rendering is controlled using the command
 * \code
 * shape [shapefilename] mark [markerfilename] [blendingrule] [blendfactor]
 * \endcode
 * where <em>markefilename</em> is the image to be used as a marker.
 * The blending rules are explained in section \ref blending_section,
 * the <em>blendfactor</em> can be used as an extra alpha factor
 * in the range 0-1, but often one uses the value 1 to indicate no extra
 * blending is to be done.
 *
 * \subsection format_section Specifying the graphics format
 *
 * Before actually rending the final image, once should
 * choose the desired graphics format and adjust the
 * parameters related to the format as desired.
 *
 * The available commands are
 *
 * \htmlonly
 * <dl>
 * <dt>format [format]</dt>
 * <dd>where [format] may be one of png, jpg or gif. The default
 *     format is png.
 * </dd>
 * <dt>savealpha [0/1]</dt>
 * <dd>is used to choose whether the alpha channel of the rendered
 *     image should be saved or ignored. Typically on ignores
 *     the alpha channel, unless the image intentionally contains
 *     transparent areas. The default is to save alpha channel.
 * </dd>
 * <dt>wantpalette [0/1]</dt>
 * <dd>This option is relevant only to the png format, which is
 *     the only one that allows both palette and truecolor storage.
 *     The flag indicates whether qdcontour should <em>try</em>
 *     to store the image in palette mode, if it can. For png
 *     this means there can be atmost 256 different colors,
 *     including differences in alpha.
 * </dd>
 * <dt>forcepalette [0/1]</dt>
 * <dd>The intent is to enable forcing a palette save for png
 *     format. However, this has <b>not</b> been implemented yet.
 * </dd>
 * <dt>jpegquality [0-100]</dt>
 * <dd>This allows one to control the quality of the compressed
 *     jpeg image. The higher the number, the better the quality,
 *     but also the larger the image file will be. The default
 *     value is -1, which implies the default chosen by
 *     libjpeg is used.
 * </dd>
 * <dt>pngquality [1-9]</dt>
 * <dd>This allows one to control the compression level of
 *     the png format. The default value is -1, which implies
 *     the default chosen by libpng is used.
 * </dd>
 * <dt>alphalimit [-1-127]</dt>
 * <dd>Alphalimit enables one to enforce binary transparency,
 *     that is, alpha is either on or off. The actual values
 *     depend on the chosen image format. The value -1 implies
 *     binary transparency is not enforced. This value is
 *     most relevant for the gif format, which does not
 *     support full transparency.
 * </dd>
 * </dl>
 * \endhtmlonly
 * Perhaps the set of most typical image format related commands is
 * \code
 * format png
 * savealpha 0
 * wantpalette 1
 * \endcode
 *
 * \subsection savingshape_section Saving the result
 *
 * Once the projection and the shapefile rendering directives
 * have been given, the actual output can be generated
 * using the command
 * \code
 * erase [color]
 * draw shapes [filename]
 * clear corners
 * clear sizes
 * clear shapes
 * \endcode
 * which causes all the shapefiles specified earlier to be
 * drawn in the given filename. The suffix of the filename
 * is chosen automatically according to the active graphics
 * format.
 *
 * The <em>erase</em> command essentially defines the background
 * color for the image. The default is to use transparent color,
 * which most likely is not suitable for most maps.
 *
 * The final three clearing commands do not have to be given,
 * but act as safety in case somebody intends to draw more
 * shapes in the same control file.
 *
 *
 * \section renderingcontours_section Rendering contours
 *
 * \subsection querydata_section Querydata control
 *
 * One may specify the querydata files to be used with the command
 * \code
 * querydata [filename1,filename2,...,filenameN]
 * \endcode
 * If the filenames are not absolute, and cannot be found
 * at the given places, the fmi.conf setting qdcontour::querydata_path
 * is used for searching for the file. As a special case, if the given
 * name is a directory, the newest querydata in that directory is used.
 * The time range one is able to contour is determined by the
 * common time span of the given queryfiles.
 *
 * Whenever a parameter is specified to be contoured, the order
 * of the queryfiles is the order in which the parameter is searched.
 *
 * Since one is only able to use one level when contouring,
 * one may choose it using the following command.
 * \code
 * querydatalevel [level]
 * \endcode
 * The default level is 1.
 *
 * \subsection filename_section Controlling generated image filenames
 *
 * As qdcontour goes through the available times in the specified
 * querydata, it will attempt to determine an unique filename
 * for each image. By default each filename is of the form
 * \code
 * [prefix][timestamp][origintimestamp][suffix].[format]
 * \endcode
 * The <em>format</em> is uniquely determined by using the
 * command
 * \code
 * format [format]
 * \endcode
 * where format is one of png, jpeg or gif.
 *
 * The prefix and suffix are by default null strings, but can
 * be modified with the commands
 * \code
 * prefix [prefix]
 * suffix [suffix]
 * \endcode
 * The <em>origintimestamp</em> is the time stamp string generated
 * from the origin time of the query data, and is intended for
 * distinguishing between various forecast times. Whether or not
 * the string is used can be modified with the command
 * \code
 * timestamp [0/1]
 * \endcode
 * Finally, the files must be stored in some directory, which
 * is controlled with
 * \code
 * savepath [path]
 * \endcode
 * The default value for <em>path</em> is ".", the current directory.
 *
 * For example, when contouring T2m forecasts one might use
 * \code
 * prefix ENN_
 * suffix _T2M
 * timestamp 1
 * savepath /foo/bar
 * \endcode
 *
 * \subsection contour_section Controlling the contours
 *
 * One is able to contour several parameters simultaneously.
 * The specifications for each individual parameter are started
 * by the command
 * \code
 * param [parametername]
 * \endcode
 * To draw contour lines on the parameter one can then repeatedly
 * use
 * \code
 * strokerule [blendrule]
 * contourline [value] [color]
 * \endcode
 * or define multiple lines simultaneously with
 * \code
 * contourlines [startvalue] [endvalue] [step] [startcolor] [endcolor]
 * \endcode
 * The colour for each individual line is interpolated linearly
 * between <em>startcolor</em> and <em>endcolor</em> in the HSV color
 * space, which is more likely suitable in this case than direct RGB
 * interpolation.
 *
 * One should remember, that the latest defined <em>strokerule</em>
 * is the one to be used for each <em>contourline</em>.
 *
 * Alternatively, one may specify some values intervals to be filled
 * with the desired colors by repeatedly applying the command
 * \code
 * fillrule [blendrule]
 * contourfill [startvalue] [endvalue] [color]
 * \endcode
 * As special cases, if <em>startvalue</em> is "-", it is interpreted
 * to mean minus infinity. Similarly <em>endvalue</em> would be interpreted
 * to be plus infinity. However, since the case
 * \code
 * contourfill - - [color]
 * \endcode
 * makes rarely sense, we define it instead to mean colouring all
 * missing values in the data. This is especially useful for data
 * which has limited range, such as radar data.
 *
 * Also, one may define multiple fills simultaneously with
 * \code
 * contourfills [startvalue] [endvalue] [step] [startcolor] [endcolor]
 * \endcode
 * which works similarly to the <em>contourlines</em> command documented
 * earlier.
 *
 * As with the case of <em>strokerule</em>, the latest defined
 * <em>blendrule</em> is the one to be used for each <em>contourfill</em>
 * command.
 *
 * Finally, one may use an image pattern instead of a fixed color.
 * This is accomplished using the command
 * \code
 * contourpattern [startvalue] [endvalue] [patternfile] [blendrule] [blendfactor]
 * \endcode
 * where <em>patternfile</em> is the path to the image file containing the
 * pattern to be used when filling. The filling is performed using
 * <em>blendrule</em> with the extra <em>blendfactor</em> alpha factor.
 *
 * There is no similar <em>patternfills</em> equivalent as before, as
 * interpolation between the start and end patterns would rarely make
 * sense.
 *
 * \subsection arrow_section Drawing arrows from querydata
 *
 * One may choose which parameters will be used as a direction - speed
 * pair when rendering arrows using the commands
 * \code
 * directionparam [parametername]
 * speedparam [parametername]
 * \endcode
 * The default values are <em>WindDirection</em> and <em>WindSpeedMS</em>
 * respectively.
 *
 * The arrow to be drawn at each location is defined with the
 * command
 * \code
 * arrowpath [filename]
 * \endcode
 * where <em>filename</em> contains an SVG-style (limited) path definition,
 * which will be rotated according to the direction parameter. The arrow
 * is rendered according to the rules specified by
 * \code
 * arrowfill [color] [blendrule]
 * arrowstroke [color] [blendrule]
 * \endcode
 * The size of the arrow is principally determined by
 * \code
 * arrowscale [scalefactor]
 * \endcode
 * whose default value is 1.0. The factor is used to scale the arrow
 * path definition uniformly. Additionally, one may also use the command
 * \code
 * windarrowscale [A] [B] [C]
 * \endcode
 * to scale the arrow nonlinearly and depending on the speed at the
 * location. The default values for a,b and c are 0,0 and 1 respectively,
 * and the formula applied to calculate an extra scaling factor for
 * the arrow is
 * \f[
 *   A \log (B S + 1) + C
 * \f]
 * where S is the speed value. Note that the value is always 1
 * for the default parameters, and no extra scaling thus occurs.
 * A is used to control linearly how fast the arrow grows, while
 * B is used to control it logarithmically. In general one should
 * try to find a suitable value for B first, to represent the speed
 * of increasing the size of the arrow, and then scale using
 * A to find a suitable final size.
 *
 * The arrows themselves are put on the image using the commands
 * \code
 * windarrow [longitude] [latitude]
 * windarrows [dx] [dy]
 * \endcode
 * The first form places the arrow at a specific location. Both the
 * direction and the speed will be interpolated, if necessary, for
 * the given coordinates.
 *
 * The second form places an arrow at each grid location, with the
 * given steps. Using <em>dx</em>=1 and <em>dy</em>=1 an arrow would
 * be placed at every grid point.
 *
 * \subsection label_section Placing text values in the images
 *
 * \subsection rendercontour_section Saving the results
 *
 * The contoured images can be saved using
 * \code
 * background [filename]
 * foreground [filename]
 * foregroundrule [blendingrule]
 * draw contours
 * clear contours
 * \endcode
 * The first command may be replaced with an <em>erase [color]</em> command,
 * provided a foreground is given. Alternatively the foreground may be
 * omitted, provided the background is given.
 *
 * The <em>clear contours</em> command is optional, and should only
 * be applied at the end of a file, or when the contour specifications
 * documented in the earlier sections really change.
 *
 * One may also modify each output image by overlaying some image on
 * top of them, for example a logo or a legend. This is accomplished
 * using
 * \code
 * combine [imagefilename] [x] [y] [blendingrule] [blendfactor]
 * \endcode
 * The extra image setting can be cleared using
 * \code
 * combine none
 * \endcode
 *
 * \subsection filtering_section Filtering the querydata
 *
 * \subsection smoothing_section Smoothening the querydata
 *
 * \subsection timestamp_section Placing timestamps in the images
 *
 * \section appendix_section Appendix
 *
 * \subsection color_section Color
 *
 * Internally the program represents colors using hexadecimal numbers
 * of form "AARRGGBB" where thr RGB components range from 00 to FF,
 * and the alpha component from 00 to 7F, 7F being transparent.
 * The highest bit is reserved for internal use to represent
 * the "none" color.
 *
 * The colors understood by the control file parser are
 *
 * \htmlonly
 * <dl>
 * <dt>none</dt>
 * <dd>This is a special color meaning no rendering is to be done.</dd>
 * <dt>transparent</dt>
 * <dd>This is equivalent to color #7f000000</dd>
 * <dt><em>colorname</em></dt>
 * <dd>The recognized names are listed in the table below</dd>
 * <dt><em>colorname</em>,<em>A</em></dt>
 * <dd>In addition to the color, one may specify alpha in the range 0-127.</dd>
 * <dt><em>R</em>,<em>G</em>,<em>B</em></dt>
 * <dd>The individual RGB components in the range 0-255.</dd>
 * <dt><em>R</em>,<em>G</em>,<em>B</em>,<em>A</em></dt>
 * <dd>The individual RGB components plus alpha in the range 0-127.</dd>
 * <dt>#AARRGGBB</dt>
 * <dd>The color directly in the hexadecimal form</dd>
 * </dl>
 * \endhtmlonly
 *
 * The recognized colors are those listed in the SVG specification
 * at the <a href="http://www.w3.org/TR/SVG/types.html#ColorKeyWords">W3C</a>
 * site.
 *
 * \htmlonly
 * <table align=center border=0 cellspacing=20>
 * <tr valign=top><td>
 * <table bgcolor='#ccffcc' cellpadding=2 border=3>
 * <tr><td>aliceblue</td><td>240,248,255</td><td style="background-color:rgb(240,248,255)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>antiquewhite</td><td>250,235,215</td><td style="background-color:rgb(250,235,215)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>aqua</td><td>0,255,255</td><td style="background-color:rgb(0,255,255)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>aquamarine</td><td>127,255,212</td><td style="background-color:rgb(127,255,212)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>azure</td><td>240,255,255</td><td style="background-color:rgb(240,255,255)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>beige</td><td>245,245,220</td><td style="background-color:rgb(245,245,220)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>bisque</td><td>255,228,196</td><td style="background-color:rgb(255,228,196)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>black</td><td>0,0,0</td><td style="background-color:rgb(0,0,0)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>blanchedalmond</td><td>255,235,205</td><td style="background-color:rgb(255,235,205)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>blue</td><td>0,0,255</td><td style="background-color:rgb(0,0,255)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>blueviolet</td><td>138,43,226</td><td style="background-color:rgb(138,43,226)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>brown</td><td>165,42,42</td><td style="background-color:rgb(165,42,42)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>burlywood</td><td>222,184,135</td><td style="background-color:rgb(222,184,135)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>cadetblue</td><td>95,158,160</td><td style="background-color:rgb(95,158,160)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>chartreuse</td><td>127,255,0</td><td style="background-color:rgb(127,255,0)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>chocolate</td><td>210,105,30</td><td style="background-color:rgb(210,105,30)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>coral</td><td>255,127,80</td><td style="background-color:rgb(255,127,80)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>cornflowerblue</td><td>100,149,237</td><td style="background-color:rgb(100,149,237)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>cornsilk</td><td>255,248,220</td><td style="background-color:rgb(255,248,220)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>crimson</td><td>220,20,60</td><td style="background-color:rgb(220,20,60)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>cyan</td><td>0,255,255</td><td style="background-color:rgb(0,255,255)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>darkblue</td><td>0,0,139</td><td style="background-color:rgb(0,0,139)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>darkcyan</td><td>0,139,139</td><td style="background-color:rgb(0,139,139)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>darkgoldenrod</td><td>184,134,11</td><td style="background-color:rgb(184,134,11)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>darkgray</td><td>169,169,169</td><td style="background-color:rgb(169,169,169)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>darkgreen</td><td>0,100,0</td><td style="background-color:rgb(0,100,0)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>darkgrey</td><td>169,169,169</td><td style="background-color:rgb(169,169,169)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>darkkhaki</td><td>189,183,107</td><td style="background-color:rgb(189,183,107)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>darkmagenta</td><td>139,0,139</td><td style="background-color:rgb(139,0,139)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>darkolivegreen</td><td>85,107,47</td><td style="background-color:rgb(85,107,47)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>darkorange</td><td>255,140,0</td><td style="background-color:rgb(255,140,0)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>darkorchid</td><td>153,50,204</td><td style="background-color:rgb(153,50,204)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>darkred</td><td>139,0,0</td><td style="background-color:rgb(139,0,0)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>darksalmon</td><td>233,150,122</td><td style="background-color:rgb(233,150,122)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>darkseagreen</td><td>143,188,143</td><td style="background-color:rgb(143,188,143)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>darkslateblue</td><td>72,61,139</td><td style="background-color:rgb(72,61,139)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>darkslategray</td><td>47,79,79</td><td style="background-color:rgb(47,79,79)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>darkslategrey</td><td>47,79,79</td><td style="background-color:rgb(47,79,79)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>darkturquoise</td><td>0,206,209</td><td style="background-color:rgb(0,206,209)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>darkviolet</td><td>148,0,211</td><td style="background-color:rgb(148,0,211)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>deeppink</td><td>255,20,147</td><td style="background-color:rgb(255,20,147)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>deepskyblue</td><td>0,191,255</td><td style="background-color:rgb(0,191,255)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>dimgray</td><td>105,105,105</td><td style="background-color:rgb(105,105,105)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>dimgrey</td><td>105,105,105</td><td style="background-color:rgb(105,105,105)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>dodgerblue</td><td>30,144,255</td><td style="background-color:rgb(30,144,255)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>firebrick</td><td>178,34,34</td><td style="background-color:rgb(178,34,34)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>floralwhite</td><td>255,250,240</td><td style="background-color:rgb(255,250,240)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>forestgreen</td><td>34,139,34</td><td style="background-color:rgb(34,139,34)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>fuchsia</td><td>255,0,255</td><td style="background-color:rgb(255,0,255)">&nbsp;&nbsp;&nbsp;</td></tr>
 * </table>
 * </td><td>
 * <table bgcolor='#ccffcc' cellpadding=2 border=3>
 * <tr><td>gainsboro</td><td>220,220,220</td><td style="background-color:rgb(220,220,220)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>ghostwhite</td><td>248,248,255</td><td style="background-color:rgb(248,248,255)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>gold</td><td>255,215,0</td><td style="background-color:rgb(255,215,0)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>goldenrod</td><td>218,165,32</td><td style="background-color:rgb(218,165,32)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>gray</td><td>128,128,128</td><td style="background-color:rgb(128,128,128)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>green</td><td>0,128,0</td><td style="background-color:rgb(0,128,0)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>greenyellow</td><td>173,255,47</td><td style="background-color:rgb(173,255,47)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>grey</td><td>128,128,128</td><td style="background-color:rgb(128,128,128)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>honeydew</td><td>240,255,240</td><td style="background-color:rgb(240,255,240)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>hotpink</td><td>255,105,180</td><td style="background-color:rgb(255,105,180)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>indianred</td><td>205,92,92</td><td style="background-color:rgb(205,92,92)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>indigo</td><td>75,0,130</td><td style="background-color:rgb(75,0,130)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>ivory</td><td>255,255,240</td><td style="background-color:rgb(255,255,240)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>khaki</td><td>240,230,140</td><td style="background-color:rgb(240,230,140)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>lavender</td><td>230,230,250</td><td style="background-color:rgb(230,230,250)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>lavenderblush</td><td>255,240,245</td><td style="background-color:rgb(255,240,245)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>lawngreen</td><td>124,252,0</td><td style="background-color:rgb(124,252,0)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>lemonchiffon</td><td>255,250,205</td><td style="background-color:rgb(255,250,205)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>lightblue</td><td>173,216,230</td><td style="background-color:rgb(173,216,230)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>lightcoral</td><td>240,128,128</td><td style="background-color:rgb(240,128,128)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>lightcyan</td><td>224,255,255</td><td style="background-color:rgb(224,255,255)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>lightgoldenrodyellow</td><td>250,250,210</td><td style="background-color:rgb(250,250,210)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>lightgray</td><td>211,211,211</td><td style="background-color:rgb(211,211,211)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>lightgreen</td><td>144,238,144</td><td style="background-color:rgb(144,238,144)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>lightgrey</td><td>211,211,211</td><td style="background-color:rgb(211,211,211)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>lightpink</td><td>255,182,193</td><td style="background-color:rgb(255,182,193)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>lightsalmon</td><td>255,160,122</td><td style="background-color:rgb(255,160,122)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>lightseagreen</td><td>32,178,170</td><td style="background-color:rgb(32,178,170)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>lightskyblue</td><td>135,206,250</td><td style="background-color:rgb(135,206,250)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>lightslategray</td><td>119,136,153</td><td style="background-color:rgb(119,136,153)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>lightslategrey</td><td>119,136,153</td><td style="background-color:rgb(119,136,153)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>lightsteelblue</td><td>176,196,222</td><td style="background-color:rgb(176,196,222)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>lightyellow</td><td>255,255,224</td><td style="background-color:rgb(255,255,224)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>lime</td><td>0,255,0</td><td style="background-color:rgb(0,255,0)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>limegreen</td><td>50,205,50</td><td style="background-color:rgb(50,205,50)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>linen</td><td>250,240,230</td><td style="background-color:rgb(250,240,230)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>magenta</td><td>255,0,255</td><td style="background-color:rgb(255,0,255)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>maroon</td><td>128,0,0</td><td style="background-color:rgb(128,0,0)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>mediumaquamarine</td><td>102,205,170</td><td style="background-color:rgb(102,205,170)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>mediumblue</td><td>0,0,205</td><td style="background-color:rgb(0,0,205)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>mediumorchid</td><td>186,85,211</td><td style="background-color:rgb(186,85,211)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>mediumpurple</td><td>147,112,219</td><td style="background-color:rgb(147,112,219)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>mediumseagreen</td><td>60,179,113</td><td style="background-color:rgb(60,179,113)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>mediumslateblue</td><td>123,104,238</td><td style="background-color:rgb(123,104,238)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>mediumspringgreen</td><td>0,250,154</td><td style="background-color:rgb(0,250,154)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>mediumturquoise</td><td>72,209,204</td><td style="background-color:rgb(72,209,204)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>mediumvioletred</td><td>199,21,133</td><td style="background-color:rgb(199,21,133)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>midnightblue</td><td>25,25,112</td><td style="background-color:rgb(25,25,112)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>mintcream</td><td>245,255,250</td><td style="background-color:rgb(245,255,250)">&nbsp;&nbsp;&nbsp;</td></tr>
 * </table>
 * </td><td>
 * <table bgcolor='#ccffcc' cellpadding=2 border=3>
 * <tr><td>mistyrose</td><td>255,228,225</td><td style="background-color:rgb(255,228,225)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>moccasin</td><td>255,228,181</td><td style="background-color:rgb(255,228,181)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>navajowhite</td><td>255,222,173</td><td style="background-color:rgb(255,222,173)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>navy</td><td>0,0,128</td><td style="background-color:rgb(0,0,128)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>oldlace</td><td>253,245,230</td><td style="background-color:rgb(253,245,230)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>olive</td><td>128,128,0</td><td style="background-color:rgb(128,128,0)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>olivedrab</td><td>107,142,35</td><td style="background-color:rgb(107,142,35)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>orange</td><td>255,165,0</td><td style="background-color:rgb(255,165,0)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>orangered</td><td>255,69,0</td><td style="background-color:rgb(255,69,0)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>orchid</td><td>218,112,214</td><td style="background-color:rgb(218,112,214)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>palegoldenrod</td><td>238,232,170</td><td style="background-color:rgb(238,232,170)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>palegreen</td><td>152,251,152</td><td style="background-color:rgb(152,251,152)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>paleturquoise</td><td>175,238,238</td><td style="background-color:rgb(175,238,238)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>palevioletred</td><td>219,112,147</td><td style="background-color:rgb(219,112,147)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>papayawhip</td><td>255,239,213</td><td style="background-color:rgb(255,239,213)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>peachpuff</td><td>255,218,185</td><td style="background-color:rgb(255,218,185)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>peru</td><td>205,133,63</td><td style="background-color:rgb(205,133,63)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>pink</td><td>255,192,203</td><td style="background-color:rgb(255,192,203)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>plum</td><td>221,160,221</td><td style="background-color:rgb(221,160,221)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>powderblue</td><td>176,224,230</td><td style="background-color:rgb(176,224,230)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>purple</td><td>128,0,128</td><td style="background-color:rgb(128,0,128)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>red</td><td>255,0,0</td><td style="background-color:rgb(255,0,0)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>rosybrown</td><td>188,143,143</td><td style="background-color:rgb(188,143,143)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>royalblue</td><td>65,105,225</td><td style="background-color:rgb(65,105,225)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>saddlebrown</td><td>139,69,19</td><td style="background-color:rgb(139,69,19)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>salmon</td><td>250,128,114</td><td style="background-color:rgb(250,128,114)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>sandybrown</td><td>244,164,96</td><td style="background-color:rgb(244,164,96)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>seagreen</td><td>46,139,87</td><td style="background-color:rgb(46,139,87)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>seashell</td><td>255,245,238</td><td style="background-color:rgb(255,245,238)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>sienna</td><td>160,82,45</td><td style="background-color:rgb(160,82,45)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>silver</td><td>192,192,192</td><td style="background-color:rgb(192,192,192)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>skyblue</td><td>135,206,235</td><td style="background-color:rgb(135,206,235)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>slateblue</td><td>106,90,205</td><td style="background-color:rgb(106,90,205)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>slategray</td><td>112,128,144</td><td style="background-color:rgb(112,128,144)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>slategrey</td><td>112,128,144</td><td style="background-color:rgb(112,128,144)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>snow</td><td>255,250,250</td><td style="background-color:rgb(255,250,250)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>springgreen</td><td>0,255,127</td><td style="background-color:rgb(0,255,127)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>steelblue</td><td>70,130,180</td><td style="background-color:rgb(70,130,180)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>tan</td><td>210,180,140</td><td style="background-color:rgb(210,180,140)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>teal</td><td>0,128,128</td><td style="background-color:rgb(0,128,128)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>thistle</td><td>216,191,216</td><td style="background-color:rgb(216,191,216)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>tomato</td><td>255,99,71</td><td style="background-color:rgb(255,99,71)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>turquoise</td><td>64,224,208</td><td style="background-color:rgb(64,224,208)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>violet</td><td>238,130,238</td><td style="background-color:rgb(238,130,238)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>wheat</td><td>245,222,179</td><td style="background-color:rgb(245,222,179)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>white</td><td>255,255,255</td><td style="background-color:rgb(255,255,255)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>whitesmoke</td><td>245,245,245</td><td style="background-color:rgb(245,245,245)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>yellow</td><td>255,255,0</td><td style="background-color:rgb(255,255,0)">&nbsp;&nbsp;&nbsp;</td></tr>
 * <tr><td>yellowgreen</td><td>154,205,50</td><td style="background-color:rgb(154,205,50)">&nbsp;&nbsp;&nbsp;</td></tr>
 * </table>
 * </tr></table>
 * \endhtmlonly
 *
 * \subsection blending_section Blending colors
 *
 * In general, one can define an arbitrary function Blend(color1,color2), which
 * given two colors would produce a result color. The qdcontour program understands
 * several such functions in common use in various graphical packages, and also
 * some in not so common use. Whenever rendering a color (source) on top of another
 * one (destination), qdcontour will store Blend(source,destination) as the new
 * destination color.
 *
 * First, below are the well known Porter-Duff rules:
 *
 * \htmlonly
 * <dl>
 * <dt>Clear</dt>
 * <dd>Both the color and the alpha of the destination color are cleared.
 *     The input colors are not used at all.
 * </dd>
 * <dt>Copy</dt>
 * <dd>The source is copied to the destination. The destination is not used
 *     at all.
 * </dd>
 * <dt>Keep</dt>
 * <dd>The destination is kept as is. This is a rather useless blending mode.
 * </dd>
 * <dt>Over</dt>
 * <dd>The source color is composited over the destination using the
 *     linear alpha blending. This is the most common blending mode
 *     whenever alpha-channels are in use.
 * </dd>
 * <dt>Under</dt>
 * <dd>This is similar to Over, but the roles of the colors are reversed
 *     in the formula. In effect, the destination is placed over the
 *     source.
 * </dd>
 * <dt>In</dt>
 * <dd>The part of the source lying inside the destination replaces
 *     the destination. Typically destination is some kind of a mask
 *     for picking a part out of the source.
 * </dd>
 * <dt>KeepIn</dt>
 * <dd>The part of the destination lying inside the source replaces
 *     the destination. This is similar to Keep, but with the mask
 *     role reversed.
 * </dd>
 * <dt>Out</dt>
 * <dd>The part of the source lying outside of the destination
 *     replaces the destination.
 * </dd>
 * <dt>KeepOut</dt>
 * <dd>The part of the destination lying outside of the source
 *     replaces the destination.
 * </dd>
 * <dt>Atop</dt>
 * <dd>The part of the source inside the destination replaces
 *     the part inside the destination.
 * </dd>
 * <dt>KeepAtop</dt>
 * <dd>The part of the destination inside the source replaces
 *     the part inside the source in the destination.
 * </dd>
 * <dt>Xor</dt>
 * <dd>Only non-overlapping areas of source and destination are kept.
 * </dd>
 * </dl>
 * \endhtmlonly
 *
 * In addition, the following extra blending modes are defined:
 *
 * \htmlonly
 * <dl>
 * <dt>Plus</dt>
 * <dd>Add alpha-factored-RGBA values (Fs=1, Fd=1)</dd>
 * <dt>Minus</dt>
 * <dd>Substract alpha-factored RGBA values (Fs=1, Fd=-1)</dd>
 * <dt>Add</dt>
 * <dd>Add RGBA values</dd>
 * <dt>Substract</dt>
 * <dd>Substract RGBA values</dd>
 * <dt>Multiply</dt>
 * <dd>Multiply RGBA values</dd>
 * <dt>Difference</dt>
 * <dd>Absolute difference of RGBA values</dd>
 * <dt>Bumpmap</dt>
 * <dd>Adjust by intensity of source color</dd>
 * <dt>Dentmap</dt>
 * <dd>Adjust by intensity of destination color</dd>
 * <dt>CopyRed</dt>
 * <dd>Copy red component only</dd>
 * <dt>CopyGreen</dt>
 * <dd>Copy green component only</dd>
 * <dt>CopyBlue</dt>
 * <dd>Copy blue component only</dd>
 * <dt>CopyMatte</dt>
 * <dd>Copy opacity only</dd>
 * <dt>CopyHue</dt>
 * <dd>Copy hue component only</dd>
 * <dt>CopyLightness</dt>
 * <dd>Copy light component only</dd>
 * <dt>CopySaturation</dt>
 * <dd>Copy saturation component only</dd>
 * <dt>KeepMatte</dt>
 * <dd>Keep target matte only</dd>
 * <dt>KeepHue</dt>
 * <dd>Keep target hue only</dd>
 * <dt>KeepLightness</dt>
 * <dd>Keep target lightness only</dd>
 * <dt>KeepSaturation</dt>
 * <dd>Keep target saturation only</dd>
 * <dt>AddContrast</dt>
 * <dd>Enhance the contrast of target pixel</dd>
 * <dt>ReduceContrast</dt>
 * <dd>Reduce the contrast of target pixel</dd>
 * <dt>OnOpaque</dt>
 * <dd>Draw on opaque areas only</dd>
 * <dt>OnTransparent</dt>
 * <dd>Draw on transparent areas only</dd>
 * </dl>
 * \endhtmlonly
 *
 * \subsection shapesample_section A sample shapefile rendering control file
 *
 * Not yet.
 *
 * \subsection sample_section A sample contouring control file
 *
 * Below is a sample control file which is used to render T2m
 * contour images for Tiehallinto. To get an idea what a
 * typical control file contains, please browse through the
 * example.
 *
 * \code
 *
 * # First we define the querydata containing the Temperature
 * # forecast we intend to contour
 *
 * querydata kepa_suomi_pieni.sqd
 *
 * # By choosing timestep 0 we ensure the natural timestep
 * # of the data, which happens to be 60 minutes, is used.
 *
 * timestep 0
 *
 * # We wish to render 36 images
 *
 * timesteps 36
 *
 * # The output images do not need to store the alpha channel
 *
 * savealpha 0
 *
 * # If possible, we want to store a palette version since
 * # it is usually smaller than a true color version
 *
 * wantpalette 1
 *
 * # We want each image filename to contain the data timestamp
 * 
 * timestamp 1
 *
 * # Each filename is to be prefixed with ENN_ and suffixed with _T2
 *
 * prefix ENN_
 * suffix _T2
 *
 * # We use the default format png
 *
 * format png
 *
 * # We wish the forecast time and lead time to be rendered
 * # on all images, 160 pixels from the right and 15 pixels
 * # from the bottom
 *
 * timestampimage forobs
 * timestampimagexy -160 -15
 *
 * # We choose linear instead of nearest-neighbour interpolation
 *
 * contourinterpolation Linear
 *
 * # We do not wish the algorithm to recursively subdivide
 * # the grid, it usually is of very little help in smoothening
 * # the appearance of the output
 *
 * contourdepth 0
 *
 * # We wish to smoothen the data itself first, to get
 * # better looking contours. We use a gaussian-lookin
 * # weighting scheme.
 *
 * smoother PseudoGaussian
 *
 * # The points used in smoothing span a radius of 100km
 * # The actual grid spacing is 30km, so this convers several points
 *
 * smootherradius 100000	# in meters!
 *
 * # The smoothing is fairly sharp, smaller numbers would smoothen
 * # the data more
 *
 * smootherfactor 12
 *
 * # Any colour filling is done by copying the color over the background
 *
 * fillrule Copy
 *
 * # And line drawing is done by alpha blending
 *
 * strokerule Over
 *
 * # The parameter we wish to render is temperature
 *
 * param Temperature
 * 
 * # From minus infinity to -38, we use RGB color 130,0,252
 *
 * contourfill - -38 130,0,252
 *
 * # From -38 to -36 RGB color 162,29,255
 *
 * contourfill -38 -36 162,29,255
 *
 * # All these colors were specified by Jouko Kantonen of Tiehallino
 *
 * contourfill -36 -34 161,61,255
 * contourfill -34 -32 177,93,255
 * contourfill -32 -30 192,125,255
 * contourfill -30 -28 37,37,255
 * contourfill -28 -26 69,69,255
 * contourfill -26 -24 101,101,255
 * contourfill -24 -22 133,139,255
 * contourfill -22 -20 165,165,255
 * contourfill -20 -18 0,114,254
 * contourfill -18 -16 27,129,255
 * contourfill -16 -14 55,145,255
 * contourfill -14 -12 83,160,255
 * contourfill -12 -10 111,176,255
 * contourfill -10 -8 0,111,124
 * contourfill -8 -6 0,138,140
 * contourfill -6 -5 0,154,156
 * contourfill -5 -4 0,178,180
 * contourfill -4 -3 0,204,206
 * contourfill -3 -2 0,231,234
 * contourfill -2 -1 45,253,255
 * contourfill -1 0 125,255,220
 * contourfill 0 1 234,244,66
 * contourfill 1 2 255,225,0
 * contourfill 2 4 255,192,1
 * contourfill 4 6 255,156,1
 * contourfill 6 8 255,121,1
 * contourfill 8 10 255,85,1
 * contourfill 10 12 255,129,129
 * contourfill 12 14 255,97,97
 * contourfill 14 16 255,65,65
 * contourfill 16 18 255,33,33
 * contourfill 18 20 255,0,0
 * contourfill 20 22 255,129,191
 * contourfill 22 24 255,97,175
 * contourfill 24 26 255,65,159
 * contourfill 26 28 255,33,143
 * contourfill 28 30 255,0,128
 * contourfill 30 32 255,129,233
 * contourfill 32 34 255,97,227
 * contourfill 34 36 255,65,222
 * contourfill 36 38 255,33,216
 *
 * # Finally fill area from 38 to plus infinity with RGB 255,1,210
 * contourfill 38 -  255,1,210
 * 
 * # Then we draw lines from -60 up to -8 at steps of 2
 *
 * contourlines -60 -8 2 #30444444 #30444444
 *
 * # The spacing goes down to 1 in temperature range -6 to 2
 *
 * contourlines -6 2 1 #30444444 #30444444
 *
 * # And back to spacing 2 for range 4 - 50 degrees
 *
 * contourlines 4 60 2 #30444444 #30444444
 * 
 * # Then we start the actual rendering
 *
 * # The projection is polar stereographic, 25 central longitude
 *
 * stereographic 25 90 60
 * 
 * ####### Suomi
 * 
 * # The area of the background and foreground images
 *
 * bottomleft 19.0 58.0
 * topright 40.0 71.0
 *
 * # The actual background and foreground images
 *
 * background /var/www/share/anim/maps/suomi_tiehallinto_296x500/background.png
 * foreground /var/www/share/anim/maps/suomi_tiehallinto_296x500/foreground.png
 *
 * # We must never use Copy as the foreground rule, always alpha blend
 *
 * foregroundrule Over
 *
 * # Where to place the rendered images
 *
 * savepath /var/www/share/anim/t2m/suomi_tiehallinto_296x500
 *
 * # Render Suomi now!
 *
 * draw contours
 * 
 * ######## Etel‰-Suomi
 *
 * # Specifications like for Suomi in above follow for Etel‰-Suomi
 *
 * bottomleft 20.0 58.7
 * topright 34.0 64.9
 * background /var/www/share/anim/maps/etelasuomi_tiehallinto_500x500/background.png
 * foreground /var/www/share/anim/maps/etelasuomi_tiehallinto_500x500/foreground.png
 * foregroundrule Over
 * savepath /var/www/share/anim/t2m/etelasuomi_tiehallinto_500x500
 * draw contours
 * 
 * # We have omitted Keski-Suomi and Pohjois-Suomi for brevity
 * 
 * # Finally we clear all contour specifications, in case somebody
 * # is contouring multiple files at once
 *
 * clear contours
 * \endcode
 *
 * \subsection deprecated_section Deprecated features
 *
 * \code
 * contourdepth <depth>
 * legenderase <color>
 * draw legend <name> <lo> <hi> <width> <height>
 * draw imagemap <fieldname> <filename>
 * \endcode
 */
// ======================================================================

#include "NFmiColorTools.h"
#include "NFmiSmoother.h"		// for smoothing data
#include "NFmiContourTree.h"	// for contouring
#include "NFmiImage.h"			// for rendering
#include "NFmiGeoShape.h"		// for esri data
#include "NFmiText.h"			// for labels
#include "NFmiFontHershey.h"	// for Hershey fonts

#include "NFmiCmdLine.h"			// command line options
#include "NFmiDataModifierClasses.h"
#include "NFmiEnumConverter.h"		// FmiParameterName<-->string
#include "NFmiFileSystem.h"			// FileExists()
#include "NFmiLatLonArea.h"			// Geographic projection
#include "NFmiSettings.h"			// Configuration
#include "NFmiStereographicArea.h"	// Stereographic projection
#include "NFmiStreamQueryData.h"
#include "NFmiPreProcessor.h"

#include <fstream>
#include <string>
#include <list>
#ifdef OLDGCC
  #include <strstream>
#else
  #include <sstream>
#endif

using namespace std;

// ----------------------------------------------------------------------
// Usage
// ----------------------------------------------------------------------

void Usage(void)
{
  cout << "Usage: qdcontour [conffiles]" << endl << endl;
  cout << "Commands in configuration files:" << endl << endl;
}

// ----------------------------------------------------------------------
// Read a file into the input string
// ----------------------------------------------------------------------

void StringReader(std::istream & is, string & theString)
{
  theString.resize(0);

  const int bufsize = 1024;
  char buffer[bufsize];
  while(!is.eof() && !is.fail())
    {
      is.read(buffer,bufsize);
      theString.append(buffer,is.gcount());
    }
}

// ----------------------------------------------------------------------
// Convert textual description of color to internal color value.
// Accepted formats:
//
// none			--> 127,0,0,0
// transparent		--> 127,0,0,0
// #rrggbb		-->   0,r,g,b
// #aarrggbb		-->   a,r,g,b
// r,g,b		-->   0,r,g,b
// a,r,g,b		-->   a,r,g,b
//
// Returns MissingColor for invalid colors.
// ----------------------------------------------------------------------

// Utility for parsing hex strings

int HexToInt(const string & theHex)
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

NFmiColorTools::Color ToColor(const string & theColor)
{
  // Handle hex format number
  
  if(theColor[0]=='#')
    {
      int a,r,g,b;
      if(theColor.length()==7)
		{
		  a = 0;
		  r = HexToInt(theColor.substr(1,2));
		  g = HexToInt(theColor.substr(3,2));
		  b = HexToInt(theColor.substr(5,2));
		}
      else if(theColor.length()==9)
		{
		  a = HexToInt(theColor.substr(1,2));
		  r = HexToInt(theColor.substr(3,2));
		  g = HexToInt(theColor.substr(5,2));
		  b = HexToInt(theColor.substr(7,2));
		}
      if(r>=0 && g>=0 && b>=0 && a>=0)
		return NFmiColorTools::MakeColor(r,g,b,a);
      else
		return NFmiColorTools::MissingColor;
    }
  
  // Handle ascii format
  
  else if(theColor[0]<'0' || theColor[0]>'9')
    {
      unsigned int pos = theColor.find(",");
      if(pos == string::npos)
		return NFmiColorTools::ColorValue(theColor);
      else
		{
		  int value = -1;
		  for(unsigned int i=pos+1; i<theColor.length(); i++)
			{
			  if(theColor[i]>='0' && theColor[i]<='9')
				{
				  if(value<0)
					value = theColor[i]-'0';
				  else
					value = value*10+theColor[i]-'0';
				}
			  else
				return NFmiColorTools::MissingColor;
			}
		  if(value<0)
			return NFmiColorTools::MissingColor;
		  NFmiColorTools::Color tmp = NFmiColorTools::ColorValue(theColor.substr(0,pos));
		  return NFmiColorTools::ReplaceAlpha(tmp,value);
		}
    }
  
  
  // Handle decimal format number
  else
    {
      vector<int> tmp;
      int value=-1;
      for(unsigned int i=0; i<theColor.length(); i++)
		{
		  if(theColor[i]>='0' && theColor[i]<='9')
			{
			  if(value<0)
				value = theColor[i]-'0';
			  else
				value = value*10+theColor[i]-'0';
			}
		  else if(theColor[i]==',')
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
}

// ----------------------------------------------------------------------
// Yksitt‰isen contour-arvon s‰ilytysluokka
// ----------------------------------------------------------------------

class ContourValue
{
public:
  
  ContourValue(float value, int color, string rule="Atop")
    : itsValue(value)
    , itsColor(color)
    , itsRule(rule)
  {}
  
  float Value(void) const { return itsValue; }
  int Color(void) const { return itsColor; }
  const string & Rule(void) const { return itsRule; }
private:
  ContourValue(void);
  float itsValue;
  int itsColor;
  string itsRule;
};

// ----------------------------------------------------------------------
// Yksitt‰isen contour-intervallin s‰ilytysluokka
// ----------------------------------------------------------------------

class ContourRange
{
public:
  
  ContourRange(float lolimit,float hilimit,int color, string rule="Atop")
    : itsLoLimit(lolimit)
    , itsHiLimit(hilimit)
    , itsColor(color)
    , itsRule(rule)
  {}
  float LoLimit(void) const { return itsLoLimit; }
  float HiLimit(void) const { return itsHiLimit; }
  int   Color(void)   const { return itsColor; }
  const string & Rule(void) const { return itsRule; }
  
private:
  ContourRange(void);
  float itsLoLimit;
  float itsHiLimit;
  int   itsColor;
  string itsRule;
};

// ----------------------------------------------------------------------
// Yksitt‰isen contour-patternin s‰ilytysluokka
// ----------------------------------------------------------------------

class ContourPattern
{
public:
  
  ContourPattern(float lolimit,float hilimit,
				 const string & thePattern,
				 const string & theRule,
				 float theFactor=1.0)
    : itsLoLimit(lolimit)
    , itsHiLimit(hilimit)
    , itsPattern(thePattern)
    , itsRule(theRule)
    , itsFactor(theFactor)
  {}
  float LoLimit(void) const		{ return itsLoLimit; }
  float HiLimit(void) const		{ return itsHiLimit; }
  const string & Pattern(void) const	{ return itsPattern; }
  const string & Rule(void) const	{ return itsRule; }
  float Factor(void) const		{ return itsFactor; }
  
private:
  ContourPattern(void);
  float itsLoLimit;
  float itsHiLimit;
  string itsPattern;
  string itsRule;
  float itsFactor;
};

// ----------------------------------------------------------------------
// Yksitt‰isen parametrin piirto-ohjeet
// ----------------------------------------------------------------------

class ContourSpec
{
public:
  ContourSpec(const string & param,
			  const string & interpolation,
			  const string & smoother,
			  int depth=0,
			  float smootherradius=1.0,
			  int smootherfactor=1,
			  float hilimit=kFloatMissing)
    : itsParam(param)
    , itsContourInterpolation(interpolation)
    , itsSmoother(smoother)
    , itsSmootherRadius(smootherradius)
    , itsSmootherFactor(smootherfactor)
    , itsExactHiLimit(hilimit)
    , itsContourDepth(depth)
    , itsDataLoLimit(kFloatMissing)
    , itsDataHiLimit(kFloatMissing)
    , itHasReplace(false)
    , itsLabelMarker("")
    , itsLabelMarkerRule("Copy")
    , itsLabelMarkerAlphaFactor(1.0)
    , itsLabelFont("TimesRoman")
    , itsLabelSize(12)
    , itsLabelStrokeColor(NFmiColorTools::NoColor)
    , itsLabelStrokeRule("Copy")
    , itsLabelFillColor(NFmiColorTools::NoColor)
    , itsLabelFillRule("Copy")
    , itsLabelAlignment("Center")
    , itsLabelFormat("%.1f")
    , itsLabelAngle(0)
    , itsLabelOffsetX(0)
    , itsLabelOffsetY(0)
    , itsLabelDX(0)
    , itsLabelDY(0)
    , itsLabelCaption("")
    , itsLabelCaptionDX(0)
    , itsLabelCaptionDY(0)
    , itsLabelCaptionAlignment("West")
  {}
  
  const list<ContourRange> & ContourFills(void) const { return itsContourFills; }
  const list<ContourPattern> & ContourPatterns(void) const { return itsContourPatterns; }
  const list<ContourValue> & ContourValues(void) const { return itsContourValues; }
  
  const string & Param(void) const		{ return itsParam; }
  const string & ContourInterpolation(void) const	{ return itsContourInterpolation; }
  const string & Smoother(void) const		{ return itsSmoother; }
  float SmootherRadius(void) const		{ return itsSmootherRadius; }
  int SmootherFactor(void) const		{ return itsSmootherFactor; }
  float ExactHiLimit(void) const		{ return itsExactHiLimit; }
  int ContourDepth(void) const			{ return itsContourDepth; }
  float DataHiLimit(void) const			{ return itsDataHiLimit; }
  float DataLoLimit(void) const			{ return itsDataLoLimit; }
  
  void ContourInterpolation(const string & val)	{ itsContourInterpolation = val; }
  void Smoother(const string & val)		{ itsSmoother = val; }
  void SmootherRadius(float radius)		{ itsSmootherRadius = radius; }
  void SmootherFactor(int factor)		{ itsSmootherFactor = factor; }
  void ExactHiLimit(float limit)		{ itsExactHiLimit = limit; }
  void ContourDepth(int depth)			{ itsContourDepth = depth; }
  
  void DataLoLimit(float limit)			{ itsDataLoLimit = limit; }
  void DataHiLimit(float limit)			{ itsDataHiLimit = limit; }
  
  void Add(ContourRange range) { itsContourFills.push_back(range); }
  void Add(ContourValue value) { itsContourValues.push_back(value); }
  void Add(ContourPattern value) { itsContourPatterns.push_back(value); }
  
  // This was done to replace 32700 with -1 in PrecipitationForm
  
  bool Replace(void) const		{ return itHasReplace; }
  float ReplaceSourceValue(void) const	{ return itsReplaceSourceValue; }
  float ReplaceTargetValue(void) const	{ return itsReplaceTargetValue; }
  
  void Replace(float src, float dst)
  {
    itHasReplace = true;
    itsReplaceSourceValue = src;
    itsReplaceTargetValue = dst;
  }
  
  // Label specific methods
  
  const list<pair<NFmiPoint,NFmiPoint> > & LabelPoints(void) const { return itsLabelPoints; }
  
  void Add(const NFmiPoint & point,
		   const NFmiPoint xy = NFmiPoint(kFloatMissing,kFloatMissing))
  { itsLabelPoints.push_back(make_pair(point,xy)); }
  
  const vector<float> & LabelValues(void) const
  { return itsLabelValues; }
  
  void AddLabelValue(float value)
  { itsLabelValues.push_back(value); }
  
  void ClearLabelValues(void)
  {
    itsLabelValues.clear();
  }
  
  void ClearLabels(void)
  {
    itsLabelPoints.clear();
    itsLabelValues.clear();
  }
  
  const string & LabelMarker(void) const	{ return itsLabelMarker; }
  const string & LabelMarkerRule(void) const 	{ return itsLabelMarkerRule; }
  float LabelMarkerAlphaFactor(void) const 	{ return itsLabelMarkerAlphaFactor; }
  const string & LabelFont(void) const		{ return itsLabelFont; }
  float LabelSize(void) const			{ return itsLabelSize; }
  int LabelStrokeColor(void) const		{ return itsLabelStrokeColor; }
  const string & LabelStrokeRule(void) const	{ return itsLabelStrokeRule; }
  int LabelFillColor(void) const		{ return itsLabelFillColor; }
  const string & LabelFillRule(void) const	{ return itsLabelFillRule; }
  const string & LabelAlignment(void) const	{ return itsLabelAlignment; }
  const string & LabelFormat(void) const	{ return itsLabelFormat; }
  float LabelAngle(void) const			{ return itsLabelAngle; }
  float LabelOffsetX(void) const		{ return itsLabelOffsetX; }
  float LabelOffsetY(void) const		{ return itsLabelOffsetY; }
  int LabelDX(void) const			{ return itsLabelDX; }
  int LabelDY(void) const			{ return itsLabelDY; }
  
  string LabelCaption(void) const		{ return itsLabelCaption; }
  float LabelCaptionDX(void) const		{ return itsLabelCaptionDX; }
  float LabelCaptionDY(void) const		{ return itsLabelCaptionDY; }
  string LabelCaptionAlignment(void) const	{ return itsLabelCaptionAlignment; }
  
  void LabelMarker(const string & value)	{ itsLabelMarker = value; }
  void LabelMarkerRule(const string & value)	{ itsLabelMarkerRule = value; }
  void LabelMarkerAlphaFactor(float value) 	{ itsLabelMarkerAlphaFactor = value; }
  void LabelFont(const string & value)		{ itsLabelFont = value; }
  void LabelSize(float value)			{ itsLabelSize = value; }
  void LabelStrokeColor(int value)		{ itsLabelStrokeColor = value; }
  void LabelStrokeRule(const string & value)	{ itsLabelStrokeRule = value; }
  void LabelFillColor(int value)		{ itsLabelFillColor = value; }
  void LabelFillRule(const string & value)	{ itsLabelFillRule = value; }
  void LabelAlignment(const string & value)	{ itsLabelAlignment = value; }
  void LabelFormat(const string & value)	{ itsLabelFormat = value; }
  void LabelAngle(float value)			{ itsLabelAngle = value; }
  void LabelOffsetX(float value)		{ itsLabelOffsetX = value; }
  void LabelOffsetY(float value)		{ itsLabelOffsetY = value; }
  void LabelDX(int value)			{ itsLabelDX = value; }
  void LabelDY(int value)			{ itsLabelDY = value; }
  
  void LabelCaption(const string & value)	{ itsLabelCaption = value; }
  void LabelCaptionDX(float value)		{ itsLabelCaptionDX = value; }
  void LabelCaptionDY(float value)		{ itsLabelCaptionDY = value; }
  void LabelCaptionAlignment(const string & value) { itsLabelCaptionAlignment = value; }
  
private:
  ContourSpec(void);
  string itsParam;
  string itsContourInterpolation;
  string itsSmoother;
  float itsSmootherRadius;
  int itsSmootherFactor;
  
  list<ContourRange> itsContourFills;
  list<ContourValue> itsContourValues;
  list<ContourPattern> itsContourPatterns;
  
  float itsExactHiLimit;
  int itsContourDepth;
  float itsDataLoLimit;
  float itsDataHiLimit;
  
  bool itHasReplace;;
  float itsReplaceSourceValue;
  float itsReplaceTargetValue;
  
  // LatLon, optional label location pairs in pixels
  list<pair<NFmiPoint,NFmiPoint> > itsLabelPoints;
  // Respective values calculated while contouring
  vector<float> itsLabelValues;
  
  string itsLabelMarker;
  string itsLabelMarkerRule;
  float itsLabelMarkerAlphaFactor;
  string itsLabelFont;
  float itsLabelSize;
  int itsLabelStrokeColor;
  string itsLabelStrokeRule;
  int itsLabelFillColor;
  string itsLabelFillRule;
  string itsLabelAlignment;
  string itsLabelFormat;
  float itsLabelAngle;
  float itsLabelOffsetX;
  float itsLabelOffsetY;
  int itsLabelDX;
  int itsLabelDY;
  
  string itsLabelCaption;
  float itsLabelCaptionDX;
  float itsLabelCaptionDY;
  string itsLabelCaptionAlignment;
  
};

// ----------------------------------------------------------------------
// Yksitt‰isen shapen piirto-ohjeet
// ----------------------------------------------------------------------

class ShapeSpec
{
public:
  ShapeSpec(const string & shapefile,
			NFmiColorTools::Color fill = NFmiColorTools::MakeColor(0,0,0,127),
			NFmiColorTools::Color stroke = NFmiColorTools::MakeColor(0,0,0,127),
			const string & fillrule = "Copy",
			const string & strokerule = "Copy")
    : itsShapeFileName(shapefile)
    , itsFillRule(fillrule)
    , itsStrokeRule(strokerule)
    , itsFillColor(fill)
    , itsStrokeColor(stroke)
    , itsMarker("")
    , itsMarkerRule("Over")
    , itsMarkerAlpha(1.0)
  {}
  
  // Data-access
  
  const string & FileName(void) const		{ return itsShapeFileName; }
  const string & FillRule(void) const		{ return itsFillRule; }
  const string & StrokeRule(void) const		{ return itsStrokeRule; }
  NFmiColorTools::Color FillColor(void) const	{ return itsFillColor; }
  NFmiColorTools::Color StrokeColor(void) const	{ return itsStrokeColor; }
  
  void FillRule(const string & rule)		{ itsFillRule = rule; }
  void StrokeRule(const string & rule)		{ itsStrokeRule = rule; }
  void FillColor(NFmiColorTools::Color color)	{ itsFillColor = color; }
  void StrokeColor(NFmiColorTools::Color color)	{ itsStrokeColor = color; }
  
  void Marker(const string & marker, const string & rule, float alpha)
  {
    itsMarker = marker;
    itsMarkerRule = rule;
    itsMarkerAlpha = alpha;
  }
  
  const string & Marker(void) const		{ return itsMarker; }
  const string & MarkerRule(void) const		{ return itsMarkerRule; }
  float MarkerAlpha(void) const			{ return itsMarkerAlpha; }
private:
  ShapeSpec(void);
  string	itsShapeFileName;
  string	itsFillRule;
  string	itsStrokeRule;
  NFmiColorTools::Color	itsFillColor;
  NFmiColorTools::Color	itsStrokeColor;
  string	itsMarker;
  string	itsMarkerRule;
  float		itsMarkerAlpha;
  
};

// ----------------------------------------------------------------------
// Main program.
// ----------------------------------------------------------------------

int main(int argc, const char *argv[])
{
  // Ymp‰ristˆn konfigurointi

  string datapath = NFmiSettings::instance().value("qdcontour::querydata_path",".");

  // Lista komentitiedostoista
  
  list<string> theFiles;
  
  // Aktiiviset contour-speksit (ja label speksit)
  
  list<ContourSpec> theSpecs;
  
  // Aktiiviset shape-speksit
  
  list<ShapeSpec> theShapeSpecs;
  
  // Aktiiviset tuulinuolet
  
  list<NFmiPoint> theArrowPoints;
  
  // Komentotiedostosta luettavat optiot
  
  string theParam = "";
  string theShapeFileName = "";
  string theContourInterpolation = "Linear";
  string theSmoother = "None";
  float theSmootherRadius = 1.0;
  int theTimeStepRoundingFlag = 1;
  int theTimeStampFlag	= 1;
  int theSmootherFactor = 1;
  int theTimeStepSkip	= 0;	// skipattava minuuttim‰‰r‰
  int theTimeStep	= 0;	// aika-askel
  int theTimeInterval   = 0;	// inklusiivinen aikam‰‰r‰
  int theTimeSteps	= 24;	// max kuvien lukum‰‰r‰
  
  int theTimeStampImageX = 0;
  int theTimeStampImageY = 0;
  string theTimeStampImage = "none";
  
  // Alueen m‰‰ritelm‰
  
  NFmiPoint theBottomLeft	= NFmiPoint(kFloatMissing,kFloatMissing);
  NFmiPoint theTopRight		= NFmiPoint(kFloatMissing,kFloatMissing);
  NFmiPoint theCenter		= NFmiPoint(kFloatMissing,kFloatMissing);

  float theCentralLongitude	= 25.0;
  float theCentralLatitude	= 90.0;
  float theTrueLatitude		= 60.0;
  
  int theWidth		= -1;
  int theHeight		= -1;
  float theScale	= -1;
  
  string theSavePath	= ".";
  string thePrefix	= "";
  string theSuffix	= "";
  string theFormat	= "png";
  bool   theSaveAlphaFlag = true;
  bool   theWantPaletteFlag = false;
  bool   theForcePaletteFlag = false;
  string theLegendErase = "white";
  string theErase	= "#7F000000";
  string theBackground	= "";
  string theForeground	= "";
  string theFillRule	= "Atop";
  string theStrokeRule	= "Atop";
  
  string theForegroundRule = "Over";

  string theCombine = "";
  int theCombineX;
  int theCombineY;
  string theCombineRule = "Over";
  float theCombineFactor = 1.0;
  
  string theFilter = "none";

  string theDirectionParameter = "WindDirection";
  string theSpeedParameter = "WindSpeedMS";
  
  float theArrowScale = 1.0;

  float theWindArrowScaleA = 0.0;	// a*log10(b*x+1)+c = 0*log10(0+1)+1 = 1
  float theWindArrowScaleB = 0.0;
  float theWindArrowScaleC = 1.0;

  string theArrowFillColor = "white";
  string theArrowStrokeColor = "black";
  string theArrowFillRule = "Over";
  string theArrowStrokeRule = "Over";
  string theArrowFile = "";
  
  unsigned int theWindArrowDX = 0;
  unsigned int theWindArrowDY = 0;
  
  int theContourDepth	= 0;
  
  int thePngQuality = -1;
  int theJpegQuality = -1;
  int theAlphaLimit = -1;
  float theGamma = -1.0;
  string theIntent = "";
  
  // Related variables
  
  NFmiImage theBackgroundImage;
  NFmiImage theForegroundImage;
  NFmiImage theCombineImage;
  
  // This holds a vector of querydatastreams
  
  vector<NFmiStreamQueryData *> theQueryStreams;
  int theQueryDataLevel = 1;
  string theQueryStreamNames = "";
  
  // These will hold the querydata for the active parameter
  
  NFmiQueryData *theQueryData = 0;
  NFmiFastQueryInfo *theQueryInfo = 0;
  
  bool verbose	= false;	// verbose mode off
  bool force	= false;	// overwrite disabled
  
  // Read command line options
  // ~~~~~~~~~~~~~~~~~~~~~~~~~
  
  NFmiCmdLine cmdline(argc,argv,"vf");
  
  // Check for parsing errors
  
  if(cmdline.Status().IsError())
    {
      cerr << "Error: Invalid command line:" << endl
		   << cmdline.Status().ErrorLog().CharPtr() << endl;
      Usage();
      return 1;
	  
    }
  
  // Read -v option
  
  if(cmdline.isOption('v'))
    verbose = true;
  
  // Read -f option
  
  if(cmdline.isOption('f'))
    force = true;
  
  // Read command filenames
  
  if(cmdline.NumberofParameters() == 0)
    {
      cerr << "Error: Expecting atleast one command file argument\n\n";
      return 1;
    }
  
  for(int i=1; i<=cmdline.NumberofParameters(); i++)
	theFiles.push_back(cmdline.Parameter(i));
  
  // Process all command files
  // ~~~~~~~~~~~~~~~~~~~~~~~~~
  
  list<string>::const_iterator fileiter = theFiles.begin();
  for( ; fileiter!=theFiles.end(); ++fileiter)
    {
      const string & cmdfilename = *fileiter;
	  
      if(verbose)
		cout << "Processing file: " << cmdfilename << endl;
	  
      // Open command file for reading

	  const bool strip_pound = true;
	  NFmiPreProcessor processor(strip_pound);
	  processor.SetIncluding("include", "", "");
	  if(!processor.ReadAndStripFile(cmdfilename))
		{
		  cerr << "Error: Could not parse " << cmdfilename << endl;
		  return 1;
		}
	  // Extract the assignments
	  string text = processor.GetString();

#ifdef OLDGCC
	  istrstream input(text.c_str());
#else
	  istringstream input(text);
#endif
	  
      // Process the commands
      string command;
      while( input >> command)
		{
		  // Handle comments
		  
		  if(command == "#" || command == "//" || command[0]=='#')
			{
			  // Should use numeric_limits<int>::max() to by definition
			  // skip to end of line, but numeric_limits does not exist
			  // in g++ v2.95
			  
			  input.ignore(1000000,'\n');
			}
		  
		  else if(command == "querydata")
			{
			  string newnames;
			  input >> newnames;
			  
			  if(theQueryStreamNames != newnames)
				{
				  theQueryStreamNames = newnames;
				  
				  // Delete possible old infos
				  
				  for(unsigned int i=0; i<theQueryStreams.size(); i++)
					delete theQueryStreams[i];
				  theQueryStreams.resize(0);
				  theQueryInfo = 0;
				  theQueryData = 0;
				  
				  // Split the comma separated list into a real list
				  
				  list<string> qnames;
				  unsigned int pos1 = 0;
				  while(pos1<theQueryStreamNames.size())
					{
					  unsigned int pos2 = theQueryStreamNames.find(',',pos1);
					  if(pos2==std::string::npos)
						pos2 = theQueryStreamNames.size();
					  qnames.push_back(theQueryStreamNames.substr(pos1,pos2-pos1));
					  pos1 = pos2 + 1;
					}
			  
				  // Read the queryfiles
				  
					{
					  list<string>::const_iterator iter;
					  for(iter=qnames.begin(); iter!=qnames.end(); ++iter)
						{
						  NFmiStreamQueryData * tmp = new NFmiStreamQueryData();
						  string filename = FileComplete(*iter,datapath);
						  if(!tmp->ReadLatestData(filename))
							exit(1);
						  theQueryStreams.push_back(tmp);
						}
					}
				}
			}

		  else if(command == "querydatalevel")
			input >> theQueryDataLevel;
		  
		  else if(command == "filter")
			input >> theFilter;
		  
		  else if(command == "timestepskip")
			input >> theTimeStepSkip;
		  
		  else if(command == "timestep")
			{
			  input >> theTimeStep;
			  theTimeInterval = theTimeStep;
			}
		  
		  else if(command == "timeinterval")
			input >> theTimeInterval;
		  
		  else if(command == "timesteps")
			input >> theTimeSteps;
		  
		  else if(command == "timestamp")
			input >> theTimeStampFlag;
		  
		  else if(command == "timesteprounding")
			input >> theTimeStepRoundingFlag;
		  
		  else if(command == "timestampimage")
			input >> theTimeStampImage;
		  
		  else if(command == "timestampimagexy")
			input >> theTimeStampImageX >> theTimeStampImageY;
		  
		  else if(command == "bottomleft")
			{
			  float lon,lat;
			  input >> lon >> lat;
			  theBottomLeft.Set(lon,lat);
			}
		  
		  else if(command == "topright")
			{
			  float lon,lat;
			  input >> lon >> lat;
			  theTopRight.Set(lon,lat);
			}
		  
		  else if(command == "center")
			{
			  float lon,lat;
			  input >> lon >> lat;
			  theCenter.Set(lon,lat);
			}

		  else if(command == "stereographic")
			input >> theCentralLongitude
				   >> theCentralLatitude
				   >> theTrueLatitude;
		  
		  else if(command == "size")
			{
			  input >> theWidth >> theHeight;
			  theBackground = "";
			}
		  
		  else if(command == "width")
			input >> theWidth;
		  
		  else if(command == "height")
			input >> theHeight;
		  
		  else if(command == "scale")
			input >> theScale;

		  else if(command == "erase")
			{
			  input >> theErase;
			  if(ToColor(theErase)==NFmiColorTools::MissingColor)
				{
				  cerr << "Error: Invalid color " << theErase << endl;
				  exit(1);
				}
			}
		  
		  else if(command == "legenderase")
			{
			  input >> theLegendErase;
			  if(ToColor(theLegendErase)==NFmiColorTools::MissingColor)
				{
				  cerr << "Error: Invalid color " << theLegendErase << endl;
				  exit(1);
				}
			}
		  
		  else if(command == "fillrule")
			{
			  input >> theFillRule;
			  if(NFmiColorTools::BlendValue(theFillRule)==NFmiColorTools::kFmiColorRuleMissing)
				{
				  cerr << "Error: Unknown blending rule " << theFillRule << endl;
				  exit(1);
				}
			  if(!theShapeSpecs.empty())
				theShapeSpecs.back().FillRule(theFillRule);
			}
		  else if(command == "strokerule")
			{
			  input >> theStrokeRule;
			  if(NFmiColorTools::BlendValue(theStrokeRule)==NFmiColorTools::kFmiColorRuleMissing)
				{
				  cerr << "Error: Unknown blending rule " << theStrokeRule << endl;
				  exit(1);
				}
			  if(!theShapeSpecs.empty())
				theShapeSpecs.back().StrokeRule(theStrokeRule);
			}
		  
		  else if(command == "directionparam")
		    input >> theDirectionParameter;

		  else if(command == "speedparam")
		    input >> theSpeedParameter;

		  else if(command == "arrowscale")
			input >> theArrowScale;

		  else if(command == "windarrowscale")
			input >> theWindArrowScaleA >> theWindArrowScaleB >> theWindArrowScaleC;
		  
		  else if(command == "arrowfill")
			{
			  input >> theArrowFillColor >> theArrowFillRule;
			  if(ToColor(theArrowFillColor)==NFmiColorTools::MissingColor)
				{
				  cerr << "Error: Invalid color " << theArrowFillColor << endl;
				  exit(1);
				}
			  if(NFmiColorTools::BlendValue(theArrowFillRule)==NFmiColorTools::kFmiColorRuleMissing)
				{
				  cerr << "Error: Unknown blending rule " << theArrowFillRule << endl;
				  exit(1);
				}
			}
		  else if(command == "arrowstroke")
			{
			  input >> theArrowStrokeColor >> theArrowStrokeRule;
			  if(ToColor(theArrowStrokeColor)==NFmiColorTools::MissingColor)
				{
				  cerr << "Error: Invalid color " << theArrowStrokeColor << endl;
				  exit(1);
				}
			  if(NFmiColorTools::BlendValue(theArrowStrokeRule)==NFmiColorTools::kFmiColorRuleMissing)
				{
				  cerr << "Error: Unknown blending rule " << theArrowStrokeRule << endl;
				  exit(1);
				}
			}
		  
		  else if(command == "arrowpath")
			input >> theArrowFile;
		  
		  else if(command == "windarrow")
			{
			  float lon,lat;
			  input >> lon >> lat;
			  theArrowPoints.push_back(NFmiPoint(lon,lat));
			}
		  
		  else if(command == "windarrows")
			input >> theWindArrowDX >> theWindArrowDY;
		  
		  else if(command == "background")
			{
			  input >> theBackground;
			  if(theBackground != "none")
				theBackgroundImage.Read(theBackground);
			  else
				theBackground = "";
			}
		  
		  else if(command == "foreground")
			{
			  input >> theForeground;
			  if(theForeground != "none")
				theForegroundImage.Read(theForeground);
			  else
				theForeground = "";
			}
		  
		  else if(command == "combine")
			{
			  input >> theCombine;
			  if(theCombine != "none")
				{
				  input >> theCombineX >> theCombineY;
				  input >> theCombineRule >> theCombineFactor;
				  if(NFmiColorTools::BlendValue(theCombineRule)==NFmiColorTools::kFmiColorRuleMissing)
					{
					  cerr << "Error: Unknown blending rule " << theCombineRule << endl;
					  exit(1);
					}
				  theCombineImage.Read(theCombine);
				}
			  else
				theCombine = "";
			}

		  else if(command == "foregroundrule")
			{
			  input >> theForegroundRule;
			  
			  if(NFmiColorTools::BlendValue(theForegroundRule)==NFmiColorTools::kFmiColorRuleMissing)
				{
				  cerr << "Error: Unknown blending rule " << theForegroundRule << endl;
				  exit(1);
				}
			}
		  
		  else if(command == "savepath")
			{
			  input >> theSavePath;
			  if(!DirectoryExists(theSavePath))
				{
				  cerr << "savepath " << theSavePath << " does not exist!" << endl;
				  return 1;
				}
			}
		  
		  else if(command == "prefix")
			input >> thePrefix;
		  
		  else if(command == "suffix")
			input >> theSuffix;
		  
		  else if(command == "format")
			input >> theFormat;
		  
		  else if(command == "gamma")
			input >> theGamma;
		  
		  else if(command == "intent")
			input >> theIntent;
		  
		  else if(command == "pngquality")
			input >> thePngQuality;
		  
		  else if(command == "jpegquality")
			input >> theJpegQuality;
		  
		  else if(command == "savealpha")
			input >> theSaveAlphaFlag;
		  
		  else if(command == "wantpalette")
			input >> theWantPaletteFlag;
		  
		  else if(command == "forcepalette")
			input >> theForcePaletteFlag;
		  
		  else if(command == "alphalimit")
			input >> theAlphaLimit;
		  
		  else if(command == "hilimit")
			{
			  float limit;
			  input >> limit;
			  if(!theSpecs.empty())
				theSpecs.back().ExactHiLimit(limit);
			}
		  else if(command == "datalolimit")
			{
			  float limit;
			  input >> limit;
			  if(!theSpecs.empty())
				theSpecs.back().DataLoLimit(limit);
			}
		  else if(command == "datahilimit")
			{
			  float limit;
			  input >> limit;
			  if(!theSpecs.empty());
			  theSpecs.back().DataHiLimit(limit);
			}
		  else if(command == "datareplace")
			{
			  float src,dst;
			  input >> src >> dst;
			  if(!theSpecs.empty())
				theSpecs.back().Replace(src,dst);
			}
		  else if(command == "contourdepth")
			{
			  input >> theContourDepth;
			  if(!theSpecs.empty())
				theSpecs.back().ContourDepth(theContourDepth);
			}
		  
		  else if(command == "contourinterpolation")
			{
			  input >> theContourInterpolation;
			  if(!theSpecs.empty())
				theSpecs.back().ContourInterpolation(theContourInterpolation);
			}
		  else if(command == "smoother")
			{
			  input >> theSmoother;
			  if(!theSpecs.empty())
				theSpecs.back().Smoother(theSmoother);
			}
		  else if(command == "smootherradius")
			{
			  input >> theSmootherRadius;
			  if(!theSpecs.empty())
				theSpecs.back().SmootherRadius(theSmootherRadius);
			}
		  else if(command == "smootherfactor")
			{
			  input >> theSmootherFactor;
			  if(!theSpecs.empty())
				theSpecs.back().SmootherFactor(theSmootherFactor);
			}
		  else if(command == "param")
			{
			  input >> theParam;
			  theSpecs.push_back(ContourSpec(theParam,
											 theContourInterpolation,
											 theSmoother,
											 theContourDepth,
											 theSmootherRadius,
											 theSmootherFactor));
			}
		  
		  else if(command == "shape")
			{
			  input >> theShapeFileName;
			  string arg1;
			  
			  input >> arg1;
			  
			  if(arg1=="mark")
				{
				  string marker, markerrule;
				  float markeralpha;
				  input >> marker >> markerrule >> markeralpha;
				  
				  if(NFmiColorTools::BlendValue(markerrule)==NFmiColorTools::kFmiColorRuleMissing)
					{
					  cerr << "Error: Unknown blending rule " << markerrule << endl;
					  exit(1);
					}
				  ShapeSpec spec(theShapeFileName);
				  spec.Marker(marker,markerrule,markeralpha);
				  theShapeSpecs.push_back(spec);
				}
			  else
				{
				  string fillcolor = arg1;
				  string strokecolor;
				  input >> strokecolor;
				  NFmiColorTools::Color fill = ToColor(fillcolor);
				  NFmiColorTools::Color stroke = ToColor(strokecolor);
				  if(fill == NFmiColorTools::MissingColor)
					{
					  cerr << "Error: fillcolor " << fillcolor << " unrecognized" << endl;
					  exit(1);
					}
				  if(stroke == NFmiColorTools::MissingColor)
					{
					  cerr << "Error: strokecolor " << strokecolor << " unrecognized" << endl;
					  exit(1);
					}
				  theShapeSpecs.push_back(ShapeSpec(theShapeFileName,
													fill,stroke,
													theFillRule,theStrokeRule));
				}
			}
		  
		  else if(command == "contourfill")
			{
			  string slo,shi,scolor;
			  input >> slo >> shi >> scolor;
			  
			  float lo,hi;
			  if(slo == "-")
				lo = kFloatMissing;
			  else
				lo = atof(slo.c_str());
			  if(shi == "-")
				hi = kFloatMissing;
			  else
				hi = atof(shi.c_str());
			  
			  NFmiColorTools::Color color = ToColor(scolor);
			  
			  if(!theSpecs.empty())
				theSpecs.back().Add(ContourRange(lo,hi,color,theFillRule));
			}
		  
		  else if(command == "contourpattern")
			{
			  string slo,shi,spattern,srule;
			  float alpha;
			  input >> slo >> shi >> spattern >> srule >> alpha;
			  
			  float lo,hi;
			  if(slo == "-")
				lo = kFloatMissing;
			  else
				lo = atof(slo.c_str());
			  if(shi == "-")
				hi = kFloatMissing;
			  else
				hi = atof(shi.c_str());
			  
			  if(!theSpecs.empty())
				theSpecs.back().Add(ContourPattern(lo,hi,spattern,srule,alpha));
			}
		  
		  else if(command == "contourline")
			{
			  string svalue,scolor;
			  input >> svalue >> scolor;
			  
			  float value;
			  if(svalue == "-")
				value = kFloatMissing;
			  else
				value = atof(svalue.c_str());
			  
			  NFmiColorTools::Color color = ToColor(scolor);
			  if(!theSpecs.empty())
				theSpecs.back().Add(ContourValue(value,color,theStrokeRule));
			}
		  
		  else if(command == "contourfills")
			{
			  float lo,hi,step;
			  string scolor1,scolor2;
			  input >> lo >> hi >> step >> scolor1 >> scolor2;
			  
			  int color1 = ToColor(scolor1);
			  int color2 = ToColor(scolor2);
			  
			  int steps = static_cast<int>((hi-lo)/step);
			  
			  for(int i=0; i<steps; i++)
				{
				  float tmplo=lo+i*step;
				  float tmphi=lo+(i+1)*step;
				  int color = color1;	// in case steps=1
				  if(steps!=1)
					color = NFmiColorTools::Interpolate(color1,color2,i/(steps-1.0));
				  if(!theSpecs.empty())
					theSpecs.back().Add(ContourRange(tmplo,tmphi,color,theFillRule));
				  // if(verbose)
				  // cout << "Interval " << tmplo << "," << tmphi
				  // << " colour is "
				  // << NFmiColorTools::GetRed(color) << ","
				  // << NFmiColorTools::GetGreen(color) << ","
				  // << NFmiColorTools::GetBlue(color) << ","
				  // << NFmiColorTools::GetAlpha(color)
				  // << endl;
				}
			}
	      
		  else if(command == "contourlines")
			{
			  float lo,hi,step;
			  string scolor1,scolor2;
			  input >> lo >> hi >> step >> scolor1 >> scolor2;
			  
			  int color1 = ToColor(scolor1);
			  int color2 = ToColor(scolor2);
			  
			  int steps = static_cast<int>((hi-lo)/step);
			  
			  for(int i=0; i<=steps; i++)
				{
				  float tmplo=lo+i*step;
				  int color = color1;	// in case steps=1
				  if(steps!=0)
					color = NFmiColorTools::Interpolate(color1,color2,i/steps);
				  if(!theSpecs.empty())
					theSpecs.back().Add(ContourValue(tmplo,color,theStrokeRule));
				  // if(verbose)
				  // cout << "Value " << tmplo
				  // << " colour is "
				  // << NFmiColorTools::GetRed(color) << ","
				  // << NFmiColorTools::GetGreen(color) << ","
				  // << NFmiColorTools::GetBlue(color) << ","
				  // << NFmiColorTools::GetAlpha(color)
				  // << endl;
				}
			}
	      
		  else if(command == "clear")
			{
			  input >> command;
			  if(command=="contours")
				theSpecs.clear();
			  else if(command=="shapes")
				theShapeSpecs.clear();
			  else if(command=="arrows")
				{
				  theArrowPoints.clear();
				  theWindArrowDX = 0;
				  theWindArrowDY = 0;
				}
			  else if(command=="labels")
				{
				  list<ContourSpec>::iterator piter;
				  for(piter=theSpecs.begin(); piter!=theSpecs.end(); ++piter)
					piter->ClearLabels();
				}
			  else if(command=="corners")
				{
				  theBottomLeft = NFmiPoint(kFloatMissing,kFloatMissing);
				  theTopRight = NFmiPoint(kFloatMissing,kFloatMissing);
				}
			  else
				{
				  cerr << "Error: Unknown clear target: " << command << endl;
				  exit(1);
				}
			}
		  
		  else if(command == "labelmarker")
			{
			  string filename, rule;
			  float alpha;
			  
			  input >> filename >> rule >> alpha;
			  
			  if(!theSpecs.empty())
				{
				  theSpecs.back().LabelMarker(filename);
				  theSpecs.back().LabelMarkerRule(rule);
				  theSpecs.back().LabelMarkerAlphaFactor(alpha);
				}
			}
		  
		  else if(command == "labelfont")
			{
			  string font;
			  input >> font;
			  if(!theSpecs.empty())
				theSpecs.back().LabelFont(font);
			}
		  
		  else if(command == "labelsize")
			{
			  float size;
			  input >> size;
			  if(!theSpecs.empty())
				theSpecs.back().LabelSize(size);
			}
		  
		  else if(command == "labelstroke")
			{
			  string color,rule;
			  input >> color >> rule;
			  if(!theSpecs.empty())
				{
				  theSpecs.back().LabelStrokeColor(ToColor(color));
				  theSpecs.back().LabelStrokeRule(rule);
				}
			}
		  
		  else if(command == "labelfill")
			{
			  string color,rule;
			  input >> color >> rule;
			  if(!theSpecs.empty())
				{
				  theSpecs.back().LabelFillColor(ToColor(color));
				  theSpecs.back().LabelFillRule(rule);
				}
			}
		  
		  else if(command == "labelalign")
			{
			  string align;
			  input >> align;
			  if(!theSpecs.empty())
				theSpecs.back().LabelAlignment(align);
			}
		  
		  else if(command == "labelformat")
			{
			  string format;
			  input >> format;
			  if(format == "-") format = "";
			  if(!theSpecs.empty())
				theSpecs.back().LabelFormat(format);
			}
		  
		  else if(command == "labelangle")
			{
			  float angle;
			  input >> angle;
			  if(!theSpecs.empty())
				theSpecs.back().LabelAngle(angle);
			}
		  
		  else if(command == "labeloffset")
			{
			  float dx,dy;
			  input >> dx >> dy;
			  if(!theSpecs.empty())
				{
				  theSpecs.back().LabelOffsetX(dx);
				  theSpecs.back().LabelOffsetY(dy);
				}
			}
		  
		  else if(command == "labelcaption")
			{
			  string name,align;
			  float dx,dy;
			  input >> name >> dx >> dy >> align;
			  if(!theSpecs.empty())
				{
				  theSpecs.back().LabelCaption(name);
				  theSpecs.back().LabelCaptionDX(dx);
				  theSpecs.back().LabelCaptionDY(dy);
				  theSpecs.back().LabelCaptionAlignment(align);
				}
			}
		  
		  else if(command == "label")
			{
			  float lon,lat;
			  input >> lon >> lat;
			  if(!theSpecs.empty())
				theSpecs.back().Add(NFmiPoint(lon,lat));
			}
		  
		  else if(command == "labelxy")
			{
			  float lon,lat;
			  input >> lon >> lat;
			  int dx, dy;
			  input >> dx >> dy;
			  if(!theSpecs.empty())
				theSpecs.back().Add(NFmiPoint(lon,lat),NFmiPoint(dx,dy));
			}
		  
		  else if(command == "labels")
			{
			  int dx,dy;
			  input >> dx >> dy;
			  if(!theSpecs.empty())
				{
				  theSpecs.back().LabelDX(dx);
				  theSpecs.back().LabelDY(dy);
				}
			}
		  
		  else if(command == "labelfile")
			{
			  string datafilename;
			  input >> datafilename;
			  ifstream datafile(datafilename.c_str());
			  if(!datafile)
				{
				  cerr << "Error: No data file named " << datafilename << endl;
				  exit(1);
				}
			  string datacommand;
			  while( datafile >> datacommand)
				{
				  if(datacommand == "#" || datacommand == "//")
					datafile.ignore(1000000,'\n');
				  else if(datacommand == "label")
					{
					  float lon,lat;
					  datafile >> lon >> lat;
					  if(!theSpecs.empty())
						theSpecs.back().Add(NFmiPoint(lon,lat));
					}
				  else
					{
					  cerr << "Error: Unknown datacommand " << datacommand << endl;
					  exit(1);
					}
				}
			  datafile.close();
			}
		  
		  else if(command == "draw")
			{
			  // Draw what?
			  
			  input >> command;
			  
			  // --------------------------------------------------
			  // Draw legend
			  // --------------------------------------------------
			  
			  if(command == "legend")
				{
				  string legendname;
				  int width, height;
				  float lolimit, hilimit;
				  input >> legendname >> lolimit >> hilimit >> width >> height;
				  
				  if(!theSpecs.empty())
					{
					  NFmiImage legend(width,height);
					  
					  NFmiColorTools::Color color = ToColor(theLegendErase);
					  legend.Erase(color);
					  
					  list<ContourRange>::const_iterator citer;
					  list<ContourRange>::const_iterator cbegin;
					  list<ContourRange>::const_iterator cend;
					  
					  cbegin = theSpecs.back().ContourFills().begin();
					  cend   = theSpecs.back().ContourFills().end();
					  
					  for(citer=cbegin ; citer!=cend; ++citer)
						{
						  float thelo = citer->LoLimit();
						  float thehi = citer->HiLimit();
						  NFmiColorTools::NFmiBlendRule rule = NFmiColorTools::BlendValue(citer->Rule());
						  
						  if(thelo==kFloatMissing) thelo=-1e6;
						  if(thehi==kFloatMissing) thehi= 1e6;
						  
						  NFmiPath path;
						  path.MoveTo(0,height*(1-(thelo-lolimit)/(hilimit-lolimit)));
						  path.LineTo(width,height*(1-(thelo-lolimit)/(hilimit-lolimit)));
						  path.LineTo(width,height*(1-(thehi-lolimit)/(hilimit-lolimit)));
						  path.LineTo(0,height*(1-(thehi-lolimit)/(hilimit-lolimit)));
						  path.CloseLineTo();
						  
						  path.Fill(legend,citer->Color(),rule);
						}
					  
					  list<ContourValue>::const_iterator liter;
					  list<ContourValue>::const_iterator lbegin;
					  list<ContourValue>::const_iterator lend;
					  
					  lbegin = theSpecs.back().ContourValues().begin();
					  lend   = theSpecs.back().ContourValues().end();
					  
					  for(liter=lbegin ; liter!=lend; ++liter)
						{
						  float thevalue = liter->Value();
						  
						  if(thevalue==kFloatMissing)
							continue;
						  
						  NFmiPath path;
						  path.MoveTo(0,height*(1-(thevalue-lolimit)/(hilimit-lolimit)));
						  path.LineTo(width,height*(1-(thevalue-lolimit)/(hilimit-lolimit)));
						  path.Stroke(legend,liter->Color());
						}
					  
					  legend.WritePng(legendname+".png");
					}
				  
				}
			  
			  // --------------------------------------------------
			  // Render shapes
			  // --------------------------------------------------
			  
			  else if(command == "shapes")
				{
				  // The output filename
				  
				  string filename;
				  input >> filename;
				  
				  if(theBottomLeft.X()==kFloatMissing ||
					 theBottomLeft.Y()==kFloatMissing ||
					 theTopRight.X()==kFloatMissing ||
					 theTopRight.Y()==kFloatMissing)
					{

					  if(theCenter.X()==kFloatMissing || theCenter.Y()==kFloatMissing)
						{
						  cerr << "Error: Area corner coordinates not given" << endl;
						  return 1;
						}
					  
					  if(theScale<0 || theWidth<0 || theHeight<0)
						{
						  cerr << "Error: scale, width and height must be given along with center coordinates" << endl;
						  return 1;
						}

					  NFmiStereographicArea area(theCenter,theCenter,
												 theCentralLongitude,
												 NFmiPoint(0,0),
												 NFmiPoint(1,1),
												 theCentralLatitude,
												 theTrueLatitude);

					  NFmiPoint c = area.LatLonToWorldXY(theCenter);
					  
					  NFmiPoint bl(c.X()-theScale*1000*theWidth, c.Y()-theScale*1000*theHeight);
					  NFmiPoint tr(c.X()+theScale*1000*theWidth, c.Y()+theScale*1000*theHeight);

					  theBottomLeft = area.WorldXYToLatLon(bl);
					  theTopRight = area.WorldXYToLatLon(tr);

					  if(verbose)
						{
						  cout << "Calculated corner points to be"
							   << endl
							   << "bottomleft\t= " 
							   << theBottomLeft.X()
							   << ','
							   << theBottomLeft.Y()
							   << endl
							   << "topright\t= "
							   << theTopRight.X()
							   << ','
							   << theTopRight.Y()
							   << endl;
						}

					}

				  
				  // Initialize XY-coordinates
				  
				  NFmiStereographicArea area(theBottomLeft,
											 theTopRight,
											 theCentralLongitude,
											 NFmiPoint(0,0),
											 NFmiPoint(1,1),
											 theCentralLatitude,
											 theTrueLatitude);
				  
				  // Calculate world coordinates
				  
				  NFmiPoint bl = area.LatLonToWorldXY(theBottomLeft);
				  NFmiPoint tr = area.LatLonToWorldXY(theTopRight);
				  
				  if(theWidth<=0 && theHeight>0)
					{
					  // Calculate width from height
					  theWidth = static_cast<int>((tr.X()-bl.X())/(tr.Y()-bl.Y())*theHeight);
					}
				  else if(theHeight<=0 && theWidth>0)
					{
					  // Calculate height from width
					  theHeight = static_cast<int>((tr.Y()-bl.Y())/(tr.X()-bl.X())*theWidth);
					}
				  else if(theWidth<=0 && theHeight<=0)
					{
					  cerr << "Error: Image width & height unspecified"
						   << endl;
					  exit(1);
					}
				  
				  // The actual area we wanted
				  
				  NFmiStereographicArea theArea(theBottomLeft,
												theTopRight,
												theCentralLongitude,
												NFmiPoint(0,0),
												NFmiPoint(theWidth,theHeight),
												theCentralLatitude,
												theTrueLatitude);
				  
				  // Initialize the background
				  
				  NFmiImage theImage(theWidth,theHeight);
				  theImage.SaveAlpha(theSaveAlphaFlag);
				  theImage.WantPalette(theWantPaletteFlag);
				  theImage.ForcePalette(theForcePaletteFlag);
				  if(theGamma>0) theImage.Gamma(theGamma);
				  if(!theIntent.empty()) theImage.Intent(theIntent);
				  if(thePngQuality>=0) theImage.PngQuality(thePngQuality);
				  if(theJpegQuality>=0) theImage.JpegQuality(theJpegQuality);
				  if(theAlphaLimit>=0) theImage.AlphaLimit(theAlphaLimit);
				  
				  NFmiColorTools::Color erasecolor = ToColor(theErase);
				  theImage.Erase(erasecolor);
				  
				  // Draw all the shapes
				  
				  list<ShapeSpec>::const_iterator iter;
				  list<ShapeSpec>::const_iterator begin = theShapeSpecs.begin();
				  list<ShapeSpec>::const_iterator end   = theShapeSpecs.end();
				  
				  for(iter=begin; iter!=end; ++iter)
					{
					  NFmiGeoShape geo(iter->FileName(),kFmiGeoShapeEsri);
					  geo.ProjectXY(theArea);
					  
					  if(iter->Marker()=="")
						{
						  NFmiColorTools::NFmiBlendRule fillrule = NFmiColorTools::BlendValue(iter->FillRule());
						  NFmiColorTools::NFmiBlendRule strokerule = NFmiColorTools::BlendValue(iter->StrokeRule());
						  geo.Fill(theImage,iter->FillColor(),fillrule);
						  geo.Stroke(theImage,iter->StrokeColor(),strokerule);
						}
					  else
						{
						  NFmiColorTools::NFmiBlendRule markerrule = NFmiColorTools::BlendValue(iter->MarkerRule());
						  
						  NFmiImage marker;
						  marker.Read(iter->Marker());
						  geo.Mark(theImage,marker,markerrule,
								   kFmiAlignCenter,
								   iter->MarkerAlpha());
						}
					}
				  
				  string outfile = filename + "." + theFormat;
				  if(verbose)
					cout << "Writing " << outfile << endl;
				  if(theFormat=="png")
					theImage.WritePng(outfile);
				  else if(theFormat=="jpg" || theFormat=="jpeg")
					theImage.WriteJpeg(outfile);
				  else if(theFormat=="gif")
					theImage.WriteGif(outfile);
				  else
					{
					  cerr << "Error: Image format " << theFormat << " is not supported" << endl;
					  return 1;
					}
				}
			  
			  // --------------------------------------------------
			  // Generate imagemap data
			  // --------------------------------------------------
			  
			  else if(command == "imagemap")
				{
				  // The relevant field name and filenames
				  
				  string fieldname, filename;
				  input >> fieldname >> filename;
				  
				  if(theBottomLeft.X()==kFloatMissing ||
					 theBottomLeft.Y()==kFloatMissing ||
					 theTopRight.X()==kFloatMissing ||
					 theTopRight.Y()==kFloatMissing)
					{
					  cerr << "Error: Area corner coordinates not given"
						   << endl;
					  return 1;


					}
				  
				  // Initialize XY-coordinates
				  
				  NFmiStereographicArea area(theBottomLeft,
											 theTopRight,
											 theCentralLongitude,
											 NFmiPoint(0,0),
											 NFmiPoint(1,1),
											 theCentralLatitude,
											 theTrueLatitude);
				  
				  // Calculate world coordinates
				  
				  NFmiPoint bl = area.LatLonToWorldXY(theBottomLeft);
				  NFmiPoint tr = area.LatLonToWorldXY(theTopRight);
				  
				  if(theWidth<=0 && theHeight>0)
					{
					  // Calculate width from height
					  theWidth = static_cast<int>((tr.X()-bl.X())/(tr.Y()-bl.Y())*theHeight);
					}
				  else if(theHeight<=0 && theWidth>0)
					{
					  // Calculate height from width
					  theHeight = static_cast<int>((tr.Y()-bl.Y())/(tr.X()-bl.X())*theWidth);
					}
				  else if(theWidth<=0 && theHeight<=0)
					{
					  cerr << "Error: Image width & height unspecified"
						   << endl;
					  exit(1);
					}
				  
				  // The actual area we wanted
				  
				  NFmiStereographicArea theArea(theBottomLeft,
												theTopRight,
												theCentralLongitude,
												NFmiPoint(0,0),
												NFmiPoint(theWidth,theHeight),
												theCentralLatitude,
												theTrueLatitude);
				  
				  // Generate map from all shapes in the list
				  
				  list<ShapeSpec>::const_iterator iter;
				  list<ShapeSpec>::const_iterator begin = theShapeSpecs.begin();
				  list<ShapeSpec>::const_iterator end   = theShapeSpecs.end();
				  
				  string outfile = filename + ".map";
				  ofstream out(outfile.c_str());
				  if(!out)
					{
					  cerr << "Error: Failed to open "
						   << outfile
						   << " for writing"
						   << endl;
					  exit(1);
					}
				  if(verbose)
					cout << "Writing " << outfile << endl;
				  
				  for(iter=begin; iter!=end; ++iter)
					{
					  NFmiGeoShape geo(iter->FileName(),kFmiGeoShapeEsri);
					  geo.ProjectXY(theArea);
					  geo.WriteImageMap(out,fieldname);
					}
				  out.close();
				}
			  
			  // --------------------------------------------------
			  // Draw contours
			  // --------------------------------------------------
			  
			  else if(command == "contours")
				{
				  // 1. Make sure query data has been read
				  // 2. Make sure image has been initialized
				  // 3. Loop over all times
				  //   4. If the time is acceptable,
				  //   5. Loop over all parameters
				  //     6. Fill all specified intervals
				  //     7. Patternfill all specified intervals
				  //     8. Stroke all specified contours
				  //   9. Overwrite with foreground if so desired
				  //   10. Loop over all parameters
				  //     11. Label all specified points
				  //   12. Draw arrows if requested
				  //   13. Save the image
				  
				  if(theQueryStreams.empty())
					{
					  cerr << "Error: No query data has been read!\n";
					  return 1;
					}
				  
				  // Check map image width & height
				  
				  if(theBackground != "")
					{
					  theWidth = theBackgroundImage.Width();
					  theHeight = theBackgroundImage.Height();
					}
				  
				  if(theBottomLeft.X()==kFloatMissing ||
					 theBottomLeft.Y()==kFloatMissing ||
					 theTopRight.X()==kFloatMissing ||
					 theTopRight.Y()==kFloatMissing)
					{

					  // Duplicate code as in "draw shapes"
					  if(theCenter.X()==kFloatMissing || theCenter.Y()==kFloatMissing)
						{
						  cerr << "Error: Area corner coordinates not given" << endl;
						  return 1;
						}
					  
					  if(theScale<0 || theWidth<0 || theHeight<0)
						{
						  cerr << "Error: scale, width and height must be given along with center coordinates" << endl;
						  return 1;
						}

					  NFmiStereographicArea area(theCenter,theCenter,
												 theCentralLongitude,
												 NFmiPoint(0,0),
												 NFmiPoint(1,1),
												 theCentralLatitude,
												 theTrueLatitude);

					  NFmiPoint c = area.LatLonToWorldXY(theCenter);
					  
					  NFmiPoint bl(c.X()-theScale*1000*theWidth, c.Y()-theScale*1000*theHeight);
					  NFmiPoint tr(c.X()+theScale*1000*theWidth, c.Y()+theScale*1000*theHeight);

					  theBottomLeft = area.WorldXYToLatLon(bl);
					  theTopRight = area.WorldXYToLatLon(tr);
					  
					}
				  
				  // Initialize XY-coordinates
				  
				  NFmiStereographicArea area(theBottomLeft,
											 theTopRight,
											 theCentralLongitude,
											 NFmiPoint(0,0),
											 NFmiPoint(1,1),
											 theCentralLatitude,
											 theTrueLatitude);
				  
				  // Calculate world coordinates
				  
				  NFmiPoint bl = area.LatLonToWorldXY(theBottomLeft);
				  NFmiPoint tr = area.LatLonToWorldXY(theTopRight);
				  
				  if(theWidth<=0 && theHeight>0)
					{
					  // Calculate width from height
					  theWidth = static_cast<int>((tr.X()-bl.X())/(tr.Y()-bl.Y())*theHeight);
					}
				  else if(theHeight<=0 && theWidth>0)
					{
					  // Calculate height from width
					  theHeight = static_cast<int>((tr.Y()-bl.Y())/(tr.X()-bl.X())*theWidth);
					}
				  else if(theWidth<=0 && theHeight<=0)
					{
					  cerr << "Error: Image width & height unspecified"
						   << endl;
					  exit(1);
					}
				  
				  // The actual area we wanted
				  
				  NFmiStereographicArea theArea(theBottomLeft,
												theTopRight,
												theCentralLongitude,
												NFmiPoint(0,0),
												NFmiPoint(theWidth,theHeight),
												theCentralLatitude,
												theTrueLatitude);
				  
				  // Establish querydata timelimits and initialize
				  // the XY-coordinates simultaneously.
				  
				  // Note that we use world-coordinates when smoothing
				  // so that we can use meters as the smoothing radius.
				  // Also, this means the contours are independent of
				  // the image size.
				  
				  NFmiTime utctime, time1, time2;
				  
				  vector<NFmiDataMatrix<NFmiPoint> > worldpts(theQueryStreams.size());
				  vector<NFmiDataMatrix<NFmiPoint> > pts(theQueryStreams.size());
				  NFmiDataMatrix<float> vals;
				  
				  unsigned int qi;
				  for(qi=0; qi<theQueryStreams.size(); qi++)
					{
					  // Initialize the queryinfo
					  
					  theQueryInfo = theQueryStreams[qi]->QueryInfoIter();
					  theQueryInfo->FirstLevel();
					  if(theQueryDataLevel>0)
						{
						  int level = theQueryDataLevel;
						  while(--level > 0)
							theQueryInfo->NextLevel();
						}
					  
					  // Establish time limits
					  theQueryInfo->LastTime();
					  utctime = theQueryInfo->ValidTime();
					  NFmiTime t2 = NFmiMetTime(utctime,1).LocalTime();
					  theQueryInfo->FirstTime();
					  utctime = theQueryInfo->ValidTime();
					  NFmiTime t1 = NFmiMetTime(utctime,1).LocalTime();
					  
					  if(qi==0)
						{
						  time1 = t1;
						  time2 = t2;
						}
					  else
						{
						  if(time1.IsLessThan(t1))
							time1 = t1;
						  if(!time2.IsLessThan(t2))
							time2 = t2;
						}
					  
					  // Establish coordinates
					  
					  theQueryInfo->LocationsWorldXY(worldpts[qi],theArea);
					  
					  theQueryInfo->LocationsXY(pts[qi],theArea);
					  
					}
				  
				  if(verbose)
					{
					  cout << "Data start time " << time1 << endl
						   << "Data end time " << time2 << endl;
					}
				  
				  // Skip to first time
				  
				  NFmiMetTime tmptime(time1,
									  theTimeStepRoundingFlag ?
									  (theTimeStep>0 ? theTimeStep : 1) :
									  1);
				  
				  tmptime.ChangeByMinutes(theTimeStepSkip);
				  if(theTimeStepRoundingFlag)
					tmptime.PreviousMetTime();
				  NFmiTime t = tmptime;
				  
				  // Loop over all times
				  
				  int imagesdone = 0;
				  while(true)
					{
					  if(imagesdone>=theTimeSteps)
						break;
					  
					  // Skip to next time to be drawn
					  
					  t.ChangeByMinutes(theTimeStep > 0 ? theTimeStep : 1);
					  
					  cout << t << endl;
					  
					  // If the time is after time2, we're done
					  
					  if(time2.IsLessThan(t))
						break;
					  
					  // Search first time >= the desired time
					  // This is quaranteed to succeed since we've
					  // already tested against time2, the last available
					  // time.
					  
					  bool ok = true;
					  for(qi=0; ok && qi<theQueryStreams.size(); qi++)
						{
						  theQueryInfo = theQueryStreams[qi]->QueryInfoIter();
						  theQueryInfo->ResetTime();
						  while(theQueryInfo->NextTime())
							{
							  NFmiTime utc = theQueryInfo->ValidTime();
							  NFmiTime loc = NFmiMetTime(utc,1).LocalTime();
							  if(!loc.IsLessThan(t))
								break;
							}
						  NFmiTime utc = theQueryInfo->ValidTime();
						  NFmiTime tnow = NFmiMetTime(utc,1).LocalTime();
						  
						  // we wanted
						  
						  if(theTimeStep==0)
							t = tnow;
						  
						  // If time is before time1, ignore it
						  
						  if(t.IsLessThan(time1))
							{
							  ok = false;
							  break;
							}
						  
						  // Is the time exact?
						  
						  bool isexact = t.IsEqual(tnow);
						  
						  // The previous acceptable time step in calculations
						  // Use NFmiTime, not NFmiMetTime to avoid rounding up!
						  
						  NFmiTime tprev = t;
						  tprev.ChangeByMinutes(-theTimeInterval);
						  
						  bool hasprevious = !tprev.IsLessThan(time1);
						  
						  // Skip this image if we are unable to render it
						  
						  if(theFilter=="none")
							{
							  // Cannot draw time with filter none
							  // if time is not exact.
							  
							  ok = isexact;
							  
							}
						  else if(theFilter=="linear")
							{
							  // OK if is exact, otherwise previous step required
							  
							  ok = !(!isexact && !hasprevious);
							}
						  else
							{
							  // Time must be exact, and previous steps
							  // are required
							  
							  ok = !(!isexact || !hasprevious);
							}
						}
					  
					  if(!ok)
						continue;
					  
					  // The image is accepted for rendering, but
					  // we might not overwrite an existing one.
					  // Hence we update the counter here already.
					  
					  imagesdone++;
					  
					  // Create the filename
					  
					  // The timestamp as a string
					  
					  NFmiString datatimestr = t.ToStr(kYYYYMMDDHHMM);
					  
					  if(verbose)
						cout << "Time is " << datatimestr.CharPtr() << endl;

					  string filename =
						theSavePath
						+ "/"
						+ thePrefix
						+ datatimestr.CharPtr();
					  
					  if(theTimeStampFlag)
						{
						  for(qi=0; qi<theQueryStreams.size(); qi++)
							{
							  theQueryInfo = theQueryStreams[qi]->QueryInfoIter();
							  NFmiTime futctime = theQueryInfo->OriginTime(); 
							  NFmiTime tfor = NFmiMetTime(futctime,1).LocalTime();
							  filename += "_" + tfor.ToStr(kDDHHMM);
							}
						}
					  
					  filename +=
						theSuffix
						+ "."
						+ theFormat;
					  
					  // In force-mode we always write, but otherwise
					  // we first check if the output image already
					  // exists. If so, we assume it is up to date
					  // and skip to the next time stamp.
					  
					  if(!force && !FileEmpty(filename))
						{
						  if(verbose)
							cout << "Not overwriting " << filename << endl;
						  continue;
						}
					  
					  // Initialize the background
					  
					  NFmiImage theImage(theWidth,theHeight);
					  theImage.SaveAlpha(theSaveAlphaFlag);
					  theImage.WantPalette(theWantPaletteFlag);
					  theImage.ForcePalette(theForcePaletteFlag);
					  if(theGamma>0) theImage.Gamma(theGamma);
					  if(!theIntent.empty()) theImage.Intent(theIntent);
					  if(thePngQuality>=0) theImage.PngQuality(thePngQuality);
					  if(theJpegQuality>=0) theImage.JpegQuality(theJpegQuality);
					  if(theAlphaLimit>=0) theImage.AlphaLimit(theAlphaLimit);
					  
					  NFmiColorTools::Color erasecolor = ToColor(theErase);
					  theImage.Erase(erasecolor);
					  
					  if(theBackground != "")
						theImage = theBackgroundImage;
					  
					  // Loop over all parameters
					  
					  list<ContourSpec>::iterator piter;
					  list<ContourSpec>::iterator pbegin = theSpecs.begin();
					  list<ContourSpec>::iterator pend   = theSpecs.end();
					  
					  for(piter=pbegin; piter!=pend; ++piter)
						{
						  // Establish the parameter
						  
						  string name = piter->Param();
						  FmiParameterName param = FmiParameterName(NFmiEnumConverter().ToEnum(name));
						  
						  if(param==kFmiBadParameter)
							{
							  cerr << "Error: Unknown parameter " << name << endl;
							  return 1;
							}
						  
						  // Find the proper queryinfo to be used
						  // Note that qi will be used later on for
						  // getting the coordinate matrices
						  
						  ok = false;
						  for(qi=0; qi<theQueryStreams.size(); qi++)
							{
							  theQueryInfo = theQueryStreams[qi]->QueryInfoIter();
							  theQueryInfo->Param(param);
							  ok = theQueryInfo->IsParamUsable();
							  if(ok) break;
							}
						  
						  if(!ok)
							{
							  cerr << "Error: The parameter is not usable: " << name << endl;
							  exit(1);
							}
						  
						  if(verbose)
							{
							  cout << "Param " << name << " from queryfile number "
								   << (qi+1) << endl;
							}
						  
						  // Establish the contour method
						  
						  string interpname = piter->ContourInterpolation();
						  NFmiContourTree::NFmiContourInterpolation interp
							= NFmiContourTree::ContourInterpolationValue(interpname);
						  if(interp==NFmiContourTree::kFmiContourMissingInterpolation)
							{
							  cerr << "Error: Unknown contour interpolation method " << interpname << endl;
							  exit(1);
							}
						  
						  // Get the values. 
						  
						  theQueryInfo->Values(vals);
						  
						  // Replace values if so requested
						  
						  if(piter->Replace())
							vals.Replace(piter->ReplaceSourceValue(),piter->ReplaceTargetValue());
						  
						  if(theFilter=="none")
							{
							  // The time is known to be exact
							}
						  else if(theFilter=="linear")
							{
							  NFmiTime utc = theQueryInfo->ValidTime();
							  NFmiTime tnow = NFmiMetTime(utc,1).LocalTime();
							  bool isexact = t.IsEqual(tnow);
							  
							  if(!isexact)
								{
								  NFmiDataMatrix<float> tmpvals;
								  NFmiTime t2utc = theQueryInfo->ValidTime();
								  NFmiTime t2 = NFmiMetTime(t2utc,1).LocalTime();
								  theQueryInfo->PreviousTime();
								  NFmiTime t1utc = theQueryInfo->ValidTime();
								  NFmiTime t1 = NFmiMetTime(t1utc,1).LocalTime();
								  theQueryInfo->Values(tmpvals);
								  if(piter->Replace())
									tmpvals.Replace(piter->ReplaceSourceValue(),
													piter->ReplaceTargetValue());
								  
								  // Data from t1,t2, we want t
								  
								  long offset = t.DifferenceInMinutes(t1);
								  long range = t2.DifferenceInMinutes(t1);
								  
								  float weight = (static_cast<float>(offset))/range;
								  
								  vals.LinearCombination(tmpvals,weight,1-weight);
								  
								}
							}
						  else
							{
							  NFmiTime tprev = t;
							  tprev.ChangeByMinutes(-theTimeInterval);
							  
							  NFmiDataMatrix<float> tmpvals;
							  int steps = 1;
							  while(true)
								{
								  theQueryInfo->PreviousTime();
								  NFmiTime utc = theQueryInfo->ValidTime();
								  NFmiTime tnow = NFmiMetTime(utc,1).LocalTime();
								  if(tnow.IsLessThan(tprev))
									break;
								  
								  steps++;
								  theQueryInfo->Values(tmpvals);
								  if(piter->Replace())
									tmpvals.Replace(piter->ReplaceSourceValue(),
													piter->ReplaceTargetValue());
								  
								  if(theFilter=="min")
									vals.Min(tmpvals);
								  else if(theFilter=="max")
									vals.Max(tmpvals);
								  else if(theFilter=="mean")
									vals += tmpvals;
								  else if(theFilter=="sum")
									vals += tmpvals;
								}
							  
							  if(theFilter=="mean")
								vals /= steps;
							}
						  
						  
						  // Smoothen the values
						  
						  NFmiSmoother smoother(piter->Smoother(),
												piter->SmootherFactor(),
												piter->SmootherRadius());
						  
						  vals = smoother.Smoothen(worldpts[qi],vals);
						  
						  // ofstream out("values.dat");
						  // out << vals;
						  // out.close();
						  
						  // Find the minimum and maximum
						  
						  float valmin = kFloatMissing;
						  float valmax = kFloatMissing;
						  for(unsigned int j=0; j<vals.NY(); j++)
							for(unsigned int i=0; i<vals.NX(); i++)
							  if(vals[i][j]!=kFloatMissing)
								{
								  if(valmin==kFloatMissing || vals[i][j]<valmin)
									valmin = vals[i][j];
								  if(valmax==kFloatMissing || vals[i][j]>valmax)
									valmax = vals[i][j];
								}
						  
						  if(verbose)
							cout << "Data range for " << name << " is " << valmin << "," << valmax << endl;
						  
						  // Save the data values at desired points for later
						  // use, this lets ups avoid using InterpolatedValue()
						  // which does not use smoothened values.
						  
						  piter->ClearLabelValues();
						  if((piter->LabelFormat() != "") &&
							 !piter->LabelPoints().empty() )
							{
							  list<pair<NFmiPoint,NFmiPoint> >::const_iterator iter;
							  
							  for(iter=piter->LabelPoints().begin();
								  iter!=piter->LabelPoints().end();
								  ++iter)
								{
								  NFmiPoint latlon = iter->first;
								  NFmiPoint ij = theQueryInfo->Grid()->LatLonToGrid(latlon);
								  
								  float value;
								  
								  if(fabs(ij.X()-FmiRound(ij.X()))<0.00001 &&
									 fabs(ij.Y()-FmiRound(ij.Y()))<0.00001)
									value = vals[static_cast<int>(ij.X())][static_cast<int>(ij.Y())];
								  else
									{
									  int i = static_cast<int>(ij.X());
									  int j = static_cast<int>(ij.Y());
									  float v00 = vals.At(i,j,kFloatMissing);
									  float v10 = vals.At(i+1,j,kFloatMissing);
									  float v01 = vals.At(i,j+1,kFloatMissing);
									  float v11 = vals.At(i+1,j+1,kFloatMissing);
									  if(!theQueryInfo->BiLinearInterpolation(ij.X(),
																			  ij.Y(),
																			  value,
																			  v00,v10,
																			  v01,v11))
										value = kFloatMissing;
									}
								  piter->AddLabelValue(value);
								}
							}
						  
						  // Fill the contours
						  
						  list<ContourRange>::const_iterator citer;
						  list<ContourRange>::const_iterator cbegin;
						  list<ContourRange>::const_iterator cend;
						  
						  cbegin = piter->ContourFills().begin();
						  cend   = piter->ContourFills().end();
						  
						  for(citer=cbegin ; citer!=cend; ++citer)
							{
							  // Skip to next contour if this one is outside
							  // the value range. As a special case
							  // min=max=missing is ok, if both the limits
							  // are missing too. That is, when we are
							  // contouring missing values.
							  
							  if(valmin==kFloatMissing || valmax==kFloatMissing)
								{
								  if(citer->LoLimit()!=kFloatMissing &&
									 citer->HiLimit()!=kFloatMissing)
									continue;
								}
							  else
								{
								  if(citer->LoLimit()!=kFloatMissing &&
									 valmax<citer->LoLimit())
									continue;
								  if(citer->HiLimit()!=kFloatMissing &&
									 valmin>citer->HiLimit())
									continue;
								}
							  
							  bool exactlo = true;
							  bool exacthi = (citer->HiLimit()!=kFloatMissing &&
											  piter->ExactHiLimit()!=kFloatMissing &&
											  citer->HiLimit()==piter->ExactHiLimit());
							  NFmiContourTree tree(citer->LoLimit(),
												   citer->HiLimit(),
												   exactlo,exacthi);
							  
							  if(piter->DataLoLimit()!=kFloatMissing)
								tree.DataLoLimit(piter->DataLoLimit());
							  if(piter->DataHiLimit()!=kFloatMissing)
								tree.DataHiLimit(piter->DataHiLimit());
							  
							  NFmiColorTools::NFmiBlendRule rule = NFmiColorTools::BlendValue(citer->Rule());
							  
							  tree.Contour(pts[qi],vals,interp,piter->ContourDepth());
							  tree.Fill(theImage,citer->Color(),rule);
							  
							  // NFmiPath path = tree.Path();
							  // path.Fill(theImage,citer->Color(),rule);
							  // cout << "<path style=\"fill=rgb("
							  // << NFmiColorTools::GetRed(citer->Color()) << ","
							  // << NFmiColorTools::GetGreen(citer->Color()) << ","
							  // << NFmiColorTools::GetBlue(citer->Color())
							  // << ")\" d=\""
							  // << path.SVG()
							  // << "\">"
							  // << endl;
							  
							}
						  
						  // Fill the contours with patterns
						  
						  list<ContourPattern>::const_iterator patiter;
						  list<ContourPattern>::const_iterator patbegin;
						  list<ContourPattern>::const_iterator patend;
						  
						  patbegin = piter->ContourPatterns().begin();
						  patend   = piter->ContourPatterns().end();
						  
						  for(patiter=patbegin ; patiter!=patend; ++patiter)
							{
							  // Skip to next contour if this one is outside
							  // the value range. As a special case
							  // min=max=missing is ok, if both the limits
							  // are missing too. That is, when we are
							  // contouring missing values.
							  
							  if(valmin==kFloatMissing || valmax==kFloatMissing)
								{
								  if(patiter->LoLimit()!=kFloatMissing &&
									 patiter->HiLimit()!=kFloatMissing)
									continue;
								}
							  else
								{
								  if(patiter->LoLimit()!=kFloatMissing &&
									 valmax<patiter->LoLimit())
									continue;
								  if(patiter->HiLimit()!=kFloatMissing &&
									 valmin>patiter->HiLimit())
									continue;
								}
							  
							  bool exactlo = true;
							  bool exacthi = (patiter->HiLimit()!=kFloatMissing &&
											  piter->ExactHiLimit()!=kFloatMissing &&
											  patiter->HiLimit()==piter->ExactHiLimit());
							  NFmiContourTree tree(patiter->LoLimit(),
												   patiter->HiLimit(),
												   exactlo,exacthi);
							  
							  if(piter->DataLoLimit()!=kFloatMissing)
								tree.DataLoLimit(piter->DataLoLimit());
							  if(piter->DataHiLimit()!=kFloatMissing)
								tree.DataHiLimit(piter->DataHiLimit());
							  
							  NFmiColorTools::NFmiBlendRule rule = NFmiColorTools::BlendValue(patiter->Rule());
							  
							  tree.Contour(pts[qi],vals,interp,piter->ContourDepth());
							  NFmiImage pattern(patiter->Pattern());
							  
							  tree.Fill(theImage,pattern,rule,patiter->Factor());
							  
							  // NFmiPath path = tree.Path();
							  // path.Fill(theImage,citer->Color(),rule);
							  // cout << "<path style=\"fill=rgb("
							  // << NFmiColorTools::GetRed(citer->Color()) << ","
							  // << NFmiColorTools::GetGreen(citer->Color()) << ","
							  // << NFmiColorTools::GetBlue(citer->Color())
							  // << ")\" d=\""
							  // << path.SVG()
							  // << "\">"
							  // << endl;
							  
							}
						  
						  // Stroke the contours
						  
						  list<ContourValue>::const_iterator liter;
						  list<ContourValue>::const_iterator lbegin;
						  list<ContourValue>::const_iterator lend;
						  
						  lbegin = piter->ContourValues().begin();
						  lend   = piter->ContourValues().end();
						  
						  for(liter=lbegin ; liter!=lend; ++liter)
							{
							  // Skip to next contour if this one is outside
							  // the value range.
							  
							  if(valmin!=kFloatMissing && valmax!=kFloatMissing)
								{
								  if(liter->Value()!=kFloatMissing &&
									 valmax<liter->Value())
									continue;
								  if(liter->Value()!=kFloatMissing &&
									 valmin>liter->Value())
									continue;
								}
							  
							  NFmiContourTree tree(liter->Value(),kFloatMissing);
							  if(piter->DataLoLimit()!=kFloatMissing)
								tree.DataLoLimit(piter->DataLoLimit());
							  if(piter->DataHiLimit()!=kFloatMissing)
								tree.DataHiLimit(piter->DataHiLimit());
							  
							  NFmiColorTools::NFmiBlendRule rule = NFmiColorTools::BlendValue(liter->Rule());
							  tree.Contour(pts[qi],vals,interp,piter->ContourDepth());
							  NFmiPath path = tree.Path();
							  path.SimplifyLines(10);
							  path.Stroke(theImage,liter->Color(),rule);
							  
							  // cout << path << endl;
							  // cout << "Value = " << liter->LoLimit() << endl;
							  // cout << "<path style=\"fill=rgb("
							  // << NFmiColorTools::GetRed(citer->Color()) << ","
							  // << NFmiColorTools::GetGreen(citer->Color()) << ","
							  // << NFmiColorTools::GetBlue(citer->Color())
							  // << ")\" d=\""
							  // << path.SVG()
							  // << "\">"
							  // << endl;
							  
							  
							}
						  
						}
					  
					  
					  
					  // Bang the foreground
					  
					  if(theForeground != "")
						{
						  NFmiColorTools::NFmiBlendRule rule = NFmiColorTools::BlendValue(theForegroundRule);
						  
						  theImage.Composite(theForegroundImage,rule,kFmiAlignNorthWest,0,0,1);
						  
						}
					  
					  // Draw labels
					  
					  for(piter=pbegin; piter!=pend; ++piter)
						{
						  // Establish the parameter
						  
						  string name = piter->Param();
						  FmiParameterName param = FmiParameterName(NFmiEnumConverter().ToEnum(name));
						  
						  if(param==kFmiBadParameter)
							{
							  cerr << "Error: Unknown parameter " << name << endl;
							  return 1;
							}
						  
						  // Find the proper queryinfo to be used
						  // Note that qi will be used later on for
						  // getting the coordinate matrices
						  
						  ok = false;
						  for(qi=0; qi<theQueryStreams.size(); qi++)
							{
							  theQueryInfo = theQueryStreams[qi]->QueryInfoIter();
							  theQueryInfo->Param(param);
							  ok = theQueryInfo->IsParamUsable();
							  if(ok) break;
							}
						  
						  if(!ok)
							{
							  cerr << "Error: The parameter is not usable: " << name << endl;
							  exit(1);
							}
						  
						  // Draw label markers first
						  
						  if(!piter->LabelMarker().empty())
							{
							  // Establish that something is to be done
							  
							  if(piter->LabelPoints().empty() &&
								 !(piter->LabelDX()==0 || piter->LabelDX()==0))
								continue;
							  
							  // Establish the marker specs
							  
							  NFmiImage marker;
							  marker.Read(piter->LabelMarker());
							  
							  NFmiColorTools::NFmiBlendRule markerrule = NFmiColorTools::BlendValue(piter->LabelMarkerRule());
							  
							  float markeralpha = piter->LabelMarkerAlphaFactor();
							  
							  // Draw individual points
							  
							  if(!piter->LabelPoints().empty())
								{
								  list<pair<NFmiPoint,NFmiPoint> >::const_iterator iter;
								  for(iter=piter->LabelPoints().begin();
									  iter!=piter->LabelPoints().end();
									  ++iter)
									{
									  // The point in question
									  
									  NFmiPoint xy = theArea.ToXY(iter->first);
									  
									  theImage.Composite(marker,
														 markerrule,
														 kFmiAlignCenter,
														 static_cast<int>(xy.X()),
														 static_cast<int>(xy.Y()),
														 markeralpha);
									}
								}
							  
							  // Draw grid
							  
							  if(piter->LabelDX()!=0 && piter->LabelDY()!=0)
								{
								  for(unsigned int j=0; j<pts[qi].NY(); j+=piter->LabelDY())
									for(unsigned int i=0; i<pts[qi].NX(); i+=piter->LabelDX())
									  theImage.Composite(marker,
														 markerrule,
														 kFmiAlignCenter,
														 static_cast<int>(pts[qi][i][j].X()),
														 static_cast<int>(pts[qi][i][j].Y()),
														 markeralpha);
								}
							}
						  
						  // Label markers now drawn, only label texts remain
						  
						  // Quick exit from loop if no labels are
						  // desired for this parameter
						  
						  if(piter->LabelPoints().empty() &&
							 !(piter->LabelDX()!=0 && piter->LabelDY()!=0))
							continue;
						  
						  // Draw markers if so requested
						  
						  // Draw labels at specifing latlon points if requested
						  
						  if( (piter->LabelFormat() != "") && !piter->LabelPoints().empty())
							{
							  
							  // Create the font object to be used
							  
							  NFmiFontHershey font(piter->LabelFont());
							  
							  // Create the text object to be used
							  
							  NFmiText text("",
											font,
											piter->LabelSize(),
											0.0,	// x
											0.0,	// y
											AlignmentValue(piter->LabelAlignment()),
											piter->LabelAngle());
							  
							  
							  NFmiText caption(piter->LabelCaption(),
											   font,
											   piter->LabelSize(),
											   0.0,
											   0.0,
											   AlignmentValue(piter->LabelCaptionAlignment()),
											   piter->LabelAngle());
							  
							  // The rules
							  
							  NFmiColorTools::NFmiBlendRule fillrule
								= NFmiColorTools::BlendValue(piter->LabelFillRule());
							  
							  NFmiColorTools::NFmiBlendRule strokerule
								= NFmiColorTools::BlendValue(piter->LabelStrokeRule());
							  
							  list<pair<NFmiPoint,NFmiPoint> >::const_iterator iter;
							  
							  int pointnumber = 0;
							  for(iter=piter->LabelPoints().begin();
								  iter!=piter->LabelPoints().end();
								  ++iter)
								{
								  float value = piter->LabelValues()[pointnumber++];
								  
								  // Convert value to string
								  
								  string strvalue("-");
								  
								  if(value!=kFloatMissing)
									{
									  char tmp[20];
									  sprintf(tmp,piter->LabelFormat().c_str(),value);
									  strvalue = tmp;
									}
								  
								  // The point in question
								  
								  float x,y;
								  if(iter->second.X() == kFloatMissing)
									{
									  NFmiPoint xy = theArea.ToXY(iter->first);
									  x = xy.X();
									  y = xy.Y();
									}
								  else
									{
									  x = iter->second.X();
									  y = iter->second.Y();
									}
								  
								  // Set new text properties
								  
								  text.Text(strvalue);
								  text.X(x + piter->LabelOffsetX());
								  text.Y(y + piter->LabelOffsetY());
								  
								  // And render the text
								  
								  text.Fill(theImage,piter->LabelFillColor(),fillrule);
								  text.Stroke(theImage,piter->LabelStrokeColor(),strokerule);
								  
								  // Then the label caption
								  
								  if(!piter->LabelCaption().empty())
									{
									  caption.X(text.X() + piter->LabelCaptionDX());
									  caption.Y(text.Y() + piter->LabelCaptionDY());
									  caption.Fill(theImage,piter->LabelFillColor(),fillrule);
									  caption.Stroke(theImage,piter->LabelStrokeColor(),strokerule);
									}
								  
								}
							  
							  // Grid would be labeled here, but it's not implemented
							  // yet. I have to decide whether to smoothen as in
							  // contouring or whether to use the original data value
							  // at the grid point.
							  
							}
						}
					  
					  
					  // Draw wind arrows if so requested
					  
					  NFmiEnumConverter converter;
					  if(converter.ToString(theQueryInfo->Param().GetParamIdent()) == theDirectionParameter &&
						 (!theArrowPoints.empty() || (theWindArrowDX!=0 && theWindArrowDY!=0)) &&
						 (theArrowFile!=""))
						{
						  // Read the arrow definition
						  
						  ifstream arrow(theArrowFile.c_str());
						  if(!arrow)
							{
							  cerr << "Error: Could not open " << theArrowFile << endl;
							  exit(1);
							}
						  // Read in the entire file
						  
						  string pathstring;
						  StringReader(arrow,pathstring);
						  arrow.close();
						  
						  // Convert to a path
						  
						  NFmiPath arrowpath;
						  arrowpath.Add(pathstring);
						  
						  // Handle all given coordinates
						  
						  list<NFmiPoint>::const_iterator iter;
						  
						  for(iter=theArrowPoints.begin();
							  iter!=theArrowPoints.end();
							  ++iter)
							{
							  float dir = theQueryInfo->InterpolatedValue(*iter);
							  if(dir==kFloatMissing)	// ignore missing
								continue;
							  
							  float speed = -1;
							  
							  if(theQueryInfo->Param(FmiParameterName(converter.ToEnum(theSpeedParameter))));
								speed = theQueryInfo->InterpolatedValue(*iter);
							  theQueryInfo->Param(FmiParameterName(converter.ToEnum(theDirectionParameter)));
							  
							  // The start point
							  
							  NFmiPoint xy0 = theArea.ToXY(*iter);
							  
							  // Direction calculations
							  
							  const float pi = 3.141592658979323;
							  const float length = 0.1;	// degrees
							  
							  float x1 = iter->X()+sin(dir*pi/180)*length;
							  float y1 = iter->Y()+cos(dir*pi/180)*length;
							  
							  NFmiPoint xy1 = theArea.ToXY(NFmiPoint(x1,y1));
							  
							  // Calculate the actual angle
							  
							  float alpha = atan2(xy1.X()-xy0.X(),
												  xy1.Y()-xy0.Y());
							  
							  // Create a new path
							  
							  NFmiPath thispath;
							  thispath.Add(arrowpath);
							  if(speed>0 && speed!=kFloatMissing)
								thispath.Scale(theWindArrowScaleA*log10(theWindArrowScaleB*speed+1)+theWindArrowScaleC);
							  thispath.Scale(theArrowScale);
							  thispath.Rotate(alpha*180/pi);
							  thispath.Translate(xy0.X(),xy0.Y());
							  
							  // And render it
							  
							  thispath.Fill(theImage,
											ToColor(theArrowFillColor),
											NFmiColorTools::BlendValue(theArrowFillRule));
							  thispath.Stroke(theImage,
											  ToColor(theArrowStrokeColor),
											  NFmiColorTools::BlendValue(theArrowStrokeRule));
							}
						  
						  // Draw the full grid if so desired
						  
						  if(theWindArrowDX!=0 && theWindArrowDY!=0)
							{
							  NFmiDataMatrix<NFmiPoint> latlons;
							  theQueryInfo->Locations(latlons);

							  NFmiDataMatrix<float> speedvalues(vals.NX(),vals.NY(),-1);
							  if(theQueryInfo->Param(kFmiWindSpeedMS))
								theQueryInfo->Values(speedvalues);
							  theQueryInfo->Param(kFmiWindDirection);
							  
							  for(unsigned int j=0; j<pts[qi].NY(); j+=theWindArrowDY)
								for(unsigned int i=0; i<pts[qi].NX(); i+=theWindArrowDX)
								  {
									float dir = vals[i][j];
									if(dir==kFloatMissing)	// ignore missing
									  continue;
									
									float speed = speedvalues[i][j];

									// The start point
									
									NFmiPoint xy0 = theArea.ToXY(latlons[i][j]);
									
									// Direction calculations
									
									const float pi = 3.141592658979323;
									const float length = 0.1;	// degrees
									
									float x0 = latlons[i][j].X();
									float y0 = latlons[i][j].Y();
									
									float x1 = x0+sin(dir*pi/180)*length;
									float y1 = y0+cos(dir*pi/180)*length;
									
									NFmiPoint xy1 = theArea.ToXY(NFmiPoint(x1,y1));
									
									// Calculate the actual angle
									
									float alpha = atan2(xy1.X()-xy0.X(),
														xy1.Y()-xy0.Y());
									
									// Create a new path
									
									NFmiPath thispath;
									thispath.Add(arrowpath);
									if(speed>0 && speed != kFloatMissing)
									  thispath.Scale(theWindArrowScaleA*log10(theWindArrowScaleB*speed+1)+theWindArrowScaleC);
									thispath.Scale(theArrowScale);
									thispath.Rotate(alpha*180/pi);
									thispath.Translate(xy0.X(),xy0.Y());
									
									// And render it
									
									thispath.Fill(theImage,
												  ToColor(theArrowFillColor),
												  NFmiColorTools::BlendValue(theArrowFillRule));
									thispath.Stroke(theImage,
													ToColor(theArrowStrokeColor),
													NFmiColorTools::BlendValue(theArrowStrokeRule));
								  }
							}
						}
					  
					  // Bang the combine image (legend, logo, whatever)
					  
					  if(theCombine != "")
						{
						  NFmiColorTools::NFmiBlendRule rule = NFmiColorTools::BlendValue(theCombineRule);
						  
						  theImage.Composite(theCombineImage,rule,kFmiAlignNorthWest,theCombineX,theCombineY,theCombineFactor);
						  
						}

					  // Finally, draw a time stamp on the image if so
					  // requested
					  
					  string thestamp = "";
					  
					  {
						int obsyy = t.GetYear();
						int obsmm = t.GetMonth();
						int obsdd = t.GetDay();
						int obshh = t.GetHour();
						int obsmi = t.GetMin();
						
						// Interpretation: The age of the forecast is the age
						// of the oldest forecast
						
						NFmiTime tfor;
						for(qi=0; qi<theQueryStreams.size(); qi++)
						  {
							theQueryInfo = theQueryStreams[qi]->QueryInfoIter();
							NFmiTime futctime = theQueryInfo->OriginTime(); 
							NFmiTime tlocal = NFmiMetTime(futctime,1).LocalTime();
							if(qi==0 || tlocal.IsLessThan(tfor))
							  tfor = tlocal;
						  }
						
						int foryy = tfor.GetYear();
						int formm = tfor.GetMonth();
						int fordd = tfor.GetDay();
						int forhh = tfor.GetHour();
						int formi = tfor.GetMin();
						
						char buffer[100];
						
						if(theTimeStampImage == "obs")
						  {
							// hh:mi dd.mm.yyyy
							sprintf(buffer,"%02d:%02d %02d.%02d.%04d",
									obshh,obsmi,obsdd,obsmm,obsyy);
							thestamp = buffer;
						  }
						else if(theTimeStampImage == "for")
						  {
							// hh:mi dd.mm.yyyy
							sprintf(buffer,"%02d:%02d %02d.%02d.%04d",
									forhh,formi,fordd,formm,foryy);
							thestamp = buffer;
						  }
						else if(theTimeStampImage == "forobs")
						  {
							// hh:mi dd.mm.yyyy +hh
							long diff = t.DifferenceInMinutes(tfor);
							if(diff%60==0 && theTimeStep%60==0)
							  sprintf(buffer,"%02d.%02d.%04d %02d:%02d %s%ldh",
									  fordd,formm,foryy,forhh,formi,
									  (diff<0 ? "" : "+"), diff/60);
							else
							  sprintf(buffer,"%02d.%02d.%04d %02d:%02d %s%ldm",
									  fordd,formm,foryy,forhh,formi,
									  (diff<0 ? "" : "+"), diff);
							thestamp = buffer;
						  }
					  }
					  
					  if(!thestamp.empty())
						{
						  NFmiFontHershey font("TimesRoman-Bold");
						  
						  int x = theTimeStampImageX;
						  int y = theTimeStampImageY;
						  
						  if(x<0) x+= theImage.Width();
						  if(y<0) y+= theImage.Height();
						  
						  NFmiText text(thestamp,font,14,x,y,kFmiAlignNorthWest,0.0);
						  
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
					  
					  // Save
					  
					  if(verbose)
						cout << "Writing " << filename << endl;
					  if(theFormat=="png")
						theImage.WritePng(filename);
					  else if(theFormat=="jpg" || theFormat=="jpeg")
						theImage.WriteJpeg(filename);
					  else if(theFormat=="gif")
						theImage.WriteGif(filename);
					  else
						{
						  cerr << "Error: Image format " << theFormat << " is not supported" << endl;
						  return 1;
						}
					}
				}
			  
			  else
				{
				  cerr << "Error: draw " << command << " not implemented\n";
				  return 1;
				}
			}
		  else
			{
			  cerr << "Error: Unknown command " << command << endl;
			  return 1;
			}
		}
    }
}

// ======================================================================

