/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Rhett <roadriverrail@gmail.com>
 * @date 2017
 * Overflow range container.
 */

#pragma once

#include <libdevcore/Common.h>
#include <libsolidity/ast/Types.h>
#include <libsolidity/ast/ASTForward.h>
#include <string>
#include <iostream>

namespace dev
{
namespace solidity
{

class IntegerOverflowBounds
{

public:
	//FIXME: This is only to make std:map's [] operator happy
	IntegerOverflowBounds()
	{
		m_min = 0;
		m_max = 0;
	}

	IntegerOverflowBounds(dev::bigint _min, dev::bigint _max) : m_min(_min), m_max(_max)
	{
		solAssert(m_min <= m_max, std::string("Range ") + this->toString() + std::string(" inverted during creation"));
	}

	dev::bigint getMin() const { return m_min; }
	dev::bigint getMax() const { return m_max; }

	bool fitsType (Type const& _type) const;
	bool fitsType (IntegerType const& _type) const;

	friend std::ostream &operator<< (std::ostream & _os, IntegerOverflowBounds const& _bounds);


	std::string toString() const;
private:

	dev::bigint m_min;
	dev::bigint m_max;
};


	IntegerOverflowBounds performBinaryOp(Token::Value const& _op, IntegerOverflowBounds const& _leftSide, IntegerOverflowBounds const& _rightSide);


	IntegerOverflowBounds zeroBounds();
	IntegerOverflowBounds boundsForType(Type const& _type);
	IntegerOverflowBounds boundsForType(IntegerType const& _type);
	IntegerOverflowBounds boundsForType(RationalNumberType const& _type);

	IntegerOverflowBounds intersectBounds(IntegerOverflowBounds const& _leftBounds, IntegerOverflowBounds const& _rightBounds);
	IntegerOverflowBounds unionBounds(IntegerOverflowBounds const& _bounds, IntegerOverflowBounds const& _rightBounds);
	IntegerOverflowBounds invertBounds(IntegerOverflowBounds const& _bounds, Type const& _type);
	IntegerOverflowBounds invertBounds(IntegerOverflowBounds const& _bounds, IntegerType const& _type);
}


}
