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

// project
#include "TimeSpecification.h"

// boost
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_push_back_actor.hpp>

// std
#include <vector>

TimeSpecification::TimeSpecification() :
	baseTime_(ValidTime)
{
}

TimeSpecification::TimeSpecification(const char * spec)
{
    std::vector<bool> subtract;
    std::vector<int> hourOffset;

    using namespace boost::spirit::classic;
    parse_info<> parseResult = parse(spec,
                                     // first, choose reference or valid time as base
                                     (as_lower_d["referencetime"][assign_a(baseTime_,ReferenceTime)] |
											 as_lower_d["validtime"][assign_a(baseTime_,ValidTime)] |
											 as_lower_d["infinity"][assign_a(baseTime_,Infinity)] |
											 as_lower_d["-infinity"][assign_a(baseTime_,NegativeInfinity)]
                                     )
                                     // then, add or subtract from that time
                                     >> * (sign_p[push_back_a(subtract)] >> int_p[push_back_a(hourOffset)] >> as_lower_d["hours"]),
                                     space_p);

    if ( not parseResult.full )
        throw InvalidSpecification(spec);

    if ( baseTime_ < 0 or SENTRY_ <= baseTime_ )
        throw InternalError(spec);

    for ( unsigned i = 0; i < subtract.size(); ++ i )
    {
        boost::local_time::local_date_time::time_duration_type offset(hourOffset[i], 0, 0);
        if ( subtract[i] )
            duration_ -= offset;
        else
            duration_ += offset;
    }

    if ( (baseTime_ == Infinity or baseTime_ == NegativeInfinity) and not hourOffset.empty() )
    	throw InvalidSpecification(spec);
}

boost::local_time::local_date_time TimeSpecification::getTime(
		const boost::local_time::local_date_time & referenceTime,
		const boost::local_time::local_date_time & validTime) const
{
    boost::local_time::local_date_time ret(boost::posix_time::not_a_date_time);

    switch ( baseTime_ )
    {
    case ReferenceTime:
        ret = referenceTime;
        break;
    case ValidTime:
    	ret = validTime;
    	break;
    case Infinity:
    	ret = boost::local_time::local_date_time(boost::local_time::pos_infin);
    	break;
    case NegativeInfinity:
		ret = boost::local_time::local_date_time(boost::local_time::neg_infin);
		break;
    }

    ret += duration_;

    return ret;
}
