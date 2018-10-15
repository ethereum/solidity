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
 * Generic AST walker.
 */

#include <libyul/optimiser/ASTWalker.h>

#include <libsolidity/inlineasm/AsmData.h>

#include <boost/range/adaptor/reversed.hpp>

using namespace std;
using namespace dev;
using namespace dev::julia;
using namespace dev::solidity;


void ASTWalker::operator()(FunctionalInstruction const& _instr)
{
	walkVector(_instr.arguments | boost::adaptors::reversed);
}

void ASTWalker::operator()(FunctionCall const& _funCall)
{
	walkVector(_funCall.arguments | boost::adaptors::reversed);
}

void ASTWalker::operator()(ExpressionStatement const& _statement)
{
	visit(_statement.expression);
}

void ASTWalker::operator()(Assignment const& _assignment)
{
	for (auto const& name: _assignment.variableNames)
		(*this)(name);
	visit(*_assignment.value);
}

void ASTWalker::operator()(VariableDeclaration const& _varDecl)
{
	if (_varDecl.value)
		visit(*_varDecl.value);
}

void ASTWalker::operator()(If const& _if)
{
	visit(*_if.condition);
	(*this)(_if.body);
}

void ASTWalker::operator()(Switch const& _switch)
{
	visit(*_switch.expression);
	for (auto const& _case: _switch.cases)
	{
		if (_case.value)
			(*this)(*_case.value);
		(*this)(_case.body);
	}
}

void ASTWalker::operator()(FunctionDefinition const& _fun)
{
	(*this)(_fun.body);
}

void ASTWalker::operator()(ForLoop const& _for)
{
	(*this)(_for.pre);
	visit(*_for.condition);
	(*this)(_for.body);
	(*this)(_for.post);
}

void ASTWalker::operator()(Block const& _block)
{
	walkVector(_block.statements);
}

void ASTModifier::operator()(FunctionalInstruction& _instr)
{
	walkVector(_instr.arguments | boost::adaptors::reversed);
}

void ASTModifier::operator()(FunctionCall& _funCall)
{
	walkVector(_funCall.arguments | boost::adaptors::reversed);
}

void ASTModifier::operator()(ExpressionStatement& _statement)
{
	visit(_statement.expression);
}

void ASTModifier::operator()(Assignment& _assignment)
{
	for (auto& name: _assignment.variableNames)
		(*this)(name);
	visit(*_assignment.value);
}

void ASTModifier::operator()(VariableDeclaration& _varDecl)
{
	if (_varDecl.value)
		visit(*_varDecl.value);
}

void ASTModifier::operator()(If& _if)
{
	visit(*_if.condition);
	(*this)(_if.body);
}

void ASTModifier::operator()(Switch& _switch)
{
	visit(*_switch.expression);
	for (auto& _case: _switch.cases)
	{
		if (_case.value)
			(*this)(*_case.value);
		(*this)(_case.body);
	}
}

void ASTModifier::operator()(FunctionDefinition& _fun)
{
	(*this)(_fun.body);
}

void ASTModifier::operator()(ForLoop& _for)
{
	(*this)(_for.pre);
	visit(*_for.condition);
	(*this)(_for.post);
	(*this)(_for.body);
}

void ASTModifier::operator()(Block& _block)
{
	walkVector(_block.statements);
}
