// SPDX-License-Identifier: GPL-3.0
/**
* Module providing metrics for the EVM optimizer.
*/

#include <libyul/backends/evm/EVMMetrics.h>

#include <libyul/AsmData.h>
#include <libyul/Exceptions.h>
#include <libyul/Utilities.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <libevmasm/Instruction.h>
#include <libevmasm/GasMeter.h>

#include <libsolutil/Visitor.h>
#include <libsolutil/CommonData.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;

size_t GasMeter::costs(Expression const& _expression) const
{
	return combineCosts(GasMeterVisitor::costs(_expression, m_dialect, m_isCreation));
}

size_t GasMeter::instructionCosts(evmasm::Instruction _instruction) const
{
	return combineCosts(GasMeterVisitor::instructionCosts(_instruction, m_dialect, m_isCreation));
}

size_t GasMeter::combineCosts(std::pair<size_t, size_t> _costs) const
{
	return _costs.first * m_runs + _costs.second;
}


pair<size_t, size_t> GasMeterVisitor::costs(
	Expression const& _expression,
	EVMDialect const& _dialect,
	bool _isCreation
)
{
	GasMeterVisitor gmv(_dialect, _isCreation);
	gmv.visit(_expression);
	return {gmv.m_runGas, gmv.m_dataGas};
}

pair<size_t, size_t> GasMeterVisitor::instructionCosts(
	evmasm::Instruction _instruction,
	EVMDialect const& _dialect,
	bool _isCreation
)
{
	GasMeterVisitor gmv(_dialect, _isCreation);
	gmv.instructionCostsInternal(_instruction);
	return {gmv.m_runGas, gmv.m_dataGas};
}

void GasMeterVisitor::operator()(FunctionCall const& _funCall)
{
	ASTWalker::operator()(_funCall);
	if (BuiltinFunctionForEVM const* f = m_dialect.builtin(_funCall.functionName.name))
		if (f->instruction)
		{
			instructionCostsInternal(*f->instruction);
			return;
		}
	yulAssert(false, "Functions not implemented.");
}

void GasMeterVisitor::operator()(Literal const& _lit)
{
	m_runGas += evmasm::GasMeter::runGas(evmasm::Instruction::PUSH1);
	m_dataGas +=
		singleByteDataGas() +
		static_cast<size_t>(evmasm::GasMeter::dataGas(
			toCompactBigEndian(valueOfLiteral(_lit), 1),
			m_isCreation,
			m_dialect.evmVersion()
		));
}

void GasMeterVisitor::operator()(Identifier const&)
{
	m_runGas += evmasm::GasMeter::runGas(evmasm::Instruction::DUP1);
	m_dataGas += singleByteDataGas();
}

size_t GasMeterVisitor::singleByteDataGas() const
{
	if (m_isCreation)
		return evmasm::GasCosts::txDataNonZeroGas(m_dialect.evmVersion());
	else
		return evmasm::GasCosts::createDataGas;
}

void GasMeterVisitor::instructionCostsInternal(evmasm::Instruction _instruction)
{
	if (_instruction == evmasm::Instruction::EXP)
		m_runGas += evmasm::GasCosts::expGas + evmasm::GasCosts::expByteGas(m_dialect.evmVersion());
	else
		m_runGas += evmasm::GasMeter::runGas(_instruction);
	m_dataGas += singleByteDataGas();
}
