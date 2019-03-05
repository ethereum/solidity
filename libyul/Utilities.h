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
 * Small useful snippets for the optimiser.
 */

#pragma once

#include <libdevcore/Common.h>
#include <libyul/AsmDataForward.h>

namespace yul
{

dev::u256 valueOfNumberLiteral(Literal const& _literal);
dev::u256 valueOfStringLiteral(Literal const& _literal);
dev::u256 valueOfBoolLiteral(Literal const& _literal);
dev::u256 valueOfLiteral(Literal const& _literal);

/**
 * Linear order on Yul AST nodes.
 *
 * Defines a linear order on Yul AST nodes to be used in maps and sets.
 * Note: the order is total and deterministic, but independent of the semantics, e.g.
 * it is not guaranteed that the false Literal is "less" than the true Literal.
 */
template<typename T>
struct Less
{
	bool operator()(T const& _lhs, T const& _rhs) const;
};

template<typename T>
struct Less<T*>
{
	bool operator()(T const* _lhs, T const* _rhs) const
	{
		if (_lhs && _rhs)
			return Less<T>{}(*_lhs, *_rhs);
		else
			return _lhs < _rhs;
	}
};

template<> bool Less<Literal>::operator()(Literal const& _lhs, Literal const& _rhs) const;
extern template struct Less<Literal>;

}
