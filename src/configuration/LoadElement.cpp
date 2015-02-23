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
#include <wdb/errors.h>

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
        throw wdb::load::LoadError(wdb::load::UnableToReadConfigFile, "Missing attribute " + name);
    }
} // end namespace



LoadElement::LoadElement(xmlNodePtr loadNode)
{
    for(xmlNodePtr subNode = loadNode->children; subNode; subNode = subNode->next)
    {
    	if(xmlStrEqual(subNode->name, (xmlChar*) "netcdf"))
    		netcdfParameterSpecification_ = NetcdfParameterSpecification(subNode);
        else if(xmlStrEqual(subNode->name, (xmlChar*) "wdb") )
            addWdbSpec_(subNode);
    }
}

LoadElement::LoadElement(const std::string & variableName, const DataSpecification & wdbDataSpecification) :
		netcdfParameterSpecification_(variableName),
		wdbDataSpecification_(wdbDataSpecification)
{
}

LoadElement::~LoadElement() { }


void LoadElement::addWdbSpec_(xmlNodePtr wdbNode)
{
    string wdbName = getAttribute(wdbNode, "name");
    string wdbUnits = getAttribute(wdbNode, "units");
    string alternativeUnitConversion = getAttributeNoThrow(wdbNode, "alternativeunitconversion");
    float scale = boost::lexical_cast<float>(getAttribute(wdbNode, "scale", "1"));
    string validFrom = getAttribute(wdbNode, "validfrom", "validtime");
    string validTo = getAttribute(wdbNode, "validto", "validtime");

    try
    {
    	wdbDataSpecification_ = DataSpecification(wdbName, wdbUnits, alternativeUnitConversion, scale, validFrom, validTo);
    }
    catch ( std::exception & e )
    {
    	throw wdb::load::LoadError(wdb::load::UnableToReadConfigFile, e.what());
    }

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
	return s << "LoadElement(" << loadElement.variableName() << ')';
}
