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


LoadElement::LoadElement(xmlNodePtr loadNode)
{
	for ( xmlNodePtr subNode = loadNode->children; subNode; subNode = subNode->next )
	{
		if ( xmlStrEqual(subNode->name, (xmlChar*) "netcdf") )
			addNetcdfSpec_(subNode);
		else if ( xmlStrEqual(subNode->name, (xmlChar*) "wdb") )
			addWdbSpec_(subNode);
	}

}

LoadElement::~LoadElement()
{
}

namespace
{
    bool equal(float a, const std::string & s)
    {
	float b = boost::lexical_cast<float>(s);
	return std::fabs(a -b) < 0.000001;
    }
}

unsigned LoadElement::IndexElement::cdmIndex(boost::shared_ptr<MetNoFimex::CDMReader>& reader) const
{
    boost::shared_ptr<MetNoFimex::Data> indexElements = reader->getData(indexName);
    boost::shared_array<float> elements = indexElements->asFloat();

    for ( unsigned index = 0; index < indexElements->size(); ++ index )
        if ( equal(elements[index], indexValue) )
            return index;

    std::ostringstream msg;
    msg << "Unable to find index (" << indexName << " = " << indexValue << ")";
    throw std::runtime_error(msg.str());
}

namespace
{
std::string getAttribute(xmlNodePtr elementNode, const std::string & name, const std::string alternative = std::string())
{
	std::string ret;
	for ( xmlAttrPtr attr = elementNode->properties; attr; attr = attr->next )
		if ( xmlStrEqual(attr->name,(xmlChar*) name.c_str()) )
			return (char*) attr->children->content;

	if ( not alternative.empty() )
		return alternative;
	throw std::runtime_error("Missing attribute " + name);
}
}

void LoadElement::addNetcdfSpec_(xmlNodePtr netcdfNode)
{
	cfName_ = getAttribute(netcdfNode, "cfname");
	for ( xmlNodePtr subNode = netcdfNode->children; subNode; subNode = subNode->next )
	{
		if ( xmlStrEqual(subNode->name, (xmlChar*) "dimension") )
		{
			std::string name = getAttribute(subNode, "name");
			std::string value = getAttribute(subNode, "value");
			IndexElement ie = {name, value};
			indices_.push_back(ie);
		}
	}
}

void LoadElement::addWdbSpec_(xmlNodePtr wdbNode)
{
	std::string wdbName = getAttribute(wdbNode, "name");
	float scale = boost::lexical_cast<float>(getAttribute(wdbNode, "scale", "1"));
	std::string validFrom = getAttribute(wdbNode, "validfrom", "validtime");

	wdbDataSpecification_ = DataSpecification(wdbName, scale, validFrom);
}
