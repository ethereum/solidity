// SPDX-License-Identifier: GPL-3.0
/**
 * Optimiser component that performs function inlining.
 */
#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/AsmDataForward.h>

#include <optional>
#include <set>

namespace solidity::yul
{
struct Dialect;
struct OptimiserStepContext;

/**
 * Optimiser component that modifies an AST in place, inlining functions that can be
 * inlined inside functional expressions, i.e. functions that
 *  - return a single value
 *  - have a body like r := <functional expression>
 *  - neither reference themselves nor r in the right hand side
 *
 * Furthermore, for all parameters, all of the following need to be true
 *  - the argument is movable
 *  - the parameter is either referenced less than twice in the function body, or the argument is rather cheap
 *    ("cost" of at most 1 like a constant up to 0xff)
 *
 * This component can only be used on sources with unique names.
 */
class ExpressionInliner: public ASTModifier
{
public:
	static constexpr char const* name{"ExpressionInliner"};
	static void run(OptimiserStepContext&, Block& _ast);

	using ASTModifier::operator();
	void operator()(FunctionDefinition& _fun) override;

	void visit(Expression& _expression) override;

private:
	ExpressionInliner(
		Dialect const& _dialect,
		std::map<YulString, FunctionDefinition const*> const& _inlinableFunctions
	): m_dialect(_dialect), m_inlinableFunctions(_inlinableFunctions)
	{}

	Dialect const& m_dialect;
	std::map<YulString, FunctionDefinition const*> const& m_inlinableFunctions;

	std::map<YulString, YulString> m_varReplacements;
	/// Set of functions we are currently visiting inside.
	std::set<YulString> m_currentFunctions;
};

}
