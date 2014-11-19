#-----------------------------------------------------------------------------
# Project Makefile for wdb's netcdfLoad
#
# This file is processed by automake to generate the non-recursive
# project Makefile
#-----------------------------------------------------------------------------


# Scanned by Autoreconf; must be static (no expansion)
ACLOCAL_AMFLAGS = 		-I config/m4

pkgsysconfdir = $(sysconfdir)/netcdfLoad
dist_pkgsysconf_DATA = 

bin_PROGRAMS = netcdfLoad

CLEANFILES =
EXTRA_DIST =

# This is to be replaced by a "find wdb" macro:
AM_CXXFLAGS =			-DSYSCONFDIR=\"$(pkgsysconfdir)\" -I$(top_srcdir)/src $(wdb_CFLAGS) $(fimex_CFLAGS) -I/usr/include/libxml2
netcdfLoad_LDADD = $(wdb_LIBS) $(fimex_LIBS) \
                      -llog4cpp -lboost_date_time -lboost_filesystem -lboost_regex -lboost_program_options -lboost_system -lfimex -lxml2 -lwdbLoad -lwdbConfig -lwdbProj -lwdbExcept -lwdbLog -lwdbMath -lproj

include src/netcdfLoad.mk
include test/netcdfLoad.mk
include etc/netcdfLoad.mk
include doc/netcdfLoad.mk

include config/mk/debian.mk
