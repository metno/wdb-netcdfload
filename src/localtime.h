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

#include <boost/date_time/local_time/local_date_time.hpp>
#include <string>

typedef boost::posix_time::time_duration Duration;
typedef boost::local_time::local_date_time Time;

extern const Time INVALID_TIME;

/// Get time based on seconds since 1970-01-01
Time get_time(long long secondsSinceEpoch);

long long get_seconds_since_epoch(const Time & t);

/// Interpret a postgresql time string
Time local_time_from_string(const std::string & localTime);

/// Create a postgresql time string
std::string string_from_local_date_time(const Time & t);

#endif /* LOCALTIME_H_ */
