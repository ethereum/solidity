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
 * Counts the number of references to a variable.
 */
#include <libyul/backends/evm/VariableReferenceCounter.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AST.h>

#include <libsolutil/Visitor.h>

using namespace solidity::yul;

void VariableReferenceCounter::operator()(Identifier const& _identifier)
{
	increaseRefIfFound(_identifier.name);
}

void VariableReferenceCounter::operator()(FunctionDefinition const& _function)
{
	Scope* originalScope = m_scope;

	yulAssert(m_info.virtualBlocks.at(&_function), "");
	m_scope = m_info.scopes.at(m_info.virtualBlocks.at(&_function).get()).get();
	yulAssert(m_scope, "Variable scope does not exist.");

	for (auto const& v: _function.returnVariables)
		increaseRefIfFound(v.name);

	(*this)(_function.body);

	m_scope = originalScope;
}

void VariableReferenceCounter::operator()(ForLoop const& _forLoop)
{
	Scope* originalScope = m_scope;
	// Special scoping rules.
	m_scope = m_info.scopes.at(&_forLoop.pre).get();

	walkVector(_forLoop.pre.statements);
	visit(*_forLoop.condition);
	(*this)(_forLoop.body);
	(*this)(_forLoop.post);

	m_scope = originalScope;
}

void VariableReferenceCounter::operator()(Block const& _block)
{
	Scope* originalScope = m_scope;
	m_scope = m_info.scopes.at(&_block).get();

	ASTWalker::operator()(_block);

	m_scope = originalScope;
}

void VariableReferenceCounter::increaseRefIfFound(YulString _variableName)
{
	m_scope->lookup(_variableName, util::GenericVisitor{
		[&](Scope::Variable const& _var)
		{
			++m_variableReferences[&_var];
		},
		[](Scope::Function const&) { }
	});
}
