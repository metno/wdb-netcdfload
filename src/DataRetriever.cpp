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

#include "DataRetriever.h"
#include "AbstractNetcdfField.h"
#include <wdbLogHandler.h>
#include <fimex/CDMReader.h>
#include <fimex/SliceBuilder.h>
#include <fimex/Data.h>
#include <boost/foreach.hpp>


DataRetriever::DataRetriever(const LoadElement & loadElement, const AbstractNetcdfField & field, boost::shared_ptr<MetNoFimex::CDMReader> reader, unsigned timeIndex, unsigned realizationIndex,
		const DataSpecification & querySpec) :
	loadElement_(loadElement),
	field_(field),
	reader_(reader),
	timeIndex_(timeIndex),
	realizationIndex_(realizationIndex),
	querySpec_(querySpec)
{
}

RawData DataRetriever::operator() () const
{
	WDB_LOG & log = WDB_LOG::getInstance( "wdb.netcdfload.get_data" );

	RawData ret;

	MetNoFimex::SliceBuilder slicer(reader_->getCDM(), field_.variableName());

	const std::string & timeDimension = field_.timeDimension();
	if ( not timeDimension.empty() )
		slicer.setStartAndSize(timeDimension, timeIndex_, 1);

	const std::string & realizationDimension = field_.realizationDimension();
	if ( not realizationDimension.empty() )
		slicer.setStartAndSize(realizationDimension, realizationIndex_, 1);

	BOOST_FOREACH(const LoadElement::IndexNameToValue::value_type & nameValue, loadElement_.indicesToLoad())
	{
		unsigned index = loadElement_.cdmIndex(* reader_, nameValue.first, nameValue.second);
		slicer.setStartAndSize(nameValue.first, index, 1);
	}

	MetNoFimex::DataPtr data = readData_(reader_, slicer);
	if ( data )
	{
		ret.numberOfValues = data->size();
		if ( ret.numberOfValues )
			ret.data = data->asFloat();
	}
	else
	{
		ret.numberOfValues = 0;
	}

	if ( querySpec_.scale() != 1 )
		for ( int i = 0; i < ret.numberOfValues; ++ i )
			ret.data[i] *= querySpec_.scale();

	return ret;
}

MetNoFimex::DataPtr DataRetriever::readData_(boost::shared_ptr<MetNoFimex::CDMReader> reader, const MetNoFimex::SliceBuilder & slicer) const
{
	WDB_LOG & log = WDB_LOG::getInstance( "wdb.netcdfload.get_data" );
	try
	{
		try
		{
			return reader->getScaledDataSliceInUnit(field_.variableName(), querySpec_.wdbUnits(), slicer);
		}
		catch ( std::exception & e )
		{
			if ( querySpec_.alternativeUnitConversion().empty() )
				throw;
		}
		return reader->getScaledDataSliceInUnit(field_.variableName(), querySpec_.alternativeUnitConversion(), slicer);
	}
	catch ( std::exception & e )
	{
		static std::set<std::string> parametersWarnedAbout;
		if ( parametersWarnedAbout.find(querySpec_.wdbParameter()) == parametersWarnedAbout.end() )
		{
			log.errorStream() << e.what();
			log.warnStream() << "Not loading parameter <" << querySpec_.wdbParameter() << '>';
			parametersWarnedAbout.insert(querySpec_.wdbParameter());
		}
	}
	return MetNoFimex::DataPtr();
}

