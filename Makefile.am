#-----------------------------------------------------------------------------
# Project Makefile for wdb's pointLoad
#
# This file is processed by automake to generate the non-recursive
# project Makefile
#-----------------------------------------------------------------------------


# Scanned by Autoreconf; must be static (no expansion)
ACLOCAL_AMFLAGS = 		-I config/m4
				
# Share directory organization
wdbdatadir = 			$(datadir)/wdb
sqldir = 		        $(wdbdatadir)/sql
wdblibdir = 			$(libdir)/wdb

#pointloadbindir = $(bindir)
#etcpointloaddir = /usr/share/doc/wdb-pointload
#dist_etcpointload_DATA =

bin_PROGRAMS = netcdfLoad

# This is to be replaced by a "find wdb" macro:
AM_CXXFLAGS =			-DSYSCONFDIR=\"$(pkgsysconfdir)\" $(wdb_CFLAGS) -I/usr/include/libxml2
netcdfLoad_LDADD = $(wdb_LIBS) \
                      libnetcdfLoadCore.a -lfimex -lgrib_api -lxml2 -lwdbLoad -lwdbConfig -lwdbProj -lwdbExcept -lwdbLog -lwdbMath

include src/netcdfLoad.mk
#include test/netcdfLoad.mk
include etc/netcdfLoad.mk
include doc/netcdfLoad.mk

include config/mk/debian.mk
