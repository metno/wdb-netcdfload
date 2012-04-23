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

// project
#include "localtime.h"
#include "configuration/CdmLoaderConfiguration.h"
#include "configuration/LoadConfiguration.h"

// wdb
#include <wdb/LoaderDatabaseConnection.h>

// std
#include <string>

namespace MetNoFimex
{
    class CDMReader;
    class Data;
}

class CdmLoader
{
public:
    CdmLoader(const CdmLoaderConfiguration & conf);
    ~CdmLoader();

    void write(const std::string & file);

    void write(boost::shared_ptr<MetNoFimex::CDMReader>& reader);

    void write(boost::shared_ptr<MetNoFimex::CDMReader>& reader,
               const LoadElement & loadElement,
               const std::string & placeName,
               const Time & referenceTime,
               const std::vector<Time> & validTimes);

    void write(boost::shared_ptr<MetNoFimex::CDMReader>& reader,
               const std::string & cfName,
               const std::string & placeName,
               const Time & referenceTime,
               const std::vector<Time> & validTimes);

private:

    void write_(const boost::shared_ptr<MetNoFimex::Data> & data,
                const std::string & wdbParameter,
                const std::string & placeName,
                const Time & referenceTime,
                const std::string & validFrom,
                const std::string & validTo);

    std::vector<Time> getTimes_(boost::shared_ptr<MetNoFimex::CDMReader>& reader) const;
    std::string getPlaceName_(boost::shared_ptr<MetNoFimex::CDMReader>& reader);
    Time getReferenceTime_(boost::shared_ptr<MetNoFimex::CDMReader>& reader) const;
    boost::shared_ptr<MetNoFimex::Data> getAltitude_(boost::shared_ptr<MetNoFimex::CDMReader>& reader) const;

    CdmLoaderConfiguration conf_;
    wdb::load::LoaderDatabaseConnection wdbConnection_;

    LoadConfiguration loadConfiguration_;
};

#endif /* CDMLOADER_H_ */
