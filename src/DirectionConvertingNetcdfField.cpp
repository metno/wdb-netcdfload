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

#include "DirectionConvertingNetcdfField.h"
#include "DataRetriever.h"
#include <cmath>


DirectionConvertingNetcdfField::DirectionConvertingNetcdfField(const std::string & variableName) :
DerivedNetcdfField(variableName, Ptr())
{
}

DirectionConvertingNetcdfField::~DirectionConvertingNetcdfField()
{
}

void DirectionConvertingNetcdfField::setX(AbstractNetcdfField::Ptr xVariable)
{
	source_ = xVariable;
}

void DirectionConvertingNetcdfField::setY(AbstractNetcdfField::Ptr yVariable)
{
	source2_ = yVariable;
}

namespace
{
class DirectionDataRetriever : public AbstractDataRetriever
{
public:
	DirectionDataRetriever(boost::shared_ptr<AbstractDataRetriever> x, boost::shared_ptr<AbstractDataRetriever> y ):
		x_(x), y_(y)
	{}
	virtual RawData operator() () const
	{
		const RawData xData = (*x_)();
		const RawData yData = (*y_)();
		RawData ret;
		ret.numberOfValues = xData.numberOfValues;
		ret.data = boost::shared_array<float>(new float[ret.numberOfValues]);
		for ( unsigned i = 0; i < ret.numberOfValues; ++ i )
		{
			float x = xData.data[i];
			float y = yData.data[i];

			// Rotating 180 degrees to get from direction.
			ret.data[i] = std::atan2(-y, -x);
		}
		return ret;
	}

private:
	boost::shared_ptr<AbstractDataRetriever> x_;
	boost::shared_ptr<AbstractDataRetriever> y_;
};
}

boost::shared_ptr<AbstractDataRetriever> DirectionConvertingNetcdfField::retriever(
		const LoadElement & loadElement,
		unsigned timeIndex, unsigned realizationIndex,
		const DataSpecification & querySpec) const
{
	boost::shared_ptr<AbstractDataRetriever> x = xVariable_()->retriever(loadElement, timeIndex, realizationIndex, querySpec);
	boost::shared_ptr<AbstractDataRetriever> y = yVariable_()->retriever(loadElement, timeIndex, realizationIndex, querySpec);

	return boost::shared_ptr<AbstractDataRetriever>(new DirectionDataRetriever(x, y));
}

