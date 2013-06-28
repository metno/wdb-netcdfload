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
#include <fimex/CDMReaderUtils.h>
#include <fimex/CDMFileReaderFactory.h>
#include <fimex/coordSys/Projection.h>

// boost
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>

// std
#include <vector>
#include <functional>
#include <algorithm>


using namespace MetNoFimex;

CdmLoader::CdmLoader(const CdmLoaderConfiguration & conf) :
	conf_(conf),
	wdbConnection_(conf),
	loadConfiguration_(conf.loadConfiguration())
{
}

CdmLoader::~CdmLoader() { }

void CdmLoader::load(const std::string& file)
{
    WDB_LOG & log = WDB_LOG::getInstance("wdb.load.netcdf");
    log.debug("Loading file " + file);

    // cache common data (for one file to be loaded)
    pReader_ = CDMFileReaderFactory::create(conf_.fileType(), file, conf_.fileTypeConfiguration());
    pReferenceTime_ = boost::shared_ptr<Time>(new Time(getReferenceTime_()));

    write_();
}

void CdmLoader::write_()
{
//    WDB_LOG & log = WDB_LOG::getInstance("wdb.load.netcdf");

// Ruins support for multiple dimensions in netcdf file:
//	const CDM & cdm = pReader_->getCDM();
//	BOOST_FOREACH( const CDMVariable & variable, cdm.getVariables() )
//	{
//		const LoadElement * loadElement = loadConfiguration_.getLoadElement(variable.getName());
//		if ( loadElement )
//		{
//			LoadElement element = * loadElement;
//			write_(element);
//		}
//	}

    for(LoadConfiguration::load_const_iterator cit = loadConfiguration_.load_begin(); cit != loadConfiguration_.load_end(); ++cit) {
        LoadElement element = *cit;
        write_(element);
    }
}

namespace
{
struct scale_data
{
	const float scale_;
	scale_data(float scale) : scale_(scale) {}
	float operator () (float val) const
	{
		return val * scale_;
	}
};
}

CdmLoader::Blob CdmLoader::getData_(const SliceBuilder & slicer, LoadElement & loadElement, float undef)
{
	boost::shared_ptr<Data> data;
	if (loadElement.wdbDataSpecification().wdbUnits().empty())
		data = pReader_->getDataSlice(loadElement.cfName(), slicer);
	else
		data = pReader_->getScaledDataSliceInUnit(loadElement.cfName(),
				loadElement.wdbDataSpecification().wdbUnits(), slicer);

	Blob ret;
	ret.length = data->size();
	ret.data = data->asFloat();

	float * begin = ret.data.get();
	float * end = begin + ret.length;
	std::replace(begin, end, undef, std::numeric_limits<float>::quiet_NaN());

	float scale = loadElement.wdbDataSpecification().scale();
	if ( scale != 1 )
	{
		float * newData = new float[ret.length];
		std::transform(ret.data.get(), ret.data.get() + ret.length, newData, scale_data(scale));
		ret.data.reset(newData);
	}

	return ret;
}

