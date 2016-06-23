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

#include "NetcdfField.h"
#include "NetcdfFile.h"
#include "DataRetriever.h"
#include "configuration/LoadElement.h"
#include "configuration/parameter/DataSpecification.h"
#include <wdbLogHandler.h>
#include <wdb/errors.h>
#include <fimex/CDMReader.h>
#include <fimex/CDM.h>
#include <fimex/CDMAttribute.h>
#include <fimex/SliceBuilder.h>
#include <fimex/Data.h>
#include <fimex/DataDecl.h>
#include <boost/foreach.hpp>
#include <boost/date_time.hpp>
#include <set>
#include <sstream>
#include <iomanip>


using namespace MetNoFimex;


NetcdfField::NetcdfField(const NetcdfFile & netcdfFile, boost::shared_ptr<MetNoFimex::CDMReader> reader, const std::string & variableName) :
		netcdfFile_(netcdfFile),
		reader_(reader),
		variableName_(variableName)
{
	const CDM & cdm = reader->getCDM();

	const CDMVariable & variable = cdm.getVariable(variableName);
	BOOST_FOREACH(const std::string & dimension, variable.getShape())
	{
		unsigned count = cdm.getDimension(dimension).getLength();
		indexList_[dimension] = count;
	}
}

NetcdfField::~NetcdfField()
{
}

NetcdfField::IndexList NetcdfField::unHandledIndexes() const
{
	IndexList ret;

	BOOST_FOREACH(const IndexList::value_type & val, indexList_)
	{
		if ( not canHandleIndex(val.first) )
		{
			const CDM & cdm = reader_->getCDM();
			const CDMDimension & dimension = cdm.getDimension(val.first);
			if ( dimension.getLength() > 1 )
				ret.insert(val);
		}
	}
	return ret;
}

bool NetcdfField::canHandleIndex(const std::string & name) const
{
	return name == timeDimension() or
			name == realizationDimension() or
			name == xDimension() or
			name == yDimension();
}

double NetcdfField::indexValue(std::string dimension, unsigned index) const
{
	IndexList::const_iterator find = indexList_.find(dimension);
	if ( find == indexList_.end() )
		throw wdb::load::LoadError(wdb::load::ErrorWhenReadingFile, dimension + " does not exist for variable " + variableName_);
	if ( find->second <= index )
	{
		std::ostringstream s;
		s << "Invalid index for dimension " << dimension << ": " << index;
		throw wdb::load::LoadError(wdb::load::ErrorWhenReadingFile, s.str());
	}

	boost::shared_array<double> & values = dimensionValues_[dimension];
	if ( ! values )
	{
		DataPtr data = reader_->getData(dimension);
		values = data->asDouble();
	}
	return values[index];
}

std::vector<Time> NetcdfField::times() const
{
	std::vector<Time> ret;
	std::string dim = timeDimension();
	if ( not dim.empty() )
	{
		DataPtr data = reader_->getScaledDataInUnit(dim, "seconds since 1970-01-01 00:00:00+00");

		boost::shared_array<long long> values = data->asInt64();

		size_t numberOfValues = data->size();
		for ( size_t i = 0; i < numberOfValues; ++ i )
			ret.push_back(time_from_seconds_since_epoch(values[i]));
	}
	else
		ret.push_back(INVALID_TIME);

	return ret;
}

std::vector<int> NetcdfField::realizations() const
{
	std::vector<int> ret;

	std::string dim = realizationDimension();
	if ( dim.empty() )
		ret.push_back(0);
	else
	{
		DataPtr data = reader_->getData(dim);
		boost::shared_array<int> values = data->asInt();
		std::copy(values.get(), values.get() + data->size(), std::back_inserter(ret));
	}

	return ret;
}

std::string NetcdfField::timeDimension() const
{
    const CDM & cdm = reader_->getCDM();
	return cdm.getTimeAxis(variableName_);
}

std::string NetcdfField::realizationDimension() const
{
    const CDM & cdm = reader_->getCDM();

    for ( IndexList::const_iterator it = indexList_.begin(); it != indexList_.end(); ++ it )
    {
    	CDMAttribute attribute;
    	if ( cdm.getAttribute(it->first, "standard_name", attribute) )
    		if ( attribute.getStringValue() == "realization" )
    			return it->first;
    }

    return std::string();
}

std::string NetcdfField::xDimension() const
{
    const CDM & cdm = reader_->getCDM();
	return cdm.getHorizontalXAxis(variableName_);
}

std::string NetcdfField::yDimension() const
{
    const CDM & cdm = reader_->getCDM();
	return cdm.getHorizontalYAxis(variableName_);
}

std::string NetcdfField::attribute(const std::string & name) const
{
    const CDM & cdm = reader_->getCDM();

	CDMAttribute attribute;
	if ( cdm.getAttribute(variableName_, name, attribute) )
		return attribute.getStringValue();

	return std::string();
}

Time NetcdfField::referenceTime() const
{
	return netcdfFile_.referenceTime();
}

boost::shared_ptr<GridGeometry> NetcdfField::placeSpecification() const
{
    const CDM & cdm = reader_->getCDM();

    std::string xAxisName = cdm.getHorizontalXAxis(variableName_);
    std::string yAxisName = cdm.getHorizontalYAxis(variableName_);
    if(xAxisName.empty() || yAxisName.empty())
        return boost::shared_ptr<GridGeometry>();

    const CDMDimension& xAxis = cdm.getDimension(xAxisName);
    const CDMDimension& yAxis = cdm.getDimension(yAxisName);

    int xNum = xAxis.getLength();
    boost::shared_array<float> xValues = reader_->getData(xAxis.getName())->asFloat();
    float startX = xValues[0];
    float xIncrement = xValues[1] - xValues[0];

    int yNum = yAxis.getLength();
    boost::shared_array<float> yValues = reader_->getData(yAxis.getName())->asFloat();
    float startY = yValues[0];
    float yIncrement = yValues[1] - yValues[0];

    boost::shared_ptr<const Projection> projections = cdm.getProjectionOf(variableName_);

    if ( projections )
    {
		std::string projDefinition = projections->getProj4String();

		return boost::shared_ptr<GridGeometry>( new
				GridGeometry(projDefinition, GridGeometry::LeftLowerHorizontal, xNum, yNum, xIncrement, yIncrement, startX, startY));
    }
    return boost::shared_ptr<GridGeometry>();
}

boost::shared_ptr<AbstractDataRetriever> NetcdfField::retriever(
		const LoadElement & loadElement,
		unsigned timeIndex, unsigned realizationIndex,
		const DataSpecification & querySpec) const
{
	return boost::shared_ptr<AbstractDataRetriever>(new DataRetriever(loadElement, * this, reader_, timeIndex, realizationIndex, querySpec));
}
