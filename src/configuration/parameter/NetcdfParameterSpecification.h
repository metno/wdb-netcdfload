/*
 netcdfload

 Copyright (C) 2014 met.no

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

#ifndef NETCDFPARAMETERSPECIFICATION_H_
#define NETCDFPARAMETERSPECIFICATION_H_

#include <string>
#include <map>
#include <istream>

namespace MetNoFimex
{
    class CDMReader;
}
extern "C"
{
    typedef struct _xmlNode xmlNode;
    typedef xmlNode* xmlNodePtr;
}


class NetcdfParameterSpecification
{
public:
	NetcdfParameterSpecification() {}
	explicit NetcdfParameterSpecification(xmlNodePtr netcdfNode);
	NetcdfParameterSpecification(const std::string & specification);

	~NetcdfParameterSpecification();

    const std::string & variableName() const { return variableName_; }
    const std::string & standardName() const { return standardName_; }

    typedef std::map<std::string, double> IndexNameToValue;
    const IndexNameToValue & indicesToLoad() const { return indicesToLoad_; }

    unsigned cdmIndex(MetNoFimex::CDMReader & reader, const std::string & dimensionName, double dimensionValue) const;

private:

    std::string variableName_;
    std::string standardName_;

    IndexNameToValue indicesToLoad_;
};

std::istream & operator >> (std::istream & s, NetcdfParameterSpecification & out);


#endif /* NETCDFPARAMETERSPECIFICATION_H_ */
