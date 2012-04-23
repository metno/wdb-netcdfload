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

// wdb
#include <wdbException.h>
#include <wdbLogHandler.h>

// fimexS
#include <fimex/CDM.h>
#include <fimex/Data.h>
#include <fimex/CDMReader.h>
#include <fimex/CDMFileReaderFactory.h>

// boost
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>

// std
#include <vector>
#include <functional>


using namespace MetNoFimex;

CdmLoader::CdmLoader(const CdmLoaderConfiguration & conf) :
	conf_(conf),
	wdbConnection_(conf),
	loadConfiguration_(conf.loadConfiguration())
{
}

CdmLoader::~CdmLoader() { }

void CdmLoader::write(const std::string & file)
{
    WDB_LOG & log = WDB_LOG::getInstance("wdb.load.netcdf");
    log.debug("Loading file " + file);

    boost::shared_ptr<CDMReader> pReader = CDMFileReaderFactory::create(MIFI_FILETYPE_NETCDF, file);

    write(pReader);
}

void CdmLoader::write(boost::shared_ptr<CDMReader>& reader)
{
    WDB_LOG & log = WDB_LOG::getInstance("wdb.load.netcdf");

    std::string placeName = getPlaceName_(reader);
    Time referenceTime = getReferenceTime_(reader);
    std::vector<Time> times = getTimes_(reader);

    boost::shared_ptr<MetNoFimex::Data> altitude = getAltitude_(reader);
    if ( altitude )
        write_(altitude, "altitude", placeName, referenceTime, "-infinity", "infinity");
    else
	log.warn("Unable to find altitude in source data");

    BOOST_FOREACH(const LoadElement & loadElement, loadConfiguration_)
        write(reader, loadElement, placeName, referenceTime, times);

}


void CdmLoader::write(boost::shared_ptr<CDMReader>& reader, 
                      const LoadElement & loadElement,
		      const std::string & placeName,
		      const Time & referenceTime,
		      const std::vector<Time> & validTimes)
{
    WDB_LOG & log = WDB_LOG::getInstance("wdb.load.netcdf");

    const MetNoFimex::CDM & cdm = reader->getCDM();

    const DataSpecification & specification = loadElement.wdbDataSpecification();
	std::string wdbParameter = specification.wdbParameter();

	const MetNoFimex::CDMDimension * unlimitedDimension = cdm.getUnlimitedDim();

	float undef = std::numeric_limits<float>::quiet_NaN();
	MetNoFimex::CDMAttribute fillValue;
	if ( cdm.getAttribute(loadElement.cfName(), "_FillValue", fillValue) )
		undef = fillValue.getData()->asFloat()[0];

	for (unsigned i = 0; i < validTimes.size(); ++i)
	{
		log.debugStream() << "Loading valid time " << string_from_local_date_time(validTimes[i]);

		Time validFrom = specification.validTimeFrom().getTime(referenceTime, validTimes[i]);
		Time validTo = validTimes[i];

		MetNoFimex::SliceBuilder slicer(cdm, loadElement.cfName());
		if ( unlimitedDimension )
			slicer.setStartAndSize(unlimitedDimension->getName(), i, 1);

		BOOST_FOREACH(const LoadElement::IndexElement & ie, loadElement.indices() )
			slicer.setStartAndSize(ie.indexName, ie.cdmIndex(reader), 1);

		const boost::shared_ptr<MetNoFimex::Data> & data = reader->getScaledDataSlice(loadElement.cfName(), slicer);

		write_(data, wdbParameter, placeName, referenceTime,
				string_from_local_date_time(validFrom),
				string_from_local_date_time(validTo));
	}
}

void CdmLoader::write_(const boost::shared_ptr<MetNoFimex::Data> & data, const std::string & wdbParameter,
		const std::string & placeName, const Time & referenceTime,
		const std::string & validFrom, const std::string & validTo)
{
	boost::shared_array<float> values = data->asFloat();

	WDB_LOG & log = WDB_LOG::getInstance("wdb.load.netcdf");
	log.debugStream() << "Saving: " << placeName <<", "<<
			string_from_local_date_time(referenceTime) <<", "<<
			validFrom <<", "<<
			validTo <<", "<<
			wdbParameter;

	wdbConnection_.write(
			values.get(), data->size(),
			std::string(),
			placeName,
			string_from_local_date_time(referenceTime),
			validFrom,
			validTo,
			wdbParameter,
			"height above ground", 0, 0, 0, 0);
}


