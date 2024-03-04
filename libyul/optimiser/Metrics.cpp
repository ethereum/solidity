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
#include <libyul/optimiser/OptimizerUtilities.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <libyul/AST.h>
#include <libyul/Exceptions.h>
#include <libyul/Utilities.h>

#include <libevmasm/Instruction.h>

#include <libsolutil/CommonData.h>

using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;

size_t CodeWeights::costOf(Statement const& _statement) const
{
	if (std::holds_alternative<ExpressionStatement>(_statement))
		return expressionStatementCost;
	else if (std::holds_alternative<Assignment>(_statement))
		return assignmentCost;
	else if (std::holds_alternative<VariableDeclaration>(_statement))
		return variableDeclarationCost;
	else if (std::holds_alternative<FunctionDefinition>(_statement))
		return functionDefinitionCost;
	else if (std::holds_alternative<If>(_statement))
		return ifCost;
	else if (std::holds_alternative<Switch>(_statement))
		return switchCost + caseCost * std::get<Switch>(_statement).cases.size();
	else if (std::holds_alternative<ForLoop>(_statement))
		return forLoopCost;
	else if (std::holds_alternative<Break>(_statement))
		return breakCost;
	else if (std::holds_alternative<Continue>(_statement))
		return continueCost;
	else if (std::holds_alternative<Leave>(_statement))
		return leaveCost;
	else if (std::holds_alternative<Block>(_statement))
		return blockCost;
	else
		yulAssert(false, "If you add a new statement type, you must update CodeWeights.");
}

size_t CodeWeights::costOf(Expression const& _expression) const
{
	if (std::holds_alternative<FunctionCall>(_expression))
		return functionCallCost;
	else if (std::holds_alternative<Identifier>(_expression))
		return identifierCost;
	else if (Literal const* literal = std::get_if<Literal>(&_expression))
	{
		// Avoid strings because they could be longer than 32 bytes.
		if (literal->kind != LiteralKind::String && valueOfLiteral(*literal) == 0)
			return literalZeroCost;
		else
			return literalCost;
	}
	else
		yulAssert(false, "If you add a new expression type, you must update CodeWeights.");
}


size_t CodeSize::codeSize(Statement const& _statement, CodeWeights const& _weights)
{
	CodeSize cs(true, _weights);
	cs.visit(_statement);
	return cs.m_size;
}

size_t CodeSize::codeSize(Expression const& _expression, CodeWeights const& _weights)
{
	CodeSize cs(true, _weights);
	cs.visit(_expression);
	return cs.m_size;
}

size_t CodeSize::codeSize(Block const& _block, CodeWeights const& _weights)
{
	CodeSize cs(true, _weights);
	cs(_block);
	return cs.m_size;
}

size_t CodeSize::codeSizeIncludingFunctions(Block const& _block, CodeWeights const& _weights)
{
	CodeSize cs(false, _weights);
	cs(_block);
	return cs.m_size;
}

void CodeSize::visit(Statement const& _statement)
{
	if (std::holds_alternative<FunctionDefinition>(_statement) && m_ignoreFunctions)
		return;

	m_size += m_weights.costOf(_statement);
	ASTWalker::visit(_statement);
}

void CodeSize::visit(Expression const& _expression)
{
	m_size += m_weights.costOf(_expression);
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

	if (auto instruction = toEVMInstruction(m_dialect, _funCall.functionName.name))
	{
		addInstructionCost(*instruction);
		return;
	}

	m_cost += 49;
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
		if (valueOfLiteral(_literal) == 0)
			if (auto evmDialect = dynamic_cast<EVMDialect const*>(&m_dialect))
				if (evmDialect->evmVersion().hasPush0())
					--m_cost;
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

void CodeCost::addInstructionCost(evmasm::Instruction _instruction)
{
	evmasm::Tier gasPriceTier = evmasm::instructionInfo(_instruction, evmVersionFromDialect(m_dialect)).gasPriceTier;
	if (gasPriceTier < evmasm::Tier::VeryLow)
		m_cost -= 1;
	else if (gasPriceTier < evmasm::Tier::High)
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
