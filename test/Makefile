all: test

#---
# perltest:
# 
# Old test using Perl script and self-made 'imagecompare'
#
perltest:
	./perltest.pl

clean:
	rm -f *~ */*~ results/* results_diff/*

#---
# New tests, done solely by Make and Imagemagick
#
PROGRAM=../qdcontour

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
	-@$(MAKE) --quiet $(_CHECK) TEST=timestampimage
	-@$(MAKE) --quiet $(_CHECK) TEST=timestampimage2
	-@$(MAKE) --quiet $(_CHECK) TEST=timestampimageutc
	-@$(MAKE) --quiet $(_CHECK) TEST=shape_fill
	-@$(MAKE) --quiet $(_CHECK) TEST=shape_stroke
	-@$(MAKE) --quiet $(_CHECK) TEST=shape_mark
	-@$(MAKE) --quiet $(_CHECK) TEST=shape_mark_alpha_factor
	-@$(MAKE) --quiet $(_CHECK) TEST=shape_combined
	-@$(MAKE) --quiet $(_CHECK) TEST=automatic_width
	-@$(MAKE) --quiet $(_CHECK) TEST=automatic_height
	-@$(MAKE) --quiet $(_CHECK) TEST=center
	-@$(MAKE) --quiet $(_CHECK) TEST=scale
	-@$(MAKE) --quiet $(_CHECK) TEST=trivial
	-@$(MAKE) --quiet $(_CHECK) TEST=contourline
	-@$(MAKE) --quiet $(_CHECK) TEST=contourlinewidth
	-@$(MAKE) --quiet $(_CHECK) TEST=contourfill
	-@$(MAKE) --quiet $(_CHECK) TEST=contourpattern
	-@$(MAKE) --quiet $(_CHECK) TEST=contoursymbol
	-@$(MAKE) --quiet $(_CHECK) TEST=contourfont
	-@$(MAKE) --quiet $(_CHECK) TEST=contourfont2
	-@$(MAKE) --quiet $(_CHECK) TEST=contourfont_grid
	-@$(MAKE) --quiet $(_CHECK) TEST=contourlabels
	-@$(MAKE) --quiet $(_CHECK) TEST=contourlabeltexts
	-@$(MAKE) --quiet $(_CHECK) TEST=windarrow_grid
	-@$(MAKE) --quiet $(_CHECK) TEST=windarrow_grid_step
	-@$(MAKE) --quiet $(_CHECK) TEST=windarrow_grid_masked
	-@$(MAKE) --quiet $(_CHECK) TEST=windarrow_pixelgrid
	-@$(MAKE) --quiet $(_CHECK) TEST=windarrow_pixelgrid_masked
	-@$(MAKE) --quiet $(_CHECK) TEST=windarrowscale
	-@$(MAKE) --quiet $(_CHECK) TEST=directionparam
	-@$(MAKE) --quiet $(_CHECK) TEST=speedparam
	-@$(MAKE) --quiet $(_CHECK) TEST=labels_points
	-@$(MAKE) --quiet $(_CHECK) TEST=labels_grid
	-@$(MAKE) --quiet $(_CHECK) TEST=labels_grid_masked
	-@$(MAKE) --quiet $(_CHECK) TEST=labels_grid_step
	-@$(MAKE) --quiet $(_CHECK) TEST=labelformat
	-@$(MAKE) --quiet $(_CHECK) TEST=labels_integers
	-@$(MAKE) --quiet $(_CHECK) TEST=labels_and_arrows
	-@$(MAKE) --quiet $(_CHECK) TEST=arrows_and_labels
	-@$(MAKE) --quiet $(_CHECK) TEST=labels_pixelgrid
	-@$(MAKE) --quiet $(_CHECK) TEST=labels_pixelgrid_masked
	-@$(MAKE) --quiet $(_CHECK) TEST=meteorological_arrows
	-@$(MAKE) --quiet $(_CHECK) TEST=meteorological_arrows_and_labels
	-@$(MAKE) --quiet $(_CHECK) TEST=meta_elevation_angle
	-@$(MAKE) --quiet $(_CHECK) TEST=meta_wind_chill
	-@$(MAKE) --quiet $(_CHECK) TEST=meta_t2m_advection
	-@$(MAKE) --quiet $(_CHECK) TEST=meta_thermal_front
	-@$(MAKE) --quiet $(_CHECK) TEST=missing_values
	-@$(MAKE) --quiet $(_CHECK) TEST=expanddata
	-@$(MAKE) --quiet $(_CHECK) TEST=fahrenheit
	-@$(MAKE) --quiet $(_CHECK) TEST=despeckle_none
	-@$(MAKE) --quiet $(_CHECK) TEST=despeckle_median1
	-@$(MAKE) --quiet $(_CHECK) TEST=despeckle_median2
	-@$(MAKE) --quiet $(_CHECK) TEST=despeckle_median1_iter2
	-@$(MAKE) --quiet $(_CHECK) TEST=despeckle_median1_upper
	-@$(MAKE) --quiet $(_CHECK) TEST=despeckle_median1_lower
	-@$(MAKE) --quiet $(_CHECK) TEST=despeckle_median1_lower_range

# ImageMagick usage was throw to a separate shell script. It should return 0
# for approvable differences, and non-zero for once that could stop the make
# (currently, all tests are run through anyhow).

PNG=$(notdir $(wildcard results_ok/$(TEST)_*.png)$(wildcard results_ok/$(TEST).png))

_check:
	@echo -n "$(TEST)..........................................." | sed -e 's/^\(.\{40\}\).*/\1/g'
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
