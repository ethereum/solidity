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
 * Specific AST walker that collects all defined names.
 */

#include <libyul/optimiser/NameCollector.h>

#include <libyul/AST.h>

using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;

void NameCollector::operator()(VariableDeclaration const& _varDecl)
{
	if (m_collectWhat != OnlyFunctions)
		for (auto const& var: _varDecl.variables)
			m_names.emplace(var.name);
}

void NameCollector::operator()(FunctionDefinition const& _funDef)
{
	if (m_collectWhat != OnlyVariables)
		m_names.emplace(_funDef.name);
	if (m_collectWhat != OnlyFunctions)
	{
		for (auto const& arg: _funDef.parameters)
			m_names.emplace(arg.name);
		for (auto const& ret: _funDef.returnVariables)
			m_names.emplace(ret.name);
	}
	ASTWalker::operator ()(_funDef);
}

void ReferencesCounter::operator()(Identifier const& _identifier)
{
	++m_references[_identifier.name];
}

void ReferencesCounter::operator()(FunctionCall const& _funCall)
{
	++m_references[_funCall.functionName.name];
	ASTWalker::operator()(_funCall);
}

std::map<YulString, size_t> ReferencesCounter::countReferences(Block const& _block)
{
	ReferencesCounter counter;
	counter(_block);
	return std::move(counter.m_references);
}

std::map<YulString, size_t> ReferencesCounter::countReferences(FunctionDefinition const& _function)
{
	ReferencesCounter counter;
	counter(_function);
	return std::move(counter.m_references);
}

std::map<YulString, size_t> ReferencesCounter::countReferences(Expression const& _expression)
{
	ReferencesCounter counter;
	counter.visit(_expression);
	return std::move(counter.m_references);
}

void VariableReferencesCounter::operator()(Identifier const& _identifier)
{
	++m_references[_identifier.name];
}

std::map<YulString, size_t> VariableReferencesCounter::countReferences(Block const& _block)
{
	VariableReferencesCounter counter;
	counter(_block);
	return std::move(counter.m_references);
}

std::map<YulString, size_t> VariableReferencesCounter::countReferences(FunctionDefinition const& _function)
{
	VariableReferencesCounter counter;
	counter(_function);
	return std::move(counter.m_references);
}

std::map<YulString, size_t> VariableReferencesCounter::countReferences(Expression const& _expression)
{
	VariableReferencesCounter counter;
	counter.visit(_expression);
	return std::move(counter.m_references);
}

std::map<YulString, size_t> VariableReferencesCounter::countReferences(Statement const& _statement)
{
	VariableReferencesCounter counter;
	counter.visit(_statement);
	return std::move(counter.m_references);
}

void AssignmentsSinceContinue::operator()(ForLoop const& _forLoop)
{
	m_forLoopDepth++;
	ASTWalker::operator()(_forLoop);
	m_forLoopDepth--;
}

void AssignmentsSinceContinue::operator()(Continue const&)
{
	if (m_forLoopDepth == 0)
		m_continueFound = true;
}

void AssignmentsSinceContinue::operator()(Assignment const& _assignment)
{
	if (m_continueFound)
		for (auto const& var: _assignment.variableNames)
			m_names.emplace(var.name);
}

void AssignmentsSinceContinue::operator()(FunctionDefinition const&)
{
	yulAssert(false, "");
}

std::set<YulString> solidity::yul::assignedVariableNames(Block const& _code)
{
	std::set<YulString> names;
	forEach<Assignment const>(_code, [&](Assignment const& _assignment) {
		for (auto const& var: _assignment.variableNames)
			names.emplace(var.name);
	});
	return names;
}

std::map<YulString, FunctionDefinition const*> solidity::yul::allFunctionDefinitions(Block const& _block)
{
	std::map<YulString, FunctionDefinition const*> result;
	forEach<FunctionDefinition const>(_block, [&](FunctionDefinition const& _function) {
		result[_function.name] = &_function;
	});
	return result;
}
