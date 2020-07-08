// SPDX-License-Identifier: GPL-3.0
/**
 * Small useful snippets for the optimiser.
 */

#pragma once

#include <libsolutil/Common.h>
#include <libyul/AsmDataForward.h>

namespace solidity::yul
{

std::string reindent(std::string const& _code);

u256 valueOfNumberLiteral(Literal const& _literal);
u256 valueOfStringLiteral(Literal const& _literal);
u256 valueOfBoolLiteral(Literal const& _literal);
u256 valueOfLiteral(Literal const& _literal);

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

// This can only be used for cases within one switch statement and
// relies on the fact that there are no duplicate cases.
struct SwitchCaseCompareByLiteralValue
{
	bool operator()(Case const* _lhsCase, Case const* _rhsCase) const;
};

}
