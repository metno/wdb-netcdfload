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


class NetcdfField : boost::noncopyable
{
public:
	~NetcdfField();

	typedef boost::shared_ptr<NetcdfField> Ptr;


	const boost::shared_array<float> & data() const;
	unsigned dataSize() const;

	std::string str() const;

	boost::shared_ptr<GridGeometry> placeSpecification() const;

	Time validtime() const;

	const std::string & variableName() const
	{
		return variableName_;
	}

	unsigned dataVersion() const;
	unsigned maxDataVersion() const;

	typedef std::map<std::string, unsigned> IndexList;

private:

	friend class NetcdfFile;

	static void get(std::vector<Ptr> & out, const boost::shared_ptr<MetNoFimex::CDMReader> & reader, const NetcdfFile & netcdfFile);

	NetcdfField(const IndexList & indexList, const NetcdfFile & netcdfFile);
	NetcdfField(); // undefined

	MetNoFimex::SliceBuilder getSliceBuilder_() const;

	const NetcdfFile & netcdfFile_;
	boost::shared_ptr<MetNoFimex::CDMReader> reader_;
	std::string variableName_;


	IndexList indexList_;

	mutable boost::shared_array<float> data_;
};

#endif /* NETCDFFIELD_H_ */
