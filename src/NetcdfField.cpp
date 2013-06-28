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
#include <fimex/CDMReader.h>
#include <fimex/CDM.h>
#include <fimex/CDMAttribute.h>
#include <fimex/Data.h>
#include <fimex/DataDecl.h>
#include <boost/foreach.hpp>
#include <boost/date_time.hpp>
#include <set>


using namespace MetNoFimex;



namespace
{
typedef std::map<std::string, unsigned> DimensionList;
void buildSlices(std::vector<SliceBuilder> & out,
		DimensionList::const_reverse_iterator start, DimensionList::const_reverse_iterator stop,
		const SliceBuilder & slicer)
{
	if (start == stop)
	{
		out.push_back(slicer);
		return;
	}

	std::string name = start->first;
	unsigned count = start->second;
	++start;
	for (unsigned i = 0; i < count; ++i)
	{
		SliceBuilder newSlicer(slicer);
		newSlicer.setStartAndSize(name, i, 1);
		buildSlices(out, start, stop, newSlicer);
	}
}

void buildIndexList(std::vector<NetcdfField::IndexList> & out,
		DimensionList::const_reverse_iterator start, DimensionList::const_reverse_iterator stop,
		const NetcdfField::IndexList & indexList = NetcdfField::IndexList())
{
	if (start == stop)
	{
		out.push_back(indexList);
		return;
	}

	std::string name = start->first;
	unsigned count = start->second;
	++start;
	for (unsigned i = 0; i < count; ++i)
	{
		NetcdfField::IndexList newIndexList(indexList);
		newIndexList[name] = i;
		buildIndexList(out, start, stop, newIndexList);
	}
}


}

void NetcdfField::get(std::vector<Ptr> & out, const boost::shared_ptr<MetNoFimex::CDMReader> & reader, const NetcdfFile & netcdfFile)
{
	const CDM & cdm = reader->getCDM();

	BOOST_FOREACH( const CDMVariable & variable, cdm.getVariables() )
	{
		const std::string var = variable.getName();

		std::set<std::string> ignoreDimensions;
		ignoreDimensions.insert(cdm.getHorizontalXAxis(var));
		ignoreDimensions.insert(cdm.getHorizontalYAxis(var));
		//ignoreDimensions.insert(cdm.getTimeAxis(var));

		DimensionList dimensions;
		BOOST_FOREACH(const std::string & dimension, variable.getShape())
			if ( ignoreDimensions.find(dimension) == ignoreDimensions.end() )
			{
				unsigned count = cdm.getDimension(dimension).getLength();
				dimensions[dimension] = count;
			}


		std::vector<SliceBuilder> slicers;
		buildSlices(slicers, dimensions.rbegin(), dimensions.rend(), SliceBuilder(cdm, var));

		std::vector<IndexList> indexLists;
		buildIndexList(indexLists, dimensions.rbegin(), dimensions.rend());

		BOOST_FOREACH(const IndexList & indexList, indexLists)
		{
			Ptr ret(new NetcdfField(indexList, netcdfFile));
			ret->reader_ = reader;
			ret->variableName_ = var;
			out.push_back(ret);
		}
	}
}


const boost::shared_array<float> & NetcdfField::data() const
{
	if ( ! data_ )
	{
		DataPtr data = reader_->getScaledDataSlice(variableName_, getSliceBuilder_());
		data_ = data->asFloat();
	}
	return data_;
}

unsigned NetcdfField::dataSize() const
{
	unsigned ret = 1;
	const SliceBuilder & sliceBuilder = getSliceBuilder_();
	BOOST_FOREACH(size_t size, sliceBuilder.getDimensionSizes())
		ret *= size;

	return ret;
}

std::string NetcdfField::str() const
{
	std::ostringstream s;

	s << variableName_ << '(' << dataSize() << ") ";// << slicer_;

	return s.str();
}

