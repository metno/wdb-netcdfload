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
    pReader_ = CDMFileReaderFactory::create(MIFI_FILETYPE_NETCDF, file);
    pReferenceTime_ = boost::shared_ptr<Time>(new Time(getReferenceTime_()));
    std::cerr << time_to_postgresql_string(getReferenceTime_()) << std::endl;
    timeAxis_ = getTimes_();

    write_();
}

void CdmLoader::write_()
{
//    WDB_LOG & log = WDB_LOG::getInstance("wdb.load.netcdf");

    for(LoadConfiguration::load_const_iterator cit = loadConfiguration_.load_begin(); cit != loadConfiguration_.load_end(); ++cit) {
        LoadElement element = *cit;
        write_(element);
    }
}


void CdmLoader::write_(LoadElement& loadElement)
{
    WDB_LOG & log = WDB_LOG::getInstance("wdb.load.netcdf");

    std::string placeName = getPlaceName_(loadElement.cfName());
    if(placeName.empty()) {
        std::cerr << "no placename for cfname: " << loadElement.cfName() << std::endl;
        return;
    }

    const CDM& cdm = pReader_->getCDM();

    const DataSpecification& specification = loadElement.wdbDataSpecification();
    std::string wdbParameter = specification.wdbParameter();

    std::string tAxisName = cdm.getTimeAxis(loadElement.cfName());
    if(tAxisName.empty()) {
        std::cerr << "no time axis for cfname: " << loadElement.cfName() << std::endl;
        return;
    }

    // check vertical axis
    std::string zAxisName = cdm.getVerticalAxis(loadElement.cfName());
    if(zAxisName.empty()) {
        std::cerr << "no level for cfname: " << loadElement.cfName() << std::endl;
        return;
    } else {
        if(loadElement.indiceKeys().find(zAxisName) == loadElement.indiceKeys().end())
            loadElement.expandIndicePermutations(pReader_, zAxisName);
    }

    // check if CDM modell has eps axis at all
    // based on the non-standardized "realization"
    std::string eAxisName;
    std::string epsDimStandardName = std::string("realization");
    if(not cdm.findVariables("standard_name", epsDimStandardName).empty()) {
        eAxisName = cdm.findVariables("standard_name", epsDimStandardName)[0];
    }

    if(not eAxisName.empty()) {
        // see if variable has ensemble in its shape
        const CDMVariable& var = cdm.getVariable(loadElement.cfName());
        const std::vector<std::string>& shape = var.getShape();
        if(std::find(shape.begin(), shape.end(), eAxisName) == shape.end()) {
            std::cerr << "not eps dependent: " << loadElement.cfName() << std::endl;
        } else {
            if(loadElement.indiceKeys().find(zAxisName) == loadElement.indiceKeys().end())
                loadElement.expandIndicePermutations(pReader_, eAxisName);
        }
    } else {
        std::cerr << " eps axis not existing " << std::endl;
    }

    loadElement.removeNotToLoadPermutations();

    LoadConfiguration::axis_iterator it = loadConfiguration_.findAxisByCfName(zAxisName);
    if(it == loadConfiguration_.axis_end()) {
        std::cerr << "no wdbname for level: " << zAxisName << std::endl;
        return;
    }

    std::string wdbLevelName = it->wdbDataSpecification().wdbParameter();

    boost::shared_ptr<Data> levels = pReader_->getData(zAxisName);
    if(levels.get() == 0 or levels->size() == 0)
        return;

    const CDMDimension* unlimitedDimension = cdm.getUnlimitedDim();

    float undef = std::numeric_limits<float>::quiet_NaN();
    CDMAttribute fillValue;
    if(cdm.getAttribute(loadElement.cfName(), "_FillValue", fillValue))
        undef = fillValue.getData()->asFloat()[0];

    for(size_t t = 0; t < timeAxis_.size(); ++t)
    {
        std::clog << "Loading valid time " << time_to_postgresql_string(timeAxis_[t]) << std::endl;

        Time validFrom = specification.validTimeFrom().getTime(*pReferenceTime_, timeAxis_[t]);
        Time validTo = timeAxis_[t];

        SliceBuilder slicer(cdm, loadElement.cfName());
        if(unlimitedDimension)
            slicer.setStartAndSize(unlimitedDimension->getName(), t, 1);

        for(size_t permutationindex = 0; permutationindex < loadElement.permutations().size(); ++permutationindex)
        {
            const std::vector<LoadElement::IndexElement>& permutation = loadElement.permutations()[permutationindex];

            size_t dataVersion = 0;
            boost::shared_ptr<double> pCurrentLevelValue;

            for(size_t i = 0; i < permutation.size(); ++i)
            {
                const LoadElement::IndexElement& ie = permutation[i];

                if(zAxisName == ie.indexName) {
                    pCurrentLevelValue = boost::shared_ptr<double>(new double (boost::lexical_cast<double>(ie.indexValue)));
                }

                if(eAxisName == ie.indexName) {
                    dataVersion = boost::lexical_cast<size_t>(ie.indexValue);
                }

                slicer.setStartAndSize(ie.indexName, ie.cdmIndex(pReader_), 1);
            }

            if(pCurrentLevelValue.get() == 0)
                throw;

            double levelFrom = *pCurrentLevelValue;
            double levelTo = *pCurrentLevelValue;

            std::clog << "Loading level: " << *pCurrentLevelValue << std::endl;

            const boost::shared_ptr<Data>& data = pReader_->getScaledDataSliceInUnit(loadElement.cfName(), loadElement.wdbDataSpecification().wdbUnits(), slicer);

            write_(data,
                   wdbParameter,
                   placeName,
                   time_to_postgresql_string(validFrom),
                   time_to_postgresql_string(validTo),
                   wdbLevelName,
                   levelFrom,
                   levelTo,
                   dataVersion);
        }
    }
}

void CdmLoader::write_(const boost::shared_ptr<MetNoFimex::Data>& data,
                       const std::string& wdbParameter,
                       const std::string& placeName,
                       const std::string& validFrom,
                       const std::string& validTo,
                       const std::string& wdbLevelName,
                       double levelFrom,
                       double levelTo,
                       size_t dataVersion)
{
    std::string referencetime = time_to_postgresql_string(*pReferenceTime_);

    boost::shared_array<double> values = data->asDouble();

    std::clog       << "data size: "
                    << data->size()<<", "
                    << "Saving: "
                    << "placename: " << placeName <<", "
                    << referencetime <<", "
                    << validFrom <<", "
                    <<  validTo <<", "
                    << "levelname: " << wdbLevelName <<", "
                    << "levelfrom: " << levelFrom <<", "
                    << "levelto: " << levelTo <<", "
                    << "paramname: " <<  wdbParameter <<", "
                    << "version: " <<  dataVersion <<", "
                    << std::endl;

    wdbConnection_.write(
                values.get(),
                data->size(),
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
//        throw std::runtime_error("Unable to locate grid definition in netcdf file");

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
            std::cerr << "Saving data using placename: " << conf_.loading().placeName << " even if data is identfied as " << ret << std::endl;
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
