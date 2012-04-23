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
#include "parameter/DataSpecification.h"

// boost
#include <boost/shared_ptr.hpp>

// std
#include <string>
#include <vector>

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
	~LoadElement();

	struct IndexElement
	{
            std::string indexName;
	    std::string indexValue;

	    unsigned cdmIndex(boost::shared_ptr<MetNoFimex::CDMReader>& reader) const;
	};

	const std::string & cfName() const { return cfName_; }
	const std::vector<IndexElement> & indices() const { return indices_; }
	const DataSpecification & wdbDataSpecification() const { return wdbDataSpecification_; }


private:
	void addNetcdfSpec_(xmlNodePtr netcdfNode);
	void addWdbSpec_(xmlNodePtr wdbNode);

	std::string cfName_;
	std::vector<IndexElement> indices_;

	DataSpecification wdbDataSpecification_;
};

#endif /* LOADELEMENT_H_ */
