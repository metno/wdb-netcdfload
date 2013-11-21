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

#ifndef DERIVEDNETCDFFIELD_H_
#define DERIVEDNETCDFFIELD_H_

#include "AbstractNetcdfField.h"

class DerivedNetcdfField: public AbstractNetcdfField
{
public:
	DerivedNetcdfField(const std::string & variableName, Ptr source);
	virtual ~DerivedNetcdfField();

	virtual const std::string & variableName() const
	{
		return variableName_;
	}

	virtual const IndexList & indexes() const;

	virtual IndexList unHandledIndexes() const;

	virtual bool canHandleIndex(const std::string & name) const;

	virtual double indexValue(std::string dimension, unsigned index) const;

	virtual std::vector<Time> times() const;
	virtual std::vector<int> realizations() const;

	virtual std::string timeDimension() const;
	virtual std::string realizationDimension() const;
	virtual std::string xDimension() const;
	virtual std::string yDimension() const;

	virtual std::string attribute(const std::string & name) const;

	virtual Time referenceTime() const;

	virtual boost::shared_ptr<GridGeometry> placeSpecification() const;

protected:
	std::string variableName_;
	Ptr source_;
};

#endif /* DERIVEDNETCDFFIELD_H_ */
