#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT(netcdfLoad, 0.6.0, [wdb@met.no])

AC_CONFIG_SRCDIR([src/localtime.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_RANLIB

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_LN_S


# Checks for libraries.

WDB_BOOST_BASE_HEADERS(1.40.0)
WDB_BOOST_PROGRAM_OPTIONS(1.40.0)
WDB_BOOST_REGEX(1.40.0)
WDB_BOOST_DATE_TIME(1.40.0)
WDB_BOOST_FILESYSTEM(1.40.0)

PKG_CHECK_MODULES([fimex], [fimex],
   	[
		AC_SUBST(fimex_CFLAGS)
		AC_SUBST(fimex_LIBS)
	],
	[AC_MSG_ERROR([Unable to find fimex.])
	])

PKG_CHECK_MODULES([wdb], [libwdbLoad],
   	[
		AC_SUBST(wdb_CFLAGS)
		AC_SUBST(wdb_LIBS)
	],
	[AC_MSG_ERROR([Unable to find libwdbload.])
	])

AC_PATH_PROG([XMLTO], [xmlto], [false])

GTEST_CHECK
		

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE

AM_INIT_AUTOMAKE([foreign -Wall -Werror subdir-objects])
AM_PROG_CC_C_O

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
