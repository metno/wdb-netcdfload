man1_MANS = netcdfLoad.man


netcdfLoad.man:	netcdfLoad doc/netcdfLoad.txt
	$(HELP2MAN) -N -n"Load a netcdf file into the WDB database" -i$(top_srcdir)/doc/netcdfLoad.txt ./$< -o$@
