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
        throw std::runtime_error(translationFile.file_string() + ": No netcdfo-wdb translation file");
    if(is_directory(translationFile))
        throw std::runtime_error(translationFile.file_string() + " is a directory, netcdf-wdb-transaltion file expecdted");

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
    // TODO Auto-generated destructor stub
}

void LoadConfiguration::init_(xmlXPathContextPtr context)
{
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

        loadElements_.push_back(LoadElement(elementNode));
    }

    boost::shared_ptr<xmlXPathObject>
            xpathObjAxis(xmlXPathEvalExpression((const xmlChar *) "/netcdfloadconfiguration/axis", context),
                     xmlXPathFreeObject);

    if(not xpathObjAxis)
        return;

    xmlNodeSetPtr nodesAxis = xpathObjAxis->nodesetval;

    for ( int i = 0; i < nodesAxis->nodeNr; ++ i )
    {
        xmlNodePtr axisNode =  nodesAxis->nodeTab[i];
        if (axisNode->type != XML_ELEMENT_NODE)
            throw std::runtime_error("Expected element node");

        axisElements_.push_back(AxisElement(axisNode));
    }
}

LoadConfiguration::axis_iterator LoadConfiguration::findAxisByCfName(const std::string& cfName)
{
    LoadConfiguration::axis_iterator it;
    for(it = axisElements_.begin(); it != axisElements_.end(); ++it) {
        if(it->cfName() == cfName)
            break;
    }
    return it;
}
