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


#include "NetcdfLoader.h"
#include "NetcdfFile.h"
#include <wdbLogHandler.h>
#include <boost/foreach.hpp>
#include <boost/date_time.hpp>
#include <ostream>


NetcdfLoader::NetcdfLoader(const CdmLoaderConfiguration & conf) :
	conf_(conf),
	wdbConnection_(conf),
	loadConfiguration_(conf.loadConfiguration())
{
}

NetcdfLoader::~NetcdfLoader()
{
}

void NetcdfLoader::load(const NetcdfFile & file)
{
	Time referenceTime = referenceTime_(file);

	BOOST_FOREACH( NetcdfField::Ptr field, file.getFields() )
	{
		const LoadElement * loadElement = loadConfiguration_.getLoadElement(field->variableName());
		if ( ! loadElement )
			continue;
		const DataSpecification & spec = loadElement->wdbDataSpecification();

		const DataSpecification::Level & level = spec.level();

		wdbConnection_.write(field->data().get(), field->dataSize(),
				dataProvider_(),
				placename_(field),
				time_to_postgresql_string(referenceTime),
				time_to_postgresql_string(spec.validTimeFrom().getTime(referenceTime, field->validtime())),
				time_to_postgresql_string(spec.validTimeTo().getTime(referenceTime, field->validtime())),
				spec.wdbParameter(),
				level.name(),
				level.value(),
				level.value(),
				dataVersion_(field),
				0
		);

	}
}

void NetcdfLoader::list(const NetcdfFile & file, std::ostream & s) const
{
	s << dataProvider_() << "\t88,88,88\n";

	Time referenceTime = referenceTime_(file);


	BOOST_FOREACH(NetcdfField::Ptr field, file.getFields())
	{
		const LoadElement * loadElement = loadConfiguration_.getLoadElement(field->variableName());
		if ( ! loadElement )
			continue;
		const DataSpecification & spec = loadElement->wdbDataSpecification();

		s << "<val>";
		s << "\t" << placename_(field);

		s << '\t' << time_to_postgresql_string(referenceTime);
		s << '\t' << time_to_postgresql_string(spec.validTimeFrom().getTime(referenceTime, field->validtime()));
		s << '\t' << time_to_postgresql_string(spec.validTimeTo().getTime(referenceTime, field->validtime()));
		s << '\t' << spec.wdbParameter();
		const DataSpecification::Level & level = spec.level();
		s << '\t' << level.name();
		s << '\t' << level.value();
		s << '\t' << level.value();

		s << '\t' << dataVersion_(field);
		s << '\t' << maxDataVersion_(field);
		s << '\n';
	}
	s << std::flush;
}

std::string NetcdfLoader::dataProvider_() const
{
	return conf_.loading().dataProvider;
}

std::string NetcdfLoader::placename_(const NetcdfField::Ptr & field) const
{
    WDB_LOG & log = WDB_LOG::getInstance("wdb.load.netcdf");

	boost::shared_ptr<GridGeometry> placeSpecification = field->placeSpecification();

    try {
        std::string ret = wdbConnection_.getPlaceName(* placeSpecification);
        if(not ret.empty())
            return ret;

        if(not conf_.loading().placeName.empty()) {
        	log.debugStream() << "Saving data using placename: " << conf_.loading().placeName << " even if data is identfied as " << ret;
            return conf_.loading().placeName;
        }
        return ret;
    } catch (wdb::empty_result& e) {
        if(conf_.loading().loadPlaceDefinition) {
            std::string placeName = conf_.loading().placeName;
            if(placeName.empty())
                placeName = "netcdf auto";

            return wdbConnection_.addPlaceDefinition(placeName, * placeSpecification);
        } else if(not conf_.loading().placeName.empty()) {
            log.infoStream() << "No existing place name for grid. Using placename " << conf_.loading().placeName;
            return conf_.loading().placeName;
        }
        else throw;
    }
}

Time NetcdfLoader::referenceTime_(const NetcdfFile & file) const
{
	std::string ret = conf_.loading().referenceTime;
	if ( ret.empty() )
		return file.getReferenceTime();
	return time_from_postgresql_string(ret);
}

unsigned NetcdfLoader::dataVersion_(const NetcdfField::Ptr & field) const
{
	int ret = conf_.loading().dataVersion;
	if ( ret < 0 )
		ret = field->dataVersion();
	return ret;
}

unsigned NetcdfLoader::maxDataVersion_(const NetcdfField::Ptr & field) const
{
	// maybe some handling of explicit data version as command-line argument?
	return field->maxDataVersion();
}

