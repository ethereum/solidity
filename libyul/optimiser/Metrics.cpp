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

#include <libevmasm/Instruction.h>

#include <libdevcore/Visitor.h>

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

void CodeSize::visit(Statement const& _statement)
{
	if (_statement.type() == typeid(FunctionDefinition))
		return;
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


size_t CodeCost::codeCost(Expression const& _expr)
{
	CodeCost cc;
	cc.visit(_expr);
	return cc.m_cost;
}


void CodeCost::operator()(FunctionCall const& _funCall)
{
	yulAssert(m_cost >= 1, "Should assign cost one in visit(Expression).");
	m_cost += 49;
	ASTWalker::operator()(_funCall);
}

void CodeCost::operator()(FunctionalInstruction const& _instr)
{
	using namespace dev::solidity;
	yulAssert(m_cost >= 1, "Should assign cost one in visit(Expression).");
	Tier gasPriceTier = instructionInfo(_instr.instruction).gasPriceTier;
	if (gasPriceTier < Tier::VeryLow)
		m_cost -= 1;
	else if (gasPriceTier < Tier::High)
		m_cost += 1;
	else
		m_cost += 49;
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
