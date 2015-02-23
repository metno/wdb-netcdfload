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
#include <wdb/errors.h>
#include <wdb/LoaderDatabaseConnection.h>
#include <wdbLogHandler.h>
#include "fimex/CDM.h"
#include "fimex/CDMReader.h"
#include "fimex/CDMFileReaderFactory.h"
#include <boost/foreach.hpp>
#include <boost/scoped_ptr.hpp>
#include <iostream>


namespace
{
std::ostream& version(std::ostream& out)
{
	return out << PACKAGE_STRING << std::endl;
}

std::ostream & help(std::ostream& out,
		const boost::program_options::options_description& options)
{
	out << "Usage: " << PACKAGE_NAME
			<< " -c CONFIGURATION_FILE [OPTIONS] FILES...\n";
	out << '\n';
	out << "Options:\n";
	out << options << std::endl;
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

/**
 * Will return number of loaded fields
 */
unsigned write(const std::vector<AbstractNetcdfField::Ptr> & fields, NetcdfTranslator & translator,
		wdb::load::LoaderDatabaseConnection & wdbConnection)
{
	unsigned ret = 0;

	WDB_LOG & log = WDB_LOG::getInstance("wdb.load.netcdf");
	BOOST_FOREACH(const AbstractNetcdfField::Ptr & field, fields)
		BOOST_FOREACH(const WriteQuery & query, translator.queries(* field))
			try
			{
				query.write(wdbConnection);
				++ ret;
			}
			catch ( std::exception & e )
			{
				std::ostringstream s;
				query.list(s);
				log.errorStream() << e.what() << ": Uanble to load field " << s.str();
			}
	return ret;
}

}

int main(int argc, char ** argv)
{
	CdmLoaderConfiguration conf;
	try
	{
		conf.parse(argc, argv);
	} catch (wdb::load::LoadError & e)
	{
		std::clog << e.what() << std::endl;
		return wdb::load::exitStatus();
	}

	if (conf.general().version)
	{
		version(std::cout);
		return 0;
	}
	if (conf.general().help)
	{
		help(std::cout, conf.shownOptions());
		return 0;
	}

	wdb::WdbLogHandler logHandler(conf.logging().loglevel,
			conf.logging().logfile);

	// Counter for loaded fields
	unsigned loadCount = 0;

	try
	{
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

			BOOST_FOREACH ( const NetcdfParameterSpecification & spec, conf.elementsToLoad() )
				if ( not toLoad.contains(spec) )
					throw wdb::load::LoadError(wdb::load::ErrorWhenReadingFile, "Unable to find parameter " + spec.variableName() + " with the given dimensions");

			if ( conf.points() )
			{
				if ( conf.output().list )
					list(toLoad, translator, * conf.points());
				else
					throw wdb::load::LoadError(wdb::load::InvalidCommandLineArguments, "Point extraction is not supported when directly loading into wdb");
			}
			else
			{
				if ( conf.output().list )
					list(toLoad, translator);
				else
					loadCount += write(toLoad.getFields(), translator, * wdbConnection);
			}
		}

		if ( not wdb::load::success() )
		{
			WDB_LOG & log = WDB_LOG::getInstance("wdb.load.netcdf");
			if ( loadCount == 0 and wdb::load::errorCode() == wdb::load::FieldFailedToLoad )
			{
				wdb::load::registerError(wdb::load::NoFieldsLoaded);
				log.error(wdb::load::getErrorMessage());
			}
			else
				log.warn(wdb::load::getErrorMessage());
		}
	}
	catch ( wdb::load::LoadError & e )
	{
		WDB_LOG & log = WDB_LOG::getInstance("wdb.load.netcdf");
		log.fatal(e.what());
		return wdb::load::exitStatus();
	}
	catch (std::exception& e)
	{
		WDB_LOG & log = WDB_LOG::getInstance("wdb.load.netcdf");
		log.fatal(e.what());
		return int(wdb::load::UnknownError);
	}

	WDB_LOG & log = WDB_LOG::getInstance("wdb.load.netcdf");
	log.infoStream() << "Loaded " << loadCount << " fields into database";

	return wdb::load::exitStatus();
}
