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

#include <libsolutil/CommonData.h>
#include <libsolutil/Algorithms.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;


SideEffectsCollector::SideEffectsCollector(
		Dialect const& _dialect,
		Expression const& _expression,
		map<YulString, SideEffects> const* _functionSideEffects
):
	SideEffectsCollector(_dialect, _functionSideEffects)
{
	visit(_expression);
}

SideEffectsCollector::SideEffectsCollector(Dialect const& _dialect, Statement const& _statement):
	SideEffectsCollector(_dialect)
{
	visit(_statement);
}

SideEffectsCollector::SideEffectsCollector(
	Dialect const& _dialect,
	Block const& _ast,
	map<YulString, SideEffects> const* _functionSideEffects
):
	SideEffectsCollector(_dialect, _functionSideEffects)
{
	operator()(_ast);
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

void MSizeFinder::operator()(FunctionCall const& _functionCall)
{
	ASTWalker::operator()(_functionCall);

	if (BuiltinFunction const* f = m_dialect.builtin(_functionCall.functionName.name))
		if (f->isMSize)
			m_msizeFound = true;
}

map<YulString, SideEffects> SideEffectsPropagator::sideEffects(
	Dialect const& _dialect,
	CallGraph const& _directCallGraph
)
{
	// Any loop currently makes a function non-movable, because
	// it could be a non-terminating loop.
	// The same is true for any function part of a call cycle.
	// In the future, we should refine that, because the property
	// is actually a bit different from "not movable".

	map<YulString, SideEffects> ret;
	for (auto const& function: _directCallGraph.functionsWithLoops)
	{
		ret[function].movable = false;
		ret[function].sideEffectFree = false;
		ret[function].sideEffectFreeIfNoMSize = false;
	}

	// Detect recursive functions.
	for (auto const& call: _directCallGraph.functionCalls)
	{
		// TODO we could shortcut the search as soon as we find a
		// function that has as bad side-effects as we can
		// ever achieve via recursion.
		auto search = [&](YulString const& _functionName, util::CycleDetector<YulString>& _cycleDetector, size_t) {
			for (auto const& callee: _directCallGraph.functionCalls.at(_functionName))
				if (!_dialect.builtin(callee))
					if (_cycleDetector.run(callee))
						return;
		};
		if (util::CycleDetector<YulString>(search).run(call.first))
		{
			ret[call.first].movable = false;
			ret[call.first].sideEffectFree = false;
			ret[call.first].sideEffectFreeIfNoMSize = false;
		}
	}

	for (auto const& call: _directCallGraph.functionCalls)
	{
		YulString funName = call.first;
		SideEffects sideEffects;
		util::BreadthFirstSearch<YulString>{call.second, {funName}}.run(
			[&](YulString _function, auto&& _addChild) {
				if (sideEffects == SideEffects::worst())
					return;
				if (BuiltinFunction const* f = _dialect.builtin(_function))
					sideEffects += f->sideEffects;
				else
				{
					if (ret.count(_function))
						sideEffects += ret[_function];
					for (YulString callee: _directCallGraph.functionCalls.at(_function))
						_addChild(callee);
				}
			}
		);
		ret[funName] += sideEffects;
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
	return {ControlFlow::FlowOut, numeric_limits<size_t>::max()};
}

TerminationFinder::ControlFlow TerminationFinder::controlFlowKind(Statement const& _statement)
{
	if (
		holds_alternative<ExpressionStatement>(_statement) &&
		isTerminatingBuiltin(std::get<ExpressionStatement>(_statement))
	)
		return ControlFlow::Terminate;
	else if (holds_alternative<Break>(_statement))
		return ControlFlow::Break;
	else if (holds_alternative<Continue>(_statement))
		return ControlFlow::Continue;
	else if (holds_alternative<Leave>(_statement))
		return ControlFlow::Leave;
	else
		return ControlFlow::FlowOut;
}

bool TerminationFinder::isTerminatingBuiltin(ExpressionStatement const& _exprStmnt)
{
	if (holds_alternative<FunctionCall>(_exprStmnt.expression))
		if (auto const* dialect = dynamic_cast<EVMDialect const*>(&m_dialect))
			if (auto const* builtin = dialect->builtin(std::get<FunctionCall>(_exprStmnt.expression).functionName.name))
				if (builtin->instruction)
					return evmasm::SemanticInformation::terminatesControlFlow(*builtin->instruction);
	return false;
}
