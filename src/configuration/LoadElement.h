/*
 netcdfLoad

 Copyright (C) 2011 met.no

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

#ifndef LOADELEMENT_H_
#define LOADELEMENT_H_

// project
#include "parameter/NetcdfParameterSpecification.h"
#include "parameter/DataSpecification.h"

// boost
#include <boost/shared_ptr.hpp>

// std
#include <map>
#include <set>
#include <string>
#include <vector>
#include <iosfwd>

extern "C"
{
    typedef struct _xmlNode xmlNode;
    typedef xmlNode* xmlNodePtr;
}

namespace MetNoFimex
{
    class CDMReader;
}

class LoadElement
{
public:
    explicit LoadElement(xmlNodePtr loadNode);
    LoadElement(const std::string & variableName, const DataSpecification & wdbDataSpecification);
    ~LoadElement();

    const std::string & variableName() const { return netcdfParameterSpecification_.variableName(); }
    const std::string & standardName() const { return netcdfParameterSpecification_.standardName(); }

    const NetcdfParameterSpecification & netcdfParameterSpecification() const { return netcdfParameterSpecification_; }
    const DataSpecification & wdbDataSpecification() const { return wdbDataSpecification_; }

//    typedef std::map<std::string, double> IndexNameToValue;
//    const IndexNameToValue & indicesToLoad() const { return netcdfParameterSpecification_.indicesToLoad(); }

//    unsigned cdmIndex(MetNoFimex::CDMReader & reader, const std::string & dimensionName, double dimensionValue) const
//    {
//    	return netcdfParameterSpecification_.cdmIndex(reader, dimensionName, dimensionValue);
//    }

private:
    void addWdbSpec_(xmlNodePtr wdbNode);
    DataSpecification::Level getWdbLevelSpec_(xmlNodePtr levelNode);

    NetcdfParameterSpecification netcdfParameterSpecification_;
    DataSpecification wdbDataSpecification_;
};


std::ostream & operator << (std::ostream & s, const LoadElement & loadElement);


#endif /* LOADELEMENT_H_ */
