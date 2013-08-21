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

#ifndef DATASPECIFICATION_H_
#define DATASPECIFICATION_H_

#include "TimeSpecification.h"
#include <string>
#include <exception>

class DataSpecification
{
public:
    DataSpecification() {}
    DataSpecification(const std::string & wdbParameterName, const std::string & units, const std::string & alternativeUnitConversion = "", float scale = 1, const std::string& validFrom = "validtime", const std::string& validTo = "validtime");
    virtual ~DataSpecification();

    const std::string & wdbParameter() const { return wdbParameter_; }
    const std::string & wdbUnits() const { return wdbUnits_; }
    const std::string & alternativeUnitConversion() const { return alternativeUnitConversion_; }
    float scale() const { return scale_; }
    const TimeSpecification & validTimeFrom() const { return validTimeFrom_; }
    const TimeSpecification & validTimeTo() const { return validTimeTo_; }

    class Level
    {
    public:
    	Level(const std::string & name = "height above ground", float value = 0) :
    		name_(name), value_(value)
    	{}
    	const std::string & name() const { return name_; }
    	float value() const { return value_; }
    private:
    	std::string name_;
    	float value_;
    };
    const Level & level() const { return level_; }
    void level(const Level & level) { level_ = level; }

private:
    std::string wdbParameter_;
    std::string wdbUnits_;
    std::string alternativeUnitConversion_;
    float scale_;
    TimeSpecification validTimeFrom_;
    TimeSpecification validTimeTo_;

    Level level_;
};


#endif /* DATASPECIFICATION_H_ */
