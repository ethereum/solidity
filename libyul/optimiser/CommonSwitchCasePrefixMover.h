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

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/optimiser/OptimiserStep.h>

namespace solidity::yul
{

/**
 * Component that moves a prefix that is shared between all cases (including the default case) of a switch statement
 * to the scope of the basic block containing the switch statement.
 *
 * In case of a shared variable declaration, the declaration is pulled out of the first case and the
 * names of the variables in the other cases are translated accordingly.
 *
 * Prerequisite: Disambiguator, FunctionHoister
 *
 * Works best, if the ExpressionSplitter is run before.
 */

class CommonSwitchCasePrefixMover: public ASTModifier
{
public:
	static constexpr char const* name{"CommonSwitchCasePrefixMover"};
	static void run(OptimiserStepContext& _context, Block& _ast);

	void operator()(Block& _block) override;
};

}
