/*
    netcdfload

    Copyright (C) 2013 met.no

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


#ifndef NETCDFLOADER_H_
#define NETCDFLOADER_H_

#include "NetcdfField.h"
#include "configuration/CdmLoaderConfiguration.h"
#include "configuration/LoadConfiguration.h"
#include "localtime.h"
#include <wdb/LoaderDatabaseConnection.h>
#include <string>
#include <iosfwd>


class NetcdfFile;

class NetcdfLoader
{
public:
	explicit NetcdfLoader(const CdmLoaderConfiguration & conf);
	~NetcdfLoader();

	void load(const NetcdfFile & file);

	void list(const NetcdfFile & file, std::ostream & s) const;

private:
	std::string dataProvider_() const;
	std::string placename_(const NetcdfField::Ptr & field) const;
	Time referenceTime_(const NetcdfFile & file) const;
	unsigned dataVersion_(const NetcdfField::Ptr & file) const;
	unsigned maxDataVersion_(const NetcdfField::Ptr & file) const;

	CdmLoaderConfiguration conf_;
	mutable wdb::load::LoaderDatabaseConnection wdbConnection_;
    LoadConfiguration loadConfiguration_;
};



#endif /* NETCDFLOADER_H_ */
