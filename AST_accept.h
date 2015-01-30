/*
    This file is part of cpp-ethereum.

    cpp-ethereum is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    cpp-ethereum is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Implementation of the accept functions of AST nodes, included by AST.cpp to not clutter that
 * file with these mechanical implementations.
 */

#pragma once

#include <libsolidity/AST.h>
#include <libsolidity/ASTVisitor.h>

namespace dev
{
namespace solidity
{

void SourceUnit::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
		listAccept(m_nodes, _visitor);
	_visitor.endVisit(*this);
}

void SourceUnit::accept(ASTConstVisitor& _visitor) const
{
	if (_visitor.visit(*this))
		listAccept(m_nodes, _visitor);
	_visitor.endVisit(*this);
}

void ImportDirective::accept(ASTVisitor& _visitor)
{
	_visitor.visit(*this);
	_visitor.endVisit(*this);
}

void ImportDirective::accept(ASTConstVisitor& _visitor) const
{
	_visitor.visit(*this);
	_visitor.endVisit(*this);
}

void ContractDefinition::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
	{
		listAccept(m_baseContracts, _visitor);
		listAccept(m_definedStructs, _visitor);
		listAccept(m_stateVariables, _visitor);
		listAccept(m_events, _visitor);
		listAccept(m_functionModifiers, _visitor);
		listAccept(m_definedFunctions, _visitor);
	}
	_visitor.endVisit(*this);
}

void ContractDefinition::accept(ASTConstVisitor& _visitor) const
{
	if (_visitor.visit(*this))
	{
		listAccept(m_baseContracts, _visitor);
		listAccept(m_definedStructs, _visitor);
		listAccept(m_stateVariables, _visitor);
		listAccept(m_events, _visitor);
		listAccept(m_functionModifiers, _visitor);
		listAccept(m_definedFunctions, _visitor);
	}
	_visitor.endVisit(*this);
}

void InheritanceSpecifier::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
	{
		m_baseName->accept(_visitor);
		listAccept(m_arguments, _visitor);
	}
	_visitor.endVisit(*this);
}

void InheritanceSpecifier::accept(ASTConstVisitor& _visitor) const
{
	if (_visitor.visit(*this))
	{
		m_baseName->accept(_visitor);
		listAccept(m_arguments, _visitor);
	}
	_visitor.endVisit(*this);
}

void StructDefinition::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
		listAccept(m_members, _visitor);
	_visitor.endVisit(*this);
}

void StructDefinition::accept(ASTConstVisitor& _visitor) const
{
	if (_visitor.visit(*this))
		listAccept(m_members, _visitor);
	_visitor.endVisit(*this);
}

void StructDefinition::checkValidityOfMembers() const
{
	checkMemberTypes();
	checkRecursion();
}

void ParameterList::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
		listAccept(m_parameters, _visitor);
	_visitor.endVisit(*this);
}

void ParameterList::accept(ASTConstVisitor& _visitor) const
{
	if (_visitor.visit(*this))
		listAccept(m_parameters, _visitor);
	_visitor.endVisit(*this);
}

void FunctionDefinition::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
	{
		m_parameters->accept(_visitor);
		if (m_returnParameters)
			m_returnParameters->accept(_visitor);
		listAccept(m_functionModifiers, _visitor);
		m_body->accept(_visitor);
	}
	_visitor.endVisit(*this);
}

void FunctionDefinition::accept(ASTConstVisitor& _visitor) const
{
	if (_visitor.visit(*this))
	{
		m_parameters->accept(_visitor);
		if (m_returnParameters)
			m_returnParameters->accept(_visitor);
		listAccept(m_functionModifiers, _visitor);
		m_body->accept(_visitor);
	}
	_visitor.endVisit(*this);
}

void VariableDeclaration::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
		if (m_typeName)
			m_typeName->accept(_visitor);
	_visitor.endVisit(*this);
}

void VariableDeclaration::accept(ASTConstVisitor& _visitor) const
{
	if (_visitor.visit(*this))
		if (m_typeName)
			m_typeName->accept(_visitor);
	_visitor.endVisit(*this);
}

void ModifierDefinition::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
	{
		m_parameters->accept(_visitor);
		m_body->accept(_visitor);
	}
	_visitor.endVisit(*this);
}

void ModifierDefinition::accept(ASTConstVisitor& _visitor) const
{
	if (_visitor.visit(*this))
	{
		m_parameters->accept(_visitor);
		m_body->accept(_visitor);
	}
	_visitor.endVisit(*this);
}

void ModifierInvocation::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
	{
		m_modifierName->accept(_visitor);
		listAccept(m_arguments, _visitor);
	}
	_visitor.endVisit(*this);
}

