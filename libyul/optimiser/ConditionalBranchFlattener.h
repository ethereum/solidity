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
// SPDX-License-Identifier: GPL-3.
#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/AST.h>

namespace solidity::yul
{

/**
 * Replaces conditional execution with branchless bitwise operations.
 *
 * Rewrites:
 * if c { x := a }
 * to:
 * let condition := iszero(iszero(c))
 * x := xor(x, and(sub(0, condition), xor(a, x)))
 *
 * This transformation is applied if:
 * - The body of the `if` statement contains a single assignment.
 * - All RHS expressions in the assignment are movable.
 * - There are no control flow statements in the body.
 *
 * The logic `xor(x, and(sub(0, condition), xor(a, x)))` effectively implements
 * `condition ? a : x` using bitwise operations, relying on `condition` being 0 or 1.
 * `sub(0, condition)` generates a mask of all ones if condition is 1, and all zeros if condition
 *  is 0.
 *
 * Prerequisites: Disambiguator
 */
class ConditionalBranchFlattener: public ASTModifier
{
public:
	static constexpr char const* name = "ConditionalBranchFlattener";
	static void run(OptimiserStepContext& _context, Block& _ast);

	using ASTModifier::operator();
	void operator()(Block& _block) override;

private:
	ConditionalBranchFlattener(OptimiserStepContext& _context): m_context(_context) {}

	FunctionCall createBuiltinCall(
		langutil::DebugData::ConstPtr _debugData,
		std::string const& _name,
		std::vector<Expression> _arguments
	);

	OptimiserStepContext& m_context;
};

}
