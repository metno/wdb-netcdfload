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
#include <map>

extern "C"
{
    typedef struct _xmlXPathContext xmlXPathContext;
    typedef xmlXPathContext *xmlXPathContextPtr;
}
class AbstractNetcdfField;



class LoadConfiguration
{
    public:
	explicit LoadConfiguration(const boost::filesystem::path & translationFile);
	~LoadConfiguration();

	/**
	 * Get the load specification for the given name, or NULL if no such name
	 * exists in configuration.
	 */
	std::vector<LoadElement> getLoadElement(const AbstractNetcdfField & field) const;

private:
	void init_(xmlXPathContextPtr context);

	typedef std::multimap<std::string, LoadElement> LoadElementMap;

	bool getVariableLoadElement_(std::vector<LoadElement> & out, const AbstractNetcdfField & field) const;
	bool getStandardNameLoadElement_(std::vector<LoadElement> & out, const AbstractNetcdfField & field) const;


	LoadElementMap variableLoadElements_;
	LoadElementMap standardNameLoadElements_;
};

#endif /* LOADCONFIGURATION_H_ */
