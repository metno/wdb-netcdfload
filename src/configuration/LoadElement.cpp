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

    bool equal(float a, const std::string & s)
    {
        float b = boost::lexical_cast<float>(s);
        return fabs(a - b) < 0.000001;
    }

    string getAttribute(xmlNodePtr elementNode, const string & name, const string alternative = string())
    {
        for ( xmlAttrPtr attr = elementNode->properties; attr; attr = attr->next )
                if ( xmlStrEqual(attr->name,(xmlChar*) name.c_str()) )
                        return (char*) attr->children->content;

        if ( not alternative.empty() )
                return alternative;
        throw runtime_error("Missing attribute " + name);
    }
} // end namespace


AxisElement::AxisElement(xmlNodePtr loadNode)
{
    for(xmlNodePtr subNode = loadNode->children; subNode; subNode = subNode->next)
    {
        if(xmlStrEqual(subNode->name, (xmlChar*) "netcdf"))
            addNetcdfSpec_(subNode);
        else if(xmlStrEqual(subNode->name, (xmlChar*) "wdb") )
            addWdbSpec_(subNode);
    }
}

AxisElement::~AxisElement() { }

void AxisElement::addNetcdfSpec_(xmlNodePtr netcdfNode)
{
    cfName_ = getAttribute(netcdfNode, "cfname");
//    for(xmlNodePtr subNode = netcdfNode->children; subNode; subNode = subNode->next)
//    {
//        if(xmlStrEqual(subNode->name, (xmlChar*) "dimension"))
//        {
//            string name = getAttribute(subNode, "name");
//            string value = getAttribute(subNode, "value");
//        }
//    }
}

void AxisElement::addWdbSpec_(xmlNodePtr wdbNode)
{
    string wdbName = getAttribute(wdbNode, "name");
    string wdbUnits = getAttribute(wdbNode, "units");
    float scale = boost::lexical_cast<float>(getAttribute(wdbNode, "scale", "1"));
    string validFrom = getAttribute(wdbNode, "validfrom", "validtime");

    wdbDataSpecification_ = DataSpecification(wdbName, wdbUnits, scale, validFrom);
}



LoadElement::LoadElement(xmlNodePtr loadNode)
{
    for(xmlNodePtr subNode = loadNode->children; subNode; subNode = subNode->next)
    {
        if(xmlStrEqual(subNode->name, (xmlChar*) "netcdf"))
            addNetcdfSpec_(subNode);
        else if(xmlStrEqual(subNode->name, (xmlChar*) "wdb") )
            addWdbSpec_(subNode);
    }

    makeIndicePermutations_();
}

LoadElement::~LoadElement() { }

unsigned LoadElement::IndexElement::cdmIndex(boost::shared_ptr<MetNoFimex::CDMReader>& reader) const
{
    boost::shared_ptr<MetNoFimex::Data> indexElements = reader->getData(indexName);
    boost::shared_array<float> elements = indexElements->asFloat();

    for(unsigned index = 0; index < indexElements->size(); ++ index)
        if(equal(elements[index], indexValue))
            return index;

    ostringstream msg;
    msg << "Unable to find index (" << indexName << " = " << indexValue << ")";
    throw runtime_error(msg.str());
}

void LoadElement::expandIndicePermutations(const boost::shared_ptr<MetNoFimex::CDMReader>& reader, const std::string& dimName)
{
    const MetNoFimex::CDM& cdmRef = reader->getCDM();
    if(not cdmRef.hasDimension(dimName))
        return;

    boost::shared_ptr<MetNoFimex::Data> indexElements = reader->getData(dimName);
    boost::shared_array<float> elements = indexElements->asFloat();

    vector<IndexElement> indices2add;
    for(size_t index = 0; index < indexElements->size(); ++ index) {
        IndexElement ie = {dimName, boost::lexical_cast<string>(elements[index])};
        indices2add.push_back(ie);
    }

    vector<vector<IndexElement> > subproduct;
    for(size_t index = 0; index < indices2add.size(); ++index)
    {
        IndexElement ie = indices2add[index];

        if(indicesPermutations_.empty()) {
            vector<IndexElement> subvector;
            subvector.push_back(ie);
            subproduct.push_back(subvector);
        } else {
            for(size_t index = 0; index < indicesPermutations_.size(); ++index)
            {
                vector<IndexElement> subvector;
                subvector.push_back(ie);
                subvector.insert(subvector.end(), indicesPermutations_[index].begin(), indicesPermutations_[index].end());
                subproduct.push_back(subvector);
            }
        }
    }

    indicesPermutations_ = subproduct;

}