double CdmLoader::getScaleFactor_(const boost::shared_ptr<CDMReader>& reader, const std::string & cfName, float extraScaling) const
{
	WDB_LOG & log = WDB_LOG::getInstance("wdb.load.netcdf");
	double scaleFactor = 1;
	MetNoFimex::CDMAttribute scale_factor;
	if ( reader->getCDM().getAttribute(cfName, "scale_factor", scale_factor) )
	{
		try {
			scaleFactor = boost::lexical_cast<double>(scale_factor.getStringValue());
		}
		catch ( boost::bad_lexical_cast & )
		{
			log.warnStream() << "Could not understand netcdf value for scale_factor: " << scale_factor.getStringValue() << " unsing value 1 instead.";
		}
	}

	scaleFactor *= extraScaling;

	log.debugStream() << cfName << " scale factor (from netcdf and config): " << scaleFactor;

	return scaleFactor;
}

std::vector<Time> CdmLoader::getTimes_(boost::shared_ptr<CDMReader>& reader) const
{
    std::vector<Time> ret;

    boost::shared_ptr<MetNoFimex::Data> times = reader->getData("time");
    boost::shared_array<double> values = times->asDouble();
    for ( unsigned i = 0; i < times->size(); ++ i )
        ret.push_back(get_time((long long) values[i]));
    return ret;
}

std::string CdmLoader::getPlaceName_(boost::shared_ptr<CDMReader>& reader)
{
    WDB_LOG & log = WDB_LOG::getInstance("wdb.load.netcdf");

    const CDM & cdm = reader->getCDM();

    const CDMDimension * xAxis = 0;
    const CDMDimension * yAxis = 0;

    BOOST_FOREACH( const CDMDimension & dim, cdm.getDimensions() )
    {
        CDMAttribute axis;
	if ( cdm.getAttribute(dim.getName(), "axis", axis) )
	{
	    const std::string & axisValue = axis.getStringValue();
	    if ( axisValue == "X" )
	        xAxis = & dim;
	    else if ( axisValue == "Y" )
		yAxis = & dim;
	}
    }

	if ( ! xAxis or ! yAxis )
		throw std::runtime_error("Unable to locate grid definition in netcdf file");

	int xNum = xAxis->getLength();
	boost::shared_array<float> xValues = reader->getData(xAxis->getName())->asFloat();
	float startX = xValues[0];
	float xIncrement = xValues[1] - xValues[0];

	int yNum = yAxis->getLength();
	boost::shared_array<float> yValues = reader->getData(yAxis->getName())->asFloat();
	float startY = yValues[0];
	float yIncrement = yValues[1] - yValues[0];

	std::string projDefinition;
	BOOST_FOREACH(const CDMVariable & variable, cdm.getVariables() )
		BOOST_FOREACH(const CDMAttribute & attribute, cdm.getAttributes(variable.getName()))
			if ( attribute.getName() == "proj4" )
			{
				if ( not projDefinition.empty() )
					throw std::runtime_error("Many proj definitions in cdm. Cannot proceed");
				projDefinition = attribute.getStringValue();
			}

	try
	{
		std::string ret = wdbConnection_.getPlaceName(xNum, yNum, xIncrement, yIncrement, startX, startY,
				projDefinition);

		if ( not conf_.loading().placeName.empty() )
		{
			log.infoStream() << "Saving data using placename: " << conf_.loading().placeName << " even if data is identfied as " << ret;
			return conf_.loading().placeName;
		}

		return ret;
	}
	catch ( wdb::empty_result & e )
	{
		if ( conf_.loading().loadPlaceDefinition )
		{
			std::string placeName = conf_.loading().placeName;
			if ( placeName.empty() )
				placeName = "netcdf auto";
			return wdbConnection_.addPlaceDefinition(
					placeName,
					xNum, yNum, xIncrement, yIncrement, startX, startY,
					projDefinition
					);
		}
		else if ( not conf_.loading().placeName.empty() )
		{
			log.infoStream() << "No existing place name for grid. Using placename " << conf_.loading().placeName;
			return conf_.loading().placeName;
		}

		else throw;
	}
}

Time CdmLoader::getReferenceTime_(boost::shared_ptr<CDMReader>& reader) const
{
	try
	{
		boost::shared_ptr<MetNoFimex::Data> data;
		try
		{
			data = reader->getData("forecast_reference_time");
		}
		catch( std::exception &)
		{
			data = reader->getData("runtime");
		}

		if (data->size() > 1)
			throw std::runtime_error("Can only handle a single runtime");
		if (data->size() == 0)
			throw std::runtime_error("Missing runtime specification"); // should never happen

		double time = data->asDouble()[0];

		return get_time((long long) time);
	}
	catch (std::exception &)
	{
		// runtime did not exist in spec - it must therefore be specified in configuration.
		return INVALID_TIME;
	}
}

boost::shared_ptr<MetNoFimex::Data> CdmLoader::getAltitude_(boost::shared_ptr<CDMReader>& reader) const
{
    return reader->getScaledData("altitude");
}
