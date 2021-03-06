#!/usr/bin/perl

$imagecompare = "imagecompare";

# Use '-c "format pdf"' to force all output in PDF (default: PNG)
#
#$program = "../qdcontour -c \"format pdf\"";
$program = "../qdcontour";

%usednames = ();

# A trivial test

DoTest("trivial contour",
       "trivial.conf",
       "trivial_200210140600_200210140937.png");

DoTest("labels with truetype",
       "labels_grid_ttf.conf",
       "labels_grid_ttf_200210140600_200210140937.png");

# Test contourline

DoTest("contourline",
       "contourline.conf",
       "contourline_200210140600_200210140937.png");

# Test contourlinewidth

DoTest("contourlinewidth",
       "contourlinewidth.conf",
       "contourlinewidth_200210140600_200210140937.png");

# Test contourfill

DoTest("contourfill",
       "contourfill.conf",
       "contourfill_200210140600_200210140937.png");

# Test contourpattern

DoTest("contourpattern",
       "contourpattern.conf",
       "contourpattern_200210140600_200210140937.png");

# Test contoursymbol

DoTest("contoursymbol1",
       "contoursymbol1.conf",
       "contoursymbol1_200210140600_200210140937.png");

DoTest("contoursymbol2",
       "contoursymbol2.conf",
       "contoursymbol2_200210140600_200210140937.png");

# Test contourfont
#
# AKa 22-Aug-2008: These tests disabled since non-UTF-8 codes cause exception
#                  and delay speed tests.
#
#DoTest("contourfont",
#       "contourfont.conf",
#       "contourfont_200210140600_200210140937.png");
#
#DoTest("contourfont2",
#       "contourfont2.conf",
#       "contourfont2_200210140600_200210140937.png");
#
#DoTest("contourfont_grid",
#       "contourfont_grid.conf",
#       "contourfont_grid_200210140600_200210140937.png");

# Timestamping

DoTest("timestampimage",
       "timestampimage.conf",
       "timestampimage_200210140600_200210140937.png");

DoTest("timestampimage2",
       "timestampimage2.conf",
       "timestampimage2_200210140600_200210140937.png");

DoTest("timestampimageutc",
       "timestampimageutc.conf",
       "timestampimageutc_200210140700_200210140937.png");


# Shape filling

DoTest("shape filling",
       "shape_fill.conf",
       "shape_fill.png");

# Shape stroking

DoTest("shape stroking",
       "shape_stroke.conf",
       "shape_stroke.png");

# Shape marking

DoTest("shape marking",
       "shape_mark.conf",
       "shape_mark.png");

# Shape marking with alpha factor

DoTest("shape marking with alpha factor",
       "shape_mark_alpha_factor.conf",
       "shape_mark_alpha_factor.png");

# Combination of above tests

DoTest("shape rendering, all combined",
       "shape_combined.conf",
       "shape_combined.png");

# Automatic size calculation

DoTest("automatic width calculation",
       "automatic_width.conf",
       "automatic_width.png");

DoTest("automatic height calculation",
       "automatic_height.conf",
       "automatic_height.png");

# Center and scale

DoTest("center",
       "center.conf",
       "center.png");

DoTest("scale",
       "scale.conf",
       "scale.png");

# Test contourlabels

DoTest("contourlabels",
       "contourlabels.conf",
       "contourlabels_200210140600_200210140937.png");

DoTest("contourlabeltexts",
       "contourlabeltexts.conf",
       "contourlabeltexts_200210140600_200210140937.png");

# Test windarrows

DoTest("windarrows at points",
       "windarrow_points.conf",
       "windarrow_points200210140600_200210140937.png");

DoTest("windarrows in a grid",
       "windarrow_grid_normal.conf",
       "windarrow_grid_normal_200210140600_200210140937.png");

DoTest("windarrows in a grid with fractional step",
       "windarrow_grid_step.conf",
       "windarrow_grid_step_200210140600_200210140937.png");

DoTest("windarrows in a grid masked",
       "windarrow_grid_masked.conf",
       "windarrow_grid_masked_200210140600_200210140937.png");

DoTest("windarrows in a pixel grid",
       "windarrow_pixelgrid_normal.conf",
       "windarrow_pixelgrid_normal_200210140600_200210140937.png");

DoTest("windarrows in a pixelgrid masked",
       "windarrow_pixelgrid_masked.conf",
       "windarrow_pixelgrid_masked_200210140600_200210140937.png");

DoTest("windarrowscale",
       "windarrowscale.conf",
       "windarrowscale_200210140600_200210140937.png");

DoTest("directionparam",
       "directionparam.conf",
       "directionparam_200210150600_200210151318.png");

DoTest("speedparam",
       "speedparam.conf",
       "speedparam_200210150600_200210151318.png");

# Test labels

DoTest("labels at points",
       "labels_points.conf",
       "labels_points_200210140600_200210140937.png");

DoTest("labels at gridpoints",
       "labels_grid_normal.conf",
       "labels_grid_normal_200210140600_200210140937.png");

DoTest("labels at masked gridpoints",
       "labels_grid_masked.conf",
       "labels_grid_masked_200210140600_200210140937.png");

DoTest("labels at fractional gridpoints",
       "labels_grid_step.conf",
       "labels_grid_step_200210140600_200210140937.png");

DoTest("labels with 2 decimals",
       "labelformat.conf",
       "labelformat_200210140600_200210140937.png");

DoTest("labels rounded to integers",
       "labels_integers.conf",
       "labels_integers_200210140600_200210140937.png");


# Labels must be on top of arrows

