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
// SPDX-License-Identifier: GPL-3.0
#pragma once

#include <libsolutil/LP.h>

#include <libsolutil/Common.h>
#include <libsolutil/CommonData.h>
#include <libsolutil/StringUtils.h>
#include <liblangutil/Exceptions.h>

#include <boost/rational.hpp>

#include <range/v3/view/tail.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/algorithm/all_of.hpp>

#include <optional>
#include <stack>


namespace solidity::util
{

using rational = boost::rational<bigint>;

/**
 * A linear expression of the form
 * factors[0] + factors[1] * X1 + factors[2] * X2 + ...
 * The set and order of variables is implied.
 */
class LinearExpression
{
public:
	/// Creates the expression "_factor * X_index"
	static LinearExpression factorForVariable(size_t _index, rational _factor)
	{
		LinearExpression result;
		result.resize(_index + 1);
		result[_index] = std::move(_factor);
		return result;
	}

	static LinearExpression constant(rational _factor)
	{
		LinearExpression result;
		result.resize(1);
		result[0] = std::move(_factor);
		return result;
	}

	rational const& get(size_t _index) const
	{
		static rational const zero;
		return _index < factors.size() ? factors[_index] : zero;
	}

	rational const& operator[](size_t _index) const
	{
		return factors[_index];
	}

	rational& operator[](size_t _index)
	{
		return factors[_index];
	}

	auto enumerate() const { return factors | ranges::view::enumerate; }
	// leave out the zero if exists
	auto enumerateTail() const { return factors | ranges::view::enumerate | ranges::view::tail; }

	rational const& front() const { return factors.front(); }

	void push_back(rational _value) { factors.push_back(std::move(_value)); }

	size_t size() const { return factors.size(); }

	void resize(size_t _size, rational _default = {})
	{
		factors.resize(_size, std::move(_default));
	}

	/// @returns true if all factors of variables are zero.
	bool isConstant() const
	{
		return ranges::all_of(factors | ranges::views::tail, [](rational const& _v) { return !_v; });
	}

	LinearExpression& operator/=(rational const& _divisor)
	{
		for (rational& x: factors)
			if (x)
				x /= _divisor;
		return *this;
	}

	LinearExpression& operator*=(rational const& _factor)
	{
		for (rational& x: factors)
			if (x)
				x *= _factor;
		return *this;
	}

	friend LinearExpression operator*(rational const& _factor, LinearExpression _expr)
	{
		for (rational& x: _expr.factors)
			if (x)
				x *= _factor;
		return _expr;
	}

	LinearExpression& operator-=(LinearExpression const& _y)
	{
		if (size() < _y.size())
			resize(_y.size());
		for (size_t i = 0; i < size(); ++i)
			if (i < _y.size() && _y[i])
				(*this)[i] -= _y[i];
		return *this;
	}

	LinearExpression operator-(LinearExpression const& _y) const
	{
		LinearExpression result = *this;
		result -= _y;
		return result;
	}

	LinearExpression& operator+=(LinearExpression const& _y)
	{
		if (size() < _y.size())
			resize(_y.size());
		for (size_t i = 0; i < size(); ++i)
			if (i < _y.size() && _y[i])
				(*this)[i] += _y[i];
		return *this;
	}

	LinearExpression operator+(LinearExpression const& _y) const
	{
		LinearExpression result = *this;
		result += _y;
		return result;
	}


	/// Multiply two linear expression. This only works if at least one of them is a constant.
	/// Returns nullopt otherwise.
	friend std::optional<LinearExpression> operator*(
		std::optional<LinearExpression> _x,
		std::optional<LinearExpression> _y
	)
	{
		if (!_x || !_y)
			return std::nullopt;
		if (!_y->isConstant())
			swap(_x, _y);
		if (!_y->isConstant())
			return std::nullopt;

		rational const& factor = _y->get(0);

		for (rational& element: _x->factors)
			element *= factor;
		return _x;
	}

private:
	std::vector<rational> factors;
};

}
