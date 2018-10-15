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
 * Specific AST walker that collects all defined names.
 */

#include <libyul/optimiser/NameCollector.h>

#include <libsolidity/inlineasm/AsmData.h>

using namespace std;
using namespace dev;
using namespace dev::yul;

void NameCollector::operator()(VariableDeclaration const& _varDecl)
{
	for (auto const& var: _varDecl.variables)
		m_names.insert(var.name);
}

void NameCollector::operator ()(FunctionDefinition const& _funDef)
{
	m_names.insert(_funDef.name);
	for (auto const arg: _funDef.parameters)
		m_names.insert(arg.name);
	for (auto const ret: _funDef.returnVariables)
		m_names.insert(ret.name);
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

map<string, size_t> ReferencesCounter::countReferences(Block const& _block)
{
	ReferencesCounter counter;
	counter(_block);
	return counter.references();
}

map<string, size_t> ReferencesCounter::countReferences(Expression const& _expression)
{
	ReferencesCounter counter;
	counter.visit(_expression);
	return counter.references();
}

void Assignments::operator()(Assignment const& _assignment)
{
	for (auto const& var: _assignment.variableNames)
		m_names.insert(var.name);
}
