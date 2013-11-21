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

#include "DerivedNetcdfField.h"

DerivedNetcdfField::DerivedNetcdfField(const std::string & variableName, Ptr source) :
	variableName_(variableName),
	source_(source)
{
}

DerivedNetcdfField::~DerivedNetcdfField()
{
}

const DerivedNetcdfField::IndexList & DerivedNetcdfField::indexes() const
{
	return source_->indexes();
}

DerivedNetcdfField::IndexList DerivedNetcdfField::unHandledIndexes() const
{
	return source_->unHandledIndexes();
}

bool DerivedNetcdfField::canHandleIndex(const std::string & name) const
{
	return source_->canHandleIndex(name);
}

double DerivedNetcdfField::indexValue(std::string dimension, unsigned index) const
{
	return source_->indexValue(dimension, index);
}

std::vector<Time> DerivedNetcdfField::times() const
{
	return source_->times();
}

std::vector<int> DerivedNetcdfField::realizations() const
{
	return source_->realizations();
}

std::string DerivedNetcdfField::timeDimension() const
{
	return source_->timeDimension();
}

std::string DerivedNetcdfField::realizationDimension() const
{
	return source_->realizationDimension();
}

std::string DerivedNetcdfField::xDimension() const
{
	return source_->xDimension();
}
std::string DerivedNetcdfField::yDimension() const
{
	return source_->yDimension();
}

std::string DerivedNetcdfField::attribute(const std::string & name) const
{
	return source_->attribute(name);
}

Time DerivedNetcdfField::referenceTime() const
{
	return source_->referenceTime();
}

boost::shared_ptr<GridGeometry> DerivedNetcdfField::placeSpecification() const
{
	return source_->placeSpecification();
}
