AC_PREREQ(2.57)
AC_INIT(libeoi, 0.1, dottedmag@dottedmag.net)
AM_INIT_AUTOMAKE(foreign)

AC_PROG_CC_C99
AM_PROG_LIBTOOL

PKG_CHECK_MODULES(EFL, [eina-0 evas edje])
PKG_CHECK_MODULES(ECORE, [ecore])
PKG_CHECK_MODULES(ECORE_X, [ecore-x])
PKG_CHECK_MODULES(EFREET, [efreet])
PKG_CHECK_MODULES(ECORE_EVAS, [ecore-evas])
PKG_CHECK_MODULES(LIBKEYS, [libkeys])

AC_OUTPUT([Makefile src/Makefile data/Makefile po/Makefile themes/Makefile themes/azbooka/Makefile themes/default/Makefile libeoi.pc libchoicebox.pc])
