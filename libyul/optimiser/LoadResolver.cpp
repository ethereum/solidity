// SPDX-License-Identifier: GPL-3.0
/**
 * Optimisation stage that replaces expressions of type ``sload(x)`` by the value
 * currently stored in storage, if known.
 */

#include <libyul/optimiser/LoadResolver.h>

#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/optimiser/CallGraphGenerator.h>
#include <libyul/SideEffects.h>
#include <libyul/AsmData.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;

void LoadResolver::run(OptimiserStepContext& _context, Block& _ast)
{
	bool containsMSize = MSizeFinder::containsMSize(_context.dialect, _ast);
	LoadResolver{
		_context.dialect,
		SideEffectsPropagator::sideEffects(_context.dialect, CallGraphGenerator::callGraph(_ast)),
		!containsMSize
	}(_ast);
}

void LoadResolver::visit(Expression& _e)
{
	DataFlowAnalyzer::visit(_e);

	if (!dynamic_cast<EVMDialect const*>(&m_dialect))
		return;

	if (holds_alternative<FunctionCall>(_e))
	{
		FunctionCall const& funCall = std::get<FunctionCall>(_e);
		if (auto const* builtin = dynamic_cast<EVMDialect const&>(m_dialect).builtin(funCall.functionName.name))
			if (builtin->instruction)
				tryResolve(_e, *builtin->instruction, funCall.arguments);
	}
}

void LoadResolver::tryResolve(
	Expression& _e,
	evmasm::Instruction _instruction,
	vector<Expression> const& _arguments
)
{
	if (_arguments.empty() || !holds_alternative<Identifier>(_arguments.at(0)))
		return;

	YulString key = std::get<Identifier>(_arguments.at(0)).name;
	if (
		_instruction == evmasm::Instruction::SLOAD &&
		m_storage.values.count(key)
	)
		_e = Identifier{locationOf(_e), m_storage.values[key]};
	else if (
		m_optimizeMLoad &&
		_instruction == evmasm::Instruction::MLOAD &&
		m_memory.values.count(key)
	)
		_e = Identifier{locationOf(_e), m_memory.values[key]};
}
