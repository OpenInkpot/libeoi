lib_LTLIBRARIES = libeoi.la
libeoi_la_SOURCES = libeoi.c eoi_textbox.c
include_HEADERS = libeoi.h libeoi_textbox.h

noinst_PROGRAMS = eoi_test

AM_CFLAGS = -Wall -DTHEME_DIR='"$(datadir)/eoi"' -DSYSCONFDIR='"$(sysconfdir)"'

INCLUDES = @EFL_CFLAGS@ @LIBCHOICEBOX_CFLAGS@
libeoi_la_LDFLAGS = -no-undefined -version-info 1:0:0 -Wl,--as-needed
libeoi_la_LIBADD = @EFL_LIBS@ @LIBCHOICEBOX_LIBS@

eoi_test_LDFLAGS = @EFL_LIBS@ @LIBCHOICEBOX_LIBS@ libeoi.la
