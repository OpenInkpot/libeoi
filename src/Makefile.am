lib_LTLIBRARIES = libeoi.la
libeoi_la_SOURCES = libeoi.c
include_HEADERS = libeoi.h

noinst_PROGRAMS = eoi_test

AM_CFLAGS = -Wall -DTHEME_DIR='"$(pkgdatadir)"'

INCLUDES = @EFL_CFLAGS@
libeoi_la_LDFLAGS = @EFL_LIBS@ -no-undefined -version-info 1:0:0

eoi_test_LDFLAGS = @EFL_LIBS@ libeoi.la