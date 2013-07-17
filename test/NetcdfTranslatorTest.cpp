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
#include <NetcdfTranslator.h>
#include <NetcdfFile.h>
#include <NetcdfField.h>

TEST(NetcdfTranslatorTest, test)
{
	CdmLoaderConfiguration conf;
	char * options[2] = {"name", "-c"TESTDATADIR"/config.xml"};
	conf.parse(2, options);

	NetcdfTranslator translator(conf);

	NetcdfFile ncFile(TESTDATADIR"/test.nc", "");
	NetcdfField::Ptr temperature = ncFile.getField("air_temperature_2m");

	std::vector<WriteQuery> queries = translator.queries(* temperature);

	EXPECT_EQ(5, queries.size());
}


