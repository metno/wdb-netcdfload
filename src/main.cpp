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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "NetcdfFile.h"
#include "NetcdfTranslator.h"
#include "configuration/CdmLoaderConfiguration.h"
#include <wdb/LoaderDatabaseConnection.h>
#include <wdbLogHandler.h>
#include "fimex/CDM.h"
#include "fimex/CDMReader.h"
#include "fimex/CDMFileReaderFactory.h"
#include <boost/foreach.hpp>
#include <boost/scoped_ptr.hpp>
#include <iostream>

using namespace std;

namespace
{
ostream& version(ostream& out)
{
	return out << PACKAGE_STRING << endl;
}

ostream & help(ostream& out,
		const boost::program_options::options_description& options)
{
	out << "Usage: " << PACKAGE_NAME
			<< " -c CONFIGURATION_FILE [OPTIONS] FILES...\n";
	out << '\n';
	out << "Options:\n";
	out << options << endl;
	return out;
}

void list(const AbstractNetcdfField::Ptr field, const NetcdfTranslator & translator)
{
	BOOST_FOREACH(WriteQuery query, translator.queries(* field))
		query.list(std::cout);
}

void list(const NetcdfFile & file, const NetcdfTranslator & translator)
{
	BOOST_FOREACH(const AbstractNetcdfField::Ptr & field, file.getFields())
		list(field, translator);
}

void list(const AbstractNetcdfField::Ptr field, const NetcdfTranslator & translator, const std::vector<CdmLoaderConfiguration::Point> & points)
{
	BOOST_FOREACH(const WriteQuery & query, translator.queries(* field, points))
		query.list(std::cout);
}

void list(NetcdfFile & file, const NetcdfTranslator & translator, const std::vector<CdmLoaderConfiguration::Point> & points)
{
	std::vector<double> longitude;
	std::vector<double> latitude;
	BOOST_FOREACH ( const CdmLoaderConfiguration::Point & point, points )
	{
		longitude.push_back(point.longitude());
		latitude.push_back(point.latitude());
	}
	file.setPointFilter(longitude, latitude);

	BOOST_FOREACH(const AbstractNetcdfField::Ptr & field, file.getFields())
		list(field, translator, points);
}

void write(const std::vector<AbstractNetcdfField::Ptr> & fields, NetcdfTranslator & translator,
		wdb::load::LoaderDatabaseConnection & wdbConnection)
{
	BOOST_FOREACH(const AbstractNetcdfField::Ptr & field, fields)
		BOOST_FOREACH(const WriteQuery & query, translator.queries(* field))
				query.write(wdbConnection);
}

}

int main(int argc, char ** argv)
{
	CdmLoaderConfiguration conf;
	try
	{
		conf.parse(argc, argv);
	} catch (std::exception & e)
	{
		std::clog << "ERROR: Command line arguments: " << e.what() << std::endl;
		return 1;
	}

	if (conf.general().version)
	{
		version(cout);
		return 0;
	}
	if (conf.general().help)
	{
		help(std::cout, conf.shownOptions());
		return 0;
	}

	wdb::WdbLogHandler logHandler(conf.logging().loglevel,
			conf.logging().logfile);

//	try
//	{
		boost::scoped_ptr<wdb::load::LoaderDatabaseConnection> wdbConnection(
				conf.output().list ? 0 : new wdb::load::LoaderDatabaseConnection(conf));

		if ( conf.output().list )
		{
			std::cout << conf.loading().dataProvider;
			if ( conf.points() )
				std::cout << "\t88,0,88";
			std::cout << '\n';
		}

		NetcdfTranslator translator(conf);
		BOOST_FOREACH(const std::string & file, conf.input().file)
		{
			NetcdfFile toLoad(file, conf.fileTypeConfiguration(), conf.fileType(), translator.loadConfiguration().vectorConversions());

			if ( conf.points() )
			{
				if ( conf.output().list )
					list(toLoad, translator, * conf.points());
				else
					throw std::runtime_error("point extraction is not supported when directly loading into wdb");
			}
			else
			{
				if ( conf.output().list )
					list(toLoad, translator);
				else
					write(toLoad.getFields(), translator, * wdbConnection);
			}
		}
//	}
//	catch (std::exception& e)
//	{
//		WDB_LOG & log = WDB_LOG::getInstance("wdb.load.netcdf");
//		log.fatal(e.what());
//		return 1;
//	}
}