void ModifierInvocation::accept(ASTConstVisitor& _visitor) const
{
	if (_visitor.visit(*this))
	{
		m_modifierName->accept(_visitor);
		listAccept(m_arguments, _visitor);
	}
	_visitor.endVisit(*this);
}

void EventDefinition::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
		m_parameters->accept(_visitor);
	_visitor.endVisit(*this);
}

void EventDefinition::accept(ASTConstVisitor& _visitor) const
{
	if (_visitor.visit(*this))
		m_parameters->accept(_visitor);
	_visitor.endVisit(*this);
}

void TypeName::accept(ASTVisitor& _visitor)
{
	_visitor.visit(*this);
	_visitor.endVisit(*this);
}

void TypeName::accept(ASTConstVisitor& _visitor) const
{
	_visitor.visit(*this);
	_visitor.endVisit(*this);
}

void ElementaryTypeName::accept(ASTVisitor& _visitor)
{
	_visitor.visit(*this);
	_visitor.endVisit(*this);
}

void ElementaryTypeName::accept(ASTConstVisitor& _visitor) const
{
	_visitor.visit(*this);
	_visitor.endVisit(*this);
}

void UserDefinedTypeName::accept(ASTVisitor& _visitor)
{
	_visitor.visit(*this);
	_visitor.endVisit(*this);
}

void UserDefinedTypeName::accept(ASTConstVisitor& _visitor) const
{
	_visitor.visit(*this);
	_visitor.endVisit(*this);
}

void Mapping::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
	{
		m_keyType->accept(_visitor);
		m_valueType->accept(_visitor);
	}
	_visitor.endVisit(*this);
}

void Mapping::accept(ASTConstVisitor& _visitor) const
{
	if (_visitor.visit(*this))
	{
		m_keyType->accept(_visitor);
		m_valueType->accept(_visitor);
	}
	_visitor.endVisit(*this);
}

void Block::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
		listAccept(m_statements, _visitor);
	_visitor.endVisit(*this);
}

void Block::accept(ASTConstVisitor& _visitor) const
{
	if (_visitor.visit(*this))
		listAccept(m_statements, _visitor);
	_visitor.endVisit(*this);
}

void PlaceholderStatement::accept(ASTVisitor& _visitor)
{
	_visitor.visit(*this);
	_visitor.endVisit(*this);
}

void PlaceholderStatement::accept(ASTConstVisitor& _visitor) const
{
	_visitor.visit(*this);
	_visitor.endVisit(*this);
}

void IfStatement::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
	{
		m_condition->accept(_visitor);
		m_trueBody->accept(_visitor);
		if (m_falseBody)
			m_falseBody->accept(_visitor);
	}
	_visitor.endVisit(*this);
}

void IfStatement::accept(ASTConstVisitor& _visitor) const
{
	if (_visitor.visit(*this))
	{
		m_condition->accept(_visitor);
		m_trueBody->accept(_visitor);
		if (m_falseBody)
			m_falseBody->accept(_visitor);
	}
	_visitor.endVisit(*this);
}

void WhileStatement::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
	{
		m_condition->accept(_visitor);
		m_body->accept(_visitor);
	}
	_visitor.endVisit(*this);
}

void WhileStatement::accept(ASTConstVisitor& _visitor) const
{
	if (_visitor.visit(*this))
	{
		m_condition->accept(_visitor);
		m_body->accept(_visitor);
	}
	_visitor.endVisit(*this);
}

void ForStatement::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
	{
		if (m_initExpression)
			m_initExpression->accept(_visitor);
		if (m_condExpression)
			m_condExpression->accept(_visitor);
		if (m_loopExpression)
			m_loopExpression->accept(_visitor);
		m_body->accept(_visitor);
	}
	_visitor.endVisit(*this);
}

void ForStatement::accept(ASTConstVisitor& _visitor) const
{
	if (_visitor.visit(*this))
	{
		if (m_initExpression)
			m_initExpression->accept(_visitor);
		if (m_condExpression)
			m_condExpression->accept(_visitor);
		if (m_loopExpression)
			m_loopExpression->accept(_visitor);
		m_body->accept(_visitor);
	}
	_visitor.endVisit(*this);
}

void Continue::accept(ASTVisitor& _visitor)
{
	_visitor.visit(*this);
	_visitor.endVisit(*this);
}

void Continue::accept(ASTConstVisitor& _visitor) const
{
	_visitor.visit(*this);
	_visitor.endVisit(*this);
}

void Break::accept(ASTVisitor& _visitor)
{
	_visitor.visit(*this);
	_visitor.endVisit(*this);
}

