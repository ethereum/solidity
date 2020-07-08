// SPDX-License-Identifier: GPL-3.0
/**
 * Specific AST walker that collects all defined names.
 */

#include <libyul/optimiser/NameCollector.h>

#include <libyul/AsmData.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;

void NameCollector::operator()(VariableDeclaration const& _varDecl)
{
	for (auto const& var: _varDecl.variables)
		m_names.emplace(var.name);
}

void NameCollector::operator ()(FunctionDefinition const& _funDef)
{
	m_names.emplace(_funDef.name);
	for (auto const& arg: _funDef.parameters)
		m_names.emplace(arg.name);
	for (auto const& ret: _funDef.returnVariables)
		m_names.emplace(ret.name);
	ASTWalker::operator ()(_funDef);
}

void ReferencesCounter::operator()(Identifier const& _identifier)
{
	++m_references[_identifier.name];
}

void ReferencesCounter::operator()(FunctionCall const& _funCall)
{
	if (m_countWhat == VariablesAndFunctions)
		++m_references[_funCall.functionName.name];
	ASTWalker::operator()(_funCall);
}

map<YulString, size_t> ReferencesCounter::countReferences(Block const& _block, CountWhat _countWhat)
{
	ReferencesCounter counter(_countWhat);
	counter(_block);
	return counter.references();
}

map<YulString, size_t> ReferencesCounter::countReferences(FunctionDefinition const& _function, CountWhat _countWhat)
{
	ReferencesCounter counter(_countWhat);
	counter(_function);
	return counter.references();
}

map<YulString, size_t> ReferencesCounter::countReferences(Expression const& _expression, CountWhat _countWhat)
{
	ReferencesCounter counter(_countWhat);
	counter.visit(_expression);
	return counter.references();
}

void Assignments::operator()(Assignment const& _assignment)
{
	for (auto const& var: _assignment.variableNames)
		m_names.emplace(var.name);
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
