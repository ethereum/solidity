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
#include <libyul/optimiser/UnusedFunctionReturnParameterPruner.h>
#include <libyul/optimiser/UnusedFunctionsCommon.h>
#include <libyul/optimiser/NameCollector.h>
#include <libyul/optimiser/NameDisplacer.h>
#include <libyul/optimiser/ASTWalker.h>

#include <libyul/AsmDataForward.h>
#include <libyul/AsmData.h>
#include <libyul/Dialect.h>

#include <libsolutil/CommonData.h>
#include <variant>

using namespace std;
using namespace solidity::util;
using namespace solidity::yul;
using namespace solidity::langutil;
using namespace solidity::yul::unusedFunctionsCommon;

namespace
{

/**
 * A helper class that removes expressions of the form `pop(a)` where a is an identifier. Generally,
 * pop can be the discard function.
 */
class PopRemover: public ASTModifier
{
public:
	using ASTModifier::operator();
	void operator()(Block& _block) override;

	static void removePop(Block& _ast, Dialect const& _dialect);
private:
	explicit PopRemover(Dialect const& _dialect): m_dialect(_dialect)
	{}
	Dialect const& m_dialect;
};

void PopRemover::operator()(Block& _block)
{
	iterateReplacing(_block.statements, [&](Statement& _s) -> optional<vector<Statement>> {
		if (holds_alternative<ExpressionStatement>(_s))
			if (
				auto& expressionStatement = get<ExpressionStatement>(_s);
				holds_alternative<FunctionCall>(expressionStatement.expression)
			)
			{
				FunctionCall& f = get<FunctionCall>(expressionStatement.expression);
				if (f.functionName.name == m_dialect.discardFunction(m_dialect.boolType)->name)
					if (holds_alternative<Identifier>(f.arguments[0]))
						return vector<Statement>{};
			}

		return nullopt;
	});
}

void PopRemover::removePop(Block& _ast, Dialect const& _dialect)
{
	PopRemover p{_dialect};
	p(_ast);
}

}

/**
 * Step 1 of UnusedFunctionReturnParameterPruner: Find functions whose return parameters are not
 * used in the code, i.e.,
 *
 * Look at all VariableDeclaration `let x_1, ..., x_i, ..., x_n := f(y_1, ..., y_m)` and check if
 * `x_i` is unused. If all function calls to `f` has its i-th return parameter unused, we will mark
 * its i-th index so that it can be removed in later steps.
 */
class FindFunctionsWithUnusedReturns: public ASTWalker
{
public:
	using ASTWalker::operator();
	void operator()(VariableDeclaration const& _v) override;
	void operator()(FunctionCall const& _v) override;

	/// Returns a map with function name as key and vector of bools such that if the index `i` of the
	/// vector is false, then the function's `i`-th return parameter is unused at *all* callsites.
	static map<YulString, vector<bool>> functions(
		Dialect const& _dialect,
		map<YulString, size_t> _references,
		Block& _block
	);

private:
	explicit FindFunctionsWithUnusedReturns(
		Dialect const& _dialect,
		map<YulString, size_t> const& _references
	):
		m_references(_references),
		m_dialect(_dialect)
	{
	}
	/// Function name and a boolean mask, where `false` at index `i` indicates that the function
	/// return-parameter at index `i` in `FunctionDefinition::parameter` is unused at every function
	/// call-site.
	map<YulString, vector<bool>> m_unusedReturnParameter;
	/// A count of all references to all Declarations.
	map<YulString, size_t> const& m_references;
	/// Functions whose return parameters are all used.
	set<YulString> m_usedFunctions;
	Dialect const& m_dialect;
};

// Find functions whose return parameters are unused. Assuming that the code is in SSA form and that
// ForLoopConditionIntoBody, ExpressionSplitter were run, all FunctionCalls with at least one return
// value will have the form `let x_1, ..., x_n := f(y_1, ..., y_m)`
void FindFunctionsWithUnusedReturns::operator()(VariableDeclaration const& _v)
{
	if (holds_alternative<FunctionCall>(*_v.value))
	{
		FunctionCall const& function = get<FunctionCall>(*_v.value);
		YulString const& name = function.functionName.name;

		// We avoid visiting `operator()(FunctionCall const&)` on purpose.
		walkVector(function.arguments);

		if (m_dialect.builtin(name))
			return;

		if (!m_unusedReturnParameter.count(name))
			m_unusedReturnParameter[name].resize(_v.variables.size(), false);

		for (size_t i = 0; i < _v.variables.size(); ++i)
			if (m_references.count(_v.variables[i].name))
				m_unusedReturnParameter.at(name)[i] = true;
	}
}

