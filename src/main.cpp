/*
    pgen_wdbSave

    Copyright (C) 2011 met.no

    Contact information:
    Norwegian Meteorological Institute
    Box 43 Blindern
    0313 OSLO
    NORWAY
    E-mail: post@met.no

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
    MA  02110-1301, USA
*/


#include "CdmLoader.h"
#include "configuration/CdmLoaderConfiguration.h"
#include <wdbLogHandler.h>
#include <boost/foreach.hpp>
#include <iostream>

#define PROGRAM "cdmLoad"
#define VERSION "1.0.0"


namespace
{
std::ostream & version(std::ostream & out)
{
	return out << PROGRAM" "VERSION << std::endl;
}

std::ostream & help(std::ostream & out, const boost::program_options::options_description & options)
{
	version(out);
	out << '\n';
	out << "Usage: "PROGRAM" [OPTIONS] FILE...\n";
	out << '\n';
	out << "Loads data from a netcdf file into a wdb database\n";
	out << '\n';
	out << "Options:\n";
	out << options << std::endl;
	return out;
}
}

int main(int argc, char ** argv)
{
	CdmLoaderConfiguration conf;
	conf.parse(argc, argv);

	if ( conf.general().version )
	{
		version(std::cout);
		return 0;
	}
	if ( conf.general().help )
	{
		help(std::cout, conf.shownOptions());
		return 0;
	}

	if ( conf.output().list )
	{
		std::clog << "content listing is not supported (yet). Use ncdump instead" << std::endl;
		return 1;
	}

	wdb::WdbLogHandler logHandler( conf.logging().loglevel, conf.logging().logfile );

	try
	{
		CdmLoader loader(conf);
		BOOST_FOREACH(const std::string & file, conf.input().file )
			loader.write(file);
	}
	catch ( std::exception & e )
	{
		WDB_LOG & log = WDB_LOG::getInstance("wdb.load.netcdf");
		log.fatal(e.what());
		return 1;
	}
}
