ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -DSYSCONFDIR='"$(sysconfdir)"' $(fimex_CFLAGS) $(wdb_CFLAGS)

include src/netcdfLoad.mk
include test/netcdfLoad.mk
include etc/netcdfLoad.mk
include doc/netcdfLoad.mk
include mk/debian.mk