void FindFunctionsWithUnusedReturns::operator()(FunctionCall const& _f)
{
	// Any function that can reach here is added to the "do-not-prune" list. Ideally, with the
	// correct pre-requisites, no function (with at least one return parameter) will visit here.
	// However, we have to do this to guarantee correctness for 'all' optimization sequences.
	m_usedFunctions.insert(_f.functionName.name);
	walkVector(_f.arguments);
}

map<YulString, vector<bool>> FindFunctionsWithUnusedReturns::functions(
	Dialect const& _dialect,
	map<YulString, size_t> _references,
	Block& _block
)
{
	FindFunctionsWithUnusedReturns find{_dialect, _references};
	find(_block);

	map<YulString, vector<bool>> functions;

	for (auto const& [name, mask]: find.m_unusedReturnParameter)
		if (
			!find.m_usedFunctions.count(name) &&
			any_of(mask.begin(), mask.end(), [](bool _b) { return !_b; })
		)
			functions[name] = mask;

	return functions;
}

void UnusedFunctionReturnParameterPruner::run(OptimiserStepContext& _context, Block& _ast)
{
	PopRemover::removePop(_ast, _context.dialect);

	map<YulString, size_t> references = ReferencesCounter::countReferences(_ast);
	map<YulString, vector<bool>> functions = FindFunctionsWithUnusedReturns::functions(_context.dialect, references, _ast);

	set<YulString> simpleFunctions;
	for (auto const& s: _ast.statements)
		if (holds_alternative<FunctionDefinition>(s))
		{
			FunctionDefinition const& f = get<FunctionDefinition>(s);
			if (tooSimpleToBePruned(f))
				simpleFunctions.insert(f.name);
		}

	set<YulString> functionNamesToFree = util::keys(functions) - simpleFunctions;

	// Step 2 of UnusedFunctionReturnParameterPruner: Renames the function and replaces all references to
	// the function, say `f`, by its new name, say `f_1`.
	NameDisplacer replace{_context.dispenser, functionNamesToFree};
	replace(_ast);

	// Inverse-Map of the above translations. In the above example, this will store an element with
	// key `f_1` and value `f`.
	std::map<YulString, YulString> newToOriginalNames = invertMap(replace.translations());

	// Step 3 of UnusedFunctionReturnParameterPruner: introduce a new function in the block with body of
	// the old one. Replace the body of the old one with a function call to the new one with reduced
	// parameters.
	//
	// For example: introduce a new 'linking' function `f` with the same the body as `f_1`, but with
	// reduced return parameters, e.g., if `y` is unused at all callsites in the following
	// definition: `function f() -> y { y := 1 }`. We create a linking function `f_1` whose body
	// calls to `f`, i.e., `f_1(x) -> y { y := f() }`.
	//
	// Note that all parameter names of the linking function has to be renamed to avoid conflict.
	iterateReplacing(_ast.statements, [&](Statement& _s) -> optional<vector<Statement>> {
		if (holds_alternative<FunctionDefinition>(_s))
		{
			// The original function except that it has a new name (e.g., `f_1`)
			FunctionDefinition& originalFunction = get<FunctionDefinition>(_s);
			if (newToOriginalNames.count(originalFunction.name))
			{
				YulString linkingFunctionName = originalFunction.name;
				YulString originalFunctionName = newToOriginalNames.at(linkingFunctionName);
				pair<vector<bool>, vector<bool>> used =	{
					vector<bool>(originalFunction.parameters.size(), true),
					functions.at(originalFunctionName)
				};

				FunctionDefinition linkingFunction = createLinkingFunction(
					originalFunction,
					used,
					originalFunctionName,
					linkingFunctionName,
					_context.dispenser
				);

				originalFunction.name = originalFunctionName;
				auto missingVariables = filter(
					originalFunction.returnVariables,
					applyMap(used.second, [](bool _b) {return !_b;})
				);
				originalFunction.returnVariables =
					filter(originalFunction.returnVariables, used.second);
				// Return variables that are pruned can still be used inside the function body.
				// Therefore, a extra `let missingVariables` needs to be added at the beginning of
				// the function body.
				vector<Statement> missingVariableDeclarations =
					applyMap(missingVariables, [&](auto& _v) -> Statement {
						return VariableDeclaration{
							originalFunction.location,
							vector<TypedName>{_v},
							nullptr
						};
					});

				originalFunction.body.statements =
					move(missingVariableDeclarations) + move(originalFunction.body.statements);

				return make_vector<Statement>(move(originalFunction), move(linkingFunction));
			}
		}

		return nullopt;
	});

}
