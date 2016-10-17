/*
 netcdfload

 Copyright (C) 2013 met.no

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

#include "WriteQuery.h"
#include <wdb/LoaderDatabaseConnection.h>
#include <wdb/errors.h>
#include <wdbLogHandler.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <sstream>
#include <ctime>


std::map<std::string, std::string> WriteQuery::placedef2placename;

WriteQuery::WriteQuery() :
	loadPlaceDefinition_(false),
	referenceTime_(INVALID_TIME),
	validTimeFrom_(INVALID_TIME),
	validTimeTo_(INVALID_TIME),
	levelFrom_(0),
	levelTo_(0),
	dataVersion_(0),
	maxDataVersion_(0)
{
}

WriteQuery::~WriteQuery()
{
}

std::ostream & WriteQuery::list(std::ostream & out) const
{
	RawData rawData = (*function_)();

	if ( not rawData.valid() )
		return out;

	if ( rawData.numberOfValues == 1 )
	{
		float value = rawData.data[0];
		if ( value != value ) // NaN
			return out;
		out << value << '\t';
	}
	else
		out << "<data>\t";

	if ( not placeName_.empty() )
		out << placeName_;
	else if ( location_ )
		out << location_->wktRepresentation();
	else
		out << "<unknown place>";
	out << '\t';
	out << time_to_postgresql_string(referenceTime_) << '\t';
	out << time_to_postgresql_string(validTimeFrom_) << '\t';
	out << time_to_postgresql_string(validTimeTo_) << '\t';
	out << valueParameterName_ << '\t';
	out << levelParameterName_ << '\t';
	out << levelFrom_ << '\t';
	out << levelTo_ << '\t';
	if ( maxDataVersion_ )
	{
		out << dataVersion_ << '\t';
		out << maxDataVersion_;
	}
	out << '\n';

	return out;
}

void WriteQuery::write(wdb::load::LoaderDatabaseConnection & wdbConnection) const
{
	RawData rawData = (*function_)();
	if ( not rawData.valid() )
		return;

	std::ostringstream loc;
	loc << * location_;
	std::string & placeName = placedef2placename[loc.str()];
	if ( placeName.empty() )
	{
		try
		{
			placeName = wdbConnection.getPlaceName(* location_);
		}
		catch ( std::exception & e )
		{
		}
	}
	if ( loadPlaceDefinition_ and placeName.empty() )
	{
		std::ostringstream name;
		name << "netcdfLoad auto: ";
		name << boost::uuids::random_generator()();
		placeName = name.str();
		wdbConnection.addPlaceDefinition(placeName, * location_);
	}
	if ( placeName.empty() )
		throw wdb::load::LoadError(wdb::load::UnableToReadFromDatabase, "Uanble to find place name for grid");


	WDB_LOG & log = WDB_LOG::getInstance( "wdb.netcdfload.query" );
	log.debugStream() << "SELECT wci.write(<data>, "
			<< dataProvider_ << ", "
			<< placeName << ", "
			<< time_to_postgresql_string(referenceTime_) << ", "
			<< time_to_postgresql_string(validTimeFrom_) << ", "
			<< time_to_postgresql_string(validTimeTo_) << ", "
			<< valueParameterName_ << ", "
			<< levelParameterName_ << ", "
			<< levelFrom_ << ", "
			<< levelTo_ << ", "
			<< dataVersion_ << ", "
			<< "0);";

	wdbConnection.write(
			rawData.data.get(),
			rawData.numberOfValues,
			dataProvider_,
			placeName,
			time_to_postgresql_string(referenceTime_),
			time_to_postgresql_string(validTimeFrom_),
			time_to_postgresql_string(validTimeTo_),
			valueParameterName_,
			levelParameterName_,
			levelFrom_,
			levelTo_,
			dataVersion_,
			0
	);
}
