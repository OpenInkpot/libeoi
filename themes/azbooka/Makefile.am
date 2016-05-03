THEMES=$(wildcard $(srcdir)/*.edc)
COMPILED_THEMES=$(patsubst %.edc,%.edj,$(THEMES)) choicebox_vk.edj

themedir=$(datadir)/eoi/themes/azbooka
theme_DATA=$(COMPILED_THEMES)

%.edj: %.edc
	edje_cc -id $(<D)/.. -id $(<D) $< $@

choicebox_vk.edj: choicebox.edc
	edje_cc -DMIN_HEIGHT_VALUE=53 -id $(<D)/.. -id $(<D) $< $@

CLEANFILES=$(COMPILED_THEMES)
