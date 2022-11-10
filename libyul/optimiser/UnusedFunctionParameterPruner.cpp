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
 * UnusedFunctionParameterPruner: Optimiser step that removes unused parameters from function
 * definition.
 */

#include <libyul/optimiser/UnusedFunctionParameterPruner.h>
#include <libyul/optimiser/UnusedFunctionsCommon.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/optimiser/NameCollector.h>
#include <libyul/optimiser/NameDisplacer.h>
#include <libyul/optimiser/NameDispenser.h>
#include <libyul/YulString.h>
#include <libyul/AST.h>

#include <libsolutil/CommonData.h>

#include <range/v3/algorithm/all_of.hpp>

#include <optional>
#include <variant>

using namespace std;
using namespace solidity::util;
using namespace solidity::yul;
using namespace solidity::yul::unusedFunctionsCommon;

void UnusedFunctionParameterPruner::run(OptimiserStepContext& _context, Block& _ast)
{
	map<YulString, size_t> references = VariableReferencesCounter::countReferences(_ast);
	auto used = [&](auto v) -> bool { return references.count(v.name); };

	// Function name and a pair of boolean masks, the first corresponds to parameters and the second
	// corresponds to returnVariables.
	//
	// For the first vector in the pair, a value `false` at index `i` indicates that the function
	// argument at index `i` in `FunctionDefinition::parameters` is unused inside the function body.
	//
	// Similarly for the second vector in the pair, a value `false` at index `i` indicates that the
	// return parameter at index `i` in `FunctionDefinition::returnVariables` is unused inside
	// function body.
	map<YulString, pair<vector<bool>, vector<bool>>> usedParametersAndReturnVariables;

	// Step 1 of UnusedFunctionParameterPruner: Find functions whose parameters (both arguments and
	// return-parameters) are not used in its body.
	for (auto const& statement: _ast.statements)
		if (holds_alternative<FunctionDefinition>(statement))
		{
			FunctionDefinition const& f = std::get<FunctionDefinition>(statement);

			if (tooSimpleToBePruned(f) || ranges::all_of(f.parameters + f.returnVariables, used))
				continue;

			usedParametersAndReturnVariables[f.name] = {
				applyMap(f.parameters, used),
				applyMap(f.returnVariables, used)
			};
		}

	set<YulString> functionNamesToFree = util::keys(usedParametersAndReturnVariables);

	// Step 2 of UnusedFunctionParameterPruner: Renames the function and replaces all references to
	// the function, say `f`, by its new name, say `f_1`.
	NameDisplacer replace{_context.dispenser, functionNamesToFree};
	replace(_ast);

	// Inverse-Map of the above translations. In the above example, this will store an element with
	// key `f_1` and value `f`.
	std::map<YulString, YulString> newToOriginalNames = invertMap(replace.translations());

	// Step 3 of UnusedFunctionParameterPruner: introduce a new function in the block with body of
	// the old one. Replace the body of the old one with a function call to the new one with reduced
	// parameters.
	//
	// For example: introduce a new 'linking' function `f` with the same the body as `f_1`, but with
	// reduced parameters, i.e., `function f() -> y { y := 1 }`. Now replace the body of `f_1` with
	// a call to `f`, i.e., `f_1(x) -> y { y := f() }`.
	iterateReplacing(_ast.statements, [&](Statement& _s) -> optional<vector<Statement>> {
		if (holds_alternative<FunctionDefinition>(_s))
		{
			// The original function except that it has a new name (e.g., `f_1`)
			FunctionDefinition& originalFunction = get<FunctionDefinition>(_s);
			if (newToOriginalNames.count(originalFunction.name))
			{

				YulString linkingFunctionName = originalFunction.name;
				YulString originalFunctionName = newToOriginalNames.at(linkingFunctionName);
				pair<vector<bool>, vector<bool>> used =
					usedParametersAndReturnVariables.at(originalFunctionName);

				FunctionDefinition linkingFunction = createLinkingFunction(
					originalFunction,
					used,
					originalFunctionName,
					linkingFunctionName,
					_context.dispenser
				);

				originalFunction.name = originalFunctionName;
				originalFunction.parameters =
					filter(originalFunction.parameters, used.first);
				originalFunction.returnVariables =
					filter(originalFunction.returnVariables, used.second);

				return make_vector<Statement>(std::move(originalFunction), std::move(linkingFunction));
			}
		}

		return nullopt;
	});
}
