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

#include "NetcdfFile.h"
#include "NetcdfField.h"
#include "ForceConvertingNetcdfField.h"
#include "DirectionConvertingNetcdfField.h"
#include "VariableConversion.h"
#include <fimex/CDMFileReaderFactory.h>
#include <fimex/CDMReaderUtils.h>
#include <fimex/CDM.h>
#include <fimex/CDMDimension.h>
#include <fimex/CDMVariable.h>
#include <fimex/CDMInterpolator.h>
#include <boost/date_time.hpp>
#include <boost/foreach.hpp>


using namespace MetNoFimex;


NetcdfFile::NetcdfFile(const std::string & fileName, const std::string & configurationFile, const std::string & fileType, const std::vector<VectorConversion> & conversions) :
	referenceTime_(INVALID_TIME),
	conversions_(conversions)
{
    reader_ = MetNoFimex::CDMFileReaderFactory::create(fileType, fileName, configurationFile);

    boost::posix_time::ptime time = getUniqueForecastReferenceTime(reader_);
	static boost::local_time::time_zone_ptr zone(new boost::local_time::posix_time_zone("+00"));
	referenceTime_ = Time(time, zone);
}

NetcdfFile::~NetcdfFile()
{
}


namespace
{
class FieldAdapter
{
public:
	template<typename Field>
	AbstractNetcdfField::Ptr get(const CDMVariable & var, AbstractNetcdfField::Ptr field, const std::string & name)
	{
		AbstractNetcdfField::Ptr & ret = fields_[name];
		Field * f;
		if ( ! ret )
		{
			f = new Field(name);
			ret = AbstractNetcdfField::Ptr(f);
		}
		else
			f = dynamic_cast<Field *>(ret.get());

		if ( var.getSpatialVectorDirection()[0] == 'x' )
			f->setX(field);
		else
			f->setY(field);

		if ( f->ready() )
			return ret;

		return AbstractNetcdfField::Ptr();
	}

private:
	std::map<std::string, AbstractNetcdfField::Ptr> fields_;
};

}

std::vector<AbstractNetcdfField::Ptr> NetcdfFile::getFields() const
{
	std::vector<AbstractNetcdfField::Ptr> ret;

	VariableConversion conversions(conversions_);

	const CDM & cdm = reader_->getCDM();

	const CDM::DimVec & dims = cdm.getDimensions();
	std::set<std::string> dimensions;
	BOOST_FOREACH(const CDMDimension & dimension, dims)
		dimensions.insert(dimension.getName());

//	std::map<std::string, AbstractNetcdfField::Ptr> fields;
	FieldAdapter adapter;

	BOOST_FOREACH(const CDMVariable & var, cdm.getVariables())
	{
		std::string variableName = var.getName();

		if ( dimensions.find(variableName) != dimensions.end() )
			continue;

		AbstractNetcdfField::Ptr field(new NetcdfField(* this, reader_, variableName));
		ret.push_back(field);

#define NEW_CONVERSION
#ifdef NEW_CONVERSION
		BOOST_FOREACH( AbstractNetcdfField::Ptr p, conversions.add(field) )
			ret.push_back(p);
#else
		if ( var.isSpatialVector() )
		{
			//const std::string & other = var.getSpatialVectorCounterpart();

			AbstractNetcdfField::Ptr p;
			p = adapter.get<ForceConvertingNetcdfField>(var, field, "wind_speed");
			if ( p )
				ret.push_back(p);
			p = adapter.get<DirectionConvertingNetcdfField>(var, field, "wind_from_direction");
			if ( p )
				ret.push_back(p);
		}
#endif
	}

	return ret;
}

AbstractNetcdfField::Ptr NetcdfFile::getField(const std::string & variableName) const
{
	BOOST_FOREACH(const AbstractNetcdfField::Ptr & field, getFields())
	{
		if ( field->variableName() == variableName )
			return field;
	}
	throw std::runtime_error(variableName + ": no such variable found");
}

void NetcdfFile::setPointFilter(double longitude, double latitude)
{
	std::vector<double> lon(1, longitude);
	std::vector<double> lat(1, latitude);

	CDMInterpolator * interpolator = new CDMInterpolator(reader_);
	interpolator->changeProjection(MIFI_INTERPOL_BILINEAR, lon, lat);

	reader_ = boost::shared_ptr<MetNoFimex::CDMReader>(interpolator);
}
