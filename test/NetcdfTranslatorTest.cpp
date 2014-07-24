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
#include <boost/foreach.hpp>

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

TEST(NetcdfTranslatorTest, ignoreEmptyExtractOption)
{
	CdmLoaderConfiguration conf;
	char * options[2] = {"name", "-c"TESTDATADIR"/config.xml"};
	conf.parse(2, options);
	EXPECT_TRUE(conf.elementsToLoad().empty());

	NetcdfTranslator translator(conf);

	std::map<std::string, unsigned> count;
	NetcdfFile ncFile(TESTDATADIR"/test.nc", "");
	BOOST_FOREACH(const AbstractNetcdfField::Ptr & field, ncFile.getFields())
		BOOST_FOREACH(const WriteQuery & query, translator.queries(* field))
			++ count[query.valueParameterName()];

	EXPECT_EQ(2, count.size());
	EXPECT_EQ(5, count["air temperature"]);
	EXPECT_EQ(5, count["lwe thickness of precipitation amount"]);
}

TEST(NetcdfTranslatorTest, respectExtractOption)
{
	CdmLoaderConfiguration conf;
	char * options[3] = {"name", "-c"TESTDATADIR"/config.xml", "--extract=air_temperature_2m"};
	conf.parse(3, options);

	NetcdfTranslator translator(conf);

	std::map<std::string, unsigned> count;
	NetcdfFile ncFile(TESTDATADIR"/test.nc", "");

	EXPECT_EQ(1, conf.elementsToLoad().size());
	EXPECT_TRUE(ncFile.contains(conf.elementsToLoad().at(0)));

	BOOST_FOREACH(const AbstractNetcdfField::Ptr & field, ncFile.getFields())
		BOOST_FOREACH(const WriteQuery & query, translator.queries(* field))
			++ count[query.valueParameterName()];

	EXPECT_EQ(1, count.size());
	EXPECT_EQ(5, count["air temperature"]);
	EXPECT_EQ(0, count["lwe thickness of precipitation amount"]);
}

TEST(NetcdfTranslatorTest, extractNonExistingVariable)
{
	CdmLoaderConfiguration conf;
	char * options[3] = {"name", "-c"TESTDATADIR"/config.xml",
			"--extract=noSuchVariable"};
	conf.parse(3, options);

	NetcdfTranslator translator(conf);

	std::map<std::string, unsigned> count;
	NetcdfFile ncFile(TESTDATADIR"/test2.nc", "");

	EXPECT_EQ(1, conf.elementsToLoad().size());
	EXPECT_FALSE(ncFile.contains(conf.elementsToLoad().at(0)));

	BOOST_FOREACH(const AbstractNetcdfField::Ptr & field, ncFile.getFields())
		BOOST_FOREACH(const WriteQuery & query, translator.queries(* field))
			++ count[query.valueParameterName()];

	EXPECT_EQ(0, count.size());
}

TEST(NetcdfTranslatorTest, respectExtractOptionWithDimensionSpec)
{
	CdmLoaderConfiguration conf;
	char * options[4] = {"name", "-c"TESTDATADIR"/config.xml",
			"--extract=cloud_area_fraction:percentile=0.5",
			"--extract=cloud_area_fraction:percentile=0.25"};
	conf.parse(4, options);

	NetcdfTranslator translator(conf);

	std::map<std::string, unsigned> count;
	NetcdfFile ncFile(TESTDATADIR"/test2.nc", "");

	EXPECT_EQ(2, conf.elementsToLoad().size());
	EXPECT_TRUE(ncFile.contains(conf.elementsToLoad()[0]));
	EXPECT_TRUE(ncFile.contains(conf.elementsToLoad()[1]));

	BOOST_FOREACH(const AbstractNetcdfField::Ptr & field, ncFile.getFields())
		BOOST_FOREACH(const WriteQuery & query, translator.queries(* field))
			++ count[query.valueParameterName()];

	EXPECT_EQ(2, count.size());
	EXPECT_EQ(41, count["25th percentile of cloud area fraction"]);
	EXPECT_EQ(41, count["50th percentile of cloud area fraction"]);
}

TEST(NetcdfTranslatorTest, extractNonExistingDimension)
{
	CdmLoaderConfiguration conf;
	char * options[3] = {"name", "-c"TESTDATADIR"/config.xml",
			"--extract=cloud_area_fraction:percentile=1.5"};
	conf.parse(3, options);

	NetcdfTranslator translator(conf);

	std::map<std::string, unsigned> count;
	NetcdfFile ncFile(TESTDATADIR"/test2.nc", "");

	EXPECT_EQ(1, conf.elementsToLoad().size());
	EXPECT_FALSE(ncFile.contains(conf.elementsToLoad().at(0)));

	BOOST_FOREACH(const AbstractNetcdfField::Ptr & field, ncFile.getFields())
		BOOST_FOREACH(const WriteQuery & query, translator.queries(* field))
			++ count[query.valueParameterName()];

	EXPECT_EQ(0, count.size());
}
