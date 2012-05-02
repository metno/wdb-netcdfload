#if HAVE_GTEST

TESTS = netcdfLoadTest

check_PROGRAMS = netcdfLoadTest
netcdfLoadTest_SOURCES = \
        src/localtime.cpp \
        src/configuration/parameter/TimeSpecification.cpp \
	test/TimeSpecificationTest.cpp \
	test/timeTest.cpp

BOOST_LIBS = \
	$(BOOST_PROGRAM_OPTIONS_LIB) \
	$(BOOST_REGEX_LIB) \
	$(BOOST_DATE_TIME_LIB) \
	$(BOOST_FILESYSTEM_LIB)

netcdfLoadTest_CPPFLAGS = -I$(top_srcdir)/src $(gtest_CFLAGS)
netcdfLoadTest_LDADD = $(netcdfLoad_LDADD) $(BOOST_LIBS) $(gtest_LIBS) -lgtest_main

#endif

EXTRA_DIST += test/netcdfLoad.mk
