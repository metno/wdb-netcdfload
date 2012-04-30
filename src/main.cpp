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

// project
#include "CdmLoader.h"
#include "configuration/CdmLoaderConfiguration.h"

// wdb
#include <wdbLogHandler.h>

// fimex
#include "fimex/CDM.h"
#include "fimex/CDMReader.h"
#include "fimex/CDMFileReaderFactory.h"

// boost
#include <boost/foreach.hpp>

//std
#include <iostream>

#define PROGRAM "cdmLoad"
#define VERSION "1.0.0"

using namespace std;

namespace {
    ostream& version(ostream& out)
    {
	return out << PROGRAM" "VERSION << endl;
    }

    ostream & help(ostream& out, const boost::program_options::options_description& options)
    {
	version(out);
	out << '\n';
	out << "Usage: "PROGRAM" [OPTIONS] FILE...\n";
	out << '\n';
	out << "Loads data from a netcdf file into a wdb database\n";
	out << '\n';
	out << "Options:\n";
	out << options << endl;
	return out;
    }
}

int main(int argc, char ** argv)
{
    CdmLoaderConfiguration conf;
    conf.parse(argc, argv);

    if(conf.general().version)
    {
        version(cout);
	return 0;
    }
    if(conf.general().help)
    {
        help(std::cout, conf.shownOptions());
	return 0;
    }

    if(conf.output().list)
    {
        BOOST_FOREACH(const std::string& file, conf.input().file)
        {
            boost::shared_ptr<MetNoFimex::CDMReader> reader = MetNoFimex::CDMFileReaderFactory::create(MIFI_FILETYPE_NETCDF, file);
            reader->getCDM().toXMLStream(cout);
            cout <<"--------------------------------------------------------------------------------------------------------------"<< endl;
        }

        return 0;
    }

    wdb::WdbLogHandler logHandler(conf.logging().loglevel, conf.logging().logfile);

    try{
        CdmLoader loader(conf);
	BOOST_FOREACH(const std::string & file, conf.input().file)
            loader.load(file);
    } catch (std::exception& e) {
        WDB_LOG & log = WDB_LOG::getInstance("wdb.load.netcdf");
	log.fatal(e.what());
	return 1;
    }
}
