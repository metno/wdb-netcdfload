#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT(netcdfLoad, 0.13.12, [wdb@met.no])

AC_CONFIG_SRCDIR([src/localtime.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_PROG_AR
AC_PROG_RANLIB

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_LN_S

AX_BOOST_BASE(1.40.0)
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_DATE_TIME
AX_BOOST_REGEX
AX_BOOST_FILESYSTEM

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

GMOCK_DIST_CHECK(.)

AC_PATH_PROG(HELP2MAN, help2man)

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE

AM_INIT_AUTOMAKE([foreign -Wall -Werror subdir-objects])
AM_PROG_CC_C_O

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

# Print Out Summary of Configuration
AC_MSG_RESULT([ ])
AC_MSG_RESULT([$PACKAGE_NAME $PACKAGE_VERSION])
AC_MSG_RESULT([Configuration:])
AC_MSG_RESULT([-------------------------------------------------------------------------])
AC_MSG_RESULT([ ])
AC_MSG_RESULT([ Database Name:    $DATABASE_NAME])
AC_MSG_RESULT([ Source Code:      $srcdir])
AC_MSG_RESULT([ Host System:      $host])
AC_MSG_RESULT([  ])
AC_MSG_RESULT([ Prefix:           $prefix])
AC_MSG_RESULT([ Binaries:         $exec_prefix])
AC_MSG_RESULT([ Manuals:          $mandir])
AC_MSG_RESULT([ Data:             $datadir])
AC_MSG_RESULT([ System Config:    $sysconfdir])
AC_MSG_RESULT([ ])
AC_MSG_RESULT([ CPPFLAGS:         $CPPFLAGS])
AC_MSG_RESULT([ LDFLAGS:          $LDFLAGS])
AC_MSG_RESULT([ LIBS:             $LIBS])
AC_MSG_RESULT([  ])
AC_MSG_RESULT([-------------------------------------------------------------------------])
AC_MSG_RESULT([ ])
