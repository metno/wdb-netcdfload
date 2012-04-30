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

        typedef std::vector<LoadElement>::const_iterator load_iterator;
        typedef std::vector<LoadElement>::const_iterator load_const_iterator;
        load_iterator load_begin() const { return loadElements_.begin(); }
        load_iterator load_end() const { return loadElements_.end(); }

        typedef std::vector<AxisElement>::iterator axis_iterator;
        typedef std::vector<AxisElement>::const_iterator axis_const_iterator;
        axis_iterator axis_begin() { return axisElements_.begin(); }
        axis_iterator axis_end() { return axisElements_.end(); }
        axis_iterator findAxisByCfName(const std::string& cfName);

	typedef std::vector<LoadElement>::size_type size_type;
	size_type size() const { return loadElements_.size(); }

	typedef std::vector<LoadElement>::value_type value_type;

private:
	void init_(xmlXPathContextPtr context);
	std::vector<LoadElement> loadElements_;
        std::vector<AxisElement> axisElements_;
};

#endif /* LOADCONFIGURATION_H_ */
