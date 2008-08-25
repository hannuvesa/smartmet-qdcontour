all: test

#---
# Tests used to be run by a Perl script, but makefile is actually better
# (stops at failures etc.)
#
perltest:
	LD_LIBRARY_PATH=/usr/local/lib \
	  ./test_qdcontour

clean:
	rm -f *~ */*~ results/*.png

#---
PROGRAM=../qdcontour
IMAGE_COMPARE=../tools/imagecompare

koe:
	-@$(MAKE) _check TEST=koe1
	-@$(MAKE) _check TEST=koe2
	-@$(MAKE) _check TEST=koe3
	-@$(MAKE) _check TEST=koe4
	-@$(MAKE) _check TEST=koe5
	-@$(MAKE) _check TEST=koe6
	-@$(MAKE) _check TEST=koe7
	-@$(MAKE) _check TEST=koe8


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
koe4 koe5 \
:
	-@$(MAKE) _check TEST=$@

meta_wind_chill_pdf \
:
	-@$(MAKE) _check_pdf TEST=$(@:_pdf=)

# All tests:   (TBD: some are disabled)
#
test new_test:
	-@$(MAKE) _check TEST=timestampimage
	-@$(MAKE) _check TEST=timestampimage2
	-@$(MAKE) _check TEST=timestampimageutc
	-@$(MAKE) _check TEST=shape_fill
	-@$(MAKE) _check TEST=shape_stroke
	-@$(MAKE) _check TEST=shape_mark
	-@$(MAKE) _check TEST=shape_mark_alpha_factor
	-@$(MAKE) _check TEST=shape_combined
	-@$(MAKE) _check TEST=automatic_width
	-@$(MAKE) _check TEST=automatic_height
	-@$(MAKE) _check TEST=center
	-@$(MAKE) _check TEST=scale
	-@$(MAKE) _check TEST=trivial
	-@$(MAKE) _check TEST=contourline
	-@$(MAKE) _check TEST=contourlinewidth
	-@$(MAKE) _check TEST=contourfill
	-@$(MAKE) _check TEST=contourpattern
	-@$(MAKE) _check TEST=contoursymbol
	-@$(MAKE) _check TEST=contourfont
	-@$(MAKE) _check TEST=contourfont2
	-@$(MAKE) _check TEST=contourfont_grid
	-@$(MAKE) _check TEST=contourlabels
	-@$(MAKE) _check TEST=contourlabeltexts
	-@$(MAKE) _check TEST=windarrow_grid
	-@$(MAKE) _check TEST=windarrow_grid_step
	-@$(MAKE) _check TEST=windarrow_grid_masked
	-@$(MAKE) _check TEST=windarrow_pixelgrid
	-@$(MAKE) _check TEST=windarrow_pixelgrid_masked
	-@$(MAKE) _check TEST=windarrowscale
	-@$(MAKE) _check TEST=directionparam
	-@$(MAKE) _check TEST=speedparam
	-@$(MAKE) _check TEST=labels_points
	-@$(MAKE) _check TEST=labels_grid
	-@$(MAKE) _check TEST=labels_grid_masked
	-@$(MAKE) _check TEST=labels_grid_step
	-@$(MAKE) _check TEST=labelformat
	-@$(MAKE) _check TEST=labels_integers
	-@$(MAKE) _check TEST=labels_and_arrows
	-@$(MAKE) _check TEST=arrows_and_labels
	-@$(MAKE) _check TEST=labels_pixelgrid
	-@$(MAKE) _check TEST=labels_pixelgrid_masked
	-@$(MAKE) _check TEST=meteorological_arrows
	-@$(MAKE) _check TEST=meteorological_arrows_and_labels
	-@$(MAKE) _check TEST=meta_elevation_angle
	-@$(MAKE) _check TEST=meta_wind_chill
	-@$(MAKE) _check TEST=meta_t2m_advection
	-@$(MAKE) _check TEST=meta_thermal_front
	-@$(MAKE) _check TEST=missing_values
	-@$(MAKE) _check TEST=expanddata
	-@$(MAKE) _check TEST=fahrenheit
	-@$(MAKE) _check TEST=despeckle_none
	-@$(MAKE) _check TEST=despeckle_median1
	-@$(MAKE) _check TEST=despeckle_median2
	-@$(MAKE) _check TEST=despeckle_median1_iter2
	-@$(MAKE) _check TEST=despeckle_median1_upper
	-@$(MAKE) _check TEST=despeckle_median1_lower
	-@$(MAKE) _check TEST=despeckle_median1_lower_range
	
# 
# Note: The current implementation of '_check' does not stop if there
#       are differing pixels. This is intentional in the transition to Cairo.
#           -- AKa 14-Aug-2008
#
PNG=$(notdir $(wildcard results_ok/$(TEST)_*.png)$(wildcard results_ok/$(TEST).png))
_check:
	@echo
	@echo "*** $(TEST) ***"
	@echo
	@# Need to clear all previous output; 'qdcontour' will otherwise think they
	@# are up to date (even with '-f', really??).
	@#
	#-@rm results/$(PNG)
	$(PROGRAM) -f conf/$(TEST).conf
	$(IMAGE_COMPARE) results_ok/$(PNG) results/$(PNG)

_check_pdf:
	@echo
	@echo "*** $(TEST) ***"
	@echo
	$(PROGRAM) -c "format pdf" -f conf/$(TEST).conf

echo:
	@echo $(PNG)