void CdmLoader::write_(LoadElement& loadElement)
{
    WDB_LOG & log = WDB_LOG::getInstance("wdb.load.netcdf");

    log.infoStream() << "Loading " << loadElement;

    std::string placeName = getPlaceName_(loadElement.cfName());
    if(placeName.empty()) {
        log.errorStream() << "no placename for cfname: " << loadElement.cfName();
        return;
    }

    const CDM& cdm = pReader_->getCDM();

    const DataSpecification& specification = loadElement.wdbDataSpecification();
    std::string wdbParameter = specification.wdbParameter();
    std::cout << wdbParameter << std::endl;

//    std::string tAxisName = cdm.getTimeAxis(loadElement.cfName());
//    if(tAxisName.empty()) {
//    	log.errorStream() << "no time axis for cfname: " << loadElement.cfName();
//        return;
//    }

    // check if CDM modell has eps axis at all
    // based on the non-standardized "realization"
    std::string eAxisName;
    std::string epsDimStandardName = "realization";
    if(not cdm.findVariables("standard_name", epsDimStandardName).empty()) {
        eAxisName = cdm.findVariables("standard_name", epsDimStandardName)[0];
    }

    if(not eAxisName.empty()) {
        // see if variable has ensemble in its shape
        const CDMVariable& var = cdm.getVariable(loadElement.cfName());
        const std::vector<std::string>& shape = var.getShape();
        if( std::find(shape.begin(), shape.end(), eAxisName) != shape.end() )
			loadElement.expandIndicePermutations(pReader_, eAxisName);
    }

    float undef = std::numeric_limits<float>::quiet_NaN();
    CDMAttribute fillValue;
    if(cdm.getAttribute(loadElement.cfName(), "_FillValue", fillValue))
        undef = fillValue.getData()->asFloat()[0];

    std::vector<Time> times = getTimes_(cdm, loadElement);
    if ( times.empty() )
    	times.push_back(Time(boost::local_time::pos_infin)); // infinity

    for(size_t t = 0; t < times.size(); ++t)
    {
        log.infoStream() << "Loading valid time " << time_to_postgresql_string(times[t]);

        const Time & validFrom = specification.validTimeFrom().getTime(*pReferenceTime_, times[t]);
        const Time & validTo = specification.validTimeTo().getTime(*pReferenceTime_, times[t]);

        SliceBuilder slicer(cdm, loadElement.cfName());
        std::string timeDimensionName = cdm.getTimeAxis(loadElement.cfName());
        if ( not timeDimensionName.empty() )
            slicer.setStartAndSize(timeDimensionName, t, 1);

        if ( loadElement.permutations().empty() )
        {
			Blob data = getData_(slicer, loadElement, undef);
			float * begin = data.data.get();
			float * end = begin + data.length;
			std::replace(begin, end, undef, std::numeric_limits<float>::quiet_NaN());

			size_t dataVersion = 0;



			write_(data,
                   wdbParameter,
                   placeName,
                   time_to_postgresql_string(validFrom),
                   time_to_postgresql_string(validTo),
                   specification.level().name(),
                   specification.level().value(),
                   specification.level().value(),
                   dataVersion);
        }
        for(size_t permutationindex = 0; permutationindex < loadElement.permutations().size(); ++permutationindex)
        {
            const std::vector<LoadElement::IndexElement>& permutation = loadElement.permutations()[permutationindex];

            size_t dataVersion = 0;

            for(size_t i = 0; i < permutation.size(); ++i)
            {
                const LoadElement::IndexElement& ie = permutation[i];

                if(eAxisName == ie.indexName) {
                    dataVersion = boost::lexical_cast<size_t>(ie.indexValue);
                }

                slicer.setStartAndSize(ie.indexName, ie.cdmIndex(pReader_), 1);
            }

            write_(getData_(slicer, loadElement, undef),
                   wdbParameter,
                   placeName,
                   time_to_postgresql_string(validFrom),
                   time_to_postgresql_string(validTo),
                   "height above ground",
                   0,
                   0,
                   dataVersion);
        }
    }
}

void CdmLoader::write_(const Blob & data,
                       const std::string& wdbParameter,
                       const std::string& placeName,
                       const std::string& validFrom,
                       const std::string& validTo,
                       const std::string& wdbLevelName,
                       double levelFrom,
                       double levelTo,
                       size_t dataVersion)
{
	WDB_LOG & log = WDB_LOG::getInstance("wdb.load.netcdf");

    std::string referencetime = time_to_postgresql_string(*pReferenceTime_);

    log.debugStream()<< "data size: "
                    << data.length<<", "
                    << "Saving: "
                    << "placename: " << placeName <<", "
                    << referencetime <<", "
                    << validFrom <<", "
                    <<  validTo <<", "
                    << "levelname: " << wdbLevelName <<", "
                    << "levelfrom: " << levelFrom <<", "
                    << "levelto: " << levelTo <<", "
                    << "paramname: " <<  wdbParameter <<", "
                    << "version: " <<  dataVersion;

    wdbConnection_.write(
                data.data.get(),
                data.length,
                conf_.loading().dataProvider,
                placeName,
                referencetime,
                validFrom,
                validTo,
                wdbParameter,
                wdbLevelName, levelFrom, levelTo, dataVersion, 0);
}

