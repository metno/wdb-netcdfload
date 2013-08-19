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

// project
#include "LoadElement.h"

// fimex
#include <fimex/CDM.h>
#include <fimex/Data.h>
#include <fimex/CDMReader.h>

// libxml2
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

// std
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
        throw runtime_error("Missing attribute " + name);
    }
} // end namespace



LoadElement::LoadElement(xmlNodePtr loadNode)
{
    for(xmlNodePtr subNode = loadNode->children; subNode; subNode = subNode->next)
    {
        if(xmlStrEqual(subNode->name, (xmlChar*) "netcdf"))
            addNetcdfSpec_(subNode);
        else if(xmlStrEqual(subNode->name, (xmlChar*) "wdb") )
            addWdbSpec_(subNode);
    }

    //makeIndicePermutations_();
}

LoadElement::~LoadElement() { }

//unsigned LoadElement::IndexElement::cdmIndex(boost::shared_ptr<MetNoFimex::CDMReader>& reader) const
unsigned LoadElement::cdmIndex(MetNoFimex::CDMReader & reader, const std::string & dimensionName, double dimensionValue) const
{
    boost::shared_ptr<MetNoFimex::Data> indexElements = reader.getData(dimensionName);
    boost::shared_array<float> elements = indexElements->asFloat();

    for(unsigned index = 0; index < indexElements->size(); ++ index)
        if(equal(elements[index], dimensionValue))
            return index;

    ostringstream msg;
    msg << "Unable to find index (" << dimensionName << " = " << dimensionValue << ")";
    throw runtime_error(msg.str());
}

void LoadElement::addNetcdfSpec_(xmlNodePtr netcdfNode)
{
    cfName_ = getAttribute(netcdfNode, "cfname");
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

void LoadElement::addWdbSpec_(xmlNodePtr wdbNode)
{
    string wdbName = getAttribute(wdbNode, "name");
    string wdbUnits = getAttribute(wdbNode, "units");
    string alternativeUnitConversion = getAttributeNoThrow(wdbNode, "alternativeunitconversion");
    float scale = boost::lexical_cast<float>(getAttribute(wdbNode, "scale", "1"));
    string validFrom = getAttribute(wdbNode, "validfrom", "validtime");
    string validTo = getAttribute(wdbNode, "validto", "validtime");

    wdbDataSpecification_ = DataSpecification(wdbName, wdbUnits, alternativeUnitConversion, scale, validFrom, validTo);

    for(xmlNodePtr subNode = wdbNode->children; subNode; subNode = subNode->next)
        if(xmlStrEqual(subNode->name, (xmlChar*) "level"))
        	wdbDataSpecification_.level(getWdbLevelSpec_(subNode));
}

DataSpecification::Level LoadElement::getWdbLevelSpec_(xmlNodePtr levelNode)
{
    string name = getAttribute(levelNode, "name");
    float value = 0;
    std::string val;
    try
    {
    	val = getAttribute(levelNode, "value");
    }
    catch (std::exception & )
    {}
	value = boost::lexical_cast<float>(val);

	return DataSpecification::Level(name, value);
}

std::ostream & operator << (std::ostream & s, const LoadElement & loadElement)
{
	return s << "LoadElement(" << loadElement.cfName() << ')';
}
