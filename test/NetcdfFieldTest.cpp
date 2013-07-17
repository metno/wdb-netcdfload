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
#include <NetcdfField.h>
#include <boost/foreach.hpp>

TEST(NetcdfFieldTest, test)
{
	NetcdfFile ncFile(TESTDATADIR"/test.nc", "");
	const std::vector<NetcdfField::Ptr> & entries = ncFile.getFields();

	NetcdfField::Ptr temperature;
	BOOST_FOREACH(const NetcdfField::Ptr & field, entries)
		if ( field->variableName() == "air_temperature_2m" )
		{
			temperature = field;
			break;
		}

	ASSERT_TRUE(temperature) << "no temperature variable";

	EXPECT_EQ(3, temperature->indexes().size());
	EXPECT_TRUE(temperature->unHandledIndexes().empty());
	EXPECT_EQ(5, temperature->times().size());
	EXPECT_EQ(1, temperature->realizations().size());
}
