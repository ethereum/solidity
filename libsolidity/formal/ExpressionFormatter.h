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

/**
 * Helper methods for formatting SMT expressions
 */

#include <libsmtutil/SolverInterface.h>

#include <libsolidity/ast/Types.h>

#include <map>
#include <string>

namespace solidity::frontend::smt
{

/// @returns another smtutil::Expressions where every term in _from
/// may be replaced if it is in the substitution map _subst.
smtutil::Expression substitute(smtutil::Expression _from, std::map<std::string, std::string> const& _subst);

/// @returns a Solidity-like expression string built from _expr.
/// This is done at best-effort and is not guaranteed to always create a perfect Solidity expression string.
std::string toSolidityStr(smtutil::Expression const& _expr);

/// @returns a string representation of the SMT expression based on a Solidity type.
std::optional<std::string> expressionToString(smtutil::Expression const& _expr, Type const* _type);

/// @returns the formatted version of the given SMT expressions. Those expressions must be SMT constants.
std::vector<std::optional<std::string>> formatExpressions(std::vector<smtutil::Expression> const& _exprs, std::vector<Type const*> const& _types);
}
