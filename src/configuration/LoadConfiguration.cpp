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

#include "LoadConfiguration.h"
//#include "AutomaticLoadConfiguration.h"
#include <AbstractNetcdfField.h>
#include <wdbLogHandler.h>
#include <boost/filesystem.hpp>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <stdexcept>

namespace
{
    struct XmlSession
    {
	XmlSession()
	{
		xmlInitParser();
	}
	~XmlSession()
	{
		xmlCleanupParser();
	}
    };
}


LoadConfiguration::LoadConfiguration(const boost::filesystem::path& translationFile)
{
    if(not exists(translationFile))
        throw std::runtime_error(translationFile.file_string() + ": No netcdf-wdb translation file");
    if(is_directory(translationFile))
        throw std::runtime_error(translationFile.file_string() + " is a directory, netcdf-wdb-transaltion file expected");

    const std::string & fileName = translationFile.file_string();

    XmlSession session;
    boost::shared_ptr<xmlDoc> doc(xmlParseFile(fileName.c_str()), xmlFreeDoc);
    if (not doc)
        throw std::runtime_error("Unable to parse doc");

    boost::shared_ptr<xmlXPathContext> xpathCtx(xmlXPathNewContext(doc.get()), xmlXPathFreeContext);
    if (not xpathCtx)
        throw std::runtime_error("unable to create xpath context");

    init_(xpathCtx.get());
}

LoadConfiguration::~LoadConfiguration()
{
}



std::vector<LoadElement> LoadConfiguration::getLoadElement(const AbstractNetcdfField & field) const
{
	std::vector<LoadElement> ret;

	if ( ! getVariableLoadElement_(ret, field) )
		getStandardNameLoadElement_(ret, field);

	return ret;
}

void LoadConfiguration::init_(xmlXPathContextPtr context)
{
	WDB_LOG & log = WDB_LOG::getInstance("wdb.load.netcdf.configuration");

	boost::shared_ptr<xmlXPathObject>
            xpathObjLoad(xmlXPathEvalExpression((const xmlChar *) "/netcdfloadconfiguration/load", context),
                     xmlXPathFreeObject);

    if(not xpathObjLoad)
        return;

    xmlNodeSetPtr nodesLoad = xpathObjLoad->nodesetval;

    for ( int i = 0; i < nodesLoad->nodeNr; ++ i )
    {
        xmlNodePtr elementNode =  nodesLoad->nodeTab[i];
        if ( elementNode->type != XML_ELEMENT_NODE )
            throw std::runtime_error("Expected element node");

        LoadElement loadElement(elementNode);
        if ( not loadElement.variableName().empty() )
        	variableLoadElements_.insert(std::make_pair(loadElement.variableName(), loadElement));
        else if ( not loadElement.standardName().empty() )
        	standardNameLoadElements_.insert(std::make_pair(loadElement.variableName(), loadElement));
        else
    		log.warnStream() << "Element configuration without variable_name or standard_name";
    }
}

bool LoadConfiguration::getVariableLoadElement_(std::vector<LoadElement> & out, const AbstractNetcdfField & field) const
{
	std::pair<LoadElementMap::const_iterator,LoadElementMap::const_iterator> find = variableLoadElements_.equal_range(field.variableName());
	for ( LoadElementMap::const_iterator it = find.first; it != find.second; ++ it )
	{
		const LoadElement * loadElement = & it->second;

		/// Indexes in field (name->size)
		const AbstractNetcdfField::IndexList & fieldIndexes = field.unHandledIndexes();

		/// Indexes specified in configuration (name->value)
		const LoadElement::IndexNameToValue & configIndexes = loadElement->indicesToLoad();

		// simple case
		if ( configIndexes.empty() and fieldIndexes.empty() )
		{
			out.push_back(* loadElement);
		}
		else if ( configIndexes.size() == fieldIndexes.size() )
		{
			for ( AbstractNetcdfField::IndexList::const_iterator fieldIndex = fieldIndexes.begin(); fieldIndex != fieldIndexes.end(); ++ fieldIndex )
			{
				const std::string & indexName = fieldIndex->first;
				LoadElement::IndexNameToValue::const_iterator configIndex = configIndexes.find(indexName);
				if ( configIndex != configIndexes.end() )
				{
					unsigned indexSize = fieldIndex->second;
					for ( int i = 0; i < indexSize; ++ i )
					{
						double dimensionValue = field.indexValue(indexName, i);
						if ( std::abs(dimensionValue - configIndex->second) < 0.00001 )
							out.push_back(* loadElement);
					}
				}
			}
		}
	}

	return find.first != find.second;
}

bool LoadConfiguration::getStandardNameLoadElement_(std::vector<LoadElement> & out, const AbstractNetcdfField & field) const
{
	return false;
}
