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
#include <libdevcore/Algorithms.h>

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

	m_sideEffects += EVMDialect::sideEffectsOfInstruction(_instr.instruction);
}

void SideEffectsCollector::operator()(FunctionCall const& _functionCall)
{
	ASTWalker::operator()(_functionCall);

	YulString functionName = _functionCall.functionName.name;
	if (BuiltinFunction const* f = m_dialect.builtin(functionName))
		m_sideEffects += f->sideEffects;
	else if (m_functionSideEffects && m_functionSideEffects->count(functionName))
		m_sideEffects += m_functionSideEffects->at(functionName);
	else
		m_sideEffects += SideEffects::worst();
}

bool MSizeFinder::containsMSize(Dialect const& _dialect, Block const& _ast)
{
	MSizeFinder finder(_dialect);
	finder(_ast);
	return finder.m_msizeFound;
}

void MSizeFinder::operator()(FunctionalInstruction const& _instr)
{
	ASTWalker::operator()(_instr);

	if (_instr.instruction == eth::Instruction::MSIZE)
		m_msizeFound = true;
}

void MSizeFinder::operator()(FunctionCall const& _functionCall)
{
	ASTWalker::operator()(_functionCall);

	if (BuiltinFunction const* f = m_dialect.builtin(_functionCall.functionName.name))
		if (f->isMSize)
			m_msizeFound = true;
}


map<YulString, SideEffects> SideEffectsPropagator::sideEffects(
	Dialect const& _dialect,
	map<YulString, std::set<YulString>> const& _directCallGraph
)
{
	map<YulString, SideEffects> ret;
	for (auto const& call: _directCallGraph)
	{
		YulString funName = call.first;
		SideEffects sideEffects;
		BreadthFirstSearch<YulString>{call.second, {funName}}.run(
			[&](YulString _function, auto&& _addChild) {
				if (sideEffects == SideEffects::worst())
					return;
				if (BuiltinFunction const* f = _dialect.builtin(_function))
					sideEffects += f->sideEffects;
				else
					for (YulString callee: _directCallGraph.at(_function))
						_addChild(callee);
			}
		);
		ret[funName] = sideEffects;
	}
	return ret;
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
