/*(
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
/**
* Module providing metrics for the optimizer.
*/

#include <libyul/optimiser/Metrics.h>

#include <libyul/AsmData.h>
#include <libyul/Exceptions.h>
#include <libyul/Utilities.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <libevmasm/Instruction.h>
#include <libevmasm/GasMeter.h>

#include <libdevcore/Visitor.h>
#include <libdevcore/CommonData.h>

using namespace std;
using namespace dev;
using namespace yul;

size_t CodeSize::codeSize(Statement const& _statement)
{
	CodeSize cs;
	cs.visit(_statement);
	return cs.m_size;
}

size_t CodeSize::codeSize(Expression const& _expression)
{
	CodeSize cs;
	cs.visit(_expression);
	return cs.m_size;
}

size_t CodeSize::codeSize(Block const& _block)
{
	CodeSize cs;
	cs(_block);
	return cs.m_size;
}

size_t CodeSize::codeSizeIncludingFunctions(Block const& _block)
{
	CodeSize cs(false);
	cs(_block);
	return cs.m_size;
}

void CodeSize::visit(Statement const& _statement)
{
	if (_statement.type() == typeid(FunctionDefinition) && m_ignoreFunctions)
		return;
	else if (
		_statement.type() == typeid(If) ||
		_statement.type() == typeid(Break) ||
		_statement.type() == typeid(Continue)
	)
		m_size += 2;
	else if (_statement.type() == typeid(ForLoop))
		m_size += 3;
	else if (_statement.type() == typeid(Switch))
		m_size += 1 + 2 * boost::get<Switch>(_statement).cases.size();
	else if (!(
		_statement.type() == typeid(Block) ||
		_statement.type() == typeid(ExpressionStatement) ||
		_statement.type() == typeid(Assignment) ||
		_statement.type() == typeid(VariableDeclaration)
	))
		++m_size;

	ASTWalker::visit(_statement);
}

void CodeSize::visit(Expression const& _expression)
{
	if (_expression.type() != typeid(Identifier))
		++m_size;
	ASTWalker::visit(_expression);
}


size_t CodeCost::codeCost(Dialect const& _dialect, Expression const& _expr)
{
	CodeCost cc(_dialect);
	cc.visit(_expr);
	return cc.m_cost;
}


void CodeCost::operator()(FunctionCall const& _funCall)
{
	ASTWalker::operator()(_funCall);

	if (EVMDialect const* dialect = dynamic_cast<EVMDialect const*>(&m_dialect))
		if (BuiltinFunctionForEVM const* f = dialect->builtin(_funCall.functionName.name))
			if (f->instruction)
			{
				addInstructionCost(*f->instruction);
				return;
			}

	m_cost += 49;
}

void CodeCost::operator()(FunctionalInstruction const& _instr)
{
	yulAssert(m_cost >= 1, "Should assign cost one in visit(Expression).");
	addInstructionCost(_instr.instruction);
	ASTWalker::operator()(_instr);
}

void CodeCost::operator()(Literal const& _literal)
{
	yulAssert(m_cost >= 1, "Should assign cost one in visit(Expression).");
	size_t cost = 0;
	switch (_literal.kind)
	{
	case LiteralKind::Boolean:
		break;
	case LiteralKind::Number:
		for (u256 n = u256(_literal.value.str()); n >= 0x100; n >>= 8)
			cost++;
		break;
	case LiteralKind::String:
		cost = _literal.value.str().size();
		break;
	}

	m_cost += cost;
}

void CodeCost::visit(Statement const& _statement)
{
	++m_cost;
	ASTWalker::visit(_statement);
}

void CodeCost::visit(Expression const& _expression)
{
	++m_cost;
	ASTWalker::visit(_expression);
}

void CodeCost::addInstructionCost(eth::Instruction _instruction)
{
	dev::eth::Tier gasPriceTier = dev::eth::instructionInfo(_instruction).gasPriceTier;
	if (gasPriceTier < dev::eth::Tier::VeryLow)
		m_cost -= 1;
	else if (gasPriceTier < dev::eth::Tier::High)
		m_cost += 1;
	else
		m_cost += 49;
}

void AssignmentCounter::operator()(Assignment const& _assignment)
{
	for (auto const& variable: _assignment.variableNames)
		++m_assignmentCounters[variable.name];
}

size_t AssignmentCounter::assignmentCount(YulString _name) const
{
	auto it = m_assignmentCounters.find(_name);
	return (it == m_assignmentCounters.end()) ? 0 : it->second;
}
