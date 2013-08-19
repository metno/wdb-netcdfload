/*
 pgen-probability

 Copyright (C) 2010 met.no

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

#include "DataSpecification.h"
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/trim.hpp>

DataSpecification::DataSpecification(const std::string& wdbParameterName, const std::string& wdbUnits, const std::string & alternativeUnitConversion, float scale, const std::string& validFrom, const std::string& validTo)
    : wdbParameter_(wdbParameterName),
      wdbUnits_(wdbUnits),
      alternativeUnitConversion_(alternativeUnitConversion),
      scale_(scale),
      validTimeFrom_(validFrom.c_str()),
      validTimeTo_(validTo.c_str())
{
}

DataSpecification::~DataSpecification()
{
}
