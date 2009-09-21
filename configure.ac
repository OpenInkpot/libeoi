AC_PREREQ(2.57)
AC_INIT(libeoi, 0.1, dottedmag@dottedmag.net)
AM_INIT_AUTOMAKE(foreign)

AC_PROG_CC_C99
AM_PROG_LIBTOOL

PKG_CHECK_MODULES(EFL, [ecore ecore-file ecore-evas evas edje efreet])
PKG_CHECK_MODULES(LIBCHOICEBOX, [libchoicebox])

AC_OUTPUT([Makefile src/Makefile data/Makefile themes/Makefile libeoi.pc])
