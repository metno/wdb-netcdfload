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

	BOOST_FOREACH( const LoadElement & loadElement, loadConfiguration_.getLoadElement(field) )
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

				//DataRetriever retriever(loadElement, field, timeIndex, realization, querySpec);

				boost::shared_ptr<AbstractDataRetriever> retriever =
						field.retriever(loadElement, timeIndex, realization, querySpec);

				base.data(retriever);

				ret.push_back(base);
			}
		}
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
