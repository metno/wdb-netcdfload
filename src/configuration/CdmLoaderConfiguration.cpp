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

#include "CdmLoaderConfiguration.h"
#include <fimex/CDMconstants.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/filesystem.hpp>
#include <set>
#include <fstream>

CdmLoaderConfiguration::CdmLoaderConfiguration()
{
	using namespace boost::program_options;

    options_description conf( "Load configuration" );
    conf.add_options()
    ( "type,t", value(& fileType_)->default_value("netcdf"), "Assume file is of the given type")
    ("type-configuration", value(& fileTypeConfiguration_), "Read file-to-netcdf configuration from the given file")
    ( "configuration,c", value(& loadConfiguration_), "Read netcdf-to-wdb configuration from the given file.")
    ;

    options_description extract( "Parameter extraction" );
    extract.add_options()
    ("extract,e", value(& elementsToLoad_), "Only read the given variables. You may express a dimension subset like this: --extract=air_temperature:dimension=value");
    ;

    options_description point( "Point extraction" );
    point.add_options()
    //( "longitude", value(& point_.longitude_), "Extract only data from this longitude (must also give latitude)")
    //( "latitude",  value(& point_.latitude_),  "Extract only data from this latitude (must also give longitude)")
    ( "point-file", value(& pointsFile_), "Read lat/lon coordinates from this file (one set per line, space-separated)")
    ;

    configOptions().add(conf).add(extract).add(point);
	shownOptions().add(conf).add(extract).add(point);
}

CdmLoaderConfiguration::~CdmLoaderConfiguration()
{
}

CdmLoaderConfiguration::Point::Point(const std::string & spec)
{
	std::vector<std::string> data;
	boost::algorithm::split(data, spec, boost::algorithm::is_space());
	if ( data.size() < 2 )
		throw std::runtime_error("Invalid specification: " + spec);
	if ( data.size() > 2 and data[2][0] != '#' )
		throw std::runtime_error("Invalid specification: " + spec);
	latitude_ = data[0];
	longitude_ = data[1];
}

double CdmLoaderConfiguration::Point::longitude() const
{
	return boost::lexical_cast<double>(longitude_);
}

double CdmLoaderConfiguration::Point::latitude() const
{
	return boost::lexical_cast<double>(latitude_);
}

std::string CdmLoaderConfiguration::Point::getPlaceName() const
{
	std::ostringstream ret;
	ret << "point(" << longitude_ << ' ' << latitude_ << ')';
	return ret.str();
}


//const CdmLoaderConfiguration::Point * CdmLoaderConfiguration::point() const
//{
//	if ( point_.longitude_.empty() and point_.latitude_.empty() )
//		return 0;
//	if ( point_.longitude_.empty() or point_.latitude_.empty() )
//		throw std::runtime_error("Must give both latitude and longitude");
//	return & point_;
//}


namespace
{
std::set<std::string> getAvailableFileTypes()
{
	std::set<std::string> ret;

	int numberOfFileTypes = mifi_get_max_filetype_number();
	for (int i = 0; i < numberOfFileTypes; ++ i )
	{
		std::string typeName = mifi_get_filetype_name(i);
		ret.insert(typeName);
	}

	return ret;
}
}

void CdmLoaderConfiguration::parse( int argc, char ** argv )
{
    wdb::load::LoaderConfiguration::parse(argc, argv);

    const std::set<std::string> types = getAvailableFileTypes();
    if ( types.find(fileType_) == types.end() )
    {
    	std::ostringstream msg;
    	msg << "Invalid file type (" << fileType_ <<"). Available types are:";
    	for ( std::set<std::string>::const_iterator it = types.begin(); it != types.end(); ++ it )
    		msg << ' ' << * it;
    	throw std::runtime_error(msg.str());
    }

    if ( not pointsFile_.empty() )
    {
    	boost::filesystem::path pointPath(pointsFile_);
    	if ( not exists(pointPath) )
    		throw std::runtime_error(pointsFile_ + " does not exist");
    	if ( boost::filesystem::is_directory(pointsFile_) )
    		throw std::runtime_error(pointsFile_ + " is a directory");

    	std::ifstream s(pointsFile_.c_str());
    	std::string line;
    	while ( std::getline(s, line) )
    		points_.push_back(line);
    }

//    if ( point() )
//	{
//    	if ( not output().list )
//    		throw std::runtime_error("Direct loading of single points is not supported yet");
//    	if ( loading_.placeName.empty() )
//    	{
//    		std::ostringstream s;
//    		s << "point(" << point_.longitude_ << ' ' << point_.latitude_ << ')';
//    		loading_.placeName = s.str();
//    	}
//	}
}
