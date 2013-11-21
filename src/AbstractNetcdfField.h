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


#ifndef ABSTRACTNETCDFFIELD_H_
#define ABSTRACTNETCDFFIELD_H_

#include "localtime.h"
#include <GridGeometry.h>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <string>
#include <map>
#include <vector>


namespace MetNoFimex
{
class CDMReader;
}
class AbstractDataRetriever;
class DataSpecification;
class LoadElement;

class AbstractNetcdfField : boost::noncopyable
{
public:

	virtual ~AbstractNetcdfField() {}

	typedef boost::shared_ptr<AbstractNetcdfField> Ptr;

	virtual const std::string & variableName() const = 0;

	typedef std::map<std::string, unsigned> IndexList;

	/**
	 * Get a list of all indexes with their sizes
	 */
	virtual const IndexList & indexes() const = 0;

	virtual IndexList unHandledIndexes() const = 0;

	/**
	 * Does this object have a built-in handling for this index?
	 */
	virtual bool canHandleIndex(const std::string & name) const = 0;

	/**
	 * Get the variable value of an index i a dimension, converted to double
	 */
	virtual double indexValue(std::string dimension, unsigned index) const = 0;

	virtual std::vector<Time> times() const = 0;
	virtual std::vector<int> realizations() const = 0;

	virtual std::string timeDimension() const = 0;
	virtual std::string realizationDimension() const = 0;
	virtual std::string xDimension() const = 0;
	virtual std::string yDimension() const = 0;

	virtual std::string attribute(const std::string & name) const = 0;

	/**
	 * Get the reference time for the this variable
	 */
	virtual Time referenceTime() const = 0;

	virtual boost::shared_ptr<GridGeometry> placeSpecification() const = 0;

	virtual boost::shared_ptr<AbstractDataRetriever> retriever(
			const LoadElement & loadElement,
			unsigned timeIndex, unsigned realizationIndex,
			const DataSpecification & querySpec) const = 0;
};



#endif /* ABSTRACTNETCDFFIELD_H_ */
