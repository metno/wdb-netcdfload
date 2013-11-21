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

#ifndef DIRECTIONCONVERTINGNETCDFFIELD_H_
#define DIRECTIONCONVERTINGNETCDFFIELD_H_

#include "DerivedNetcdfField.h"

class DirectionConvertingNetcdfField: public DerivedNetcdfField
{
public:
	DirectionConvertingNetcdfField(const std::string & variableName);
	virtual ~DirectionConvertingNetcdfField();

	void setX(AbstractNetcdfField::Ptr xVariable);
	void setY(AbstractNetcdfField::Ptr yVariable);

	bool ready() const { return xVariable_() and yVariable_(); }

	virtual boost::shared_ptr<AbstractDataRetriever> retriever(
			const LoadElement & loadElement,
			unsigned timeIndex, unsigned realizationIndex,
			const DataSpecification & querySpec) const;

private:
	AbstractNetcdfField::Ptr xVariable_() const { return source_; }
	AbstractNetcdfField::Ptr yVariable_() const { return source2_; }

	AbstractNetcdfField::Ptr source2_;

};

#endif /* DIRECTIONCONVERTINGNETCDFFIELD_H_ */
