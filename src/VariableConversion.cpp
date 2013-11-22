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

#include "VariableConversion.h"
#include "ForceConvertingNetcdfField.h"
#include "DirectionConvertingNetcdfField.h"
#include <boost/foreach.hpp>


VariableConversion::VariableConversion(const std::vector<VectorConversion> & vectorConversions)
{
	BOOST_FOREACH(const VectorConversion & conversion, vectorConversions)
	{
		AbstractNetcdfField::Ptr speed(new ForceConvertingNetcdfField(conversion.speed));
		AbstractNetcdfField::Ptr direction(new DirectionConvertingNetcdfField(conversion.direction));
		fields_.insert(std::make_pair(conversion.xElement, speed));
		fields_.insert(std::make_pair(conversion.yElement, speed));
		fields_.insert(std::make_pair(conversion.xElement, direction));
		fields_.insert(std::make_pair(conversion.yElement, direction));
		xVectors_.insert(conversion.xElement);
		yVectors_.insert(conversion.yElement);
	}
}

VariableConversion::~VariableConversion()
{
}



std::vector<AbstractNetcdfField::Ptr> VariableConversion::add(AbstractNetcdfField::Ptr inputElement)
{
	std::vector<AbstractNetcdfField::Ptr> ret;

	const std::string & variableName = inputElement->variableName();

	typedef std::multimap<std::string, AbstractNetcdfField::Ptr>::iterator Iter;
	typedef std::pair<Iter, Iter> Iters;
	Iters range = fields_.equal_range(variableName);
	for ( Iter it = range.first; it != range.second; ++ it )
	{
		ForceConvertingNetcdfField * f = dynamic_cast<ForceConvertingNetcdfField *>(it->second.get());
		if ( f )
		{
			if ( xVectors_.find(variableName) != xVectors_.end() )
				f->setX(inputElement);
			else if ( yVectors_.find(variableName) != yVectors_.end() )
				f->setY(inputElement);

			if ( f->ready() )
				ret.push_back(it->second);
		}
		DirectionConvertingNetcdfField * d = dynamic_cast<DirectionConvertingNetcdfField *>(it->second.get());
		if ( d )
		{
			if ( xVectors_.find(variableName) != xVectors_.end() )
				d->setX(inputElement);
			else if ( yVectors_.find(variableName) != yVectors_.end() )
				d->setY(inputElement);
			if ( d->ready() )
				ret.push_back(it->second);
		}
	}

	return ret;
}
