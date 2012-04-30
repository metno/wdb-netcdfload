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
#include <map>
#include <set>
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

class AxisElement
{
public:
    explicit AxisElement(xmlNodePtr loadNode);
    ~AxisElement();

    const std::string & cfName() const { return cfName_; }
//    const std::string & wdbUnit() const { return wdbUnits_; }
    const DataSpecification& wdbDataSpecification() const { return wdbDataSpecification_; }

private:
    void addWdbSpec_(xmlNodePtr wdbNode);
    void addNetcdfSpec_(xmlNodePtr netcdfNode);

    std::string cfName_;
//    std::string wdbUnits_;
    DataSpecification wdbDataSpecification_;
};

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

    const std::string& cfName() const { return cfName_; }
//    const std::string& wdbUnit() const { return wdbUnits_; }
    const std::vector<std::vector<IndexElement> >& permutations() const;
    const std::set<std::string>& indiceKeys() { return indiceKeys_; }
    const DataSpecification& wdbDataSpecification() const { return wdbDataSpecification_; }
    void expandIndicePermutations(const boost::shared_ptr<MetNoFimex::CDMReader>& reader, const std::string& dimName);
    void removeNotToLoadPermutations();

private:
    void addWdbSpec_(xmlNodePtr wdbNode);
    void addNetcdfSpec_(xmlNodePtr netcdfNode);
    void makeIndicePermutations_();

    std::string cfName_;
//    std::string wdbUnits_;
    DataSpecification wdbDataSpecification_;

    std::set<std::string> indiceKeys_;
    std::multimap<std::string, IndexElement> indicesToLoad_;
    std::multimap<std::string, IndexElement> indicesNotToLoad_;
    std::vector<std::vector<IndexElement> >  indicesPermutations_;

};

#endif /* LOADELEMENT_H_ */
