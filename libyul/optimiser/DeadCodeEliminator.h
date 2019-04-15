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
 * Optimisation stage that removes unused variables and functions.
 */

#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/YulString.h>

#include <map>
#include <set>

namespace yul
{

/**
 * Optimisation stage that removes unreachable code
 *
 * Unreachable code is any code within a block which is preceded by a
 * return, invalid, break, continue, selfdestruct or revert.
 *
 * Function definitions are retained as they might be called by earlier
 * code and thus are considered reachable.
 *
 * Because variables declared in a for loop's init block have their scope extended to the loop body,
 * we require ForLoopInitRewriter to run before this step.
 *
 * Prerequisite: ForLoopInitRewriter
 */
class DeadCodeEliminator: public ASTModifier
{
public:
	using ASTModifier::operator();
	void operator()(ForLoop& _for) override;
	void operator()(Block& _block) override;
};

}
