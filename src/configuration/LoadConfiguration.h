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

#ifndef LOADCONFIGURATION_H_
#define LOADCONFIGURATION_H_

#include "LoadElement.h"
#include <boost/filesystem/path.hpp>
#include <vector>

extern "C"
{
typedef struct _xmlXPathContext xmlXPathContext;
typedef xmlXPathContext *xmlXPathContextPtr;
}



class LoadConfiguration
{
public:
	explicit LoadConfiguration(const boost::filesystem::path & translationFile);
	~LoadConfiguration();

	typedef std::vector<LoadElement>::const_iterator iterator;
	typedef std::vector<LoadElement>::const_iterator const_iterator;

	iterator begin() const { return loadElements_.begin(); }
	iterator end() const { return loadElements_.end(); }

	typedef std::vector<LoadElement>::size_type size_type;
	size_type size() const { return loadElements_.size(); }

	typedef std::vector<LoadElement>::value_type value_type;

private:
	void init_(xmlXPathContextPtr context);
	std::vector<LoadElement> loadElements_;
};

#endif /* LOADCONFIGURATION_H_ */
