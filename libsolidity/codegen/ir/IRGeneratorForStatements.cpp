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
 * Component that translates Solidity code into Yul at statement level and below.
 */

#include <libsolidity/codegen/ir/IRGeneratorForStatements.h>

#include <libsolidity/codegen/ir/IRGenerationContext.h>
#include <libsolidity/codegen/YulUtilFunctions.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

bool IRGeneratorForStatements::visit(VariableDeclarationStatement const& _varDeclStatement)
{
	for (auto const& decl: _varDeclStatement.declarations())
		if (decl)
			m_context.addLocalVariable(*decl);

	if (Expression const* expression = _varDeclStatement.initialValue())
	{
		solUnimplementedAssert(_varDeclStatement.declarations().size() == 1, "");

		expression->accept(*this);

		solUnimplementedAssert(
			*expression->annotation().type == *_varDeclStatement.declarations().front()->type(),
			"Type conversion not yet implemented"
		);
		m_code <<
			"let " <<
			m_context.variableName(*_varDeclStatement.declarations().front()) <<
			" := " <<
			m_context.variable(*expression) <<
			"\n";
	}
	else
		for (auto const& decl: _varDeclStatement.declarations())
			if (decl)
				m_code << "let " << m_context.variableName(*decl) << "\n";

	return false;
}

bool IRGeneratorForStatements::visit(Assignment const& _assignment)
{
	solUnimplementedAssert(_assignment.assignmentOperator() == Token::Assign, "");

	_assignment.rightHandSide().accept(*this);

	solUnimplementedAssert(
		*_assignment.rightHandSide().annotation().type == *_assignment.leftHandSide().annotation().type,
		"Type conversion not yet implemented"
	);
	// TODO proper lvalue handling
	auto const& identifier = dynamic_cast<Identifier const&>(_assignment.leftHandSide());
	string varName = m_context.variableName(dynamic_cast<VariableDeclaration const&>(*identifier.annotation().referencedDeclaration));
	m_code << varName << " := " << m_context.variable(_assignment.rightHandSide()) << "\n";
	m_code << "let " << m_context.variable(_assignment) << " := " << varName << "\n";
	return false;
}

void IRGeneratorForStatements::endVisit(BinaryOperation const& _binOp)
{
	solUnimplementedAssert(_binOp.getOperator() == Token::Add, "");
	solUnimplementedAssert(*_binOp.leftExpression().annotation().type == *_binOp.rightExpression().annotation().type, "");
	if (IntegerType const* type = dynamic_cast<IntegerType const*>(_binOp.annotation().commonType.get()))
	{
		solUnimplementedAssert(!type->isSigned(), "");
		m_code <<
			"let " <<
			m_context.variable(_binOp) <<
			" := " <<
			m_utils.overflowCheckedUIntAddFunction(type->numBits()) <<
			"(" <<
			m_context.variable(_binOp.leftExpression()) <<
			", " <<
			m_context.variable(_binOp.rightExpression()) <<
			")\n";
	}
	else
		solUnimplementedAssert(false, "");
}

bool IRGeneratorForStatements::visit(Identifier const& _identifier)
{
	auto const& decl = dynamic_cast<VariableDeclaration const&>(
		*_identifier.annotation().referencedDeclaration
	);
	m_code << "let " << m_context.variable(_identifier) << " := " << m_context.variableName(decl) << "\n";
	return false;
}
