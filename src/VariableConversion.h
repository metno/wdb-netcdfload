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

#ifndef VARIABLECONVERSION_H_
#define VARIABLECONVERSION_H_

#include "AbstractNetcdfField.h"
#include "configuration/Conversions.h"
#include <boost/noncopyable.hpp>
#include <vector>
#include <string>
#include <map>
#include <set>


/**
 * Converting netcdf variables into other "virtual" parameters. This class
 * keeps state, so you need to create a new instance of this class for each
 * netcdf file you load.
 *
 * Currently, only conversions from vectors (u,v) to speed and direction are
 * suported.
 */
class VariableConversion : boost::noncopyable
{
public:
	explicit VariableConversion(const std::vector<VectorConversion> & vectorConversions);
	~VariableConversion();

	/**
	 * Check a field for conversions. If any conversions need the given input
	 * field, and this function has been called for all needed input fields, a
	 * list of converted fields is returned.
	 */
	std::vector<AbstractNetcdfField::Ptr> add(AbstractNetcdfField::Ptr inputElement);

private:
	std::multimap<std::string, AbstractNetcdfField::Ptr> fields_;

	std::set<std::string> xVectors_;
	std::set<std::string> yVectors_;
};

#endif /* VARIABLECONVERSION_H_ */
