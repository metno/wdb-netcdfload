/*
 pgen_wdbSave

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

#ifndef CDMLOADER_H_
#define CDMLOADER_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// project
#include "localtime.h"
#include "configuration/CdmLoaderConfiguration.h"
#include "configuration/LoadConfiguration.h"

// wdb
#include <wdb/LoaderDatabaseConnection.h>

#include <boost/shared_array.hpp>
// std
#include <string>

namespace MetNoFimex
{
	class CDM;
    class CDMReader;
    class Data;
    class SliceBuilder;
}

class CdmLoader
{
public:
    CdmLoader(const CdmLoaderConfiguration & conf);
    ~CdmLoader();

    void load(const std::string & file);

private:

    void write_();
    void write_(LoadElement& loadElement);

    struct Blob
    {
    	unsigned length;
    	boost::shared_array<float> data;
    };

    void write_(const Blob & data,
                const std::string& wdbParameter,
                const std::string& placeName,
                const std::string& validFrom,
                const std::string& validTo,
                const std::string& wdbLevelName,
                double levelFrom,
                double levelTo,
                size_t dataVersion);

    Time getReferenceTime_();
    std::vector<Time> getTimes_();
    std::vector<Time> getTimes_(const MetNoFimex::CDM & cdm, const LoadElement & loadElement) const;


    std::string getPlaceName_(const std::string& cfName);
    boost::shared_ptr<MetNoFimex::Data> getLevels_(const std::string& cfName) const;

    Blob getData_(const MetNoFimex::SliceBuilder & slicer,
			LoadElement& loadElement, float undef);

    CdmLoaderConfiguration conf_;
    wdb::load::LoaderDatabaseConnection wdbConnection_;

    LoadConfiguration loadConfiguration_;

    boost::shared_ptr<Time> pReferenceTime_;
    boost::shared_ptr<MetNoFimex::CDMReader> pReader_;
};

#endif /* CDMLOADER_H_ */
