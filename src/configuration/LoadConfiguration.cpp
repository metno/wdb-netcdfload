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


LoadConfiguration::LoadConfiguration(const boost::filesystem::path & translationFile)
{
	if ( not exists(translationFile) )
		throw std::runtime_error(translationFile.file_string() + ": No such file");
	if ( is_directory(translationFile) )
		throw std::runtime_error(translationFile.file_string() + " is a directory");

	const std::string & fileName = translationFile.file_string();

	XmlSession session;
	boost::shared_ptr<xmlDoc> doc(xmlParseFile(fileName.c_str()), xmlFreeDoc);
	if ( ! doc )
		throw std::runtime_error("Unable to parse doc");

	boost::shared_ptr<xmlXPathContext> xpathCtx(xmlXPathNewContext(doc.get()), xmlXPathFreeContext);
	if ( ! xpathCtx )
		throw std::runtime_error("unable to create xpath context");

	init_(xpathCtx.get());
}

LoadConfiguration::~LoadConfiguration()
{
	// TODO Auto-generated destructor stub
}

void LoadConfiguration::init_(xmlXPathContextPtr context)
{
	boost::shared_ptr<xmlXPathObject> xpathObj(
			xmlXPathEvalExpression((const xmlChar *) "/netcdfloadconfiguration/load", context),
			xmlXPathFreeObject);

	if ( ! xpathObj )
        return;
	xmlNodeSetPtr nodes = xpathObj->nodesetval;

	for ( int i = 0; i < nodes->nodeNr; ++ i )
	{
		xmlNodePtr elementNode =  nodes->nodeTab[i];
		if ( elementNode->type != XML_ELEMENT_NODE )
			throw std::runtime_error("Expected element node");

		loadElements_.push_back(LoadElement(elementNode));
	}
}
