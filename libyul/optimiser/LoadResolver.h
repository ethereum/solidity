// SPDX-License-Identifier: GPL-3.0
/**
 * Optimisation stage that replaces expressions of type ``sload(x)`` by the value
 * currently stored in storage, if known.
 */

#pragma once

#include <libyul/optimiser/DataFlowAnalyzer.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libevmasm/Instruction.h>

namespace solidity::yul
{

struct EVMDialect;
struct BuiltinFunctionForEVM;

/**
 * Optimisation stage that replaces expressions of type ``sload(x)`` and ``mload(x)`` by the value
 * currently stored in storage resp. memory, if known.
 *
 * Works best if the code is in SSA form.
 *
 * Prerequisite: Disambiguator, ForLoopInitRewriter.
 */
class LoadResolver: public DataFlowAnalyzer
{
public:
	static constexpr char const* name{"LoadResolver"};
	/// Run the load resolver on the given complete AST.
	static void run(OptimiserStepContext&, Block& _ast);

private:
	LoadResolver(
		Dialect const& _dialect,
		std::map<YulString, SideEffects> _functionSideEffects,
		bool _optimizeMLoad
	):
		DataFlowAnalyzer(_dialect, std::move(_functionSideEffects)),
		m_optimizeMLoad(_optimizeMLoad)
	{}

protected:
	using ASTModifier::visit;
	void visit(Expression& _e) override;

	void tryResolve(
		Expression& _e,
		evmasm::Instruction _instruction,
		std::vector<Expression> const& _arguments
	);

	bool m_optimizeMLoad = false;
};

}
