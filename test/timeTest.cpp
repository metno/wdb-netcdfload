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

#include <gtest/gtest.h>
#include <localtime.h>
#include <string>

using namespace std;
using namespace boost::local_time;
using namespace boost::posix_time;
using namespace boost::gregorian;

TEST(TimeConversionTest, specificTimeZone)
{
	const local_date_time in = local_time_from_string("2008-11-25 13:00:00+01");
	date d(2008, 11, 25);
	ptime expected(d, hours(12));

	EXPECT_EQ(expected, in.utc_time());
	EXPECT_EQ(hours(1), in.zone()->base_utc_offset());
}

TEST(TimeConversionTest, negativeTimeZone)
{
	const local_date_time in = local_time_from_string("2008-11-25 13:00:00-01");
	date d(2008, 11, 25);
	ptime expected(d, hours(14));

	EXPECT_EQ(expected, in.utc_time());
	EXPECT_EQ(hours(-1), in.zone()->base_utc_offset());
}


TEST(TimeConversionTest, implicitlyUtc)
{
	const local_date_time in = local_time_from_string("2008-11-25 13:00:00");
	date d(2008, 11, 25);
	ptime expected(d, hours(13));

	EXPECT_EQ(expected, in.utc_time());
	EXPECT_EQ(hours(0), in.zone()->base_utc_offset());
}

TEST(TimeConversionTest, explicitlyUtc)
{
	const local_date_time in = local_time_from_string("2008-11-25 13:00:00Z");
	date d(2008, 11, 25);
	ptime expected(d, hours(13));

	EXPECT_EQ(expected, in.utc_time());
	EXPECT_EQ(hours(0), in.zone()->base_utc_offset());
}
