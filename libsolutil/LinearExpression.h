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
		result[_index] = std::move(_factor);
		return result;
	}

	static LinearExpression constant(rational _factor)
	{
		LinearExpression result;
		result[0] = std::move(_factor);
		return result;
	}

	rational const& constantFactor() const
	{
		return get(0);
	}

	rational const& get(size_t _index) const
	{
		static rational const zero;
		auto it = factors.find(_index);
		if (it == factors.end())
			return zero;
		else
			return it->second;
	}

	rational const& operator[](size_t _index) const
	{
		return get(_index);
	}

	rational& operator[](size_t _index)
	{
		return factors[_index];
	}

	auto const& enumerate() const { return factors; }
	// leave out the constant factor if exists
	auto enumerateTail() const
	{
		auto it = factors.begin();
		if (it != factors.end() && !it->first)
			++it;
		return ranges::subrange(it, factors.end());
	}

	void eraseIndices(std::vector<bool> const& _indices)
	{
		for (auto it = factors.begin(); it != factors.end();)
		{
			size_t i = it->first;
			if (i < _indices.size() && _indices[i])
				it = factors.erase(it);
			else
				++it;
		}
	}
	/// Erases all indices greater or equal to _index.
	void eraseIndicesGE(size_t _index)
	{
		auto it = factors.begin();
		while (it != factors.end() && it->first < _index) ++it;
		factors.erase(it, factors.end());
	}
	void erase(size_t _index) { factors.erase(_index); }

	size_t maxIndex() const
	{
		if (factors.empty())
			return 0;
		else
			return factors.rbegin()->first;
	}

	/// @returns true if all factors of variables are zero.
	bool isConstant() const
	{
		return ranges::all_of(enumerateTail(), [](auto const& _item) -> bool { return !_item.second; });
	}

	bool operator<(LinearExpression const& _other) const
	{
		// "The comparison igrones the map's ordering"
		return factors < _other.factors;
	}

	bool operator==(LinearExpression const& _other) const
	{
		// TODO this might be wrong if there are stray zeros.
		return factors == _other.factors;
	}

	LinearExpression& operator/=(rational const& _divisor)
	{
		for (auto& item: factors)
			item.second /= _divisor;
		return *this;
	}

	LinearExpression& operator*=(rational const& _factor)
	{
		for (auto& item: factors)
			item.second *= _factor;
		return *this;
	}

	friend LinearExpression operator*(rational const& _factor, LinearExpression _expr)
	{
		for (auto& item: _expr.factors)
			item.second *= _factor;
		return _expr;
	}

	LinearExpression& operator+=(LinearExpression const& _y)
	{
		for (auto const& [i, x]: _y.enumerate())
		{
			// TODO this could be even more efficient.
			if (rational v = get(i) + x)
				factors[i] = move(v);
			else
				factors.erase(i);
		}
		return *this;
	}

	LinearExpression& operator-=(LinearExpression const& _y)
	{
		for (auto const& [i, x]: _y.enumerate())
		{
			// TODO this could be even more efficient.
			if (rational v = get(i) - x)
				factors[i] = move(v);
			else
				factors.erase(i);
		}
		return *this;
	}

	LinearExpression operator+(LinearExpression const& _y) const
	{
		LinearExpression result = *this;
		result += _y;
		return result;
	}

	LinearExpression operator-(LinearExpression const& _y) const
	{
		LinearExpression result = *this;
		result -= _y;
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

		*_x *= _y->constantFactor();
		return _x;
	}

private:
	// TODO maybe a vector of pairs could be more efficient.
	std::map<size_t, rational> factors;
};

}
