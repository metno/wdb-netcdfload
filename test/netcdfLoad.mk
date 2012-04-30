#if HAVE_GTEST

TESTS = netcdfLoadTest

check_PROGRAMS = netcdfLoadTest
netcdfLoadTest_SOURCES = \
	test/TimeSpecificationTest.cpp \
	test/timeTest.cpp

BOOST_LIBS = \
	$(BOOST_PROGRAM_OPTIONS_LIB) \
	$(BOOST_REGEX_LIB) \
	$(BOOST_DATE_TIME_LIB) \
	$(BOOST_FILESYSTEM_LIB)

netcdfLoadTest_CPPFLAGS = -I$(top_srcdir)/src/ $(gtest_CFLAGS)
netcdfLoadTest_LDADD = libnetcdfLoadCore.a $(BOOST_LIBS) $(gtest_LIBS) -lgtest_main

#endif
