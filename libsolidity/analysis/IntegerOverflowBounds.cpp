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
 * Integer range object
 */
#include <libsolidity/analysis/IntegerOverflowBounds.h>
#include <iostream>
#include <sstream>

using namespace std;

namespace dev
{
namespace solidity
{


bool IntegerOverflowBounds::fitsType(Type const& _type) const
{
	if (_type.category() == Type::Category::Integer)
		return fitsType(static_cast<IntegerType const&>(_type));
	solAssert(false, "IntegerOverflowBounds fits only IntegerType");
}

bool IntegerOverflowBounds::fitsType(IntegerType const& _type) const
{
	return (m_min >= _type.minValue() && m_max <= _type.maxValue());
}

std::string IntegerOverflowBounds::toString() const
{
	std::ostringstream ss;
	ss << *this;
	return ss.str();
}

/// NON CLASS MEMBERS HERE

std::ostream &operator<< (std::ostream & _os, IntegerOverflowBounds const& _bounds)
{
	return _os << "[" << _bounds.getMin() << "," << _bounds.getMax() << "]";
}

IntegerOverflowBounds invertBounds(IntegerOverflowBounds const& _bounds, Type const& _type)
{
	solAssert(_type.category() == Type::Category::Integer, "Cannot invert non-integer bounds!");
	return invertBounds(_bounds, static_cast<IntegerType const&>(_type));
}

IntegerOverflowBounds invertBounds(IntegerOverflowBounds const& _bounds, IntegerType const& _type)
{
	IntegerOverflowBounds maxRange = boundsForType(_type);
	dev::bigint min = _bounds.getMin();
	dev::bigint max = _bounds.getMax();

	solAssert((min == maxRange.getMin() && max != maxRange.getMax()) || (min != maxRange.getMin() && max == maxRange.getMax()),
			string("Cannot invert range ")
			+ _bounds.toString()
			+ string(" against type range ")
			+ maxRange.toString()
			+ string(" because result would be a disjoint range."));

	if (min == maxRange.getMin())
	{
		// We know that the maximum value was constrained then.
		min = max+1;
		max = maxRange.getMax();
	}
	else
	{
		// We know that the minimum value was constrained then.
		max = min-1;
		min = maxRange.getMin();
	}
	return IntegerOverflowBounds(min, max);
}

IntegerOverflowBounds boundsForType(Type const& _type)
{
	if (_type.category() == Type::Category::Integer)
		return boundsForType(static_cast<IntegerType const&>(_type));
	else if (_type.category() == Type::Category::RationalNumber)
		return boundsForType(static_cast<RationalNumberType const&>(_type));
	else
		solAssert(false, string("IntegerOverflowBounds doesn't support type ")+_type.toString());
}

IntegerOverflowBounds boundsForType(IntegerType const& _type)
{
	return IntegerOverflowBounds(_type.minValue(), _type.maxValue());
}

IntegerOverflowBounds boundsForType(RationalNumberType const& _type)
{
	solAssert(denominator(_type.value()) == 1, "Fractional value passed into IntegerOverflowBounds");
	return IntegerOverflowBounds(numerator(_type.value()), numerator(_type.value()));
}

IntegerOverflowBounds zeroBounds()
{
	return IntegerOverflowBounds(0, 0);
}

IntegerOverflowBounds intersectBounds(IntegerOverflowBounds const& _leftBounds, IntegerOverflowBounds const& _rightBounds)
{

	// Intersecting two bounds objects means creating the smallest range covered by both

	dev::bigint min = _leftBounds.getMin() > _rightBounds.getMin() ? _leftBounds.getMin() : _rightBounds.getMin();
	dev::bigint max = _leftBounds.getMax() < _rightBounds.getMax() ? _leftBounds.getMax() : _rightBounds.getMax();

	return IntegerOverflowBounds(min,max);

}

IntegerOverflowBounds unionBounds(IntegerOverflowBounds const& _leftBounds, IntegerOverflowBounds const& _rightBounds)
{
	// Unioning two bounds objects means expanding to the widest range which would
	// fit both.

	dev::bigint min = _leftBounds.getMin() < _rightBounds.getMin() ? _leftBounds.getMin() : _rightBounds.getMin();
	dev::bigint max = _leftBounds.getMax() > _rightBounds.getMax() ? _leftBounds.getMax() : _rightBounds.getMax();

	return IntegerOverflowBounds(min,max);
}

IntegerOverflowBounds performBinaryOp(
		Token::Value const& _op,
		IntegerOverflowBounds const& _left,
		IntegerOverflowBounds const& _right
		)
{
	dev::bigint min;
	dev::bigint max;
	// see here: https://en.wikipedia.org/wiki/Interval_arithmetic
	// Not sure if modulus is right, but it feels right
	if (_op == Token::Add)
	{
		min = _left.getMin() + _right.getMin();
		max = _left.getMax() + _right.getMax();
	}
	else if (_op == Token::Sub)
	{
		min = _left.getMin() - _right.getMax();
		max = _left.getMax() - _right.getMin();
	}
	else if (_op == Token::Mul)
	{
		dev::bigint products[4];
		products[0] = _left.getMin() * _right.getMin();
		products[1] = _left.getMin() * _right.getMax();
		products[2] = _left.getMax() * _right.getMin();
		products[3] = _left.getMax() * _right.getMax();

		min = products[0];
		max = products[0];

		for (int i=1; i<4; i++)
		{
			if (products[i] < min)
				min = products[i];
			if (products[i] > max)
				max = products[i];
		}
	}
	else if (_op == Token::Div)
	{
		// TODO: Add check for divide by zero
		// we don't send the ErrorReporter to this level
		// so not sure yet how to report it
		//
		// Formula is:
		// [x1,x2]/[y1,y2] = [x1,x2]*(1/[y1,y2])
		// Where, if [y1,y2] doesn't contain 0,
		// 1/[y1,y2] = [1/y2,1/y1]
		//
		// Thus, we get [x1,x2]*[1/y2,1/y1].
		// And since a*(1/b) = a/b, this should be
		// the multiplication op, but with division
		// and a slight reordering.
		//
		dev::bigint quotients[4];
		quotients[0] = _left.getMin() / _right.getMax();
		quotients[1] = _left.getMin() / _right.getMin();
		quotients[2] = _left.getMax() / _right.getMax();
		quotients[3] = _left.getMax() / _right.getMin();

		min = quotients[0];
		max = quotients[0];

		for (int i = 1; i < 4; i++)
		{
			if (quotients[i] < min)
				min = quotients[i];
			if (quotients[i] > max)
				max = quotients[i];
		}
	}
	else if (_op == Token::Mod)
	{
		// a % b will generally yield values [0, b-1] in the general case
		// it's probably possible to do better, though.
		min = 0;
		max = _right.getMax()-1;
		if (min > max)
		{
			dev::bigint swap = min;
			min = max;
			max = swap;
		}

	}
	// The < and > cases are most assuredly wrong in the general case.
	// They fail to cover scenarios like [5,6] < [2,3], which is quite
	// tautologically false.  Scenarios like that should actually generate
	// their own warning or something and they could seriously impact
	// how we can create and apply constraints.
	//
	// For now, the LHS passed in here is always the widest range for the type
	// so this is safe-ish but still wrong.
	else if (_op == Token::LessThan)
	{
		max = _right.getMin()-1;
	}
	else if (_op == Token::GreaterThan)
	{
		min = _right.getMax()+1;
	}
	else if (_op == Token::Equal)
	{
		min = _right.getMin();
		max = _right.getMax();
	}

	return IntegerOverflowBounds(min, max);
}


}
}
