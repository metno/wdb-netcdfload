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

#ifndef NETCDFTRANSLATOR_H_
#define NETCDFTRANSLATOR_H_

#include "WriteQuery.h"
#include "configuration/CdmLoaderConfiguration.h"
#include "configuration/LoadConfiguration.h"
#include <vector>

class NetcdfField;

/**
 * Creates WriteQuery objects from NetcdfField objects, based on program configuration.
 */
class NetcdfTranslator
{
public:
	NetcdfTranslator(const CdmLoaderConfiguration & conf);
	~NetcdfTranslator();

	std::vector<WriteQuery> queries(const NetcdfField & field) const;

private:
	void setLocation_(WriteQuery & out, const NetcdfField & field) const;

	CdmLoaderConfiguration conf_;
	LoadConfiguration loadConfiguration_;
};

#endif /* NETCDFTRANSLATOR_H_ */
