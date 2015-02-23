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

#include "NetcdfParameterSpecification.h"
#include <wdb/errors.h>
#include <fimex/CDM.h>
#include <fimex/Data.h>
#include <fimex/CDMReader.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <stdexcept>

using namespace std;

namespace {

    bool equal(double a, double b)
    {
        return fabs(a - b) < 0.000001;
    }

    /**
     * return an empty string if attribute does not exist
     */
    string getAttributeNoThrow(xmlNodePtr elementNode, const string & name)
    {
        for ( xmlAttrPtr attr = elementNode->properties; attr; attr = attr->next )
                if ( xmlStrEqual(attr->name,(xmlChar*) name.c_str()) )
                        return (char*) attr->children->content;
        return string();
    }

    string getAttribute(xmlNodePtr elementNode, const string & name, const string alternative = string())
    {
    	string ret = getAttributeNoThrow(elementNode, name);
    	if ( not ret.empty() )
    		return ret;

        if ( not alternative.empty() )
                return alternative;
        throw wdb::load::LoadError(wdb::load::UnableToReadConfigFile, "Missing attribute " + name);
    }
} // end namespace



NetcdfParameterSpecification::NetcdfParameterSpecification(xmlNodePtr netcdfNode)
{
    variableName_ = getAttributeNoThrow(netcdfNode, "variable_name");
    standardName_ = getAttributeNoThrow(netcdfNode, "standard_name");

    for(xmlNodePtr subNode = netcdfNode->children; subNode; subNode = subNode->next)
    {
        if(xmlStrEqual(subNode->name, (xmlChar*) "dimension"))
        {
            string name = getAttribute(subNode, "name");
            double value = boost::lexical_cast<double>(getAttribute(subNode, "value"));
			indicesToLoad_[name] = value;
        }
    }
}


NetcdfParameterSpecification::NetcdfParameterSpecification(const std::string & specification)
{
	//air_temperature:percentile=50:somethingelse=2.1

	std::vector<std::string> elements;
	boost::algorithm::split(elements, specification, boost::algorithm::is_any_of(":"));

	std::vector<std::string>::const_iterator it = elements.begin();
	variableName_ = * it;

	while ( ++ it != elements.end())
	{
		const std::string & dimensionSpec = * it;
		std::vector<std::string> nameValue;
		boost::algorithm::split(nameValue, dimensionSpec, boost::algorithm::is_any_of("="));
		if ( nameValue.size() != 2 )
			throw wdb::load::LoadError(wdb::load::UnableToReadConfigFile, "Strange specification of netcdf dimension: " + dimensionSpec);
		const std::string & name = nameValue[0];
		double value = boost::lexical_cast<double>(nameValue[1]);
		indicesToLoad_[name] = value;
	}
}

NetcdfParameterSpecification::~NetcdfParameterSpecification() { }

//unsigned NetcdfParameterSpecification::IndexElement::cdmIndex(boost::shared_ptr<MetNoFimex::CDMReader>& reader) const
unsigned NetcdfParameterSpecification::cdmIndex(MetNoFimex::CDMReader & reader, const std::string & dimensionName, double dimensionValue) const
{
    boost::shared_ptr<MetNoFimex::Data> indexElements = reader.getData(dimensionName);
    boost::shared_array<float> elements = indexElements->asFloat();

    for(unsigned index = 0; index < indexElements->size(); ++ index)
        if(equal(elements[index], dimensionValue))
            return index;

    ostringstream msg;
    msg << "Unable to find index (" << dimensionName << " = " << dimensionValue << ")";
    throw wdb::load::LoadError(wdb::load::UnableToReadConfigFile, msg.str());
}

std::istream & operator >> (std::istream & s, NetcdfParameterSpecification & out)
{
	std::string spec;
	s >> spec;
	if ( s )
		out = NetcdfParameterSpecification(spec);
	return s;
}
