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

#include "NetcdfTranslator.h"
#include "AbstractDataRetriever.h"
#include <AbstractNetcdfField.h>
#include <wdbLogHandler.h>
#include <fimex/CDMReader.h>
#include <fimex/Data.h>
#include <boost/foreach.hpp>


NetcdfTranslator::NetcdfTranslator(const CdmLoaderConfiguration & conf) :
	conf_(conf),
	loadConfiguration_(conf.loadConfiguration())
{
}

NetcdfTranslator::~NetcdfTranslator()
{
}

std::vector<WriteQuery> NetcdfTranslator::queries(const AbstractNetcdfField & field) const
{
	std::vector<WriteQuery> ret;

	const std::vector<LoadElement> & loadElements = loadConfiguration_.getLoadElement(field);
	BOOST_FOREACH( const LoadElement & loadElement, filter_(loadElements) )
	{
		const DataSpecification & querySpec = loadElement.wdbDataSpecification();

		WriteQuery base;
		base.dataProvider(conf_.loading().dataProvider);
		base.referenceTime(field.referenceTime());
		setLocation_(base, field);

		const std::vector<int> & realizations = field.realizations();
		base.maxDataVersion(* std::max_element(realizations.begin(), realizations.end()));

		base.valueParameterName(querySpec.wdbParameter());

		base.levelParameterName(querySpec.level().name());
		base.levelFrom(querySpec.level().value());
		base.levelTo(querySpec.level().value());

		const std::vector<Time> & times = field.times();
		for ( unsigned timeIndex = 0; timeIndex < times.size(); ++ timeIndex )
		{
			base.validTimeFrom(querySpec.validTimeFrom().getTime(field.referenceTime(), times[timeIndex]));
			base.validTimeTo(querySpec.validTimeTo().getTime(field.referenceTime(), times[timeIndex]));

			BOOST_FOREACH(int realization, realizations)
			{
				base.dataVersion(realization);

				boost::shared_ptr<AbstractDataRetriever> retriever =
						field.retriever(loadElement, timeIndex, realization, querySpec);

				base.data(retriever);

				ret.push_back(base);
			}
		}
	}
	return ret;
}

std::vector<WriteQuery> NetcdfTranslator::queries(const AbstractNetcdfField & field, const std::vector<CdmLoaderConfiguration::Point> & points) const
{
	std::vector<WriteQuery> ret;
	BOOST_FOREACH(const WriteQuery & query, queries(field))
	{
		const RawData & data = query.data();
		for ( int i = 0; i < points.size(); ++ i )
			ret.push_back(adaptQuery_(query, points[i], data.data[i]));
	}
	return ret;
}

void NetcdfTranslator::setLocation_(WriteQuery & out, const AbstractNetcdfField & field) const
{
	if ( not conf_.loading().placeName.empty() )
		out.placeName(conf_.loading().placeName);
	out.location(field.placeSpecification());
	out.loadPlaceDefinition(conf_.loading().loadPlaceDefinition);
}


namespace
{
class IndexedDataRetriever : public AbstractDataRetriever
{
public:
	IndexedDataRetriever(float val) :
		val_(val)
	{}

	virtual RawData operator() () const
	{
		RawData ret;
		ret.numberOfValues = 1;
		ret.data = boost::shared_array<float>(new float[1]);
		ret.data[0] = val_;
		return ret;
	}

private:
	float val_;
};
}

WriteQuery NetcdfTranslator::adaptQuery_(WriteQuery query, const CdmLoaderConfiguration::Point & point, float value) const
{
	query.placeName(point.getPlaceName());
	query.data(AbstractDataRetriever::Ptr(new IndexedDataRetriever(value)));
	return query;
}

std::vector<LoadElement> NetcdfTranslator::filter_(const std::vector<LoadElement> & loadElements) const
{
	const std::vector<NetcdfParameterSpecification> & parameterSpec = conf_.elementsToLoad();

	if ( parameterSpec.empty() )
		return loadElements;

	std::vector<LoadElement> ret;
	BOOST_FOREACH ( const LoadElement & loadElement, loadElements )
	{
		BOOST_FOREACH ( const NetcdfParameterSpecification & spec, parameterSpec )
		{
			if ( loadElement.variableName() == spec.variableName() )
			{
				if ( spec.indicesToLoad().empty())
					ret.push_back(loadElement);
				else if ( loadElement.netcdfParameterSpecification().indicesToLoad() == spec.indicesToLoad() )
					ret.push_back(loadElement);
			}
		}
	}

	return ret;
}
