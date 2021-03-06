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

#include <gtest/gtest.h>
#include <NetcdfFile.h>


TEST(NetcdfFileTest, test)
{
	NetcdfFile ncFile(TESTDATADIR"/test.nc", "");
	const std::vector<AbstractNetcdfField::Ptr> & entries = ncFile.getFields();

	// BEGRENSET
//	*forecast_reference_time
//	*altitude
//	*air_temperature_2m
//	*precipitation_amount
//	latitude
//	longitude

	// UBEGRENSET
//	*forecast_reference_time
//	projection_regular_ll
//	*altitude
//	*air_temperature_2m
//	*precipitation_amount

	for ( std::vector<AbstractNetcdfField::Ptr>::const_iterator it = entries.begin(); it != entries.end(); ++ it )
		std::cout << (*it)->variableName() << std::endl;

	EXPECT_EQ(5, entries.size());
}

TEST(NetcdfFileTest, testReferenceTime)
{
	NetcdfFile ncFile(TESTDATADIR"/test.nc", "");

	EXPECT_EQ(time_from_postgresql_string("2013-05-21 12:00:00Z"), ncFile.referenceTime());
}
