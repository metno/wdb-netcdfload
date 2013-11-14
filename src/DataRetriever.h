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


#ifndef DATARETRIEVER_H_
#define DATARETRIEVER_H_

#include "WriteQuery.h"
#include "configuration/LoadElement.h"
#include "configuration/parameter/DataSpecification.h"
#include <fimex/DataDecl.h>
#include <boost/shared_ptr.hpp>

class NetcdfField;

namespace MetNoFimex
{
class CDMReader;
class SliceBuilder;
}


class DataRetriever
{
public:
	DataRetriever(const LoadElement & loadElement, const NetcdfField & field, unsigned timeIndex, unsigned realizationIndex,
			const DataSpecification & querySpec);

	WriteQuery::RawData operator() () const;

private:
	MetNoFimex::DataPtr readData_(boost::shared_ptr<MetNoFimex::CDMReader> reader, const MetNoFimex::SliceBuilder & slicer) const;

	LoadElement loadElement_;
	const NetcdfField & field_;
	unsigned timeIndex_;
	unsigned realizationIndex_;
	DataSpecification querySpec_;
};




#endif /* DATARETRIEVER_H_ */
