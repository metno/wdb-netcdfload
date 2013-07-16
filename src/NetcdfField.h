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

#ifndef NETCDFFIELD_H_
#define NETCDFFIELD_H_

#include "localtime.h"
#include <GridGeometry.h>
#include <fimex/SliceBuilder.h>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/noncopyable.hpp>
#include <string>
#include <vector>
#include <map>


namespace MetNoFimex
{
class CDMReader;
}
class NetcdfFile;

/**
 * Part of a variable in a NetcdfFile, extracted to hold exactly one valid time and dataversion.
 */
class NetcdfField : boost::noncopyable
{
public:

	~NetcdfField();

	typedef boost::shared_ptr<NetcdfField> Ptr;

	const std::string & variableName() const
	{
		return variableName_;
	}

	typedef std::map<std::string, unsigned> IndexList;

	/**
	 * Get a list of all indexes with their sizes
	 */
	const IndexList & indexes() const { return indexList_; }

	IndexList unHandledIndexes() const;

	/**
	 * Does this object have a built-in handling for this index?
	 */
	bool canHandleIndex(const std::string & name) const;

	/**
	 * Get the variable value of an index i a dimension, converted to double
	 */
	double indexValue(std::string dimension, unsigned index) const;

	std::vector<Time> times() const;
	std::vector<int> realizations() const;

	std::string timeDimension() const;
	std::string realizationDimension() const;
	std::string xDimension() const;
	std::string yDimension() const;


	/**
	 * Get the reference time for the this variable
	 */
	Time referenceTime() const;

	boost::shared_ptr<GridGeometry> placeSpecification() const;

private:

	friend class NetcdfFile;

	NetcdfField(const NetcdfFile & netcdfFile, boost::shared_ptr<MetNoFimex::CDMReader> reader, const std::string & variableName);
	NetcdfField(); // undefined

	const NetcdfFile & netcdfFile_;
	boost::shared_ptr<MetNoFimex::CDMReader> reader_;
	std::string variableName_;
	IndexList indexList_;
	mutable std::map<std::string, boost::shared_array<double> > dimensionValues_;
};

#endif /* NETCDFFIELD_H_ */
