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
 * Specific AST walkers that collect semantical facts.
 */

#include <libyul/optimiser/Semantics.h>

#include <libyul/Exceptions.h>
#include <libyul/AsmData.h>
#include <libyul/Dialect.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <libevmasm/SemanticInformation.h>

#include <libdevcore/CommonData.h>

using namespace std;
using namespace dev;
using namespace yul;

SideEffectsCollector::SideEffectsCollector(Dialect const& _dialect, Expression const& _expression):
	SideEffectsCollector(_dialect)
{
	visit(_expression);
}

SideEffectsCollector::SideEffectsCollector(Dialect const& _dialect, Statement const& _statement):
	SideEffectsCollector(_dialect)
{
	visit(_statement);
}

SideEffectsCollector::SideEffectsCollector(Dialect const& _dialect, Block const& _ast):
	SideEffectsCollector(_dialect)
{
	operator()(_ast);
}

void SideEffectsCollector::operator()(FunctionalInstruction const& _instr)
{
	ASTWalker::operator()(_instr);

	if (!eth::SemanticInformation::movable(_instr.instruction))
		m_movable = false;
	if (!eth::SemanticInformation::sideEffectFree(_instr.instruction))
		m_sideEffectFree = false;
	if (!eth::SemanticInformation::sideEffectFreeIfNoMSize(_instr.instruction))
		m_sideEffectFreeIfNoMSize = false;
	if (_instr.instruction == eth::Instruction::MSIZE)
		m_containsMSize = true;
}

void SideEffectsCollector::operator()(FunctionCall const& _functionCall)
{
	ASTWalker::operator()(_functionCall);

	if (BuiltinFunction const* f = m_dialect.builtin(_functionCall.functionName.name))
	{
		if (!f->movable)
			m_movable = false;
		if (!f->sideEffectFree)
			m_sideEffectFree = false;
		if (!f->sideEffectFreeIfNoMSize)
			m_sideEffectFreeIfNoMSize = false;
		if (f->isMSize)
			m_containsMSize = true;
	}
	else
	{
		m_movable = false;
		m_sideEffectFree = false;
		m_sideEffectFreeIfNoMSize = false;
	}
}

MovableChecker::MovableChecker(Dialect const& _dialect, Expression const& _expression):
	MovableChecker(_dialect)
{
	visit(_expression);
}

void MovableChecker::operator()(Identifier const& _identifier)
{
	SideEffectsCollector::operator()(_identifier);
	m_variableReferences.emplace(_identifier.name);
}

void MovableChecker::visit(Statement const&)
{
	assertThrow(false, OptimizerException, "Movability for statement requested.");
}

pair<TerminationFinder::ControlFlow, size_t> TerminationFinder::firstUnconditionalControlFlowChange(
	vector<Statement> const& _statements
)
{
	for (size_t i = 0; i < _statements.size(); ++i)
	{
		ControlFlow controlFlow = controlFlowKind(_statements[i]);
		if (controlFlow != ControlFlow::FlowOut)
			return {controlFlow, i};
	}
	return {ControlFlow::FlowOut, size_t(-1)};
}

TerminationFinder::ControlFlow TerminationFinder::controlFlowKind(Statement const& _statement)
{
	if (
		_statement.type() == typeid(ExpressionStatement) &&
		isTerminatingBuiltin(boost::get<ExpressionStatement>(_statement))
	)
		return ControlFlow::Terminate;
	else if (_statement.type() == typeid(Break))
		return ControlFlow::Break;
	else if (_statement.type() == typeid(Continue))
		return ControlFlow::Continue;
	else
		return ControlFlow::FlowOut;
}

bool TerminationFinder::isTerminatingBuiltin(ExpressionStatement const& _exprStmnt)
{
	if (_exprStmnt.expression.type() == typeid(FunctionalInstruction))
		return eth::SemanticInformation::terminatesControlFlow(
			boost::get<FunctionalInstruction>(_exprStmnt.expression).instruction
		);
	else if (_exprStmnt.expression.type() == typeid(FunctionCall))
		if (auto const* dialect = dynamic_cast<EVMDialect const*>(&m_dialect))
			if (auto const* builtin = dialect->builtin(boost::get<FunctionCall>(_exprStmnt.expression).functionName.name))
				if (builtin->instruction)
					return eth::SemanticInformation::terminatesControlFlow(*builtin->instruction);
	return false;
}
