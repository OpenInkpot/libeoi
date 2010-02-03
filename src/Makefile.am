lib_LTLIBRARIES = libeoi.la
libeoi_la_SOURCES = libeoi.c eoi_textbox.c eoi_help.c eoi_subst.c \
		eoi_battery.c eoi_clock.c eoi_resize.c
include_HEADERS = libeoi.h libeoi_textbox.h libeoi_help.h libeoi_battery.h \
		libeoi_clock.h

noinst_PROGRAMS = eoi_test

AM_CFLAGS = -Wall -DTHEME_DIR='"$(datadir)/eoi"' -DSYSCONFDIR='"$(sysconfdir)"'

INCLUDES = @EFL_CFLAGS@ @LIBKEYS_CFLAGS@ @LIBCHOICEBOX_CFLAGS@ @ECORE_EVAS_CFLAGS@
libeoi_la_LDFLAGS = -no-undefined -version-info 2:0:1 -Wl,--as-needed
libeoi_la_LIBADD = @EFL_LIBS@ @LIBKEYS_LIBS@ @LIBCHOICEBOX_LIBS@

eoi_test_SOURCES = eoi_test.c
eoi_test_LDFLAGS = @EFL_LIBS@ @LIBCHOICEBOX_LIBS@ @ECORE_EVAS_LIBS@ libeoi.la
