/*
    netcdfload

    Copyright (C) 2014 met.no

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
#include <configuration/parameter/NetcdfParameterSpecification.h>


TEST(NetcdfParameterSpecificationTest, simpleConstruct)
{
	NetcdfParameterSpecification spec("air_temperature");

	EXPECT_EQ("air_temperature", spec.variableName());
	EXPECT_TRUE(spec.standardName().empty());
	EXPECT_TRUE(spec.indicesToLoad().empty());
}

TEST(NetcdfParameterSpecificationTest, complexConstruct)
{
	NetcdfParameterSpecification spec("air_temperature:foo=14.5");

	EXPECT_EQ("air_temperature", spec.variableName());
	EXPECT_TRUE(spec.standardName().empty());

	const NetcdfParameterSpecification::IndexNameToValue & indices = spec.indicesToLoad();
	EXPECT_TRUE(indices.size() == 1);
	NetcdfParameterSpecification::IndexNameToValue::const_iterator find = indices.find("foo");
	ASSERT_TRUE(find != indices.end());
	EXPECT_TRUE(find->second == 14.5);
}
