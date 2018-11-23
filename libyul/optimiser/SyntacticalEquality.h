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
 * Component that can compare ASTs for equality on a syntactic basis.
 */

#pragma once

#include <libyul/AsmDataForward.h>

#include <vector>

namespace yul
{

/**
 * Component that can compare ASTs for equality on a syntactic basis.
 * Ignores source locations but requires exact matches otherwise.
 *
 * TODO: Only implemented for Expressions for now.
 * A future version might also recognize renamed variables and thus could be used to
 * remove duplicate functions.
 */
class SyntacticalEqualityChecker
{
public:
	static bool equal(Expression const& _e1, Expression const& _e2);

protected:
	static bool equalVector(std::vector<Expression> const& _e1, std::vector<Expression> const& _e2);
};

}
