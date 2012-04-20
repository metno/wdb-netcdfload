/*
 test

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

#ifndef TIMESPECIFICATION_H_
#define TIMESPECIFICATION_H_

#include <boost/date_time/local_time/local_time.hpp>
#include <exception>
#include <ostream>

/**
 * Determine a point in time, based on the given string specification.
 *
 * The specification is case-insensitive, and has the form
 *
 * referencetime|validtime (+|- \d hours)*
 *
 * for example "ReferenceTime + 6 hours -3 hours" or simply "ValidTime"
 *
 * The created object may later be used to generate a real time, based on a
 * concrete reference- and valid time. Thsi si accomplished with the getTime
 * method.
 */
class TimeSpecification
{
public:
	TimeSpecification();

	/**
	 * @throw InvalidSpecification if the specification is invalid, or
	 * InternalError if you have encountered a bug
	 */
	TimeSpecification(const char * spec);

	/**
	 * Resolve the specification into a concrete time. The result is based on
	 * one of the two given parameters, but it need not be equal to them.
	 */
	boost::local_time::local_date_time getTime(
			const boost::local_time::local_date_time & referenceTime,
			const boost::local_time::local_date_time & validTime) const;

	enum BaseTime
	{
		ReferenceTime, ValidTime
	};

	BaseTime baseTime() const { return baseTime_; }

	const boost::posix_time::time_duration & duration() const { return duration_; }

	/**
	 * Base class for errors thrown by TimeSpecification
	 */
	class Error;

private:
	BaseTime baseTime_;
	boost::posix_time::time_duration duration_;
};

inline std::ostream & operator << (std::ostream & s, const TimeSpecification & ts)
{
	if ( ts.baseTime() == TimeSpecification::ReferenceTime )
		s << "referencetime";
	else
		s << "validtime";
	long hours = ts.duration().hours();
	if ( hours != 0 )
	{
		if ( hours > 0 )
			s << '+';
		s << hours <<"hours";
	}
	return s;
}

inline bool operator == (const TimeSpecification & a, const TimeSpecification & b)
{
	return a.baseTime() == b.baseTime() and a.duration() == b.duration();
}

class TimeSpecification::Error : public std::exception
{
public:
	Error(const char * spec) :
		specification_(spec)
	{}
	~Error() throw() {}

	virtual const char * what() const throw() { return specification_.c_str(); }

private:
	const std::string specification_;
};


#define TIMESPECIFICATION_EXCEPTION(Name) struct Name : public TimeSpecification::Error { Name(const char * spec) : TimeSpecification::Error(spec) {}}


TIMESPECIFICATION_EXCEPTION(InvalidSpecification);
TIMESPECIFICATION_EXCEPTION(InternalError);

#endif /* TIMESPECIFICATION_H_ */
