/*
 probabilityForecast

 Copyright (C) 2008 met.no

 Contact information:
 Norwegian Meteorological Institute
 Box 43 Blindern
 0313 OSLO
 NORWAY
 E-mail: wdb@met.no

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


#ifndef LOCALTIME_H_
#define LOCALTIME_H_

// boost
#include <boost/date_time/local_time/local_date_time.hpp>

// std
#include <string>

typedef boost::local_time::local_date_time Time;
typedef boost::posix_time::time_duration   Duration;

extern const Time INVALID_TIME;

/// Get time based on seconds since 1970-01-01 - epoch time
Time time_from_seconds_since_epoch(long long secondsSinceEpoch);

long long time_to_seconds_since_epoch(const Time& t);

/// Interpret a postgresql time string
Time time_from_postgresql_string(const std::string & localTime);

/// Create a postgresql time string
std::string time_to_postgresql_string(const Time & t);

#endif /* LOCALTIME_H_ */
