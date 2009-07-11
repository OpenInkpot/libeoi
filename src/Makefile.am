lib_LTLIBRARIES = libeoi.la
libeoi_la_SOURCES = eoi.c
include_HEADERS = eoi.h

noinst_PROGRAMS = eoi_test

AM_CFLAGS = -W -Wall -Wno-unused-parameter -DTHEME_DIR='"$(pkgdatadir)"'

INCLUDES = @EFL_CFLAGS@
libeoi_la_LDFLAGS = @EFL_LIBS@ -no-undefined --as-needed

eoi_test_LDFLAGS = @EFL_LIBS@ libeoi.la