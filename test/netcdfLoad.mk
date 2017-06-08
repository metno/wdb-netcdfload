TESTS = netcdfLoadTest

check_PROGRAMS = netcdfLoadTest
netcdfLoadTest_SOURCES = \
		$(MAIN_SOURCES) \
		test/TimeSpecificationTest.cpp \
		test/timeTest.cpp \
		test/NetcdfFileTest.cpp \
		test/NetcdfFieldTest.cpp \
		test/NetcdfTranslatorTest.cpp \
		test/NetcdfParameterSpecificationTest.cpp
		

BOOST_LIBS = \
	$(BOOST_PROGRAM_OPTIONS_LIB) \
	$(BOOST_REGEX_LIB) \
	$(BOOST_DATE_TIME_LIB) \
	$(BOOST_FILESYSTEM_LIB)

netcdfLoadTest_CPPFLAGS = -I$(top_srcdir)/src $(gtest_CFLAGS) -DTESTDATADIR=\"$(top_srcdir)/test/data\"
netcdfLoadTest_LDADD = $(netcdfLoad_LDADD) $(BOOST_LIBS) $(gtest_LIBS) -lgtest_main
netcdfLoadTest_LDFLAGS = -pthread # Seems to be required for gtest

EXTRA_DIST += \
	test/netcdfLoad.mk \
	test/data/config.xml \
	test/data/test.nc \
	test/data/test2.nc


include config/mk/gtest.mk

