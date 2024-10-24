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
// SPDX-License-Identifier: GPL-3.0
/**
 * Specific AST walkers that collect semantical facts.
 */

#include <libyul/optimiser/Semantics.h>

#include <libyul/optimiser/OptimizerUtilities.h>
#include <libyul/Exceptions.h>
#include <libyul/AST.h>
#include <libyul/Dialect.h>
#include <libyul/Utilities.h>

#include <libevmasm/SemanticInformation.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/Algorithms.h>

#include <limits>

using namespace solidity;
using namespace solidity::yul;


SideEffectsCollector::SideEffectsCollector(
		Dialect const& _dialect,
		Expression const& _expression,
		std::map<FunctionHandle, SideEffects> const* _functionSideEffects
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
	std::map<FunctionHandle, SideEffects> const* _functionSideEffects
):
	SideEffectsCollector(_dialect, _functionSideEffects)
{
	operator()(_ast);
}

SideEffectsCollector::SideEffectsCollector(
	Dialect const& _dialect,
	ForLoop const& _ast,
	std::map<FunctionHandle, SideEffects> const* _functionSideEffects
):
	SideEffectsCollector(_dialect, _functionSideEffects)
{
	operator()(_ast);
}

void SideEffectsCollector::operator()(FunctionCall const& _functionCall)
{
	ASTWalker::operator()(_functionCall);

	FunctionHandle functionHandle = functionNameToHandle(_functionCall.functionName);
	if (BuiltinFunction const* builtin = resolveBuiltinFunction(_functionCall.functionName, m_dialect))
		m_sideEffects += builtin->sideEffects;
	else if (m_functionSideEffects && m_functionSideEffects->count(functionHandle))
		m_sideEffects += m_functionSideEffects->at(functionHandle);
	else
		m_sideEffects += SideEffects::worst();
}

bool MSizeFinder::containsMSize(Dialect const& _dialect, Block const& _ast)
{
	MSizeFinder finder(_dialect);
	finder(_ast);
	return finder.m_msizeFound;
}

bool MSizeFinder::containsMSize(Dialect const& _dialect, Object const& _object)
{
	if (containsMSize(_dialect, _object.code()->root()))
		return true;

	for (std::shared_ptr<ObjectNode> const& node: _object.subObjects)
		if (auto const* object = dynamic_cast<Object const*>(node.get()))
			if (containsMSize(_dialect, *object))
				return true;

	return false;
}

void MSizeFinder::operator()(FunctionCall const& _functionCall)
{
	ASTWalker::operator()(_functionCall);

	if (BuiltinFunction const* builtin = resolveBuiltinFunction(_functionCall.functionName, m_dialect))
		if (builtin->isMSize)
			m_msizeFound = true;
}

std::map<FunctionHandle, SideEffects> SideEffectsPropagator::sideEffects(
	Dialect const& _dialect,
	CallGraph const& _directCallGraph
)
{
	// Any loop currently makes a function non-movable, because
	// it could be a non-terminating loop.
	// The same is true for any function part of a call cycle.
	// In the future, we should refine that, because the property
	// is actually a bit different from "not movable".

	std::map<FunctionHandle, SideEffects> ret;
	for (auto const& function: _directCallGraph.functionsWithLoops)
	{
		ret[function].movable = false;
		ret[function].canBeRemoved = false;
		ret[function].canBeRemovedIfNoMSize = false;
		ret[function].cannotLoop = false;
	}

	for (auto const& function: _directCallGraph.recursiveFunctions())
	{
		ret[function].movable = false;
		ret[function].canBeRemoved = false;
		ret[function].canBeRemovedIfNoMSize = false;
		ret[function].cannotLoop = false;
	}

	for (auto const& call: _directCallGraph.functionCalls)
	{
		FunctionHandle funName = call.first;
		SideEffects sideEffects;
		auto _visit = [&, visited = std::set<FunctionHandle>{}](FunctionHandle _function, auto&& _recurse) mutable {
			if (!visited.insert(_function).second)
				return;
			if (sideEffects == SideEffects::worst())
				return;
			if (BuiltinHandle const* builtinHandle = std::get_if<BuiltinHandle>(&_function))
				sideEffects += _dialect.builtin(*builtinHandle).sideEffects;
			else
			{
				if (ret.count(_function))
					sideEffects += ret[_function];
				for (FunctionHandle const& callee: _directCallGraph.functionCalls.at(_function))
					_recurse(callee, _recurse);
			}
		};
		for (auto const& _v: call.second)
			_visit(_v, _visit);
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

std::pair<TerminationFinder::ControlFlow, size_t> TerminationFinder::firstUnconditionalControlFlowChange(
	std::vector<Statement> const& _statements
)
{
	for (size_t i = 0; i < _statements.size(); ++i)
	{
		ControlFlow controlFlow = controlFlowKind(_statements[i]);
		if (controlFlow != ControlFlow::FlowOut)
			return {controlFlow, i};
	}
	return {ControlFlow::FlowOut, std::numeric_limits<size_t>::max()};
}

TerminationFinder::ControlFlow TerminationFinder::controlFlowKind(Statement const& _statement)
{
	if (
		std::holds_alternative<VariableDeclaration>(_statement) &&
		std::get<VariableDeclaration>(_statement).value &&
		containsNonContinuingFunctionCall(*std::get<VariableDeclaration>(_statement).value)
	)
		return ControlFlow::Terminate;
	else if (
		std::holds_alternative<Assignment>(_statement) &&
		containsNonContinuingFunctionCall(*std::get<Assignment>(_statement).value)
	)
		return ControlFlow::Terminate;
	else if (
		std::holds_alternative<ExpressionStatement>(_statement) &&
		containsNonContinuingFunctionCall(std::get<ExpressionStatement>(_statement).expression)
	)
		return ControlFlow::Terminate;
	else if (std::holds_alternative<Break>(_statement))
		return ControlFlow::Break;
	else if (std::holds_alternative<Continue>(_statement))
		return ControlFlow::Continue;
	else if (std::holds_alternative<Leave>(_statement))
		return ControlFlow::Leave;
	else
		return ControlFlow::FlowOut;
}

bool TerminationFinder::containsNonContinuingFunctionCall(Expression const& _expr)
{
	if (auto functionCall = std::get_if<FunctionCall>(&_expr))
	{
		for (auto const& arg: functionCall->arguments)
			if (containsNonContinuingFunctionCall(arg))
				return true;

		if (BuiltinFunction const* builtin = resolveBuiltinFunction(functionCall->functionName, m_dialect))
			return !builtin->controlFlowSideEffects.canContinue;

		yulAssert(std::holds_alternative<Identifier>(functionCall->functionName));
		auto const& name = std::get<Identifier>(functionCall->functionName).name;
		if (m_functionSideEffects && m_functionSideEffects->count(name))
			return !m_functionSideEffects->at(name).canContinue;
	}
	return false;
}