void Break::accept(ASTConstVisitor& _visitor) const
{
	_visitor.visit(*this);
	_visitor.endVisit(*this);
}

void Return::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
		if (m_expression)
			m_expression->accept(_visitor);
	_visitor.endVisit(*this);
}

void Return::accept(ASTConstVisitor& _visitor) const
{
	if (_visitor.visit(*this))
		if (m_expression)
			m_expression->accept(_visitor);
	_visitor.endVisit(*this);
}

void ExpressionStatement::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
		if (m_expression)
			m_expression->accept(_visitor);
	_visitor.endVisit(*this);
}

void ExpressionStatement::accept(ASTConstVisitor& _visitor) const
{
	if (_visitor.visit(*this))
		if (m_expression)
			m_expression->accept(_visitor);
	_visitor.endVisit(*this);
}

void VariableDefinition::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
	{
		m_variable->accept(_visitor);
		if (m_value)
			m_value->accept(_visitor);
	}
	_visitor.endVisit(*this);
}

void VariableDefinition::accept(ASTConstVisitor& _visitor) const
{
	if (_visitor.visit(*this))
	{
		m_variable->accept(_visitor);
		if (m_value)
			m_value->accept(_visitor);
	}
	_visitor.endVisit(*this);
}

void Assignment::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
	{
		m_leftHandSide->accept(_visitor);
		m_rightHandSide->accept(_visitor);
	}
	_visitor.endVisit(*this);
}

void Assignment::accept(ASTConstVisitor& _visitor) const
{
	if (_visitor.visit(*this))
	{
		m_leftHandSide->accept(_visitor);
		m_rightHandSide->accept(_visitor);
	}
	_visitor.endVisit(*this);
}

void UnaryOperation::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
		m_subExpression->accept(_visitor);
	_visitor.endVisit(*this);
}

void UnaryOperation::accept(ASTConstVisitor& _visitor) const
{
	if (_visitor.visit(*this))
		m_subExpression->accept(_visitor);
	_visitor.endVisit(*this);
}

void BinaryOperation::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
	{
		m_left->accept(_visitor);
		m_right->accept(_visitor);
	}
	_visitor.endVisit(*this);
}

void BinaryOperation::accept(ASTConstVisitor& _visitor) const
{
	if (_visitor.visit(*this))
	{
		m_left->accept(_visitor);
		m_right->accept(_visitor);
	}
	_visitor.endVisit(*this);
}

void FunctionCall::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
	{
		m_expression->accept(_visitor);
		listAccept(m_arguments, _visitor);
	}
	_visitor.endVisit(*this);
}

void FunctionCall::accept(ASTConstVisitor& _visitor) const
{
	if (_visitor.visit(*this))
	{
		m_expression->accept(_visitor);
		listAccept(m_arguments, _visitor);
	}
	_visitor.endVisit(*this);
}

void NewExpression::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
		m_contractName->accept(_visitor);
	_visitor.endVisit(*this);
}

void NewExpression::accept(ASTConstVisitor& _visitor) const
{
	if (_visitor.visit(*this))
		m_contractName->accept(_visitor);
	_visitor.endVisit(*this);
}

void MemberAccess::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
		m_expression->accept(_visitor);
	_visitor.endVisit(*this);
}

void MemberAccess::accept(ASTConstVisitor& _visitor) const
{
	if (_visitor.visit(*this))
		m_expression->accept(_visitor);
	_visitor.endVisit(*this);
}

void IndexAccess::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
	{
		m_base->accept(_visitor);
		m_index->accept(_visitor);
	}
	_visitor.endVisit(*this);
}

void IndexAccess::accept(ASTConstVisitor& _visitor) const
{
	if (_visitor.visit(*this))
	{
		m_base->accept(_visitor);
		m_index->accept(_visitor);
	}
	_visitor.endVisit(*this);
}

void Identifier::accept(ASTVisitor& _visitor)
{
	_visitor.visit(*this);
	_visitor.endVisit(*this);
}

void Identifier::accept(ASTConstVisitor& _visitor) const
{
	_visitor.visit(*this);
	_visitor.endVisit(*this);
}

void ElementaryTypeNameExpression::accept(ASTVisitor& _visitor)
{
	_visitor.visit(*this);
	_visitor.endVisit(*this);
}

void ElementaryTypeNameExpression::accept(ASTConstVisitor& _visitor) const
{
	_visitor.visit(*this);
	_visitor.endVisit(*this);
}

void Literal::accept(ASTVisitor& _visitor)
{
	_visitor.visit(*this);
	_visitor.endVisit(*this);
}

void Literal::accept(ASTConstVisitor& _visitor) const
{
	_visitor.visit(*this);
	_visitor.endVisit(*this);
}

}
}