DoTest("labels on top of arrows",
       "labels_and_arrows.conf",
       "labels_and_arrows_200210140600_200210140937.png");

DoTest("labels on top of arrows regardless of definition order",
       "arrows_and_labels.conf",
       "arrows_and_labels_200210140600_200210140937.png");

# Test pixellabels

DoTest("labels at pixelgridpoints",
       "labels_pixelgrid_normal.conf",
       "labels_pixelgrid_normal_200210140600_200210140937.png");

DoTest("labels at masked pixelgridpoints",
       "labels_pixelgrid_masked.conf",
       "labels_pixelgrid_masked_200210140600_200210140937.png");

# Meteorological arrows

DoTest("meteorological arrows",
       "meteorological_arrows_normal.conf",
       "meteorological_arrows_normal_200210140600_200210140937.png");

DoTest("meteorological arrows with labels",
       "meteorological_arrows_and_labels.conf",
       "meteorological_arrows_and_labels_200210140600_200210140937.png");

# MetaElevationAngle

DoTest("MetaElevationAngle",
       "meta_elevation_angle.conf",
       "meta_elevation_angle_200210141500_200210140937.png");

# MetaWindChill

DoTest("MetaWindChill",
       "meta_wind_chill.conf",
       "meta_wind_chill_200210141500_200210140937.png");

# MetaT2mAdvection

DoTest("MetaT2mAdvection",
       "meta_t2m_advection.conf",
       "meta_t2m_advection_200210140600_200210140937.png");

# MetaThermalFront

DoTest("MetaThermalFront",
       "meta_thermal_front.conf",
       "meta_thermal_front_200210140600_200210140937.png");

# Missing values

DoTest("rendering missing values",
       "missing_values.conf",
       "missing_values_200505271800_200506020900.png");

DoTest("expanding data",
       "expanddata.conf",
       "expanddata_200505271800_200506020900.png");

# Fahrenheit conversions

DoTest("fahrenheit conversion",
       "fahrenheit.conf",
       "fahrenheit_200210140600_200210140937.png");

# Test despeckling

DoTest("despeckle_none",
       "despeckle_none.conf",
       "despeckle_none_200605090000.png");

DoTest("despeckle_median1",
       "despeckle_median1_normal.conf",
       "despeckle_median1_normal_200605090000.png");

DoTest("despeckle_median2",
       "despeckle_median2_normal.conf",
       "despeckle_median2_normal_200605090000.png");

DoTest("despeckle_median1_iter2",
       "despeckle_median1_iter2.conf",
       "despeckle_median1_iter2_200605090000.png");

DoTest("despeckle_median1_upper",
       "despeckle_median1_upper.conf",
       "despeckle_median1_upper_200605090000.png");

DoTest("despeckle_median1_lower",
       "despeckle_median1_lower_normal.conf",
       "despeckle_median1_lower_normal_200605090000.png");

DoTest("despeckle_median1_lower_range",
       "despeckle_median1_lower_range.conf",
       "despeckle_median1_lower_range_200605090000.png");


print "Done\n";


# ----------------------------------------------------------------------
# Run a single test
# ----------------------------------------------------------------------

sub DoTest
{
    my($text) = shift(@_);
    my($conf) = shift(@_);
    my(@outputfiles) = @_;

    foreach $outputfile(@outputfiles)
    {
	if(exists($usednames{$outputfile}))
	{
	    print "Virhe regressiotesteissä: $outputfile käytössä useamman kerran\n";
	    exit(1);
	}
	$usednames{$outputfile} = 1;
    }

    # Ajetaan käsky

    $cmd = "$program -f conf/$conf";
    $output = `$cmd`;

    # Vertaa tuloksia

    print padname($text);

    $failed = 0;
    $extratext = "";
    foreach $outputfile(@outputfiles)
    {
	$expected = "results_ok/$outputfile";
	$actual = "results/$outputfile";

	$diff = imagedifference($expected,$actual);
	if($diff == 0)
	{
	    # If compared ok, remove the actual output
	    unlink($actual);
	}
	elsif($diff == -1)
	{
	    $failed++;
	    $extratext .= "(files $expected and $actual differ in size)\n";
	}
	else
	{
	    $failed++;
	    $extratext .= "(files $expected and $actual differ in $diff pixels)\n";
	}
    
	if($failed==0)
	{
	    print "ok\n";
	}
	else
	{
	    print "FAILED\n$extratext";
	}
    }
}

# ----------------------------------------------------------------------
# Pad the given string to 70 characters with dots
# ----------------------------------------------------------------------

sub padname
{
    my($str) = @_[0];

    while(length($str) < 70)
    {
	$str .= ".";
    }
    return $str;
}

# ----------------------------------------------------------------------
# Compare two image files. The returned number is zero for
# equal images, -1 for images of different size. Otherwise the
# images are of equal size, and N is returned, indicating the
# number of different pixels.
# ----------------------------------------------------------------------

sub imagedifference
{
    my($file1,$file2) = @_;

    # print "$imagecompare $file1 $file2\n";

    # Files must exits

    if(!(-e $file1) || !(-e $file2))
    { return -1; }

    my($result) = `$imagecompare $file1 $file2`;
    chomp($result);
    my(@out) = split(" ",$result);

    if(scalar(@out) == 1 && $result eq "equal")
    { return 0; }

    if(scalar(@out) == 2 && $result eq "different size")
    { return -1; }

    if(scalar(@out) == 3 && $out[0] eq "different" && $out[1] eq "pixels:") {
        return $out[2];
    }

    return -1;
}

# ----------------------------------------------------------------------
