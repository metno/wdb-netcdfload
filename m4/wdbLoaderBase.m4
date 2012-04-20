AC_DEFUN([WDB_LOADERBASE_CHECK],
[
PKG_CHECK_MODULES([wdbload], [libwdbLoad],
   	[
		AC_SUBST(wdbload_CFLAGS)
		AC_SUBST(wdbload_LIBS)
	],
	[AC_MSG_ERROR([Unable to find libwdbLoad.])
])
CPPFLAGS="$CPPFLAGS $wdbload_CFLAGS"
LIBS="$LIBS $wdbload_LIBS"
])

