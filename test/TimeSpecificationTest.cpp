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

#include <gtest/gtest.h>
#include <configuration/parameter/TimeSpecification.h>
#include <localtime.h>

using namespace boost::local_time;

TEST(TimeSpecificationTest, invalidSpecification)
{
	ASSERT_THROW(TimeSpecification("sometime"), InvalidSpecification);
}

TEST(TimeSpecificationTest, suddenEnd)
{
	EXPECT_THROW(TimeSpecification ts("validtime + 6 hours -"), InvalidSpecification);
	EXPECT_THROW(TimeSpecification ts("validtime + 6 hours - 3"), InvalidSpecification);
}

TEST(TimeSpecificationTest, returnReferenceTime)
{
	TimeSpecification ts("referencetime");

	local_date_time referenceTime = local_time_from_string("2010-02-01 12:00:00");
	local_date_time validTime = local_time_from_string("2010-02-03 18:00:00");

	ASSERT_EQ(referenceTime, ts.getTime(referenceTime, validTime));
}

TEST(TimeSpecificationTest, returnValidTime)
{
	TimeSpecification ts("validtime");

	local_date_time referenceTime = local_time_from_string("2010-02-01 12:00:00");
	local_date_time validTime = local_time_from_string("2010-02-03 18:00:00");

	ASSERT_EQ(validTime, ts.getTime(referenceTime, validTime));
}

TEST(TimeSpecificationTest, complexSpecificationAddTime)
{
	TimeSpecification ts("validtime + 6 hours");
	local_date_time referenceTime = local_time_from_string("2010-02-01 12:00:00");
	local_date_time validTime = local_time_from_string("2010-02-03 18:00:00");

	local_date_time expectedTime = local_time_from_string("2010-02-04 00:00:00");

	ASSERT_EQ(expectedTime, ts.getTime(referenceTime, validTime));
}

TEST(TimeSpecificationTest, complexSpecificationSubtractTime)
{
	TimeSpecification ts("referencetime - 12 hours");
	local_date_time referenceTime = local_time_from_string("2010-02-01 12:00:00");
	local_date_time validTime = local_time_from_string("2010-02-03 18:00:00");

	local_date_time expectedTime = local_time_from_string("2010-02-01 00:00:00");

	ASSERT_EQ(expectedTime, ts.getTime(referenceTime, validTime));
}

TEST(TimeSpecificationTest, noSpace)
{
	TimeSpecification ts("referencetime-12hours");
	local_date_time referenceTime = local_time_from_string("2010-02-01 12:00:00");
	local_date_time validTime = local_time_from_string("2010-02-03 18:00:00");

	local_date_time expectedTime = local_time_from_string("2010-02-01 00:00:00");

	ASSERT_EQ(expectedTime, ts.getTime(referenceTime, validTime));
}

TEST(TimeSpecificationTest, complexSpecificationMultipleOperations)
{
	TimeSpecification ts("referencetime - 12 hours +8 hours");
	local_date_time referenceTime = local_time_from_string("2010-02-01 12:00:00");
	local_date_time validTime = local_time_from_string("2010-02-03 18:00:00");

	local_date_time expectedTime = local_time_from_string("2010-02-01 08:00:00");

	ASSERT_EQ(expectedTime, ts.getTime(referenceTime, validTime));
}

TEST(TimeSpecificationTest, caseInsensitive)
{
	TimeSpecification ts("ValidTime");

	local_date_time referenceTime = local_time_from_string("2010-02-01 12:00:00");
	local_date_time validTime = local_time_from_string("2010-02-03 18:00:00");

	ASSERT_EQ(validTime, ts.getTime(referenceTime, validTime));
}
