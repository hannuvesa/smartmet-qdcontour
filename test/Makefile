all: test

#---
# perltest:
# 
# Old test using Perl script and self-made 'imagecompare'
#
perltest:
	LD_LIBRARY_PATH=/usr/local/lib \
	  ./perltest.pl

clean:
	rm -f *~ */*~ results/* results_diff/*

#---
# New tests, done solely by Make and Imagemagick
#
PROGRAM=LD_LIBRARY_PATH=/usr/local/lib ../qdcontour

_CHECK=_check

# Misc tests that have issues:
#
labels_grid_masked missing_values shape_combined shape_fill shape_mark shape_mark_alpha_factor shape_stroke \
contourpattern contoursymbol contoursymbol2 \
contourfont contourfont2 contourfont_grid \
contourlabeltexts contourlabels \
center automatic_width \
despeckle_none \
despeckle_median1 \
despeckle_median2 \
despeckle_median1_iter2 \
despeckle_median1_upper \
despeckle_median1_lower \
despeckle_median1_lower_range \
windarrow_grid \
windarrow_grid_step \
windarrow_grid_masked \
windarrow_pixelgrid \
windarrow_pixelgrid_masked \
windarrowscale \
meta_elevation_angle \
meta_wind_chill \
meta_t2m_advection \
meta_thermal_front \
arrows_and_labels \
:
	-@$(MAKE) _check TEST=$@

meta_wind_chill_pdf \
:
	-@$(MAKE) _check_pdf TEST=$(@:_pdf=)

test_pdf:
	-@$(MAKE) test _CHECK=_check_pdf

test:
	-@$(MAKE) $(_CHECK) TEST=timestampimage
	-@$(MAKE) $(_CHECK) TEST=timestampimage2
	-@$(MAKE) $(_CHECK) TEST=timestampimageutc
	-@$(MAKE) $(_CHECK) TEST=shape_fill
	-@$(MAKE) $(_CHECK) TEST=shape_stroke
	-@$(MAKE) $(_CHECK) TEST=shape_mark
	-@$(MAKE) $(_CHECK) TEST=shape_mark_alpha_factor
	-@$(MAKE) $(_CHECK) TEST=shape_combined
	-@$(MAKE) $(_CHECK) TEST=automatic_width
	-@$(MAKE) $(_CHECK) TEST=automatic_height
	-@$(MAKE) $(_CHECK) TEST=center
	-@$(MAKE) $(_CHECK) TEST=scale
	-@$(MAKE) $(_CHECK) TEST=trivial
	-@$(MAKE) $(_CHECK) TEST=contourline
	-@$(MAKE) $(_CHECK) TEST=contourlinewidth
	-@$(MAKE) $(_CHECK) TEST=contourfill
	-@$(MAKE) $(_CHECK) TEST=contourpattern
	-@$(MAKE) $(_CHECK) TEST=contoursymbol
	-@$(MAKE) $(_CHECK) TEST=contourfont
	-@$(MAKE) $(_CHECK) TEST=contourfont2
	-@$(MAKE) $(_CHECK) TEST=contourfont_grid
	-@$(MAKE) $(_CHECK) TEST=contourlabels
	-@$(MAKE) $(_CHECK) TEST=contourlabeltexts
	-@$(MAKE) $(_CHECK) TEST=windarrow_grid
	-@$(MAKE) $(_CHECK) TEST=windarrow_grid_step
	-@$(MAKE) $(_CHECK) TEST=windarrow_grid_masked
	-@$(MAKE) $(_CHECK) TEST=windarrow_pixelgrid
	-@$(MAKE) $(_CHECK) TEST=windarrow_pixelgrid_masked
	-@$(MAKE) $(_CHECK) TEST=windarrowscale
	-@$(MAKE) $(_CHECK) TEST=directionparam
	-@$(MAKE) $(_CHECK) TEST=speedparam
	-@$(MAKE) $(_CHECK) TEST=labels_points
	-@$(MAKE) $(_CHECK) TEST=labels_grid
	-@$(MAKE) $(_CHECK) TEST=labels_grid_masked
	-@$(MAKE) $(_CHECK) TEST=labels_grid_step
	-@$(MAKE) $(_CHECK) TEST=labelformat
	-@$(MAKE) $(_CHECK) TEST=labels_integers
	-@$(MAKE) $(_CHECK) TEST=labels_and_arrows
	-@$(MAKE) $(_CHECK) TEST=arrows_and_labels
	-@$(MAKE) $(_CHECK) TEST=labels_pixelgrid
	-@$(MAKE) $(_CHECK) TEST=labels_pixelgrid_masked
	-@$(MAKE) $(_CHECK) TEST=meteorological_arrows
	-@$(MAKE) $(_CHECK) TEST=meteorological_arrows_and_labels
	-@$(MAKE) $(_CHECK) TEST=meta_elevation_angle
	-@$(MAKE) $(_CHECK) TEST=meta_wind_chill
	-@$(MAKE) $(_CHECK) TEST=meta_t2m_advection
	-@$(MAKE) $(_CHECK) TEST=meta_thermal_front
	-@$(MAKE) $(_CHECK) TEST=missing_values
	-@$(MAKE) $(_CHECK) TEST=expanddata
	-@$(MAKE) $(_CHECK) TEST=fahrenheit
	-@$(MAKE) $(_CHECK) TEST=despeckle_none
	-@$(MAKE) $(_CHECK) TEST=despeckle_median1
	-@$(MAKE) $(_CHECK) TEST=despeckle_median2
	-@$(MAKE) $(_CHECK) TEST=despeckle_median1_iter2
	-@$(MAKE) $(_CHECK) TEST=despeckle_median1_upper
	-@$(MAKE) $(_CHECK) TEST=despeckle_median1_lower
	-@$(MAKE) $(_CHECK) TEST=despeckle_median1_lower_range
	
# 
# ImageMagick usage was throw to a separate shell script. It should return 0
# for approvable differences, and non-zero for once that could stop the make
# (currently, all tests are run through anyhow).
#
PNG=$(notdir $(wildcard results_ok/$(TEST)_*.png)$(wildcard results_ok/$(TEST).png))
_check:
	@echo
	@echo "*** $(TEST) ***"
	@echo
	@# Need to clear all previous output; 'qdcontour' will otherwise think they
	@# are up to date (even with '-f', really??).
	@#
	@#-@rm results/$(PNG)
	@-mkdir -p results_diff
	$(PROGRAM) -f conf/$(TEST).conf
	-./pngdiff.sh results_ok/$(PNG) results/$(PNG) results_diff/$(PNG)

_check_pdf:
	@echo
	@echo "*** $(TEST) ***"
	@echo
	$(PROGRAM) -f -c "format pdf" conf/$(TEST).conf

echo:
	@echo $(PNG)