boost::shared_ptr<GridGeometry> NetcdfField::placeSpecification() const
{
    const CDM & cdm = reader_->getCDM();

    std::string xAxisName = cdm.getHorizontalXAxis(variableName_);
    std::string yAxisName = cdm.getHorizontalYAxis(variableName_);
    if(xAxisName.empty() || yAxisName.empty())
        return boost::shared_ptr<GridGeometry>();

    const CDMDimension& xAxis = cdm.getDimension(xAxisName);
    const CDMDimension& yAxis = cdm.getDimension(yAxisName);;

    int xNum = xAxis.getLength();
    boost::shared_array<float> xValues = reader_->getData(xAxis.getName())->asFloat();
    float startX = xValues[0];
    float xIncrement = xValues[1] - xValues[0];

    int yNum = yAxis.getLength();
    boost::shared_array<float> yValues = reader_->getData(yAxis.getName())->asFloat();
    float startY = yValues[0];
    float yIncrement = yValues[1] - yValues[0];

    boost::shared_ptr<const Projection> projections = cdm.getProjectionOf(variableName_);

    std::string projDefinition = projections->getProj4String();

    return boost::shared_ptr<GridGeometry>( new
    		GridGeometry(projDefinition, GridGeometry::LeftLowerHorizontal, xNum, yNum, xIncrement, yIncrement, startX, startY));
}

Time NetcdfField::validtime() const
{
	const CDM & cdm = reader_->getCDM();
	std::string timeAxis = cdm.getTimeAxis(variableName_);

	IndexList::const_iterator find = indexList_.find(timeAxis);
	if ( find == indexList_.end() )
		return netcdfFile_.getReferenceTime();
	DataPtr data = reader_->getData(timeAxis);
	double time = data->asDouble()[find->second];

	return time_from_seconds_since_epoch(static_cast<long long> (time));
}

unsigned NetcdfField::dataVersion() const
{
	const CDMDimension * epsDimension = 0;
	const CDM & cdm = reader_->getCDM();
	BOOST_FOREACH ( const CDMDimension & dimension, cdm.getDimensions() )
	{
		try
		{
			std::string standard_name = cdm.getAttribute(dimension.getName(), "standard_name").getStringValue();
			if ( standard_name == "realization" )
			{
				epsDimension = & dimension;
				break;
			}
		}
		catch ( CDMException & )
		{
			// ignore dimension
		}
	}
	if ( ! epsDimension )
		return 0;



	IndexList::const_iterator find = indexList_.find(epsDimension->getName());
	if ( find == indexList_.end() )
		return 0;
	DataPtr data = reader_->getData(epsDimension->getName());
	return data->asInt()[find->second];
}

unsigned NetcdfField::maxDataVersion() const
{
	const CDMDimension * epsDimension = 0;
	const CDM & cdm = reader_->getCDM();
	BOOST_FOREACH ( const CDMDimension & dimension, cdm.getDimensions() )
	{
		try
		{
			std::string standard_name = cdm.getAttribute(dimension.getName(), "standard_name").getStringValue();
			if ( standard_name == "realization" )
			{
				epsDimension = & dimension;
				break;
			}
		}
		catch ( CDMException & )
		{
			// ignore dimension
		}
	}
	if ( ! epsDimension )
		return 0;

	IndexList::const_iterator find = indexList_.find(epsDimension->getName());
	if ( find == indexList_.end() )
		return 0;
	DataPtr data = reader_->getData(epsDimension->getName());
	return data->asInt()[data->size() -1];
}

NetcdfField::NetcdfField(const IndexList & indexList, const NetcdfFile & netcdfFile) :
		indexList_(indexList),
		netcdfFile_(netcdfFile)
{
}

NetcdfField::~NetcdfField()
{
}

SliceBuilder NetcdfField::getSliceBuilder_() const
{
	SliceBuilder builder(reader_->getCDM(), variableName_);
	for ( IndexList::const_iterator it = indexList_.begin(); it != indexList_.end(); ++ it )
		builder.setStartAndSize(it->first, it->second, 1);
	return builder;
}
