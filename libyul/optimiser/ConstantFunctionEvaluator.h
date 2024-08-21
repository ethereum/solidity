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
 * Optimiser component that evaluates constant functions and replace theirs body
 * with evaluated result.
 */
#pragma once

#include <libyul/ASTForward.h>

#include <libyul/optimiser/ASTCopier.h>
#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/NameDispenser.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/Exceptions.h>

#include <liblangutil/SourceLocation.h>

namespace solidity::yul
{


/**
 * Optimiser component that evaluates constant functions and replace theirs body
 * with evaluated result.
 *
 * A function is _constant_ if it satisfies all of the following criteria:
 * - It take no arguments
 * - If executed, this function only perform arithmetic operations and calls
 *   other function (that only does arithmetic expression). This means
 *   if any of reading from/writing to any memory, logging, creating contract, ...
 *   operations encountered, the function is not constant.
 *
 * Non-constant functions are left unchanged after the transformation.
 *
 * Under the hood, this component will use yul interpreter to evaluate the function.
 *
 * For example, this component may change the following code:
 *
 *	 function foo() -> x {
 *		let u, v := bar()
 *		x := add(u, v)
 *	 }
 *
 *	 function bar() -> u, v {
 *		if iszero(0) {
 *			u, v := 6, 9
 *		} else {
 *			u, v := 4, 20
 *		}
 *	 }
 *
 * into
 *
 *	 function foo() -> x {
 *		x := 15
 *	 }
 *
 *	 function bar() -> u, v {
 *		u, v := 6, 9
 *	 }
 */
class ConstantFunctionEvaluator: public ASTModifier
{
public:
	static constexpr char const* name{"ConstantFunctionEvaluator"};
	static void run(OptimiserStepContext& _context, Block& _ast);

private:
	ConstantFunctionEvaluator(Block& _ast, Dialect const& _dialect);

	Block& m_ast;
	std::map<YulName, FunctionDefinition*> m_functions;
	Dialect const& m_dialect;
};



}
