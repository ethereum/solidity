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
/**
 * Component that checks whether all variables are reachable on the stack.
 */

#pragma once

#include <libyul/Dialect.h>
#include <libyul/ASTForward.h>
#include <libyul/Object.h>

#include <map>
#include <memory>

namespace solidity::yul
{

/**
 * Component that checks whether all variables are reachable on the stack and
 * provides a mapping from function name to the largest stack difference found
 * in that function (no entry present if that function is compilable), as well
 * as the set of unreachable variables for each function.
 *
 * This only works properly if the outermost block is compilable and
 * functions are not nested. Otherwise, it might miss reporting some functions.
 *
 * Only checks the code of the object itself, does not descend into sub-objects.
 */
struct CompilabilityChecker
{
	CompilabilityChecker(Dialect const& _dialect, Object const& _object, bool _optimizeStackAllocation);
	std::map<YulString, std::vector<YulString>> unreachableVariables;
	std::map<YulString, int> stackDeficit;
};

}
