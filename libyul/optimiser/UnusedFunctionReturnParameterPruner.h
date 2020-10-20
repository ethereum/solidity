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

#include <libyul/optimiser/OptimiserStep.h>

namespace solidity::yul
{

/**
 * UnusedFunctionReturnParameterPruner: Optimiser step that removes unused return parameters at function callsite.
 *
 * If a parameter is unused, like `z` and correspondingly `x` in the following snippet:
 *
 * {
 *    let z := f(sload(1), sload(2))
 *    function f(a,b) -> x { x := div(a,b) }
 * }
 *
 * We remove the parameter and create a new "linking" function `f2` as follows:
 *
 * function f(a,b) {
 *   let x
 *   x := div(a,b)
 * }
 *
 * function f2(a,b) -> x { x := f(a,b) }
 *
 * and replace all references to `f` by `f2`.
 * The inliner should be run afterwards to make sure that all references to `f2` are replaced by
 * `f`.
 *
 * Prerequisites: Disambiguator, FunctionHoister, SSATransform, ExpressionSplitter
 *
 * Only Disambiguator and FunctionHoister are needed for correctness.
 *
 * Ideally, we want all function calls to appear in the code in the following form:
 * `let x_1, ..., x_n = f(y_1, ..., y_m)`.
 *
 * ExpressionSplitter can reduce expressions into the above form. SSATransform allows us to realize
 * that the variable `x` is unused in the following example:
 *
 *   let x := f()
 *   x := 1
 *
 * Note that ForLoopConditionIntoBody and ForLoopInitRewriter are prerequisites for ExpressionSplitter and SSATransform.
 */
struct UnusedFunctionReturnParameterPruner
{
	static constexpr char const* name{"UnusedFunctionReturnParameterPruner"};
	static void run(OptimiserStepContext& _context, Block& _ast);
};

}