std::string CdmLoader::getPlaceName_(const std::string& varName)
{
    WDB_LOG & log = WDB_LOG::getInstance("wdb.load.netcdf");

    const CDM& cdm = pReader_->getCDM();

    std::string xAxisName = cdm.getHorizontalXAxis(varName);
    std::string yAxisName = cdm.getHorizontalYAxis(varName);
    if(xAxisName.empty() || yAxisName.empty())
        return std::string();

    const CDMDimension& xAxis = cdm.getDimension(xAxisName);
    const CDMDimension& yAxis = cdm.getDimension(yAxisName);;

    int xNum = xAxis.getLength();
    boost::shared_array<float> xValues = pReader_->getData(xAxis.getName())->asFloat();
    float startX = xValues[0];
    float xIncrement = xValues[1] - xValues[0];

    int yNum = yAxis.getLength();
    boost::shared_array<float> yValues = pReader_->getData(yAxis.getName())->asFloat();
    float startY = yValues[0];
    float yIncrement = yValues[1] - yValues[0];

    boost::shared_ptr<const Projection> projections = cdm.getProjectionOf(varName);

    std::string projDefinition = projections->getProj4String();

    try {
        std::string ret = wdbConnection_.getPlaceName(xNum, yNum, xIncrement, yIncrement, startX, startY, projDefinition);
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

            return wdbConnection_.addPlaceDefinition(placeName,
                                                     xNum,
                                                     yNum,
                                                     xIncrement,
                                                     yIncrement,
                                                     startX,
                                                     startY,
                                                     projDefinition);
        } else if(not conf_.loading().placeName.empty()) {
            log.infoStream() << "No existing place name for grid. Using placename " << conf_.loading().placeName;
            return conf_.loading().placeName;
        }

        else throw;
    }
}

std::vector<Time> CdmLoader::getTimes_()
{
    std::vector<Time> ret;

    boost::shared_ptr<MetNoFimex::Data> times = pReader_->getData("time");
    boost::shared_array<double> values = times->asDouble();
    for(size_t i = 0; i < times->size(); ++ i)
        ret.push_back(time_from_seconds_since_epoch(static_cast<long long> (values[i])));
    return ret;
}

std::vector<Time> CdmLoader::getTimes_(const CDM & cdm, const LoadElement & loadElement) const
{
    std::vector<Time> ret;

    std::string tAxisName = cdm.getTimeAxis(loadElement.cfName());

    if ( not tAxisName.empty() )
    {
		boost::shared_ptr<MetNoFimex::Data> times = pReader_->getData(tAxisName);
		boost::shared_array<double> values = times->asDouble();
		for(size_t i = 0; i < times->size(); ++ i)
			ret.push_back(time_from_seconds_since_epoch(static_cast<long long> (values[i])));
    }
    return ret;

}

Time CdmLoader::getReferenceTime_()
{
    using namespace boost::posix_time;
    ptime refTime;
    try {
        refTime = getUniqueForecastReferenceTime(pReader_);
    } catch(std::exception &e) {
        if(conf_.loading().referenceTime.empty()) {
            throw std::runtime_error("Missing unique forecast reference time");
        } else {
            refTime = boost::posix_time::from_iso_string(conf_.loading().referenceTime);

        }
    }

    ptime epoch(boost::gregorian::date(1970,1,1));
    time_duration::sec_type time = (refTime - epoch).total_seconds();
    return time_from_seconds_since_epoch(static_cast<long long>(time));
}

boost::shared_ptr<Data> CdmLoader::getLevels_(const std::string& varName) const
{
    const CDM& cdm = pReader_->getCDM();
    std::string zAxisName = cdm.getVerticalAxis(varName);
    if(not zAxisName.empty()) {
        return pReader_->getData(zAxisName);
    } else {
        boost::shared_ptr<Data>();
    }
}