void LoadElement::removeNotToLoadPermutations()
{
    std::vector<std::vector<IndexElement> >  permutations;

    for(size_t p = 0; p < indicesPermutations_.size(); ++p)
    {
        vector<IndexElement> permutation = indicesPermutations_[p];

        bool toLoad = true;
        for(size_t i = 0; i < permutation.size(); ++i)
        {
            const IndexElement ie = permutation[i];
            multimap<string, IndexElement>::const_iterator cit;
            for(cit = indicesNotToLoad_.begin(); cit != indicesNotToLoad_.end(); ++cit)
            {
                const IndexElement notIe = cit->second;
                string notValue = string("!").append(ie.indexValue);
                if(ie.indexName == notIe.indexName and notIe.indexValue == notValue) {
                    toLoad = false;
                    break;
                }
            }
            if(not toLoad)
                break;

        }
        if(toLoad)
            permutations.push_back(permutation);
    }

    indicesPermutations_ = permutations;
}

void LoadElement::addNetcdfSpec_(xmlNodePtr netcdfNode)
{
    cfName_ = getAttribute(netcdfNode, "cfname");
    for(xmlNodePtr subNode = netcdfNode->children; subNode; subNode = subNode->next)
    {
        if(xmlStrEqual(subNode->name, (xmlChar*) "dimension"))
        {
            string name = getAttribute(subNode, "name");
            string value = getAttribute(subNode, "value");
            IndexElement ie = {name, value};
            if(value.find("!") == string::npos) {
                indiceKeys_.insert(name);
                indicesToLoad_.insert(make_pair<string, LoadElement::IndexElement>(name, ie));
            } else {
                indicesNotToLoad_.insert(make_pair<string, LoadElement::IndexElement>(name, ie));
            }
        }
    }
}

void LoadElement::addWdbSpec_(xmlNodePtr wdbNode)
{
    string wdbName = getAttribute(wdbNode, "name");
    string wdbUnits = getAttribute(wdbNode, "units");
    float scale = boost::lexical_cast<float>(getAttribute(wdbNode, "scale", "1"));
    string validFrom = getAttribute(wdbNode, "validfrom", "validtime");

    wdbDataSpecification_ = DataSpecification(wdbName, wdbUnits, scale, validFrom);
}

const std::vector<std::vector<LoadElement::IndexElement> >& LoadElement::permutations() const
{
    return indicesPermutations_;
}

void LoadElement::makeIndicePermutations_()
{
    set<string>::const_iterator key;

    for(key = indiceKeys_.begin(); key != indiceKeys_.end(); ++key)
    {
        vector<vector<IndexElement> > subproduct;

        pair<multimap<string, IndexElement>::const_iterator, multimap<string, IndexElement>::const_iterator > rangeIt;
        rangeIt = indicesToLoad_.equal_range(*key);

        multimap<string, IndexElement>::const_iterator cit2;
        for(cit2 = rangeIt.first; cit2 != rangeIt.second; ++cit2)
        {
            IndexElement ie = cit2->second;
            cerr << "  [" << ie.indexName << ", " << ie.indexValue << "]" << endl;

            if(indicesPermutations_.empty()) {
                vector<IndexElement> subvector;
                subvector.push_back(ie);
                subproduct.push_back(subvector);
            } else {
                for(size_t index = 0; index < indicesPermutations_.size(); ++index)
                {
                    vector<IndexElement> subvector;
                    subvector.push_back(ie);
                    subvector.insert(subvector.end(), indicesPermutations_[index].begin(), indicesPermutations_[index].end());
                    subproduct.push_back(subvector);
                }
            }
        }

        indicesPermutations_ = subproduct;
    }
}
